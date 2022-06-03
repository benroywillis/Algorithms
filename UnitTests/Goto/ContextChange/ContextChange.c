#define SIZE 	100

int counter()
{
	static int a = 0;
	goto ret;
funcJump:
	a++;
	goto jump;
ret:
	return a;
}

int main()
{
	goto jump;
jump:
	if( counter() < SIZE )
	{
		goto funcJump;
	}
	return 0;
}
