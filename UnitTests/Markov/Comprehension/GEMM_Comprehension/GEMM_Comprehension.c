
#include <stdlib.h>
#include <stdio.h>

#define SIZE 32

void GEMM(int* a, int* b, int* c)
{
	for( unsigned i = 0; i < SIZE; i++ )
	{
		for( unsigned j = 0; j < SIZE; j++ )
		{
			for( unsigned k = 0; k < SIZE; k++ )
			{
				c[i*SIZE+j] += a[i*SIZE+k] * b[k*SIZE+j];
			}
		}
	}
}

int main( int argc, char** argv )
{
	int* a = (int*)malloc(SIZE*SIZE*sizeof(int));
	int* b = (int*)malloc(SIZE*SIZE*sizeof(int));
	int* c = (int*)malloc(SIZE*SIZE*sizeof(int));
	int* d = (int*)malloc(SIZE*SIZE*sizeof(int));
	int* e = (int*)malloc(SIZE*SIZE*sizeof(int));
	for( unsigned int i = 0; i < SIZE; i++ )
	{
		for( unsigned j = 0; j < SIZE; j++ )
		{
			a[i*SIZE+j] = rand();
			b[i*SIZE+j] = rand();
			c[i*SIZE+j] = rand();
			d[i*SIZE+j] = rand();
		}
	}

	int i = 0;
	while( i < 32 )
	{
		GEMM(a, b, e);
		GEMM(a, c, e);
		GEMM(a, d, e);
		++i;
		printf("i: %d\n", i);
	}

	return 0;
}
