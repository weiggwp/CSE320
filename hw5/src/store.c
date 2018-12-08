
 // * The overall structure of the store is that of a linked hash map.
 // * A "bucket" is a singly linked list of "map entries", where each map entry
 // * contains the version list associated with a single key.

#include "store.h"
#include <stdio.h>
#include "debug.h"
struct map m;
void store_init(void)
{
    debug("Initialize object store");

    m.num_buckets = NUM_BUCKETS;
    if (pthread_mutex_init(&m.mutex, NULL) != 0)
    {
        fprintf(stderr,"\n mutex init failed\n");
        exit(EXIT_FAILURE);
    }
    m.table = calloc(m.num_buckets,sizeof(MAP_ENTRY*));//table[num_buck][]
    if(m.table==NULL)
    {
        fprintf(stderr,"\n table init failed\n");
        exit(EXIT_FAILURE);
    }
}

void store_fini(void)
{
    for (int i = 0; i < m.num_buckets; ++i)
    {
        /* code */
        MAP_ENTRY* entry = m.table[i];
        while(entry!=NULL){
            key_dispose(entry->key);
            VERSION* version = entry->versions;
            while(version!=NULL){
                VERSION* tmp_v = version->next;
                version_dispose(version);
                version = tmp_v;
            }
            MAP_ENTRY* tmp_e = entry->next;
            entry = tmp_e;
        }
    }
    free(m.table);
}
MAP_ENTRY* create_map_entry(KEY* key){
    MAP_ENTRY* entry = m.table[key->hash];
    MAP_ENTRY* new_entry = malloc(sizeof(MAP_ENTRY));
    new_entry->key = key;
    //create version
    new_entry->versions =NULL;
    new_entry->next = entry;
    m.table[key->hash] = new_entry;
    return new_entry;
}
MAP_ENTRY* find_map_entry(KEY* key){
    int index = key->hash % m.num_buckets;
    MAP_ENTRY* entry = m.table[index];
    while(entry!=NULL){
        int neq = key_compare(key, entry->key);
        if(!neq)
            break;
        entry = entry->next;
    }
    if(entry==NULL){
        debug("Create new map entry for key %p [%s] at table index %d",
            key,key->blob->content,index);
        entry = create_map_entry(key);
        if(m.table[index]==NULL){
            m.table[index] = entry;
        }
        else{
            entry->next  = m.table[index];
            m.table[index] = entry;
        }
    }
    return entry;
}
int add_dependencies(VERSION* vp){
    VERSION* cursor = vp->next;
    while(cursor != NULL){
        trans_add_dependency(vp->creator,cursor->creator);
        cursor = cursor->next;
    }

    return 0;
}
VERSION* add_version(TRANSACTION *tp, BLOB *bp, MAP_ENTRY* ep)
{
    VERSION *version =  version_create(tp, bp);
    if(ep->versions==NULL)
        ep->versions = version;
    else{
        //if there are versions exist
        //check if the new version id vs. id on the list
        //if greater, good, insert at head, add dependency
        //if same, update the Blob that is in the version
        //else, abort transaction
        //note: keep the head largest
        int id = tp->id;
        VERSION* head_v = ep->versions;
        if(id  >  head_v->creator->id){
            version-> next = head_v;
            ep->versions = version;
            //add depe
            add_dependencies(version);

        }
        else if(id == head_v->creator->id){
            // update the curernt version
            BLOB* oldbp = head_v->blob;
            head_v->blob = bp;
            blob_unref(oldbp,"update version");
        }
        else{
            return NULL;
        }
    }
    return version;
}

 // * Put a key/value mapping in the store.  The key must not be NULL.
 // * The value may be NULL, in which case this operation amounts to
 // * deleting any existing mapping for the given key.
 // *
 // * This operation inherits the key and consumes one reference on
 // * the value.
 // *
 // * @param tp  The transaction in which the operation is being performed.
 // * @param key  The key.
 // * @param value  The value.
 // * @return  Updated status of the transation, either TRANS_PENDING,
 // *   or TRANS_ABORTED.  The purpose is to be able to avoid doing further
 // *   operations in an already aborted transaction.


TRANS_STATUS store_put(TRANSACTION *tp, KEY *key, BLOB *value)
{

    if(key ==NULL || tp==NULL)  return TRANS_ABORTED;
    //if value is null, remove existing mapping for given key
    debug("Put mapping (key=%p [%s] -> value=%p [%s]) in store for transaction %d"
        ,key,
        key->blob->content,
        value,
        value-> content,
         tp->id);
    pthread_mutex_lock(&m.mutex);
    MAP_ENTRY* entry = find_map_entry(key);
    debug("Trying to put version in map entry for key %p [%s]",
        key,key->blob->content);
    VERSION* vp = add_version(tp, value,entry);
    pthread_mutex_unlock(&m.mutex);
    if(vp==NULL) return trans_abort(tp);
    // blob_unref(value,"");
    return TRANS_PENDING;
}

