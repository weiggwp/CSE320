
// #include <stdlib.h>
// #include <pthread.h>
// #include "transaction.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "data.h"

/*
 * Create a blob with given content and size.
 * The content is copied, rather than shared with the caller.
 * The returned blob has one reference, which becomes the caller's
 * responsibility.
 *
 * @param content  The content of the blob.
 * @param size  The size in bytes of the content.
 * @return  The new blob, which has reference count 1.
 */
BLOB *blob_create(char *content, size_t size)
{
    //content cannot be NULL
    if(content ==NULL)
    {
        printf("\n blob content cannot be NULL\n");
        exit(EXIT_FAILURE);
    }
    BLOB* blob = malloc(sizeof(BLOB));
    if (pthread_mutex_init(& blob->mutex, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        exit(EXIT_FAILURE);
    }
    blob->refcnt = 1;
    blob->size = size;

    //FIXME: might not need to malloc
    blob->content = malloc(size+1);
    strncpy(blob->content,content,size);
    blob->content[size]='\0'; //to ensure string end with null terminator

    blob->prefix = malloc(size+1);
    strncpy(blob->prefix,content,size);
    blob->prefix[size]='\0'; //to ensure string end with null terminator

    return blob;

}

/*
 * Increase the reference count on a blob.
 *
 * @param bp  The blob.
 * @param why  Short phrase explaining the purpose of the increase.
 * @return  The blob pointer passed as the argument.
 */
BLOB *blob_ref(BLOB *bp, char *why)
{
    if(!bp)
        return bp;
    pthread_mutex_lock(&bp->mutex);
    bp->refcnt +=1; //increase refcnt

    //TODO: how to use why?
    pthread_mutex_unlock(&bp->mutex);
    return bp;
}

/*
 * Decrease the reference count on a blob.
 * If the reference count reaches zero, the blob is freed.
 *
 * @param bp  The blob.
 * @param why  Short phrase explaining the purpose of the decrease.
 */
void blob_unref(BLOB *bp, char *why)
{
    if(!bp)
        return;
    pthread_mutex_t mutex = bp->mutex;
    pthread_mutex_lock(&mutex);
    bp->refcnt -=1; //decrement refcnt
    if(bp->refcnt==0){
        free(bp->content);//FIXME:might not need to malloc
        free(bp);
    }
    //TODO: how to use why?
    pthread_mutex_unlock(&mutex);
}


 // * Compare two blobs for equality of their content.
 // *
 // * @param bp1  The first blob.
 // * @param bp2  The second blob.
 // * @return 0 if the blobs have equal content, nonzero otherwise.

int blob_compare(BLOB *bp1, BLOB *bp2)
{
    if(bp1==NULL||bp2==NULL)
        return -1;
    return strcmp(bp1->content,bp2->content);
}

/*
 * Hash function for hashing the content of a blob.
 *
 * @param bp  The blob.
 * @return  Hash of the blob.
 */
int blob_hash(BLOB *bp)
{
    if(!bp)
        return 0;
    unsigned int sum = 0;
    char* str = bp->content;
    for (int i = 0; i < strlen(str); ++i)
    {
        /* code */
        sum += str[i] *(i+1);
    }
    return sum % 8;

}

/*
 * Create a key from a blob.
 * The key inherits the caller's reference to the blob.
 *
 * @param bp  The blob.
 * @return  the newly created key.
 */
KEY *key_create(BLOB *bp)
{
    KEY* k = malloc(sizeof(KEY));
    k->hash = blob_hash(bp);
    k->blob = bp;
    return k;
}

/*
 * Dispose of a key, decreasing the reference count of the contained blob.
 * A key must be disposed of only once and must not be referred to again
 * after it has been disposed.
 *
 * @param kp  The key.
 */
void key_dispose(KEY *kp)
{
    if(!kp->blob){
        blob_unref(kp->blob,NULL);
    }
    free(kp);
}

/*
 * Compare two keys for equality.
 *
 * @param kp1  The first key.
 * @param kp2  The second key.
 * @return  0 if the keys are equal, otherwise nonzero.
 */
int key_compare(KEY *kp1, KEY *kp2)
{
    if(kp1==NULL || kp2==NULL)
        return -1;
    if(kp1->hash!=kp2->hash)
        return -1;
    return blob_compare(kp1->blob,kp2->blob);
}

/*
 * Create a version of a blob for a specified creator transaction.
 * The version inherits the caller's reference to the blob.
 * The reference count of the creator transaction is increased to
 * account for the reference that is stored in the version.
 *
 * @param tp  The creator transaction.
 * @param bp  The blob.
 * @return  The newly created version.
 */
VERSION *version_create(TRANSACTION *tp, BLOB *bp)
{
    //FIXME:copy tp? if so, dont foget dispose
    VERSION* version = malloc(sizeof(VERSION));

    version->creator = tp;
    version->blob = bp;
    version->next = NULL;
    version->prev = NULL;
    return version;
}

/*
 * Dispose of a version, decreasing the reference count of the
 * creator transaction and contained blob.  A version must be
 * disposed of only once and must not be referred to again once
 * it has been disposed.
 *
 * @param vp  The version to be disposed.
 */
void version_dispose(VERSION *vp)
{
    if(!vp)
        return;
    if(!vp->creator)
        trans_unref(vp->creator,NULL);
    if(!vp->blob)
        blob_unref(vp->blob,NULL);

    free(vp);
}

