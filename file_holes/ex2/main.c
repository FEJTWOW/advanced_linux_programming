#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <limits.h>



void readInputAndCreateFile(int argc, char* argv[]);
//void createFile(int indexOfPathToFile, char* argv[]);
// TODO: add extra function to readInput


int main(int argc, char* argv[])
{
	readInputAndCreateFile(argc, argv);
	return 0;
}



void readInputAndCreateFile(int argc, char* argv[])
{
	int opt = 0;
	long kbSize = 0;
	char *endPointer;
	while ((opt = getopt(argc, argv, "r:")) != -1)
	{
		if (opt == 'r')
		{
			errno = 0;
			kbSize = strtol(optarg, &endPointer, 10);
			if ((errno == ERANGE && (kbSize == LONG_MAX || kbSize == LONG_MIN))
			    || (errno != 0 && kbSize == 0))
			{
				perror("strtol");
				exit(EXIT_FAILURE);
			}
			if (endPointer == optarg)
			{
				fprintf(stderr, "No digits were found\n");
				exit(EXIT_FAILURE);
			}
		}
	}


	int inputFd = 0;
	inputFd = open(argv[optind], O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR );
	if (inputFd == -1)
	{
		perror("Error in open!\n");
		exit(EXIT_FAILURE);
	}
	if (ftruncate(inputFd, kbSize * 1024) == -1)
	{
		perror("Error in ftruncate!\n");
		exit(EXIT_FAILURE);
	}

}

