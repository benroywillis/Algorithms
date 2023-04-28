#include <stdlib.h>
#include <stdio.h>

// in general, the way to break epoch boundaries is to build large loops with many high-frequency shared functions and low-frequency loops in them.
// this will create a kernel "hierarchy" that was segmented out of order, creating entrances and exits from all over hell
// it may also create a node that has many small loops stemming out of it
// both of these things create recipes for blurry epoch boundaries

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
