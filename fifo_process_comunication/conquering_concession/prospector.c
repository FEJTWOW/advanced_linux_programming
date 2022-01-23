#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <limits.h>


struct inputStruct
{
	char nameOfResource[40];
	char pathToFifoFile[80];
	char nameOfBombelek[80];
};

struct inputStruct inputData = {0};
int maxNumberOfFiles = 0;
int counterForOnExit = 0;
char globalNameTab[60][100];

void readInput(int argc, char** argv);
void doJob();
void renameFile(int, void*);



int main(int argc, char* argv[])
{

	readInput(argc, argv);
	doJob();
	return 0;
}

void doJob()
{
	char nameOfFile[100];
	read(4,"/dev/null",1);
	//printf("%s\n%s\n", inputData.nameOfResource, inputData.pathToFifoFile);
	int fifoFd = 0;
	int currentNumberOfFiles = 0;
	while (maxNumberOfFiles < 60)
	{
		fifoFd = open(inputData.pathToFifoFile, O_WRONLY | O_NONBLOCK);
		if(fifoFd == -1)
		{
			if (errno == ENXIO) { break; }
			perror("Error in opening fifo!\n");
			exit(EXIT_FAILURE);
		}
		int fd;
		while(maxNumberOfFiles < 60)
		{
			sprintf(nameOfFile, "property_%02d.%smine", maxNumberOfFiles, inputData.nameOfResource);
			errno = 0;
			fd = open(nameOfFile, O_CREAT | O_WRONLY | O_EXCL, S_IRUSR | S_IWUSR);
			if (fd == -1)
			{
				if (errno == EEXIST) // taki plik juz istnieje i nasz proces musi probowac zrobic kolejnego
				{
					++maxNumberOfFiles; // preinkrementacja jest lepsza od postinkrementacji bo nie robi kopii (funfact)
					continue;
				}
			}
			memcpy(globalNameTab[maxNumberOfFiles],nameOfFile, strlen(nameOfFile)+1);
			//printf("%s\n",globalNameTab[maxNumberOfFiles]);
			++currentNumberOfFiles;
			if (ftruncate(fd,currentNumberOfFiles*384) == -1)
			{
				perror("Error in truncate!\n");
				exit(EXIT_FAILURE);
			}
			int result = 0;
			result = on_exit(renameFile, (void*)globalNameTab[maxNumberOfFiles]);
			if (result == -1)
			{
				perror("Error in on_exit\n");
				exit(EXIT_FAILURE);
			}
			break; // sprawdzamy czy dalej możemy tworzyć pliki
		}
		++counterForOnExit;
	}
}

void renameFile(int a, void* arg)
{
	char* oldName = (char*)arg;
	char newName[140] = {0};
	sprintf(newName, "%s_%smine.#%d", inputData.nameOfBombelek, inputData.nameOfResource, --counterForOnExit);
	if (rename(oldName,newName) == -1)
	{
		perror("Error in rename");
		exit(EXIT_FAILURE);
	}
}


void readInput(int argc, char** argv)
{
	memcpy(inputData.nameOfBombelek,argv[0],strlen(argv[0])+1);
	//printf("%s\n",inputData.nameOfBombelek);

	int opt;
	while ((opt = getopt(argc, argv, "z:s:")) != -1)
	{
		switch (opt)
		{
			case 'z':
			{
				memcpy(inputData.nameOfResource, optarg, strlen(optarg) + 1);
				break;
			}
			case 's':
			{
				memcpy(inputData.pathToFifoFile, optarg, strlen(optarg) +1);
				break;
			}
			default:
			{
				perror("Invalid arguments!\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}
