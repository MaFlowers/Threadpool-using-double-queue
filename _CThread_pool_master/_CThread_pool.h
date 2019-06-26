#ifndef __CTHREAD_POOL_H__
#define __CTHREAD_POOL_H__
#include "_CThread.h"
#include "_CThread_type.h"
#include <pthread.h>

#define	CTHREAD_ON	1
#define	CTHREAD_OFF	0
#define CTHREAD_COMPLETE_OFF	2
//0.1姣
#define	CTHREAD_MASTER_USLEEP	100

CWork_node *
CWork_node_new(void);

void
CWork_node_free(void *p);


/*******************************************************************
	初始化线程池,为线程池创建max_thread_num个线程
*******************************************************************/
void CThread_pool_init(int max_thread_num);
/*******************************************************************
						销毁线程池
*******************************************************************/
void CThread_pool_destroy();

/*******************************************************************
						   添加任务
*******************************************************************/
void CThread_pool_add_worker(void *(*func)(void *), void *arg);

#endif
