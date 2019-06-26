#include "_CThread_pool.h"
#include <unistd.h>
#include <string.h>
#define POOL_MAX 1000000
int arg_int[POOL_MAX];

void *
processing_worker(void *arg)
{
	//sleep(1);
	printf("%d, thread 0x%x\n", *(int *)arg, (unsigned int)pthread_self());
	return ((void *)0);
}

int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		printf("param error.\n");
		exit(1);
	}
	memset(arg_int, 0, sizeof(int)*POOL_MAX);
	CThread_pool_init(atoi(argv[1]));
	//printf("sleep 2\n");
	//sleep(2);

	int i;
	for(i = 0; i< POOL_MAX; i++){
		arg_int[i] = i;
		CThread_pool_add_worker(processing_worker, &arg_int[i]);
	}
	sleep(100);
	printf("destroy pool.\n");
	CThread_pool_destroy();

	return 0;
}

