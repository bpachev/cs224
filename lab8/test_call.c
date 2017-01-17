#include <stdlib.h>

int test_call(int arg1,int arg2);

void main(void)
{
	int foo;
	foo = test_call(1,2)
}

int test_call(int arg1,int arg2)
{
	printf("%d",arg2);
	return arg1;
)