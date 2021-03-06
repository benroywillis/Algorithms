#include <stdio.h>

/**
 * This file should have 8 kernels in it
 * fib() should be 4 (two for loops that go out of hte indirect recursion, one for the way down, one for the way up)
 * fib2() 
 * 
 */

/**
 * John: do we need to perform parent/child fusion to do away with "loops" like the loop formed when fib() starts returning
 *  - in this case we would "just do it"
 *  - in the case that the function has within it a loop, or other type of cycle, we wouldn't
 */

int fib(int num)
{
	if( num == 0 ) {
		return 0;
	}
	else if( num == 1 ) {
		return 1;
	}
	else {
		return fib(num - 1) + fib2(num - 2);
	}
}

int fib2(int num)
{
	if( num == 0 ) {
		return 0;
	}
	else if( num == 1 ) {
		return 1;
	}
	else {
		return fib(num - 1) + fib2(num - 2);
	}
}

int main(int argc, char** argv)
{
	int num = 0;
	if( argc < 2 )
	{
		printf("Please input a number to specify the iterations for the loop of fibonacci sequences.");
		return 1;
	}
	else
	{
		num = atoi(argv[1]);
	}
	for( int i = 0; i < num; i++ )
	{
		int n = fib(i);
		printf("Fibonacci sequence for %d: %d\n", i, n);
	}
	int n = fib2(num);
	printf("For fun, the fibonacci sequence for %d is %d\n", num, n);
	return 0;
}
