
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#ifndef SIZE
#define SIZE 512
#endif
#if PRECISION == 0
#define TYPE uint32_t
#define TYPE_MAX UINT32_MAX
#elif PRECISION == 1
#define TYPE uint16_t
#define TYPE_MAX UINT16_MAX
#else 
#define TYPE uint8_t
#define TYPE_MAX UINT8_MAX
#endif

int main() {
	TYPE* gry_img = (TYPE*)malloc(SIZE*sizeof(TYPE));
	for( unsigned i = 0; i < SIZE; i++ ) {
		gry_img[i] = (TYPE)(rand() % TYPE_MAX);
	}
	int* hist = (int*)calloc(TYPE_MAX, sizeof(int));
	for( unsigned i = 0; i < SIZE; i++ ) {
		hist[gry_img[i]]++;
	}
	free(gry_img);
	free(hist);
	return 0;
}
