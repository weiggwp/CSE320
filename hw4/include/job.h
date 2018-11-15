#include "imprimer.h"

typedef struct {
    JOB *front, *rear;
} JobQueue;

JOB* dequeue_job(JobQueue* q);

void enqueue_job(JobQueue* q,JOB* j);


void  init_jobqueue(JobQueue* q);