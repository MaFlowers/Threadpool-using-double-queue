#include "_CThread_pool.h"
#include "_CThread_fifo.h"
#include <unistd.h>
 
#ifdef HAVE_THREAD_POOL_DEBUG
#include <sys/time.h>
static struct timeval start_time;
static struct timeval end_time;
static struct timeval elapsed_time;
#endif
int completed = 0;

static CThread_pool *pool = NULL;

CWork_node *
CWork_node_new(void)
{
	CWork_node * Cwork= (CWork_node *)malloc(sizeof(CWork_node));
	if(NULL == Cwork)
	{
		printf("CWork_node_new error.");
		exit(1);
	}
	
	return Cwork;
}

void
CWork_node_free(void *p)
{
	CWork_node *Cwork = (CWork_node *)p;
	free(Cwork);
	Cwork = NULL;
}

static CThread_pool *
CThread_pool_new(void)
{
	CThread_pool *pool = (CThread_pool *)malloc(sizeof(CThread_pool));
	if(NULL == pool)
	{
		printf("create thread_pool failed.");
		exit(1);
	}
	return pool;
	
}

static void 
CThread_pool_free(CThread_pool *pool)
{
	free(pool);
	pool = NULL;
}

//试图加锁一个工作线程
static int 
CThread_pool_mutex_trylock_circul_fifo_node(CThread_pool_circul_fifo_node *work_cthread_circul_node)
{
	return CThread_mutex_trylock(&work_cthread_circul_node->read_lock);
}
//加锁一个工作线程
static void
CThread_pool_mutex_lock_circul_fifo_node(CThread_pool_circul_fifo_node *work_cthread_circul_node)
{
	CThread_mutex_lock(&work_cthread_circul_node->read_lock);
}
//解锁一个工作线程
static void
CThread_pool_mutex_unlock_circul_fifo_node(CThread_pool_circul_fifo_node *work_cthread_circul_node)
{
	CThread_mutex_unlock(&work_cthread_circul_node->read_lock);
}
//工作线程进入休眠
static void
CThread_pool_cond_wait_circul_fifo_node(CThread_pool_circul_fifo_node *work_cthread_circul_node)
{
	CThread_cond_wait(&work_cthread_circul_node->read_ready, &work_cthread_circul_node->read_lock);	
}
//唤醒一个工作线程
static void 
CThread_pool_cond_signal_circul_fifo_node(CThread_pool_circul_fifo_node *work_cthread_circul_node)
{
	CThread_cond_signal(&work_cthread_circul_node->read_ready);
}

static void 
CThread_pool_mutex_init(CThread_pool *pool)
{
	CThread_mutex_init(&pool->queue_lock_write, NULL);
}

static void 
CThread_pool_mutex_lock_write(CThread_pool *pool)
{
	CThread_mutex_lock(&pool->queue_lock_write);
}

static void 
CThread_pool_mutex_unlock_write(CThread_pool *pool)
{
	CThread_mutex_unlock(&pool->queue_lock_write);
}

static void 
CThread_pool_mutex_destroy(CThread_pool *pool)
{
	CThread_mutex_destroy(&pool->queue_lock_write);
}

static void 
CThread_pool_cond_init(CThread_pool *pool)
{
	CThread_cond_init(&pool->queue_master_ready, NULL);
}

void 
CThread_pool_master_cond_wait(CThread_pool *pool)
{
	CThread_cond_wait(&pool->queue_master_ready, &pool->queue_lock_write);
}

void maketimeout(struct timespec *tsp, long msec)
{
	struct timeval now;
	/* get the current time */
	gettimeofday(&now, NULL);
	tsp->tv_sec = now.tv_sec;
	tsp->tv_nsec = now.tv_usec * 1000;
	/* add the offset to get timeout value */
	tsp->tv_sec += msec / 1000;
	tsp->tv_nsec += (msec % 1000) * 1000000;
}

static int 
CThread_pool_master_cond_timedwait(CThread_pool *pool, int msec)
{
	struct timespec time;

	maketimeout(&time, msec);
		
	return CThread_cond_timedwait(&pool->queue_master_ready, &pool->queue_lock_write, &time);
}

static void 
CThread_pool_master_cond_signal(CThread_pool *pool)
{
	CThread_cond_signal(&pool->queue_master_ready);
}

static void 
CThread_pool_cond_destroy(CThread_pool *pool)
{
	CThread_cond_destroy(&pool->queue_master_ready);
}

static void
CThread_pool_create_pthread(CThread_pool *pool, 
			const pthread_attr_t *attr,
			void *(*start_routine)(void *))
{
	int index = 0;

	assert(pool->cthread_worker_queue->max_thread_num > 0);
	
	printf("create read thread_pool counts (%d)\n", pool->cthread_worker_queue->max_thread_num);
	for(index = 0; index < pool->cthread_worker_queue->max_thread_num; index++)
	{
		CThread_create(&(pool->cthread_worker_queue->work_cthread_circul_queue[index]->threadid), 
			attr, 
			start_routine, 
			pool->cthread_worker_queue->work_cthread_circul_queue[index]);
	}
}

