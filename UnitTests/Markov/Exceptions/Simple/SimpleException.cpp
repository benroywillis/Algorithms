#include <vector>
#include <iostream>

#define SIZE 	10

using namespace std;

int main()
{
	vector<int> vec;
	for( int i = 0; i < SIZE; i++ )
	{
		try
		{
			auto first = vec.at(i);
		}
		catch( exception& e )
		{
			cout << "Exception: " << e.what() << endl;
		}
	}
	return 0;
}
