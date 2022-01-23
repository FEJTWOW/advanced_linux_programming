#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <limits.h>


struct directionAndSizeOfGrowth
{
	int direction;
	long size;
};

struct inputStruct
{
	char pathToModifyFile[100];
	char expansion[2][100];
	struct directionAndSizeOfGrowth growth[2];
	int twoSideGrowth;
};


void readInput(int argc, char** argv, struct inputStruct* input);
void parseInput(struct inputStruct* input);
void makeGrowth(struct inputStruct* input);
void growAnte(int inputFd, int sizeOfFile, long size);
void growPost(int inputFd, int sizeOfFile, long sizeofGrowth);
void growAmbo(int inputFd, int sizeOfFile, long sizeofGrowth);


int main(int argc, char* argv[])
{
	struct inputStruct inputData = {0};
	readInput(argc, argv, &inputData);
	//printf("%s\n%s\n%s\n",inputData.pathToModifyFile, inputData.expansion[0], inputData.expansion[1]);
	parseInput(&inputData);
	//printf("%d\n%d\n%ld\n%ld\n", inputData.growth[0].direction, inputData.growth[1].direction, inputData.growth[0].size, inputData.growth[1].size);
	makeGrowth(&inputData);

	return 0;
}



void readInput(int argc, char** argv, struct inputStruct* input)
{
	int opt = 0;
	if ((opt = getopt(argc, argv, "s:")) != -1)
	{
		if (opt == 's') { memcpy(input->pathToModifyFile, optarg, strlen(optarg) + 1); }
		else {
			perror("No file provided!\n");
			exit(EXIT_FAILURE);
		}
	}
	int j=0;
	for (int i=optind; i<argc; ++i)
	{
		if( j==2 )
		{
			perror("Too many arguments!\n");
			exit(EXIT_FAILURE);
		}
		memcpy(input->expansion[j],argv[i],strlen(argv[i])+1);
		++j;
	}
	if (j==2) { input->twoSideGrowth = 2; }
	else { input->twoSideGrowth = 1; }
}


void parseInput(struct inputStruct* input)
{
	char firstBuff[2][2][50];
	const char s[2] = ":";
	char *token;
	int j=0;
	for (int i=0; i<input->twoSideGrowth; ++i)
	{
		token = strtok(input->expansion[i], s);
		while( token != NULL )
		{
			memcpy(firstBuff[i][j], token, strlen(token) + 1);
			token = strtok(NULL, s);
			++j;
		}
		j=0;
	}
	//printf("%s\n%s\n%s\n%s\n",firstBuff[0][0], firstBuff[0][1], firstBuff[1][0], firstBuff[1][1] );
	long val = 0;
	char* endPointer;
	int ambo = 0;
	int post = 0;
	int ante = 0;
	for (int i=0; i<input->twoSideGrowth; ++i)
	{
		if (strcmp(firstBuff[i][0],"ante") == 0)
		{
			if (ambo == 1 || ante == 1)
			{
				perror("U cant combine ante with ambo or another ante!\n");
				exit(EXIT_FAILURE);
			}
			input->growth[i].direction = 1;
			ante = 1;
		}
		else if (strcmp(firstBuff[i][0],"post") == 0)
		{
			if (ambo == 1 || post == 1)
			{
				perror("You cant combine post with ambo or another post!\n");
				exit(EXIT_FAILURE);
			}
			input->growth[i].direction = -1;
			post = 1;
		}
		else if (strcmp(firstBuff[i][0],"ambo") == 0)
		{
			if (ambo == 1)
			{
				perror("You can't combine ambo with ambo!\n");
				exit(EXIT_FAILURE);
			}
			ambo = 1;
		}//
		else
		{
			perror("Invalid direction!\n");
			exit(EXIT_FAILURE);
		}
		val = strtol(firstBuff[i][1], &endPointer, 10);
		if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
		    || (errno != 0 && val == 0))
		{
			perror("strtol");
			exit(EXIT_FAILURE);
		}
		if (endPointer == firstBuff[i][1])
		{
			fprintf(stderr, "No digits were found\n");
			exit(EXIT_FAILURE);
		}
		if (strcmp(endPointer,"") == 0 || strcmp(endPointer,"B") == 0)
		{
			input->growth[i].size = val;
		} else if (strcmp(endPointer, "bb") == 0)
		{
			input->growth[i].size = val*512;
		} else if (strcmp(endPointer, "K") == 0)
		{
			input->growth[i].size = val * 1024;
		}
	}
}

