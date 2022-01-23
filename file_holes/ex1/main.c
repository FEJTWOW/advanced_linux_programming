#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

#define BUF_SIZE 1024
#define SIZE_OF_OUTPUT_FILE 1024*1024*1024 // 1GB

void createOutputFile(char* pathToFile);
void checkValue(char* buf, int randomFd);

int main (int argc, char* argv[])
{
	if (argc!=2)
	{
		perror("Program takes only one argument which is the name of output file!\n");
		exit(EXIT_FAILURE);
	}
	createOutputFile(argv[1]);
	return 0;
}

void createOutputFile(char* pathToFile)
{
	char outputBuf[1024] = {0};

	int randomFd = open("/dev/urandom", O_RDONLY);
	if (randomFd == -1)
	{
		perror("Error in opening randomFd!\n");
		exit(EXIT_FAILURE);
	}
	int outputFd = open(pathToFile,O_RDWR | O_CREAT | O_TRUNC);
	if (outputFd == -1)
	{
		perror("Error in opening outputFd!\n");
		exit(EXIT_FAILURE);
	}
	for (int i=0; i<(SIZE_OF_OUTPUT_FILE/BUF_SIZE); ++i)
	{
		if ( read(randomFd,outputBuf,BUF_SIZE) != BUF_SIZE )
		{
			perror("Error in read!\n");
			exit(EXIT_FAILURE);

		}
		checkValue(outputBuf,randomFd);
		if (write(outputFd, outputBuf,BUF_SIZE) != BUF_SIZE)
		{
			perror("Error in write!\n");
			exit(EXIT_FAILURE);
		}
	}

	if (close(outputFd) == -1)
	{
		perror("Error in close outputFd!\n");
		exit(EXIT_FAILURE);
	}
	if (close(randomFd) == -1)
	{
		perror("Error in close randomFd!\n");
		exit(EXIT_FAILURE);
	}
}

void checkValue(char* buf, int randomFd)
{

	double* test = (double*)buf;

	for (int j=0; j<(BUF_SIZE/8); ++j)
	{
		printf("%.20f\n", *test);
		while (fpclassify(*test) != FP_NORMAL)
		{
			if ( read(randomFd,test,8) != 8 )
			{
				perror("Error in read!\n");
				exit(EXIT_FAILURE);
			}

		}
		++test;
	}
}
