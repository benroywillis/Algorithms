#include <stdlib.h>
#include <stdio.h>

void print1(int arg)
{
	while( arg )
	{
		printf("This is function 1!\n");
		arg--;
	}
}

void print2(int arg)
{
	printf("This is function 2!\n");
}

void caller(int arg, void (*p)())
{
	p(arg);
	while(arg)
	{
		printf("This is the caller!\n");
		arg--;
	}
	p(arg);
	while(arg)
	{
		printf("This is the caller!\n");
		arg--;
	}
}

int main()
{
	caller(0, (*print1));
	caller(1, (*print2));
	caller(1, (*print1));
	caller(5, (*print1));
	caller(5, (*print2));
	caller(2, (*print1));
	caller(1, (*print2));
	caller(0, (*print1));
	return 0;
}
