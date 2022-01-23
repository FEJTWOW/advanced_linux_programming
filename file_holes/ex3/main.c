#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <time.h>

struct inputStruct
{
	int mode;
	int indexOfPositionArgument;
	char pathToModifyFile[100];
};


void readInput(int, char**, struct inputStruct* input);
void parseNumberInput(int totalNumberOfArguments, int indexOfPositionArgument, char** argv, int inputFd);
void parseTextInput(int totalNumberOfArguments, int indexOfPositionArgument, char** argv, int inputFd);


int main(int argc, char* argv[])
{
	srand(time(NULL));
	struct inputStruct input = { 0 };
	readInput(argc, argv, &input);
	printf("%d\n%d\n%s\n", input.mode, input.indexOfPositionArgument, input.pathToModifyFile);
	int inputFd = open(input.pathToModifyFile, O_RDWR);
	if (inputFd == -1)
	{
		perror("Error in open!\n");
		exit(EXIT_FAILURE);
	}
	if (input.mode == 0) { parseNumberInput(argc,input.indexOfPositionArgument,argv,inputFd); }
	else { parseTextInput(argc, input.indexOfPositionArgument, argv, inputFd); }
	//printf("%d\n%d\n%s\n", input.mode, input.indexOfPositionArgument, input.pathToModifyFile); for debug

	return 0;
}



void readInput(int argc, char** argv, struct inputStruct* inputData)
{
	int opt = 0;
	while ((opt = getopt(argc, argv, "f:s:")) != -1)
	{
		switch (opt)
		{
			case 's':
				memcpy(inputData->pathToModifyFile, optarg, strlen(optarg) + 1);
				break;
			case 'f':
				if (strcmp(optarg,"liczba") == 0) { inputData->mode = 0; } // 0 is a mode for numbers
				else if (strcmp(optarg,"tekst") == 0) { inputData->mode = 1; } // 1 is a mode for a text
				else
				{
					perror("Wrong arguments after f!\n");
					exit(EXIT_FAILURE);
				}
				break;
			default:
				perror("Invalid input data!\n");
				exit(EXIT_FAILURE);
		}
	}
	inputData->indexOfPositionArgument = optind;
}


void parseNumberInput(int totalNumberOfArguments, int indexOfPositionArgument, char** argv, int inputFd)
{
	struct stat st;
	fstat(inputFd,&st);
	int size = st.st_size;
	char* table = calloc(size,sizeof(char));
	if (totalNumberOfArguments-indexOfPositionArgument > size)
	{
		perror("Not enough space in a file!\n");
		exit(EXIT_FAILURE);
	}
	char val = 0;
	char* endPointer;
	int randomNumber;

	for (int i=indexOfPositionArgument; i<totalNumberOfArguments; ++i)
	{
		errno = 0;
		val = (char)strtol(argv[i], &endPointer, 0);
		printf("%d\n",val);
		if ((errno == ERANGE && (val == CHAR_MAX || val == CHAR_MIN))
		    || (errno != 0 && val == 0))
		{
			perror("strtol");
			exit(EXIT_FAILURE);
		}
		if (endPointer == argv[i])
		{
			fprintf(stderr, "No digits were found\n");
			exit(EXIT_FAILURE);
		}
		randomNumber = rand() % size;
		while (table[randomNumber] != 0)
			randomNumber = rand() % size;
		if (pwrite(inputFd,&val,1,randomNumber) != 1)
		{
			perror("Error in pwrite!\n");
			exit(EXIT_FAILURE);
		}
	}
	free(table);

}

void parseTextInput(int totalNumberOfArguments, int indexOfPositionArgument, char** argv, int inputFd)
{
	struct stat st;
	fstat(inputFd,&st);
	int size = st.st_size;
	char* table = calloc(size,sizeof(char));
	unsigned int currentSize = 0;

	int stopRandom = 1;
	int randomNumber = 0;
	unsigned int lengthOfText = 0;
	srand(time(NULL));
	for (int i=indexOfPositionArgument; i<totalNumberOfArguments; ++i)
	{
		lengthOfText = strlen(argv[i]);
		currentSize+=lengthOfText;
		if (currentSize > size)
		{
			perror("Need more space!\n");
			exit(EXIT_FAILURE);
		}
		int k = 0;
		for (int j=0; j<lengthOfText; ++j)
		{
			do
			{

				randomNumber = rand()%size;
				if (table[randomNumber] == 0)
				{
					stopRandom = 1;
					table[randomNumber] = 1;
				}

			} while(stopRandom == 0);

			stopRandom = 0;

			if (pwrite(inputFd,&argv[i][j],1,randomNumber) != 1)
			{
				perror("Error in pwrite!\n");
				exit(EXIT_FAILURE);
			}

		}

		/*do
		{
			randomNumber = rand()%size;
			lengthOfText = strlen(argv[i]);
			if (randomNumber+lengthOfText < size)
			{
				for (int j = randomNumber; j < randomNumber + lengthOfText; ++j)
				{
					if (table[j] != 0)
					{
						stopRandom = 0;
						break;
					}
				}
			}
		} while (stopRandom==0);

		if (pwrite(inputFd,argv[i],lengthOfText,randomNumber) != lengthOfText)
		{
			perror("Error in pwrite!\n");
			exit(EXIT_FAILURE);
		}*/
	}
	free(table);
}

