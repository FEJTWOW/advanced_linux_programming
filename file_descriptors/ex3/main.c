#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <string.h>


int main(int argc, char* argv[])
{
	struct stat f;
	int inputFd = 0;
	for (int i=1; i<argc; ++i)
	{
		if (stat(argv[1], &f) == -1)
		{
			perror("stat");
			exit(EXIT_FAILURE);
		}
		if ((f.st_mode & S_IFMT) != S_IFREG)
		{
			printf("Plik nie jest regularny\n");
			exit(EXIT_FAILURE);
		}
		inputFd = open(argv[i],O_WRONLY | O_APPEND );
		if (inputFd==-1)
		{
			perror("Incorrect argument in open functo ion!\n");
			exit(EXIT_FAILURE);
		}
		struct timespec spec;
		clock_gettime(CLOCK_REALTIME, &spec);
		char buff[200];
		sprintf(buff, "%ld\n\n", spec.tv_sec);
		if (write(inputFd,buff,strlen(buff)) != strlen(buff))
		{
			perror("Error in write!\n");
			exit(EXIT_FAILURE);
		}
		close(inputFd);
	}
	return 0;
}
