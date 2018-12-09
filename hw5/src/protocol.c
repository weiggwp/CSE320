
#include "protocol.h"
#include "csapp.h"
#include "debug.h"


int proto_send_packet(int fd, XACTO_PACKET *pkt, void *data)
{
    int size = pkt->size;
    pkt->size = htonl(pkt->size);
    pkt->timestamp_sec = htonl(pkt->timestamp_sec);
    pkt->timestamp_nsec = htonl(pkt->timestamp_nsec);

    // XACTO_PACKET newPkt = {
    //     pkt->type,
    //     pkt->status,
    //     pkt->null,
    // };

    //errno set by rio_writen if error, no need to do it ourselves
    if( rio_writen(fd,pkt,sizeof(XACTO_PACKET)) !=sizeof(XACTO_PACKET))
        return -1;

    if(size){
        if(rio_writen(fd,data,size) != size)
            return -1;
        // free(data);
    }
    return 0;
}


 // * Receive a packet, blocking until one is available.#############

 // * The returned structure has its multi-byte fields in host byte order.
 // *
 // * @param fd  The file descriptor from which the packet is to be received.
 // * @param pkt  Pointer to caller-supplied storage for the fixed-size
 // *   portion of the packet.
 // * @param datap  Pointer to variable into which to store a pointer to any
 // *   payload received.
 // * @return  0 in case of successful reception, -1 otherwise.  In the
 // *   latter case, errno is set to indicate the error.
 // *
 // * If the returned payload pointer is non-NULL, then the caller assumes
 // * responsibility for freeing the storage.

int proto_recv_packet(int fd, XACTO_PACKET *pkt, void **datap){

    XACTO_PACKET packet;
    if(rio_readn( fd, &packet, sizeof(XACTO_PACKET)) != sizeof(XACTO_PACKET))
        return -1;
    *pkt = packet;
    pkt->size           = ntohl(pkt->size);
    pkt->timestamp_sec  = ntohl(pkt->timestamp_sec);
    pkt->timestamp_nsec = ntohl(pkt->timestamp_nsec);
    if(pkt->size){
        // char* buf = *(char**)datap;
        char* buf= calloc(1,pkt->size+1);
        if( rio_readn(fd, buf, pkt->size) !=pkt->size)
            return -1;
        buf[pkt->size]= '\0';


        *(char**)datap = buf;
        // debug("****%s*****",buf);
        // debug("****%p:%s*****",*datap,*(char**)datap);

    }
    return 0;
}

