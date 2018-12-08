#include <stdlib.h>
#include "debug.h"
#include "server.h"
#include "transaction.h"
#include "protocol.h"
#include "data.h"
#include "store.h"
#include "csapp.h"

CLIENT_REGISTRY *client_registry;


int has_payload(XACTO_PACKET* pkt);
int recv_success(int res);

int send_success(int res);
int reply(XACTO_PACKET_TYPE type,TRANS_STATUS transtatus,size_t payload_size,int fd, XACTO_PACKET* pkt,char* content);
int Proto_recv_packet(int fd,XACTO_PACKET* pkt,char** key){
    int res = proto_recv_packet(fd, pkt,(void**)key);
        if(!recv_success(res))return 0;
        if(!has_payload(pkt)) return 0;
    return 1;
}
/*
 * Thread function for the thread that handles client requests.
 *
 * @param  Pointer to a variable that holds the file descriptor for
 * the client connection.  This pointer must be freed once the file
 * descriptor has been retrieved.
 */
BLOB* create_blob(char* key,XACTO_PACKET* pkt){
    return blob_create(key,pkt->size);
}
KEY* create_key(char* key,XACTO_PACKET* pkt){
    BLOB* blob = create_blob(key,pkt);
    return key_create(blob);
}
void *xacto_client_service(void *arg)
{
    // arg is a pointer to the integer file descriptor to be used
    // to communicate with the client.
    int fd = *(int*)arg;

    //free arg storage
    free(arg);
    // The thread must then become detached, so that it does not have to be explicitly reaped,
    Pthread_detach(pthread_self());
    // register the client file descriptor with the client registry.
    creg_register(client_registry, fd);

    // a transaction created to be used as the context for carrying out client requests.
    TRANSACTION* transaction =  trans_create();

    // Finally, the thread should enter a service loop
        // receives a request packet sent by the client,
        // carries out the request,
        // sends a reply packet, followed (in the case of a reply to a GET request)
        // by a data packet that contains the result.

    // boolean commited?
    int commit = 0;
    char buff[1024];
    XACTO_PACKET* pkt =  malloc(sizeof(XACTO_PACKET));
    //buffs for key/value
    char** key = malloc(sizeof(char*));
    char** value = malloc(sizeof(char*));
    while(1){
        debug("****loop********");
        //get type of request, either GET or PUT or COMMIT
        int res =  proto_recv_packet(fd, pkt,(void**) &buff);
        if(!recv_success(res)) break;
        if(pkt->type ==XACTO_PUT_PKT)
        {
            // Put a key/value mapping in the store
            // (sends key, value, and transaction ID)
            // (reply returns status)

            //get key
            if(!Proto_recv_packet(fd, pkt, key)) break;
            //get value
            //create key
            //(*key)[pkt->size] = '\0';

            KEY* k = create_key(*key,pkt);
            // BLOB* blob = blob_create(*key,pkt->size);
            // KEY* k = key_create(blob);

            if(!Proto_recv_packet(fd, pkt, value)) break;
            //create content
            // value[strlen(*value)] = '\0';
            BLOB* content = create_blob(*value,pkt);
            // blob_create(*value,pkt->size);

            TRANS_STATUS transtatus = store_put(transaction,k,content);
            store_show();
            if(!content)
                blob_unref(content,"put in store done");

            if(!reply(XACTO_REPLY_PKT,transtatus,0,fd,pkt,NULL))
                break;
            //exit loop if aborted
            if(transtatus ==TRANS_ABORTED) break;
        }
        else if(pkt->type ==XACTO_GET_PKT){
            debug("****getting********");

            // Get the store value corresponding to a key
            // (sends key and transaction ID)

            //get key
            if(!Proto_recv_packet(fd, pkt, key)) break;

            //create KEY
            // (*key)[strlen(*key)] = '\0';
            // BLOB* blob =blob_create(*key,strlen(*key)+1);
            KEY* k = create_key(*key,pkt);
            // char value2[1024];
            // content storage
            BLOB* content =NULL;
            // = blob_create(value2,1024);

            //conetent might be null
            TRANS_STATUS transtatus = store_get(transaction, k, &content);
            store_show();
            // debug("****%s********",content->content);

            if(transtatus == TRANS_ABORTED){
            debug("****%s********","TRANS_ABORTED");
                reply(XACTO_REPLY_PKT,transtatus,0,fd,pkt,NULL);
                break;
            }
            else{
                //reply with good status
                if(!reply(XACTO_REPLY_PKT,transtatus,0,fd,pkt,NULL)) break;

                if(content){
                    if(!reply(XACTO_DATA_PKT,transtatus,content->size,fd,pkt,content->content)) break;
                }
                else{
                    debug("****status:%d",transtatus);
                    if(!reply(XACTO_DATA_PKT,transtatus,0,fd,pkt,NULL)) break;
                }
            }
            // if(content)
            //     blob_unref(content,"send to client done");//FIXME

        }
        else if(pkt->type ==XACTO_COMMIT_PKT){
            TRANS_STATUS transtatus =trans_commit(transaction);
            if(transtatus==TRANS_COMMITTED)
                commit=1;
            if(!reply(XACTO_REPLY_PKT,transtatus,0,fd,pkt,NULL)) break;
            break;
        }
        else
        {
            fprintf(stderr, "Invalid type:%d\n",pkt->type );
            break;//FIXME:?
        }

    }

    free(pkt);
    free(key);
    free(value);
    // If as a result of carrying
    // out any of the requests, the transaction commits or aborts, then
    // (after sending the required reply to the current request) the service
    // loop should end and the client service thread terminate, closing the
    // client connection.
    if(!commit){
        trans_abort(transaction);
    }
    creg_unregister(client_registry,fd);
    Close(fd);
    Pthread_exit(0);
    return NULL;

}

int has_payload(XACTO_PACKET* pkt)
{
    if(pkt->null || !pkt->size){// no payload //FIXME: do I need to check
        fprintf(stderr, "key null error:\n");
        return 0;
    }
    return 1;
}
int recv_success(int res)
{
    if(!res)
        return 1;
    fprintf(stderr, "Recv packet error:%d\n",res);
    return 0;

}

int send_success(int res)
{
    if(!res)
        return 1;
    fprintf(stderr, "Send packet error:%d\n",res);
    return 0;

}

int reply(XACTO_PACKET_TYPE type,TRANS_STATUS transtatus,size_t payload_size,int fd, XACTO_PACKET* pkt,char* content)
{

    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC,&current_time);
    pkt->timestamp_sec = current_time.tv_sec;
    pkt->timestamp_nsec = current_time.tv_nsec;

    pkt->type = type;
    pkt->status = transtatus;
    pkt->null = (payload_size==0)?1:0;
    pkt->size = payload_size;
    debug("payload_size:%d,NULL:%d",pkt->size,pkt->null);
    int res = proto_send_packet(fd,pkt,content);

    if(send_success(res))
        return 1;
    return 0;
}