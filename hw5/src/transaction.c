// #include <stdlib.h>
// #include "transaction.h"
// #include "csapp.h"
// #include "debug.h"

// /*
//  * Initialize the transaction manager.
//  */
// void trans_init(void)
// {
//   trans_list.id = 0;
//   trans_list.next = &trans_list;
//   trans_list.prev = &trans_list;
//   if (pthread_mutex_init(&trans_list.mutex, NULL) != 0)
//   {
//       fprintf(stderr,"mutex init failed\n");
//       exit(EXIT_FAILURE);
//   }

// }

// /*
//  * Finalize the transaction manager.
//  */
// void trans_fini(void)
// {
//   //TODO: remove every transaction on the list?

//   // while(trans_list.next){
//   //   TRANSACTION* tp = trans_list.next;
//   //   trans_list.next = trans_list.next->next;
//   //   tran
//   // }
// }

// /*
//  * Create a new transaction.
//  *
//  * @return  A pointer to the new transaction (with reference count 1)
//  * is returned if creation is successful, otherwise NULL is returned.
//  */
// void insert(TRANSACTION* tp){
//   pthread_mutex_lock(&trans_list.mutex);
//   tp->next = trans_list.next;
//   tp->prev = &trans_list;
//   trans_list.next = tp;
//   tp->next->prev = tp;
//   tp->id = tp->next->id+1;
//   //IDK: trans_list has ref to tp, ref+2 ?
//   pthread_mutex_unlock(&trans_list.mutex);
// }
// TRANSACTION *trans_create(void)
// {
//   TRANSACTION* tp = malloc(sizeof(TRANSACTION));
//   tp->refcnt =1;
//   tp->status = TRANS_PENDING;
//   tp->depends = NULL;
//   tp->waitcnt = 0;
//   sem_init(&tp->sem, 0, 0);
//   if (pthread_mutex_init(&tp->mutex, NULL) != 0)
//   {
//       fprintf(stderr,"mutex init failed\n");
//       exit(EXIT_FAILURE);
//   }
//   insert(tp);
//   debug("Create new transaction %d", tp->id);
//   return tp;

// }

// /*
//  * Increase the reference count on a transaction.
//  *
//  * @param tp  The transaction.
//  * @param why  Short phrase explaining the purpose of the increase.
//  * @return  The transaction pointer passed as the argument.
//  */
// TRANSACTION *trans_ref(TRANSACTION *tp, char *why)
// {
//   pthread_mutex_lock(&tp->mutex);
//   debug("Increase ref count on transaction %d (%d -> %d) for %s",
//     tp->id, tp->refcnt,tp->refcnt+1,why);
//   tp->refcnt++;
//   pthread_mutex_unlock(&tp->mutex);
//   return tp;
// }

// void remove_from_list(TRANSACTION* tp){
//   tp->next->prev = tp->prev;
//   tp->prev->next = tp->next;
//   tp->next = NULL;
//   tp->prev = NULL;
// }
// /*
//  * Decrease the reference count on a transaction.
//  * If the reference count reaches zero, the transaction is freed.
//  *
//  * @param tp  The transaction.
//  * @param why  Short phrase explaining the purpose of the decrease.
//  */
// void trans_unref(TRANSACTION *tp, char *why)
// {
//   if(!tp) return;
//   pthread_mutex_lock(&tp->mutex);
//   debug("Decrease ref count on transaction %d (%d -> %d) for %s",
//     tp->id, tp->refcnt,tp->refcnt-1,why);
//   tp->refcnt--;
//   remove_from_list(tp);
//   pthread_mutex_unlock(&tp->mutex);

//   pthread_mutex_t mutex;
//   pthread_mutex_init(&mutex,NULL);
//   pthread_mutex_lock(&mutex);
//   if(tp->refcnt==0){
//     free(tp);
//     tp =  NULL;
//   }
//   pthread_mutex_unlock(&mutex);

// }

// /*
//  * Add a transaction to the dependency set for this transaction.
//  *
//  * @param tp  The transaction to which the dependency is being added.
//  * @param dtp  The transaction that is being added to the dependency set.
//  */
// void trans_add_dependency(TRANSACTION *tp, TRANSACTION *dtp)
// {
//   if(!tp ||!dtp)
//     return;
//   if(tp->id < dtp->id)
//     return;
//   int found = 0;
//   DEPENDENCY* cursor = tp->depends;
//   while(!cursor){
//     if(cursor->trans == dtp)
//       found = 1;//IDK: right way to check of dtp exist?
//     cursor = cursor->next;
//   }
//   if(found) return;

