#include "CThreadPool.h"



void Pool_Init(CThread_Pool *Pool,int max_thread_count)
{
	//Pool=(CThread_Pool*)malloc(sizeof(CThread_Pool));

	pthread_mutex_init(&(Pool->q_lock),NULL);
	pthread_cond_init(&(Pool->q_ready),NULL);

	Pool->q_head=NULL;

	Pool->max_thread_count=max_thread_count;
	Pool->current_q_size=0;

	Pool->shutdown=0;

	Pool->ThreadID=(pthread_t*)malloc(sizeof(pthread_t)*max_thread_count);

	for(int i=0;i<Pool->max_thread_count;i++)
	{
		pthread_create(&(Pool->ThreadID[i]),NULL,Thread_Routine,Pool);
		//sleep(1);
	}
}


int Pool_Destroy(CThread_Pool *Pool)
{
	if(Pool->shutdown)
		return -1;
	Pool->shutdown=1;

	pthread_cond_broadcast(&(Pool->q_ready));

	for(int i=0;i<Pool->max_thread_count;i++)
	{
		pthread_join(Pool->ThreadID[i],NULL);
	}
	free(Pool->ThreadID);

	CThread_Worker_Node *Head=NULL;
	while(Pool->q_head!=NULL)
	{
		Head=Pool->q_head;
		Pool->q_head=Pool->q_head->next;
		free(Head);
	}

	pthread_mutex_destroy(&(Pool->q_lock));
	pthread_cond_destroy(&(Pool->q_ready));

	free(Pool);
	Pool=NULL;
	
	return 0;
}


void *Thread_Routine(void *arg)
{
	CThread_Pool *Pool=(CThread_Pool*) arg;

	//printf("thread 0x%x ready\n",pthread_self());

	while(1)
	{
		pthread_mutex_lock(&(Pool->q_lock));
		while(Pool->current_q_size ==0 && !Pool->shutdown)
		{
			//printf("Thread 0x%x is waiting\n",pthread_self());
			pthread_cond_wait(&(Pool->q_ready),&(Pool->q_lock));
		}

		if(Pool->shutdown)
		{
			pthread_mutex_unlock(&(Pool->q_lock));
			pthread_exit(NULL);
		}

		//printf("Thread 0x%x is Working\n",pthread_self());

		assert(Pool->current_q_size!=0);
		assert(Pool->q_head!=NULL);

		Pool->current_q_size--;
		
		CThread_Worker_Node *WORK =Pool->q_head;
		Pool->q_head=Pool->q_head->next;
		pthread_mutex_unlock(&(Pool->q_lock));

		(*(WORK->thread_fun))(WORK->arg);
		free(WORK);
		WORK=NULL;

	}
	pthread_exit(NULL);
}


int Adding_Worker(CThread_Pool *Pool,void *(*thread_fun)(void *arg),void *arg)
{
	CThread_Worker_Node *NewWorker=(CThread_Worker_Node*)malloc(sizeof(CThread_Worker_Node));
	NewWorker->thread_fun=thread_fun;
	NewWorker->arg=arg;
	NewWorker->next=NULL;

	pthread_mutex_lock(&(Pool->q_lock));
	CThread_Worker_Node *member=Pool->q_head;
	if(member!=NULL)
	{
		while(member->next!=NULL)
			member=member->next;
		member->next=NewWorker;
	}
	else
	{
		Pool->q_head=NewWorker;
	}

	assert(Pool->q_head!=NULL);

	Pool->current_q_size++;
	pthread_mutex_unlock(&(Pool->q_lock));
	pthread_cond_signal(&(Pool->q_ready));

	return 0;
}


