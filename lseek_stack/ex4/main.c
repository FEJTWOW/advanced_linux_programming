#include <stdio.h>

int* foo() {
	int a=5;
	return &a;
}

int main()
{
	//int* a = foo();
	//printf("%d\n",*a);
	char* text = "text";
	text[0] = 'x';
	printf("%s\n",text);
	return 0;
}