static void
CThread_pool_join(CThread_pool *pool)
{
	int i = 0;
	assert(pool);
	for(i = 0; i < pool->cthread_worker_queue->current_thread_num; i++)
	{
		CThread_join(pool->cthread_worker_queue->work_cthread_circul_queue[i]->threadid, NULL);
	}

	CThread_join(pool->master_thread, NULL);
}
#if 0
static void 
CThread_pool_swap_cond_broadcast(CThread_pool *pool)
{
	CThread_cond_broadcast(&pool->queue_swap_ready);
}
#endif
void 
CThread_pool_master_cond_broadcast(CThread_pool *pool)
{
	CThread_cond_broadcast(&pool->queue_master_ready);
}

static void 
CThread_pool_fifo_swap(CThread_pool *pool)
{
	assert(pool->task_swap_queue[0]);
	assert(pool->task_swap_queue[1]);

	CThread_fifo_head *temp;
	temp = pool->task_swap_queue[0];
	pool->task_swap_queue[0] = pool->task_swap_queue[1];
	pool->task_swap_queue[1] = temp;

	pool->swap_num++;
}

/*双任务队列交换回调函数*/
void 
CThread_pool_swap_task_queue(void)
{
	CThread_pool_mutex_lock_write(pool);
	if((pool->task_swap_queue[0]->count == 0)&&(pool->task_swap_queue[1]->count > 0))
	{
		CThread_pool_fifo_swap(pool);
		
		//CThread_pool_master_cond_signal(pool);//在linux平台下，建议signal函数放在锁中间
	}
	CThread_pool_mutex_unlock_write(pool);
	
	CThread_pool_master_cond_signal(pool);
}

static void 
CThread_pool_master_find_idle_work_thread_and_add_task(CThread_pool_circul_fifo_head *work_thread_circul_queue,
																			CWork_node *cwork)
{
	assert(work_thread_circul_queue);
	CThread_pool_circul_fifo_node *check_work_thread_node = NULL;
	int check_position = work_thread_circul_queue->last_check_success_position + 1;
	
	while(1)
	{	
		if(check_position >= work_thread_circul_queue->max_thread_num)
			check_position = check_position % work_thread_circul_queue->max_thread_num;
		
		if(check_position == work_thread_circul_queue->last_check_success_position)
			usleep(CTHREAD_MASTER_USLEEP);

		check_work_thread_node = work_thread_circul_queue->work_cthread_circul_queue[check_position];

		/*试图加锁线程池中的一个工作线程node*/
		if(0 == CThread_pool_mutex_trylock_circul_fifo_node(check_work_thread_node))
		{
			//加锁成功，则为空闲工作线程
			CThread_fifo_node_add_tail(check_work_thread_node->task_queue, cwork);
			CThread_pool_mutex_unlock_circul_fifo_node(check_work_thread_node);
			CThread_pool_cond_signal_circul_fifo_node(check_work_thread_node);//唤醒该工作线程

			work_thread_circul_queue->last_check_success_position = check_position;
			break;
		}
		//printf("check next thread.\n");
		check_position++;

	}
}


/*线程池指派线程回调函数*/
static void *
CThread_pool_master_thread(void *arg)
{
	int index;
	CWork_node *Cwork = NULL;
	while(CTHREAD_ON == pool->shutdown)
	{
		if(pool->task_swap_queue[0]->count > 0)
		{
			Cwork = (CWork_node *)CThread_fifo_node_get_head(pool->task_swap_queue[0]);
			if(NULL == Cwork)
			{
				printf("task queue node is not null,but the cwork of ndoe is null");
				continue;
			}
			//fetch a cwork
			CThread_fifo_node_delete_head(pool->task_swap_queue[0]);
			
			CThread_pool_master_find_idle_work_thread_and_add_task(pool->cthread_worker_queue, Cwork);
		}
		else if((pool->task_swap_queue[0]->count == 0)&&(pool->task_swap_queue[1]->count > 0))
		{
			CThread_pool_swap_task_queue();/*swap task queue*/
			continue;
		}
		else if((pool->task_swap_queue[0]->count == 0)&&(pool->task_swap_queue[1]->count == 0))
		{
			CThread_pool_mutex_lock_write(pool);
			//CThread_pool_master_cond_wait(pool);/*No task , wait....*/
			CThread_pool_master_cond_timedwait(pool, 100000);//100ms
			CThread_pool_mutex_unlock_write(pool);
			continue;
		}
	}

	for(index = 0;index < pool->cthread_worker_queue->current_thread_num;index++)
	{
		CThread_pool_cond_signal_circul_fifo_node(pool->cthread_worker_queue->work_cthread_circul_queue[index]);
	}
	
	return ((void *)0);	
}

/*创建master线程*/
static void
CThread_pool_create_master_thread(CThread_pool *pool)
{
	
	printf("Create master thread\n");

	CThread_create(&pool->master_thread, NULL, CThread_pool_master_thread, NULL);

}

