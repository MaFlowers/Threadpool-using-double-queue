#ifndef __CTHREAD_H__
#define __CTHREAD_H__
#include <pthread.h>
#include "_CThread_type.h"
 
int CThread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int CThread_mutex_lock(pthread_mutex_t *mutex);
int CThread_mutex_trylock(pthread_mutex_t *mutex);
int CThread_mutex_unlock(pthread_mutex_t *mutex);
int CThread_mutex_destroy(pthread_mutex_t *mutex);

int CThread_cond_init(pthread_cond_t  *cond, const pthread_condattr_t *attr);
int CThread_cond_wait(pthread_cond_t  *cond, pthread_mutex_t *mutex);
int CThread_cond_timedwait(pthread_cond_t  *cond, pthread_mutex_t *mutex, struct timespec *timeout);
int CThread_cond_signal(pthread_cond_t  *cond);
int CThread_cond_destroy(pthread_cond_t  *cond);

int CThread_create(pthread_t *thread,
			const pthread_attr_t *attr,
			void *(*start_routine)(void *),
			void *arg);
int CThread_join(pthread_t thread, void **ptr);
int CThread_cond_broadcast(pthread_cond_t  *cond);
#endif
