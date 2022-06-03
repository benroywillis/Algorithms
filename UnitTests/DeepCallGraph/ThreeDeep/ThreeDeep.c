#include <stdlib.h>

#define SIZE 	64

int highRand0()
{
	return rand();
}

int highRand1()
{
	return rand();
}

int highRand2()
{
	return rand();
}

int lowRand0()
{
	return rand();
}

int lowRand1()
{
	return rand();
}

int lowRand2()
{
	return rand();
}

int randoRenaissance0(int seed, int param)
{
	if( seed < 10 )
	{
		if( param == 0 )
		{
			return highRand0();
		}
		else if( param == 1)
		{
			return highRand1();
		}
		else
		{
			return highRand2();
		}
	}
	else
	{
		if( param == 0)
		{
			return lowRand0();
		}
		else if( param == 1)
		{
			return lowRand1();
		}
		else
		{
			return lowRand2();
		}
	}
}

int randoRenaissance1(int seed, int param)
{
	if( seed < 10 )
	{
		if( param == 0 )
		{
			return highRand0();
		}
		else if( param == 1)
		{
			return highRand1();
		}
		else
		{
			return highRand2();
		}
	}
	else
	{
		if( param == 0)
		{
			return lowRand0();
		}
		else if( param == 1)
		{
			return lowRand1();
		}
		else
		{
			return lowRand2();
		}
	}
}

void thing1( int arg[SIZE][SIZE], int param )
{
	for( int i = 0; i < SIZE; i++ )
	{	
		for( int j = 0; j < SIZE; j++ )
		{
			arg[i][j] = randoRenaissance0(rand(), rand() % 3);
			if( param == 0 )
			{
				return;
			}
			arg[i][j] = randoRenaissance1(rand(), rand() % 3);
		}
	}
}

void thing2( int arg[SIZE][SIZE] )
{
	for( int i = 0; i < SIZE; i++ )
	{	
		for( int j = 0; j < SIZE; j++ )
		{
			arg[i][j] = randoRenaissance0(rand(), rand() % 3);
		}
	}
}

void thing3( int arg[SIZE][SIZE] )
{
	for( int i = 0; i < SIZE; i++ )
	{	
		for( int j = 0; j < SIZE; j++ )
		{
			arg[i][j] = randoRenaissance1(rand(), rand() % 3);
		}
	}
}

int main()
{
	int a[SIZE][SIZE];
	int b[SIZE][SIZE];
	int c[SIZE][SIZE];
	thing1(a, 0);
	thing1(a, 1);
	thing2(b);
	thing1(a, 1);
	thing2(b);
	thing3(c);
	thing2(b);
	thing3(c);
	thing3(c);
	return 0;
}
