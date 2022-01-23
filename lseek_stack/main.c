#include <stdio.h>

int main()
{
	int b = 4;
	int* a = &b;
	long* c = (long*)a;
	*c=2;
	printf("%d",*a);

	return 0;
}