/*线程池工作线程的回调函数*/
static void *
CThread_pool_routine(void *arg)
{
	CWork_node *Cwork = NULL;

	CThread_pool_circul_fifo_node *cwork_thread_self = (CThread_pool_circul_fifo_node *)arg;

	if(pthread_self() == cwork_thread_self->threadid)
	{
		printf("starting thread 0x%x\n", (unsigned int)pthread_self());
	}
	else
	{
		printf("error! Threadid is not same as arg.\n");
		exit(1);
	}
	
	CThread_pool_mutex_lock_circul_fifo_node(cwork_thread_self);
	while(1)
	{	
		while(CThread_fifo_is_empty(cwork_thread_self->task_queue) && CTHREAD_ON == pool->shutdown)
			CThread_pool_cond_wait_circul_fifo_node(cwork_thread_self); 	
		
		if(CTHREAD_OFF == pool->shutdown)
			goto CTHREAD_END;

		while((Cwork = (CWork_node *)CThread_fifo_node_get_head(cwork_thread_self->task_queue)))
		{
			CThread_fifo_node_delete_head(cwork_thread_self->task_queue);
			
			/*调用回调函数,执行任务*/
			if(Cwork->func)
				(Cwork->func)(Cwork->arg);	

			(cwork_thread_self->task_queue->del)(Cwork);
			Cwork = NULL;
		}
	}

CTHREAD_END:
	CThread_pool_mutex_unlock_circul_fifo_node(cwork_thread_self);
	printf("thread 0x%x will exit\n", (unsigned int)pthread_self());
	return ((void *)0);
}

/*队列写线程:添加任务入队列*/
void 
CThread_pool_add_worker(void *(*func)(void *), void *arg)
{
	CWork_node *Cwork = CWork_node_new();
	
	Cwork->func = func;
	Cwork->arg = arg;

	CThread_pool_mutex_lock_write(pool);
	CThread_fifo_node_add_tail(pool->task_swap_queue[1], Cwork);	
	CThread_pool_mutex_unlock_write(pool);
	
	if((pool->task_swap_queue[0]->count == 0) && (pool->task_swap_queue[1]->count > 0))
		CThread_pool_master_cond_signal(pool);//在linux平台下，建议signal函数放在锁中间
	
}


/*初始化线程池*/
void
CThread_pool_init(int max_thread_num)
{
	pool = CThread_pool_new();

	CThread_pool_mutex_init(pool);
	CThread_pool_cond_init(pool);

	pool->task_worker_queue[0] = CThread_fifo_head_new();
	pool->task_worker_queue[0]->del = CWork_node_free;
	pool->task_worker_queue[1] = CThread_fifo_head_new();
	pool->task_worker_queue[1]->del = CWork_node_free;

	pool->task_swap_queue[0] = pool->task_worker_queue[0];
	pool->task_swap_queue[1] = pool->task_worker_queue[1];
	pool->swap_num = 0;

	pool->shutdown = CTHREAD_ON;//开启

	pool->cthread_worker_queue = CThread_pool_circul_fifo_head_new(max_thread_num);
	pool->cthread_worker_queue->del = CWork_node_free;

	CThread_pool_create_master_thread(pool);
	
	CThread_pool_create_pthread(pool, 
					NULL, 
					CThread_pool_routine);
#ifdef HAVE_THREAD_POOL_DEBUG
	gettimeofday(&start_time, NULL);
#endif

}

/*销毁线程池*/
void 
CThread_pool_destroy()
{
	if(CTHREAD_OFF == pool->shutdown)
		return;
#ifdef HAVE_THREAD_POOL_DEBUG	
	gettimeofday(&end_time, NULL);
	assert(end_time.tv_sec >= start_time.tv_sec);
	while(end_time.tv_usec < start_time.tv_usec)
	{
		--end_time.tv_sec;
		end_time.tv_usec += 1000000;
	}

	elapsed_time.tv_sec = end_time.tv_sec - start_time.tv_sec;
	elapsed_time.tv_usec = end_time.tv_usec - start_time.tv_usec;

	printf("The elapsed time:(%ld)sec, (%ld)usec\n", elapsed_time.tv_sec, elapsed_time.tv_usec);

	printf("The swap num is (%d)\n",pool->swap_num);
#endif	

	pool->shutdown = CTHREAD_OFF;
	/*唤醒master,再由master唤醒线程池,退出线程master和线程池*/
	CThread_pool_master_cond_signal(pool);
	
	/*阻塞等待所有线程退出*/
	CThread_pool_join(pool);

	pool->task_swap_queue[0] = NULL;
	pool->task_swap_queue[1] = NULL;

	/*释放双缓存队列*/
	CThread_fifo_head_delete(pool->task_worker_queue[0]);
	CThread_fifo_head_delete(pool->task_worker_queue[1]);

	/*释放write互斥锁和master条件变量*/
	CThread_pool_mutex_destroy(pool);
	CThread_pool_cond_destroy(pool);

	CThread_pool_circul_fifo_head_delete(pool->cthread_worker_queue);

	CThread_pool_free(pool);

}

