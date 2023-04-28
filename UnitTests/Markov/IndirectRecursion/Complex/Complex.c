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

void shared_function()
{
	int doNothing = 1;
}

int fib(int num)
{
	shared_function();
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
	shared_function();
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

int parent0(int start)
{
	int n;
	if( start % 2 )
	{
		n = fib(start);
	}
	else
	{
		n = fib2(start);
	}
	return n;
}

int parent1(int start)
{
	int n;
	if( start % 2 == 0 )
	{
		n = fib(start);
	}
	else
	{
		n = fib2(start);
	}
	return n;
}

int parent2(int start)
{
	int n;
	if( start % 3 == 0 )
	{
		n = parent0(start);
	}
	else if( start % 3 == 1 )
	{
		n = parent1(start);
	}
	else
	{
		n = fib(start);
	}
	return n;
}

int main(int argc, char** argv)
{
	shared_function();
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
		int n = parent0(i);
		n = parent1(i);
		printf("Fibonacci sequence for %d: %d\n", i, n);
	}
	int n = parent2(num);
	n = parent2(num-1);
	n = parent2(num-2);
	printf("For fun, the fibonacci sequence for %d is %d\n", num, n);
	return 0;
}