int entry_isnew(MAP_ENTRY* entry){
    if(entry->versions==NULL){
        return 1;
    }
    return 0;
}

VERSION* findFirstAborted(MAP_ENTRY* entry){
    VERSION* cursor = entry->versions;
    while(cursor!=NULL){
        if(cursor->creator->status == TRANS_ABORTED) break;
        cursor=cursor->next;
    }
    return cursor;
}
void remove_abort_version_before(VERSION* v,MAP_ENTRY* entry){
    if(v==NULL) return;
    VERSION* cursor = entry->versions;

    //abort all version before v
    while(cursor!=v){
        if(cursor==NULL) return;
        if(cursor->creator->status!=TRANS_ABORTED)
            trans_abort(cursor->creator);
        cursor=cursor->next;
    }
    //remove from list by setting head to v->next
    entry->versions = v->next;
}

VERSION* findFirstCommited(MAP_ENTRY* entry){
    VERSION* cursor = entry->versions;
    while(cursor!=NULL){
        if(cursor->creator->status == TRANS_COMMITTED) break;
        cursor=cursor->next;
    }
    return cursor;
}
void remove_version_after(VERSION* v,MAP_ENTRY* entry){
    if(v==NULL) return;
    if(entry->versions==v){
        v->next = NULL;
    }
    VERSION* cursor = entry->versions;

    //abort all version before v
    while(cursor!=v){
        if(cursor==NULL) return;
        if(cursor->creator->status!=TRANS_ABORTED)
            trans_abort(cursor->creator);
        cursor=cursor->next;
    }
    //remove from list by setting head to v->next
    entry->versions = v->next;
}
void remove_garbage(MAP_ENTRY* entry){
    if(entry==NULL) return;
    if(entry_isnew(entry)) return;

    VERSION* v =  findFirstAborted(entry);
    remove_abort_version_before(v,entry);
    v = findFirstCommited(entry);
    remove_version_after(v,entry);
}
/*
 * Get the value associated with a specified key.  A pointer to the
 * associated value is stored in the specified variable.
 *
 * This operation inherits the key.  The caller is responsible for
 * one reference on any returned value.
 *
 * @param tp  The transaction in which the operation is being performed.
 * @param key  The key.
 * @param valuep  A variable into which a returned value pointer may be
 *   stored.  The value pointer store may be NULL, indicating that there
 *   is no value currently associated in the store with the specified key.
 * @return  Updated status of the transation, either TRANS_PENDING,
 *   or TRANS_ABORTED.  The purpose is to be able to avoid doing further
 *   operations in an already aborted transaction.
 */

TRANS_STATUS store_get(TRANSACTION *tp, KEY *key, BLOB **valuep)
{
    if(key ==NULL || tp==NULL)  return TRANS_ABORTED;
    // Get the value associated with a specified key
    debug("Get mapping (key=%p [%s]) in store for transaction %d"
        ,key,
        key->blob->content,
        tp->id);

    //find the map entry, if entry is new, create new version and add to it.
    //return the Blob either way
    pthread_mutex_lock(&m.mutex);
    MAP_ENTRY* entry = find_map_entry(key);

    remove_garbage(entry);

    VERSION* vp = NULL;
    if(entry_isnew(entry)){
        vp = add_version(tp, NULL,entry);
        *valuep = NULL;
        if(vp==NULL){
            pthread_mutex_unlock(&m.mutex);
            return trans_abort(tp);
        }

    }
    else{
        // BLOB* blob = malloc
        *valuep = entry->versions->blob;
    }
    pthread_mutex_unlock(&m.mutex);
    return TRANS_PENDING;
}

/*
 * Print the contents of the store to stderr.
 * No locking is performed, so this is not thread-safe.
 * This should only be used for debugging.
 */
void store_show(void)
{
    for (int i = 0; i < m.num_buckets; ++i)
    {
        fprintf(stderr, "%d:\t",i);
        /* code */
        MAP_ENTRY* entry = m.table[i];
        // debug("entry is null,%d",entry==NULL);
        while(entry!=NULL){
            fprintf(stderr, "{key: %p [%s],versions: ",entry->key,entry->key->blob->content);
            VERSION* version = entry->versions;
            while(version!=NULL){
                VERSION* tmp_v = version->next;
                fprintf(stderr, "{creator=%d (%s), blob=%p [%s]}",
                    version->creator->id,
                    (version->creator->status == TRANS_PENDING)     ?"pending"
                        :(version->creator->status == TRANS_ABORTED)?"aborted"
                        :                                            "committed",
                    version->blob,
                    (version->blob==NULL)?"nullblob":version->blob->content);
                version = tmp_v;
            }
            fprintf(stderr, "}");
            MAP_ENTRY* tmp_e = entry->next;
            entry = tmp_e;
        }
        fprintf(stderr, "\n");

    }
}

