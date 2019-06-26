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
 	线程池任务
 	Task Node that means a task
 */
typedef struct cwork_node
{
	void *(*func)(void *);//callback function
	void *arg;//arg of the function
}CWork_node;
/*
	任务队列结点
	The node of the task fifo queue
*/
typedef struct cthread_fifo_node
{
	void *data; //Usually point to the task node
	struct cthread_fifo_node *next; //point to the next node of the task fifo queue
}CThread_fifo_node;

/*
	任务队列首部
	The head of the task fifo queue
*/
typedef struct cthread_fifo_head
{
	unsigned int count;	//The total count of the task fifo node
	CThread_fifo_node *head; //point to the first node of the task fifo
	CThread_fifo_node *tail;
	/*用于释放node中的data*/
	void (*del)(void *val);//delete the data upon the fifo when delete the fifo node
}CThread_fifo_head;

/*
	The worker to handle the task node
*/

/*
	工作线程队列node
*/
typedef struct cthread_pool_circul_fifo_node
{
	pthread_t threadid;				/*工作线程的ID*/
	pthread_mutex_t read_lock;
	pthread_cond_t read_ready;		/*工作线程的条件变量*/
	void *data;						/*任务队列中的任务*/
	CThread_fifo_head *task_queue;
}CThread_pool_circul_fifo_node;

/*
	工作线程队列head
*/
typedef struct cthread_pool_circul_fifo_head
{
	CThread_pool_circul_fifo_node **work_cthread_circul_queue;	/*线程池循环数组*/
	int front;										/*头指针*/
	int rear;										/*尾指针*/
	int current_thread_num;							/*线程池剩余工作线程个数 The count of the idle cthread*/
	int max_thread_num;								/*线程池允许活动的工作线程数目*/
	int last_check_success_position;				/*master上一个检查为空闲的线程的指针*/
	void (*del)(void *val);							/*用于释放node中的data*/
}CThread_pool_circul_fifo_head;

/*线程池结构*/
typedef struct cthread_pool
{
	pthread_mutex_t queue_lock_write;//任务队列互斥锁
	pthread_mutex_t queue_lock_master;//用于master线程休眠时加锁
	pthread_cond_t queue_master_ready;
	
	/*双缓存队列*/
	CThread_fifo_head *task_worker_queue[2];
	
	/*
		用于交换双队列的队列指针,
		master线程使用swap_queue[0],
		往swap_queue[1]添加新的任务
	*/
	CThread_fifo_head *task_swap_queue[2];
	
	//pthread_t swap_thread;
	unsigned int swap_num;
	pthread_t master_thread;
	
	int shutdown;//线程池释放的标识
	
	/*
		线程池循环队列
		get task from task_swap_queue[0], and handle it 
	*/
	CThread_pool_circul_fifo_head *cthread_worker_queue;
	
}CThread_pool;

#endif