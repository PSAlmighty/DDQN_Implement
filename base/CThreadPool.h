#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "pthread.h"
#include "sys/types.h"
#include "assert.h"

#ifndef __THREAD_POOL__
#define __THREAD_POOL__


typedef struct Worker_Node
{
	void *(*thread_fun)(void *arg);
	void *arg;
	struct Worker_Node *next;
}CThread_Worker_Node;


typedef struct
{
	pthread_mutex_t q_lock;
	pthread_cond_t q_ready;

	CThread_Worker_Node *q_head;

	int shutdown;
	pthread_t *ThreadID;
	int max_thread_count;
	int current_q_size;
}CThread_Pool;


int Adding_Worker(CThread_Pool *Pool,void *(*thread_run)(void *arg),void *arg);

void Pool_Init(CThread_Pool *Pool,int max_thread_count);

int Pool_Destroy(CThread_Pool *Pool);

void *Thread_Routine(void *arg);




#endif
