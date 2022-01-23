#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>


int main()
{
	lseek(stdin,0,SEEK_DATA);
	return 0;
}
