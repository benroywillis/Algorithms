
#include <cstdlib>

#define SIZE 	100

using namespace std;

int main( int argc, char** argv )
{
	auto malloc_p = (int*)malloc( SIZE*sizeof(int) );
	auto new_p    = new int[SIZE];
	// llvm.memset
	for( unsigned i = 0; i < SIZE; i++ )
	{
		new_p[i] = 0;
	}
	for( unsigned i = 0; i < SIZE; i++ )
	{
		malloc_p[i] = new_p[i];
	}
	// llvm.memcpy
	for( unsigned i = 0; i < SIZE; i++ )
	{
		new_p[i] = rand();
	}
	for( unsigned i = 0; i < SIZE; i++ )
	{
		malloc_p[i] = new_p[i];
	}
	// llvm.memmov

	for( unsigned i = 0; i < SIZE; i++ )
	{
		volatile int j = malloc_p[i] * new_p[i];
	}
	free(malloc_p);
	delete new_p;
	return 0;
}
