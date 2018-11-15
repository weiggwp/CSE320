// #include <stdlib.h>
// #include <stdio.h>

#include "job.h"

void init_jobqueue(JobQueue* q)
{
    q-> front = NULL;
    q-> rear = NULL;
}

void enqueue_job(JobQueue* q,JOB *j)
{
    if (q->rear == NULL)
    {
       q->front = q->rear = j;
       return;
    }

    // Add the new node at the end of queue and change rear
    q->rear->other_info = j;
    q->rear = j;
}

JOB* dequeue_job(JobQueue* q)
{
    // If queue is empty, return NULL.
    if (q->front == NULL)
       return NULL;

    // Store previous front and move front one node ahead
    JOB *j = q->front;
    q->front = q->front->other_info;

    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
       q->rear = NULL;
    return j;
}