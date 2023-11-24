
#include <stdlib.h>
#include <stdbool.h>

#define SIZE 		64
#define PRECISION 	int

int main( int argc, char** argv )
{
	//PRECISION* a = (PRECISION*)malloc(SIZE*SIZE*sizeof(PRECISION));
	//PRECISION* b = (PRECISION*)malloc(SIZE*SIZE*sizeof(PRECISION));
	//PRECISION* c = (PRECISION*)malloc(SIZE*SIZE*sizeof(PRECISION));
	PRECISION a[SIZE*SIZE];
	PRECISION b[SIZE*SIZE];
	PRECISION c[SIZE*SIZE];

	for( int i = 0; i < SIZE; i++ )
	{
		for( int j = 0; j < SIZE; j++ )
		{
			//a[i*SIZE+j] = rand();
			//b[i*SIZE+j] = rand();
			a[i*SIZE+j] = 0;
			b[i*SIZE+j] = 0;
		}
	}

	for( unsigned i = 0 ; i < SIZE ; i++ )
	{
		for( unsigned j = 0; j < SIZE; j++ )
		{
			c[i*SIZE + j] = a[i*SIZE + j]*b[i*SIZE + j];
		}
	}

	for( unsigned i = 0; i < SIZE; i++ )
	{
		for( unsigned j = 0; j < SIZE; j++ )
		{
			if( c[i*SIZE+j] > 0 )
			{
				// ensures the optimizer won't do away with our previous work
				volatile bool doNothing = true;
			}
		}
	}

	return 0;
}
