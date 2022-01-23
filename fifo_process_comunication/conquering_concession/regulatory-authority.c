#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <dirent.h>
#include<sys/wait.h>

typedef struct inputStruct
{
	double timeToTakeResources;
	long numberOfChildren;
	char nameOfResource[40];
	char beginningOfNameForChildren[40];
	char pathToFifo[40];
} inputStruct;





void readInputAndParse(int, char**, inputStruct*);
void createChildren(inputStruct*);

int main(int argc, char* argv[])
{
	inputStruct inputData = {0};
	readInputAndParse(argc, argv, &inputData);
//	printf("%f\n%ld\n%s\n%s\n%s\n", inputData.timeToTakeResources, inputData.numberOfChildren,
//		inputData.nameOfResource, inputData.beginningOfNameForChildren, inputData.pathToFifo);
	createChildren(&inputData);
	return 0;
}

void readInputAndParse(int argc, char** argv, inputStruct* inputData)
{
	int opt = 0;
	char* endPointer = NULL;
	inputData->numberOfChildren = 16; // jak bedzie -n podane to nadpisze ta wartość
	while ((opt = getopt(argc, argv, "t:n:z:p:")) != -1)
	{
		switch(opt)
		{
			case 't':
				errno = 0;
				inputData->timeToTakeResources = strtod(optarg,&endPointer);
				if ((errno == ERANGE && (inputData->timeToTakeResources == (double)LLONG_MAX ||
				inputData->timeToTakeResources == (double)LLONG_MIN))
				|| (errno != 0 && inputData->timeToTakeResources== 0))
				{
					perror("strtol");
					exit(EXIT_FAILURE);
				}
				if (endPointer == optarg)
				{
					fprintf(stderr, "No digits were found\n");
					exit(EXIT_FAILURE);
				}
				break;
			case 'n':
				errno = 0;
				inputData->numberOfChildren = strtol(optarg,&endPointer,10);
				if ((errno == ERANGE && (inputData->numberOfChildren == LONG_MAX ||
				inputData->numberOfChildren == LONG_MIN))
				|| (errno != 0 && inputData->numberOfChildren == 0))
				{
					perror("strtol");
					exit(EXIT_FAILURE);
				}
				if (endPointer == optarg)
				{
					fprintf(stderr, "No digits were found\n");
					exit(EXIT_FAILURE);
				}
				break;
			case 'z':
				memcpy(inputData->nameOfResource, optarg, strlen(optarg) + 1);
				break;
			case 'p':
				memcpy(inputData->beginningOfNameForChildren, optarg, strlen(optarg) + 1);
				break;
			default:
				perror("Invalid input data!\n");
				exit(EXIT_FAILURE);
		}
	}
	if (argc == optind+1) { memcpy(inputData->pathToFifo, argv[optind], strlen(argv[optind])+1); }
	else {
		srand(time(NULL)+getpid());
		sprintf(inputData->pathToFifo,"%s%d","/tmp/myfifo",rand()%getpid());
		//printf("%s",inputData->pathToFifo);
	}

}

