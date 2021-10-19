#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdio.h>

#define SIZE 	1000

/* Main algorithm drawn straight from Rosetta Code: https://www.rosettacode.org/wiki/Sorting_algorithms/Heapsort#C */

int max (int *a, int n, int i, int j, int k) {
    int m = i;
    if (j < n && a[j] > a[m]) {
        m = j;
    }
    if (k < n && a[k] > a[m]) {
        m = k;
    }
    return m;
}

void downheap (int *a, int n, int i) {
    while (1) {
        int j = max(a, n, i, 2 * i + 1, 2 * i + 2);
        if (j == i) {
            break;
        }
        int t = a[i];
        a[i] = a[j];
        a[j] = t;
        i = j;
    }
}

void heapsort (int *a, int n) {
    int i;
    for (i = (n - 2) / 2; i >= 0; i--) {
        downheap(a, n, i);
    }
    for (i = 0; i < n; i++) {
        int t = a[n - i - 1];
        a[n - i - 1] = a[0];
        a[0] = t;
        downheap(a, n - i - 1, 0);
    }
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

    heapsort(array, SIZE); // In place sort

    while( clock_gettime(CLOCK_MONOTONIC, &end) ) {}
	double diff = (double)end.tv_sec - (double)start.tv_sec;
	double diff_n = ((double)end.tv_nsec - (double)start.tv_nsec) * pow( 10.0, -9.0 );
	double total = diff + diff_n;
	printf("Time: %f\n", total);
	return 0;
}
