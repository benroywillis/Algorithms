#include <thread>
#include <iostream>

#define THREAD_COUNT 	10

void threadFunc0()
{
	std::cout << "This is thread 0!" << std::endl;
}

void threadFunc1()
{
	std::cout << "This is thread 1!" << std::endl;
}

int main()
{
	std::thread* threads[THREAD_COUNT];
	for( unsigned i = 0; i < THREAD_COUNT; i++ )
	{
		if( i % 2 )
		{
			threads[i] = new std::thread(threadFunc0);
		}
		else
		{
			threads[i] = new std::thread(threadFunc1);
		}
	}
	for( unsigned i = 0; i < THREAD_COUNT; i++ )
	{
		threads[i]->join();
	}
	for( unsigned i = 0; i < THREAD_COUNT; i++ )
	{
		delete threads[i];
	}
	return 0;
}
