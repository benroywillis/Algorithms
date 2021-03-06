#include <stdio.h>
//#include <stdlib.h>

/**
 * we should get 7 kernels out of this 
 * one for the loop around fib()
 * three for the fib() call in the loop
 * three for the fib() call outside the loop
 * fib() has 3 kernels because it has 3 conditions within the function
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
		return fib(num - 1) + fib(num - 2);
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
	int n = fib(num);
	printf("For fun, the fibonacci sequence for %d is %d\n", num, n);
	return 0;
}
