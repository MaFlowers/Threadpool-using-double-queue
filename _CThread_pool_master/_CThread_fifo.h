#ifndef __CTHREAD_FIFO_H__
#define __CTHREAD_FIFO_H__
 
#include "_CThread_type.h"

#define CThread_fifo_is_empty(X) ((X)->head == NULL && (X)->tail == NULL)

/*******************************************************************************
								任务队列
								The task fifo
*******************************************************************************/
CThread_fifo_head *CThread_fifo_head_new(void);
void CThread_fifo_head_free(CThread_fifo_head *queue);
CThread_fifo_node *CThread_fifo_node_new(void);
void CThread_fifo_node_free(CThread_fifo_node *node);
void CThread_fifo_head_delete_all_node(CThread_fifo_head *queue);
void CThread_fifo_head_delete(CThread_fifo_head *queue);
void *CThread_fifo_node_get_head(CThread_fifo_head *queue);
void *CThread_fifo_node_get_tail(CThread_fifo_head *queue);
void CThread_fifo_node_add_head(CThread_fifo_head *queue, void *val);
void CThread_fifo_node_add_tail(CThread_fifo_head *queue, void *val);
void CThread_fifo_node_delete_head(CThread_fifo_head *queue);
void CThread_fifo_node_delete_specific(CThread_fifo_head *queue, void *val);

/*******************************************************************************
								工作线程队列
								The worker fifo
*******************************************************************************/
CThread_pool_circul_fifo_head *CThread_pool_circul_fifo_head_new(int max_thread_num);
void CThread_pool_circul_fifo_head_free(CThread_pool_circul_fifo_head *queue);
CThread_pool_circul_fifo_node *CThread_pool_circul_fifo_node_new();
void CThread_pool_circul_fifo_node_free(CThread_pool_circul_fifo_node *node);
void CThread_pool_circul_fifo_node_add(CThread_pool_circul_fifo_head *queue, CThread_pool_circul_fifo_node *node);
CThread_pool_circul_fifo_node *CThread_pool_circul_fifo_node_delete(CThread_pool_circul_fifo_head *queue);
void CThread_pool_circul_fifo_head_delete_all_node(CThread_pool_circul_fifo_head *queue);
void CThread_pool_circul_fifo_head_delete(CThread_pool_circul_fifo_head *queue);
#endif
