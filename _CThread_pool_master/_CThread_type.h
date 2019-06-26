#ifndef __CTHREAD_TYPE_H__
#define __CTHREAD_TYPE_H__
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/*
	for event task
*/
 /*
 	�̳߳�����
 	Task Node that means a task
 */
typedef struct cwork_node
{
	void *(*func)(void *);//callback function
	void *arg;//arg of the function
}CWork_node;
/*
	������н��
	The node of the task fifo queue
*/
typedef struct cthread_fifo_node
{
	void *data; //Usually point to the task node
	struct cthread_fifo_node *next; //point to the next node of the task fifo queue
}CThread_fifo_node;

/*
	��������ײ�
	The head of the task fifo queue
*/
typedef struct cthread_fifo_head
{
	unsigned int count;	//The total count of the task fifo node
	CThread_fifo_node *head; //point to the first node of the task fifo
	CThread_fifo_node *tail;
	/*�����ͷ�node�е�data*/
	void (*del)(void *val);//delete the data upon the fifo when delete the fifo node
}CThread_fifo_head;

/*
	The worker to handle the task node
*/

/*
	�����̶߳���node
*/
typedef struct cthread_pool_circul_fifo_node
{
	pthread_t threadid;				/*�����̵߳�ID*/
	pthread_mutex_t read_lock;
	pthread_cond_t read_ready;		/*�����̵߳���������*/
	void *data;						/*��������е�����*/
	CThread_fifo_head *task_queue;
}CThread_pool_circul_fifo_node;

/*
	�����̶߳���head
*/
typedef struct cthread_pool_circul_fifo_head
{
	CThread_pool_circul_fifo_node **work_cthread_circul_queue;	/*�̳߳�ѭ������*/
	int front;										/*ͷָ��*/
	int rear;										/*βָ��*/
	int current_thread_num;							/*�̳߳�ʣ�๤���̸߳��� The count of the idle cthread*/
	int max_thread_num;								/*�̳߳������Ĺ����߳���Ŀ*/
	int last_check_success_position;				/*master��һ�����Ϊ���е��̵߳�ָ��*/
	void (*del)(void *val);							/*�����ͷ�node�е�data*/
}CThread_pool_circul_fifo_head;

/*�̳߳ؽṹ*/
typedef struct cthread_pool
{
	pthread_mutex_t queue_lock_write;//������л�����
	pthread_mutex_t queue_lock_master;//����master�߳�����ʱ����
	pthread_cond_t queue_master_ready;
	
	/*˫�������*/
	CThread_fifo_head *task_worker_queue[2];
	
	/*
		���ڽ���˫���еĶ���ָ��,
		master�߳�ʹ��swap_queue[0],
		��swap_queue[1]����µ�����
	*/
	CThread_fifo_head *task_swap_queue[2];
	
	//pthread_t swap_thread;
	unsigned int swap_num;
	pthread_t master_thread;
	
	int shutdown;//�̳߳��ͷŵı�ʶ
	
	/*
		�̳߳�ѭ������
		get task from task_swap_queue[0], and handle it 
	*/
	CThread_pool_circul_fifo_head *cthread_worker_queue;
	
}CThread_pool;

#endif