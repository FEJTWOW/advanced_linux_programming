#include <stdio.h>

#define MULT(a) a*a

int main()
{
	int a = 1;
	int b = a+1;
	printf("%d\n",MULT(a+b));
	return 0;
}
