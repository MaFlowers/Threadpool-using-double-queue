#include "_CThread_fifo.h"
#include "_CThread.h"
#include "_CThread_pool.h"
/*******************************************************************************
								任务队列
								Task fifo
*******************************************************************************/
CThread_fifo_head *CThread_fifo_head_new(void)
{
	CThread_fifo_head *queue = (CThread_fifo_head *)malloc(sizeof(CThread_fifo_head));
	if(NULL == queue)
	{
		printf("queue new error.");
		exit(1);
	}

	queue->count = 0;
	queue->head = queue->tail = NULL;
	
	return queue;
}

void CThread_fifo_head_free(CThread_fifo_head *queue)
{
	free(queue);
	queue = NULL;
}

CThread_fifo_node *CThread_fifo_node_new(void)
{
	CThread_fifo_node *node = (CThread_fifo_node *)malloc(sizeof(CThread_fifo_node));
	if(NULL == node)
	{
		printf("fifo node malloc error.");
		exit(1);
	}

	node->next = NULL;
	return node;
}

void CThread_fifo_node_free(CThread_fifo_node *node)
{
	assert(node);
	free(node);
	node = NULL;
}

void CThread_fifo_head_delete_all_node(CThread_fifo_head *queue)
{
	CThread_fifo_node *node;
	CThread_fifo_node *next;

	assert(queue);
	for(node = queue->head; node; node = next)
	{
		next = node->next;
		if(queue->del)
		{
			(*queue->del)(node->data);
		}
		CThread_fifo_node_free(node);	
	}
	queue->head = queue->tail = NULL;
	queue->count = 0;
}
/*将队列全部释放包括结点*/
void CThread_fifo_head_delete(CThread_fifo_head *queue)
{
	assert(queue);
	CThread_fifo_head_delete_all_node(queue);
	CThread_fifo_head_free(queue);
}


void *CThread_fifo_node_get_head(CThread_fifo_head *queue)
{
	CThread_fifo_node *node;

	assert(queue);
	node = queue->head;

	if(node)
		return node->data;
	return NULL;
}

void *CThread_fifo_node_get_tail(CThread_fifo_head *queue)
{
	CThread_fifo_node *node;

	assert(queue);
	node = queue->tail;

	if(node)
		return node->data;
	return NULL;
}

void CThread_fifo_node_add_head(CThread_fifo_head *queue, void *val)
{
	CThread_fifo_node *node;

	assert(val != NULL);

	node = CThread_fifo_node_new();
	
	node->next = queue->head;
	node->data = val;

	if(queue->tail == NULL)
		queue->tail = node;

	queue->head = node;

	queue->count++;
}


void CThread_fifo_node_add_tail(CThread_fifo_head *queue, void *val)
{
	CThread_fifo_node *node;

	assert(val != NULL);
	
	node = CThread_fifo_node_new();
	node->data = val;

	if(queue->head == NULL)
		queue->head = node;
	else
		queue->tail->next = node;

	queue->tail = node;

	queue->count++;
}

void CThread_fifo_node_delete_head(CThread_fifo_head *queue)
{
	CThread_fifo_node *node;

	assert(queue);

	node = queue->head;
	if(node)
		queue->head = node->next;
	else
	{
		printf("!!!!!!!!!!!!\n");
		return;
	}
	if(!node->next)
	{
		queue->tail = node->next;
	}

	queue->count--;
	CThread_fifo_node_free(node);
}

void CThread_fifo_node_delete_specific(CThread_fifo_head *queue, void *val)
{
	CThread_fifo_node *prev_node;
	CThread_fifo_node *node;

	assert(queue);

	if(queue->head->data == val)
	{
		CThread_fifo_node_delete_head(queue);
		return;
	}

	for(prev_node = queue->head; prev_node->next; prev_node = prev_node->next)
	{
		if(prev_node->next->data == val)
		{
			node = prev_node->next;
			prev_node->next = node->next;

			if(!node->next)
			{
				queue->tail = prev_node;
			}
			
			queue->count--;
			CThread_fifo_node_free(node);
			return;
		}
	}
}

/*******************************************************************************
								线程池工作线程队列
								The worker fifo
*******************************************************************************/
CThread_pool_circul_fifo_node **CThread_pool_circul_fifo_node_fifo_new(CThread_pool_circul_fifo_head *queue)
{
#if 1
	CThread_pool_circul_fifo_node **fifo = (CThread_pool_circul_fifo_node **)malloc(queue->max_thread_num * sizeof(CThread_pool_circul_fifo_node*));
	if(NULL == fifo)
	{
		printf("cthread worker circul fifo node malloc error.");
		exit(1);
	}

	return fifo;
#else
	CThread_pool_circul_fifo_node **fifo = (CThread_pool_circul_fifo_node **)malloc(sizeof(CThread_pool_circul_fifo_node *));
	if(NULL == fifo)
	{
		printf("cthread worker circul fifo pointer malloc error.");
		exit(1);
	}

	int i;
	for(i = 0; i < queue->max_thread_num; i++)
	{
		fifo[i] = (CThread_pool_circul_fifo_node *)malloc(sizeof(CThread_pool_circul_fifo_node ));
		if(NULL == fifo[i])
		{
			printf("cthread worker circul fifo node malloc error.");
			exit(1);			
		}
	}

	return fifo;
#endif
}