void createChildren(inputStruct* inputData)
{
	int fd[2];
	if (pipe(fd) == -1)
	{
		perror("Error in pipe!\n");
		exit(EXIT_FAILURE);
	}
	pid_t p;
	//printf("%s\n",inputData->pathToFifo);
	if (mkfifo(inputData->pathToFifo, 0666) == -1)
	{
		perror("Error in mkfifo!\n");
		exit(EXIT_FAILURE);
	}
	int fifoFd = open(inputData->pathToFifo, O_RDONLY | O_NONBLOCK);
	if (fifoFd == -1)
	{
		perror("Error in open!\n");
		exit(EXIT_FAILURE);
	}
	for (int i=0; i<inputData->numberOfChildren; ++i)
	{
		p = fork();
		switch (p)
		{
			case -1: { perror("Error in fork\n"); exit(EXIT_FAILURE); }
			case 0:
			{
				if (close(fd[1]) == -1) // zamykam write'a do pipe'a
				{
					perror("Error in closing write to pipe on child side!\n");
					exit(EXIT_FAILURE);

				};
				if (dup2(fd[0],4) == -1) // duplikujemy deskryptor do czytania na 4
				{
					perror("Error in dup2!\n");
					exit(EXIT_FAILURE);
				}
				if (close(fd[0]) == -1) // zamykamy niepotrzebny deskryptor do czytania
				{
					perror("Error in closing read descriptor to pipe!\n");
					exit(EXIT_FAILURE);
				}
				if (close(fifoFd) == -1) // zamykamy deskryptor do czytania po stronie potomka
				{
					perror("Error in closing read fifo descriptor by child side!\n");
					exit(EXIT_FAILURE);
				}
				char name[50] = {0};
				char firstArgument[50] = {0};
				char secondArgument[50] = {0};
				sprintf(name, "%s_%d", inputData->beginningOfNameForChildren, i);
				sprintf(firstArgument,"-z%s", inputData->nameOfResource);
				sprintf(secondArgument, "-s%s", inputData->pathToFifo);
				execl("poszukiwacz", name, firstArgument, secondArgument, NULL);
				_exit(EXIT_SUCCESS);
			}
			default: { break; }
		}
	}
	// bombelki zaczynają pracke
	if (close(fd[0]) == -1)
	{
		perror("Error in closing pipe read!\n");
		exit(EXIT_FAILURE);
	}
	if (close(fd[1]) == -1)
	{
		perror("Error in closing pipe wrtie!\n");
		exit(EXIT_FAILURE);
	}
	// czekamy
	struct timespec ts;
	struct timespec endTime;
	ts.tv_sec = (time_t)(inputData->timeToTakeResources / 1000);
	ts.tv_nsec = (inputData->timeToTakeResources - (double)(ts.tv_sec * 1000))*1000000;
	//printf("Sec:%ld\nNanoSec:%ld\n",ts.tv_sec, ts.tv_nsec);
	nanosleep(&ts,NULL);
	// bombelki kończą pracę
	if (close(fifoFd) == -1)
	{
		perror("Error in closing fifo descriptor\n");
		exit(EXIT_FAILURE);
	}
	int status = -1;
	for (int i=0; i<inputData->numberOfChildren; ++i)
	{
		waitpid(-1,&status,0);
		if( !WIFEXITED(status) )
		{
			perror("Error in wait!@!");
		}
	}
	if (clock_gettime(CLOCK_REALTIME, &endTime) == -1) // zapisujemy sobie czas kiedy skończyła się praca
	{
		perror("Error in clock get time!\n");
		exit(EXIT_FAILURE);
	}
	//printf("END TIME Sec:%ld\nNanoSec:%ld\n",endTime.tv_sec, endTime.tv_nsec);
	if (unlink(inputData->pathToFifo) == -1)
	{
		perror("Error in unlink!\n");
		exit(EXIT_FAILURE);
	}

	//sprzatanie
	system("rm property_*"); // niby mógłbym w tej pętli poniżej robić strcmp ale jestem leniwy
	struct dirent* de;

	DIR* dr = opendir(".");
	if (dr == NULL)
	{
		perror("Error in opendir!\n");
		exit(EXIT_FAILURE);
	}

	while ((de = readdir(dr)) != NULL)
	{
		struct stat fileInfo;
		if (stat(de->d_name, &fileInfo) == -1)
		{
			perror("Error in stat!\n");
			exit(EXIT_FAILURE);
		}
		//printf("Sec: %ld\nNSec: %ld\n", fileInfo.st_mtim.tv_sec, fileInfo.st_mtim.tv_nsec);
		if (fileInfo.st_mtim.tv_sec > endTime.tv_sec)
		{
			if (remove(de->d_name) == -1)
			{
				perror("Error in remove!\n");
				exit(EXIT_FAILURE);
			}
		}
		if ((fileInfo.st_mtim.tv_sec == endTime.tv_sec) && (fileInfo.st_mtim.tv_nsec > endTime.tv_nsec))
		{
			 if (remove(de->d_name) == -1)
			 {
			 	perror("Error in remove!\n");
			 	exit(EXIT_FAILURE);
			 }
		}
	}

	if (closedir(dr) == -1)
	{
		perror("Error in closedir!\n");
		exit(EXIT_FAILURE);
	}


}