void makeGrowth(struct inputStruct* input)
{
	int inputFd = open(input->pathToModifyFile, O_RDWR);
	if (inputFd == -1)
	{
		perror("Error in open!\n");
		exit(EXIT_FAILURE);
	}

	struct stat st;
	fstat(inputFd,&st);
	int size = st.st_size;

	if (input->twoSideGrowth == 2) // mamy rozrost na dwie strony z różnymi wielkościami
	{
		for (int j=0; j<2; ++j)
		{
			switch (input->growth[j].direction)
			{
				case 1:
				{
					growAnte(inputFd, size, input->growth[j].size);
					break;
				}
				case -1:
				{
					growPost(inputFd, size, input->growth[j].size);
					break;
				}
			}
		}
	}
	else
	{
		switch (input->growth[0].direction)
		{
			case 1:
			{
				growAnte(inputFd, size, input->growth[0].size);
				break;
			}
			case -1:
			{
				growPost(inputFd, size, input->growth[0].size);
				break;
			}
			case 0:
			{
				growAmbo(inputFd, size, input->growth[0].size);
				break;
			}
		}
	}
	close(inputFd);
}




//void searchDataInWholeFile(int inputFd, int sizeOfFile, struct data** tabWithData, int* numberOfData)
//{
//	off_t offsetData;
//	char buff[4096] = {0};
//	for (long i = 0; i < sizeOfFile; )
//	{
//		offsetData = lseek(inputFd, i, SEEK_DATA);
//		printf("Offsetdata po lseeku: %ld\n", offsetData);
//		if (offsetData == -1) {
//			printf("Adres tablicy struktur %p\n", tabWithData);
//			return;
//		}
//		if (read(inputFd, buff, 4096) != 4096)
//		{
//			perror("Error in write!\n");
//			exit(EXIT_FAILURE);
//		}
//
//		for (int j = 0; j < 4096; ++j)
//		{
//			if (buff[j] != 0)
//			{
//				if (buff[j] >= 32 && buff[j] < 127)
//				{
//					printf("Znak przed dodaniem: %c\n", buff[j]);
//					(*tabWithData)[(*numberOfData)].c = buff[j];  // przypisujemy w naszej tablicy struktur znak
//					printf("Znak po dodaniu: %c\n", (*tabWithData)[(*numberOfData)].c);
//					printf("Offset przed dodaniem: %ld\n", offsetData+j);
//					(*tabWithData)[(*numberOfData)].offset = offsetData + j;
//					printf("Offset po dodaniu %ld\n", (*tabWithData)[(*numberOfData)].offset);
//					(*numberOfData)++;
//					if ( (*numberOfData) % 10 == 0) // osiagamy limit tablicy i trzeba reallocować
//					{
//						printf("Robie realloca\n");
//						*tabWithData = (struct data *) realloc(*tabWithData, *numberOfData + 10);
//					}
//
//				} else
//				{
//					perror("Invalid character!\n");
//					exit(EXIT_FAILURE);
//				}
//			}
//		}
//		i = offsetData+4097;
//		memset(&buff,0,4096);
//	}
//
//}




void growAnte(int inputFd, int sizeOfFile, long sizeofGrowth)
{ // rosniemy najpierw w przod wiec zaczynam od szukania danych
	off_t offsetData = 0;
	off_t offsetHole = 0;
	offsetData = lseek(inputFd, 0, SEEK_DATA);
	long grow = 0;
	char c = 0;
	char buff[4096] = {0};
	//for (long i=0; i<sizeOfFile; )
	while(1)
	{
		offsetHole = lseek(inputFd, offsetData, SEEK_HOLE);
		if (offsetHole == -1) { return; }
		offsetData = lseek(inputFd, offsetHole, SEEK_DATA);
		if (offsetData == -1)
		{
			if (sizeOfFile - offsetHole < sizeofGrowth) { grow = sizeOfFile - offsetHole; }
			else { grow = sizeofGrowth; }
			lseek(inputFd, offsetHole-4097, SEEK_SET);
			read(inputFd, buff, 4096);
			for (int j = 0; j < 4096; ++j)
			{
				if (buff[j] != 0)
				{
					c = buff[j];
					break;
				}
			}
			char* buffToWrite = (char*)malloc(grow*sizeof(char));
			memset(buffToWrite,c,sizeof(char)*grow);
			pwrite(inputFd,buffToWrite,sizeofGrowth,offsetHole);
			free(buffToWrite);
			return;
		}
		else if (offsetData - offsetHole < sizeofGrowth) { grow = offsetData - offsetHole; }
		else { grow = sizeofGrowth; }
		lseek(inputFd, offsetHole-4097, SEEK_SET); // offset H wskazuje na poczatek dziury czyli musimy sie cofnac o 4097 bajtów
		read(inputFd, buff, 4096);
		for (int j = 0; j < 4096; ++j)
		{
			if (buff[j] != 0)
			{
				c = buff[j];
				break;
			}
		}
		char* buffToWrite = (char*)malloc(grow*sizeof(char));
		memset(buffToWrite,c,sizeof(char)*grow);
		pwrite(inputFd,buffToWrite,sizeofGrowth,offsetHole);
		//i += offsetData;
		free(buffToWrite);
	}
}