void CThread_pool_circul_fifo_node_fifo_free(CThread_pool_circul_fifo_head *queue)
{
	assert(queue->work_cthread_circul_queue);

	free(queue->work_cthread_circul_queue);
	queue->work_cthread_circul_queue = NULL;
}

CThread_pool_circul_fifo_head *CThread_pool_circul_fifo_head_new(int max_thread_num)
{
	int index;
	CThread_pool_circul_fifo_node *node = NULL;
	CThread_pool_circul_fifo_head *queue = (CThread_pool_circul_fifo_head *)malloc(sizeof(CThread_pool_circul_fifo_head));
	if(NULL == queue)
	{
		printf("cthread worker queue new error.");
		exit(1);
	}

	queue->front = queue->rear = 0;
	queue->current_thread_num = 0;
	queue->max_thread_num = max_thread_num;
	queue->last_check_success_position = max_thread_num - 1;

	queue->work_cthread_circul_queue = CThread_pool_circul_fifo_node_fifo_new(queue);

	for (index = 0; index < queue->max_thread_num; ++index)
	{
		node = CThread_pool_circul_fifo_node_new();
		CThread_mutex_init(&node->read_lock, NULL);
		CThread_cond_init(&node->read_ready, NULL);
		node->data = NULL;
		node->task_queue = CThread_fifo_head_new();
		node->task_queue->del = CWork_node_free;		
		CThread_pool_circul_fifo_node_add(queue, node);
	}
	
	return queue;
}

void CThread_pool_circul_fifo_head_free(CThread_pool_circul_fifo_head *queue)
{
	assert(queue);

	CThread_pool_circul_fifo_head_delete_all_node(queue);
	CThread_pool_circul_fifo_node_fifo_free(queue);

	free(queue);
	queue = NULL;
}

CThread_pool_circul_fifo_node *CThread_pool_circul_fifo_node_new()
{
	CThread_pool_circul_fifo_node *node = (CThread_pool_circul_fifo_node *)malloc(sizeof(CThread_pool_circul_fifo_node));
	if(NULL == node)
	{
		printf("cthread worker circul fifo node malloc error.");
		exit(1);
	}

	return node;
}


void CThread_pool_circul_fifo_node_free(CThread_pool_circul_fifo_node *node)
{
	assert(node);
	free(node);
	node = NULL;
}

//添加工作线程node到队尾
void CThread_pool_circul_fifo_node_add(CThread_pool_circul_fifo_head *queue, CThread_pool_circul_fifo_node *node)
{
	assert(node&&queue);
	
	if(queue->current_thread_num == queue->max_thread_num)
	{
		printf("Cthread circul fifo is full,add node error.");
		return;
	}

	queue->work_cthread_circul_queue[queue->rear] = node;
	queue->rear = (queue->rear + 1) % queue->max_thread_num;

	queue->current_thread_num++;	
}
//删除队头元素：工作线程node
CThread_pool_circul_fifo_node *CThread_pool_circul_fifo_node_delete(CThread_pool_circul_fifo_head *queue)
{
	assert(queue);
	CThread_pool_circul_fifo_node *node = NULL;
	
	if(0 == queue->current_thread_num)
	{
		printf("Cthread circul fifo is null,delete node error.");
		return NULL;
	}
	node = queue->work_cthread_circul_queue[queue->front];
	
	queue->front = (queue->front + 1) % queue->max_thread_num;

	queue->current_thread_num--;

	return node;
}

void CThread_pool_circul_fifo_head_delete_all_node(CThread_pool_circul_fifo_head *queue)
{
	assert(queue);
	
	CThread_pool_circul_fifo_node *node = NULL;

	while(0 != queue->current_thread_num)//不为空
	{
		node = CThread_pool_circul_fifo_node_delete(queue);
		CThread_mutex_destroy(&node->read_lock);
		CThread_cond_destroy(&node->read_ready);
	
		if(queue->del && node->data)
		{
			(*queue->del)(node->data);
		}

		CThread_fifo_head_delete_all_node(node->task_queue);
		CThread_fifo_head_free(node->task_queue);
		
		CThread_pool_circul_fifo_node_free(node);
	}

}

void CThread_pool_circul_fifo_head_delete(CThread_pool_circul_fifo_head *queue)
{
	assert(queue);

	CThread_pool_circul_fifo_head_free(queue);
}

