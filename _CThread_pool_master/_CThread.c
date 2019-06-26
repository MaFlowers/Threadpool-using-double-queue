#include "_CThread.h"
#include <errno.h>
int CThread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
	int ret = -1;

	ret = pthread_mutex_init(mutex, attr);
	if(0 != ret)
	{
		printf("pthread_mutex_init failed.");
		exit(1);
	}
	
	return ret;
}

int CThread_mutex_lock(pthread_mutex_t *mutex)
{
	if(0 != pthread_mutex_lock(mutex))
	{
		printf("pthread_mutex_lock error.");
		exit(1);
	}

	return 0;
}
/*返回0表示加锁成功，非0表示线程繁忙*/
int CThread_mutex_trylock(pthread_mutex_t *mutex)
{
	return pthread_mutex_trylock(mutex);
}

int CThread_mutex_unlock(pthread_mutex_t *mutex)
{
	if(0 != pthread_mutex_unlock(mutex))
	{
		printf("pthread_mutex_unlock error.");
		exit(1);
	}

	return 0;
}

int CThread_mutex_destroy(pthread_mutex_t *mutex)
{
	if(0 != pthread_mutex_destroy(mutex))
	{
		printf("pthread_mutex_destroy error.");
		exit(1);
	}
	
	return 0;
}

int CThread_cond_init(pthread_cond_t  *cond, const pthread_condattr_t *attr)
{
	int ret = -1;
	
	ret = pthread_cond_init(cond, attr);
	if(0 != ret)
	{
		printf("pthread_cond_init failed.");
		exit(1);
	}
	
	return ret;
}

int CThread_cond_wait(pthread_cond_t  *cond, pthread_mutex_t *mutex)
{
	int ret = -1;

	ret = pthread_cond_wait(cond, mutex);
	if(0 != ret)
	{
		printf("pthread_cond_wait failed.");
		exit(1);
	}
	
	return ret;	
}

int CThread_cond_timedwait(pthread_cond_t  *cond, pthread_mutex_t *mutex, struct timespec *timeout)
{
	int ret = -1;
	ret = pthread_cond_timedwait(cond, mutex, timeout);
	if(ret)
	{
		if(ETIMEDOUT == errno)
			return 0;
		
		printf("pthread_cond_timedwait failed.\n");
		exit(1);
	}

	return 0;
}

int CThread_cond_signal(pthread_cond_t  *cond)
{
	int ret = -1;

	ret = pthread_cond_signal(cond);
	if(0 != ret)
	{
		printf("pthread_cond_signal failed.");
		exit(1);
	}
	
	return ret;	
}

int CThread_cond_destroy(pthread_cond_t  *cond)
{
	if(0 != pthread_cond_destroy(cond))
	{
		printf("pthread_cond_destroy error.");
		exit(1);
	}
	
	return 0;	
}

int CThread_create(pthread_t *thread,
			const pthread_attr_t *attr,
			void *(*start_routine)(void *),
			void *arg)
{
	int ret = -1;
	ret = pthread_create(thread, attr, start_routine, arg);
	if(0 != ret)
	{
		printf("pthread_create error.");
		exit(1);
	}

	return 0;
}

int CThread_join(pthread_t thread, void **ptr)
{
	int ret = -1;
	assert(thread);
	ret = pthread_join(thread, ptr);
	if(0 != ret)
	{
		printf("pthread_join error.");
		exit(1);
	}

	return 0;
}

int CThread_cond_broadcast(pthread_cond_t  *cond)
{
	int ret = -1;
	ret = pthread_cond_broadcast(cond);
	if(ret != 0)
	{
		printf("pthread_cond_broadcast error.");
		exit(1);
	}
	
	return 0;
}