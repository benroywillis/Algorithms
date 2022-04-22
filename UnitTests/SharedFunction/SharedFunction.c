#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/**
 * There are 5 loops in this file
 * super() -> 1
 * sub()   -> 1
 * main    -> 3
 * We should get 5 kernels out the other side
 */

void* allocMemory(size_t elemSize, uint64_t count) {
	return calloc(count, elemSize);
}

void freeMemory(void* ptr) {
	if( ptr ) free(ptr);
	return;
}

int* super(int* lhs, int* rhs, int count) {
	int* second = allocMemory(sizeof(int), count);
	for( int i = 0; i < count; i++ )
		second[i] = lhs[i] + rhs[i];
	return second;
}

int* sub(int* lhs, int* rhs, int count) {
	int* second = allocMemory(sizeof(int), count);
	for( int i = 0; i < count; i++ )
		second[i] = lhs[i] - rhs[i];
	return second;
}

int main(int argc, char** argv) {
	int size = atoi(argv[1]);
	for( int i = 0; i < size; i++ ) {
		int* buffer = allocMemory(sizeof(int), size); int* buffer2;
		if( i % 2 ) { buffer2 = allocMemory(sizeof(int), size);
			for( int j = 0; j < size; j++ ) {
				if( j % 2 ) {
					int* buffer3 = super(buffer, buffer2, size); freeMemory(buffer3);
				}
				else {
					int* buffer3 = sub(buffer, buffer2, size); freeMemory(buffer3);
				}
			}
		}
		else { buffer2 = allocMemory(sizeof(int), size*2);
			for( int j = 0; j < size*2; j++ ) {
				if( j % 2 ) {
					int* buffer3 = super(buffer, buffer2, size); freeMemory(buffer3);
				}
				else {
					int* buffer3 = sub(buffer, buffer2, size); freeMemory(buffer3);
				}
			}
		}
		freeMemory(buffer2); freeMemory(buffer);
	}
	return 0;
}
