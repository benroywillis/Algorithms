#define SIZE 100

int main()
{
	int a = 0;
 	goto firstLabel;
	firstLabel: 
	if( a < SIZE ) { 
		a++;
		goto firstLabel;
	}
	return 0;
}
