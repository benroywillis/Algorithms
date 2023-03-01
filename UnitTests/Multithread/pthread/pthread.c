
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define THREAD_COUNT 	10

void threadFunc0()
{
	printf("This is thread 0!\n");
}

void threadFunc1()
{
	printf("This is thread 1!\n");
}

int main()
{
	pthread_t* threads = (pthread_t*)malloc( THREAD_COUNT*sizeof(pthread_t) );
	for( unsigned int i = 0; i < THREAD_COUNT; i++ )
	{
		if( i % 2 )
		{
			pthread_create(&threads[i], NULL, threadFunc1, NULL);
		}
		else
		{
			pthread_create(&threads[i], NULL, threadFunc0, NULL);
		}
	}
	for( unsigned int i = 0; i < THREAD_COUNT; i++ )
	{
		pthread_join( threads[i], NULL );
	}

	free(threads);
	return 0;
}