void growPost(int inputFd, int sizeOfFile, long sizeofGrowth)
{
	off_t offsetData = 0;
	off_t offsetHole = 0;
	long grow = 0;
	char c = 0;
	char buff[4096] = {0};
	//for (long i=0; i<sizeOfFile; )
	while(1)
	{
		offsetHole = lseek(inputFd,offsetData, SEEK_HOLE);
		if (offsetHole == -1) { return; }
		offsetData = lseek(inputFd,offsetHole, SEEK_DATA);
		if (offsetData == -1) { return; }
		if (offsetData - offsetHole < sizeofGrowth) { grow = offsetData - offsetHole; }
		else { grow = sizeofGrowth; }
		lseek(inputFd,offsetData,SEEK_SET);
		read(inputFd, buff, 4096);
		for (int j=0; j<4096; ++j)
		{
			if (buff[j] != 0)
			{
				c = buff[j];
				break;
			}
		}
		char* buffToWrite = (char*)malloc(grow*sizeof(char));
		memset(buffToWrite,c,sizeof(char)*grow);
		pwrite(inputFd,buffToWrite,sizeofGrowth,offsetData-grow-1);
		//i+= offsetData;
		free(buffToWrite);
	}
}

void growAmbo(int inputFd, int sizeOfFile, long sizeofGrowth)
{
	off_t offsetData = 0;
	off_t offsetHole = 0;
	long grow = 0;
	char post = 0;
	char ante = 0;
	char buff[4096] = {0};
	/* poczatek pliku osobno */
	offsetData = lseek(inputFd,0, SEEK_DATA);
	if (offsetData == -1) { return; }
	if (offsetData - offsetHole < sizeofGrowth) { grow = offsetData - offsetHole; }
	else { grow = sizeofGrowth; }
	lseek(inputFd,offsetData,SEEK_SET);
	read(inputFd, buff, 4096);
	for (int j=0; j<4096; ++j)
	{
		if (buff[j] != 0)
		{
			post = buff[j];
			break;
		}
	}
	char* buffToWrite = (char*)malloc(grow*sizeof(char));
	memset(buffToWrite,post,sizeof(char)*grow);
	pwrite(inputFd,buffToWrite,grow,offsetData-grow-1);
	free(buffToWrite);
	//for (long i=0; i<sizeOfFile;)
	while(1)
	{
		offsetHole = lseek(inputFd, offsetData, SEEK_HOLE);
		offsetData = lseek(inputFd, offsetHole, SEEK_DATA);
		if (offsetData == -1)
		{
			if (sizeOfFile - offsetHole < sizeofGrowth) { grow = sizeOfFile - offsetHole; }
			else { grow = sizeofGrowth; }
			lseek(inputFd, offsetHole-4097, SEEK_SET);
			read(inputFd,buff, 4096);
			for (int j=0; j<4096; ++j)
			{
				if (buff[j] != 0)
				{
					ante = buff[j];
					break;
				}
			}
			buffToWrite = (char*)malloc(grow*sizeof(char));
			memset(buffToWrite,ante,grow*sizeof(char));
			pwrite(inputFd,buffToWrite,grow,offsetHole);
			free(buffToWrite);
			return;
		}
		if (offsetData - offsetHole < 2*sizeofGrowth) { grow = (offsetData-offsetHole)/2; }
		else { grow = sizeofGrowth; }
		lseek(inputFd, offsetHole-4097, SEEK_SET);
		read(inputFd,buff, 4096);
		for (int j=0; j<4096; ++j)
		{
			if (buff[j] != 0)
			{
				ante = buff[j];
				break;
			}
		}
		memset(buff,0,4096*sizeof(char));
		lseek(inputFd,offsetData, SEEK_SET);
		read(inputFd, buff, 4096);
		for (int j=0; j<4096; ++j)
		{
			if (buff[j] != 0)
			{
				post = buff[j];
				break;
			}
		}
		buffToWrite = (char*)malloc(grow*sizeof(char));
		memset(buffToWrite,ante,grow*sizeof(char));
		pwrite(inputFd,buffToWrite,grow,offsetHole);
		memset(buffToWrite, post, sizeof(char));
		pwrite(inputFd, buffToWrite, grow, offsetData-grow-1);
		free(buffToWrite);
		//i+=offsetData;
	}
}