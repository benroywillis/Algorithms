#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "gsl/gsl_heapsort.h"

#define SIZE 	1000

// sorts ints into ascending order (negative if a < b, 0 if equal, positive if a > b)
int compare_int(const int* a, const int* b)
{
	return *a - *b;
}

int main()
{
	struct timespec start, end;
	while( clock_gettime(CLOCK_MONOTONIC, &start) ) {}

	int* array = (int*)malloc(SIZE*sizeof(int));
	for( int i = 0; i < SIZE; i++ )
	{
		array[i] = (int)rand();
	}
	gsl_heapsort(array, SIZE, sizeof(int), compare_int);

	while( clock_gettime(CLOCK_MONOTONIC, &end) ) {}
	double diff = (double)end.tv_sec - (double)start.tv_sec;
	double diff_n = ((double)end.tv_nsec - (double)start.tv_nsec) * pow( 10.0, -9.0 );
	double total = diff + diff_n;
	printf("Time: %f\n", total);
	return 0;
}