//   //dtp not found, add it to set
//   debug("Make transaction %d dependent on transaction %d",dtp->id,tp->id);
//   DEPENDENCY* depends = malloc(sizeof(DEPENDENCY));
//   depends->trans = dtp;
//   //insert at head
//   if(tp->depends){
//     depends->next = tp->depends;
//     tp->depends = depends;
//   }
//   tp->waitcnt++;

//   trans_ref(dtp,"transaction in dependency");
// }

// void release_dependents(TRANSACTION* tp)
// {
//   debug("Release %d waiters dependent on transaction %d",tp->waitcnt,tp->id);
//   // DEPENDENCY* cursor = tp->depends;
//   while(tp->depends){
//     DEPENDENCY * tmp = tp->depends;
//     tp->depends=tp->depends->next;
//     trans_unref(tmp->trans,"release dependency");
//     free(tmp);
//   }
// }
// /*
//  * Try to commit a transaction.  Committing a transaction requires waiting
//  * for all transactions in its dependency set to either commit or abort.
//  * If any transaction in the dependency set abort, then the dependent
//  * transaction must also abort.  If all transactions in the dependency set
//  * commit, then the dependent transaction may also commit.
//  *
//  * In all cases, this function consumes a single reference to the transaction
//  * object.
//  *
//  * @param tp  The transaction to be committed.
//  * @return  The final status of the transaction: either TRANS_ABORTED,
//  * or TRANS_COMMITTED.
//  */
// TRANS_STATUS trans_commit(TRANSACTION *tp)
// {
//   if(!tp)
//     return TRANS_ABORTED;
//   debug("Transaction %d trying to commit",tp->id);
//   DEPENDENCY* cursor = tp->depends;
//   while(cursor){
//     TRANSACTION* cursor_trans = cursor->trans;
//     debug("Transaction %d checking status of dependency %d", tp->id,cursor_trans->id);
//     if(cursor_trans->status == TRANS_ABORTED) return TRANS_ABORTED;//FIXME: do I need to check for commited status
//     if(cursor_trans->status == TRANS_COMMITTED){
//       cursor = cursor->next;
//       continue;
//     }
//     debug("Transaction %d waiting for dependency %d",tp->id,cursor_trans->id);
//     P(&cursor_trans->sem); //wait
//     if(cursor_trans->status == TRANS_ABORTED) return TRANS_ABORTED;//FIXME: do I need to check for commited status

//     cursor = cursor->next;
//   }
//   V(&tp->sem);
//   release_dependents(tp);
//   trans_unref(tp, "committing transaction");
//   return TRANS_COMMITTED;
// }


// /*
//  * Abort a transaction.  If the transaction has already committed, it is
//  * a fatal error and the program crashes.  If the transaction has already
//  * aborted, no change is made to its state.  If the transaction is pending,
//  * then it is set to the aborted state, and any transactions dependent on
//  * this transaction must also abort.
//  *
//  * In all cases, this function consumes a single reference to the transaction
//  * object.
//  *
//  * @param tp  The transaction to be aborted.
//  * @return  TRANS_ABORTED.
//  */
// TRANS_STATUS trans_abort(TRANSACTION *tp)
// {
//   if(!tp)
//     return TRANS_ABORTED;
//   if(trans_get_status(tp)==TRANS_COMMITTED)
//     exit(1);
//   tp->status = TRANS_ABORTED;
//   release_dependents(tp);
//   V(&tp->sem);
//   trans_unref(tp,"aborting transaction");
//   return TRANS_ABORTED;
// }

// /*
//  * Get the current status of a transaction.
//  * If the value returned is TRANS_PENDING, then we learn nothing,
//  * because unless we are holding the transaction mutex the transaction
//  * could be aborted at any time.  However, if the value returned is
//  * either TRANS_COMMITTED or TRANS_ABORTED, then that value is the
//  * stable final status of the transaction.
//  *
//  * @param tp  The transaction.
//  * @return  The status of the transaction, as it was at the time of call.
//  */
// TRANS_STATUS trans_get_status(TRANSACTION *tp){
//   return tp->status;
// }

// /*
//  * Print information about a transaction to stderr.
//  * No locking is performed, so this is not thread-safe.
//  * This should only be used for debugging.
//  *
//  * @param tp  The transaction to be shown.
//  */
// void trans_show(TRANSACTION *tp){
//   fprintf(stderr, "[id=%d,status=%d,refcnt=%d]",tp->id,tp->status,tp->refcnt );
// }

// /*
//  * Print information about all transactions to stderr.
//  * No locking is performed, so this is not thread-safe.
//  * This should only be used for debugging.
//  */
// void trans_show_all(void){
//   TRANSACTION* transaction = trans_list.next;
//   while(transaction!= &trans_list){
//     trans_show(transaction);
//     transaction = transaction->next;
//   }

// }
