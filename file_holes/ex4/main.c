#define _GNU_SOURCE
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
	int hole;
	char* pathToFile[100];
	int indexOfPositionArgument;
};

void readInput(int argc, char** argv, struct inputStruct* input);
void exploreFile(struct inputStruct* input);
void printTab(char* buff, int size);

int main(int argc, char* argv[])
{
	struct inputStruct input = {0};
	readInput(argc, argv, &input);
	//printf("%d\n",input.hole);
	exploreFile(&input);
	return 0;
}



void readInput(int argc, char** argv, struct inputStruct* input)
{
	int opt = 0;
	if ((opt = getopt(argc, argv, "!")) != -1)
	{
		if(opt == '!') { input->hole = 1; }
		else {
				perror("Incorrect arguments!\n");
				exit(EXIT_FAILURE);
		}
	}

	input->indexOfPositionArgument = optind;
	memcpy(input->pathToFile, argv[optind], strlen(argv[optind]) + 1);
}


void exploreFile(struct inputStruct* input)
{
	int inputFd = open((const char*)input->pathToFile, O_RDWR);
	if (inputFd == -1)
	{
		perror("Error in open!\n");
		exit(EXIT_FAILURE);
	}
	off_t offsetData;
	off_t offsetHole;
	off_t currentOffset;

	struct stat st;
	fstat(inputFd,&st);
	int size = st.st_size;
	//printf("rozmiar pliku: %d\n", size);
	int i = 0;

	while ( i != size )
	{
		offsetData = lseek(inputFd,i,SEEK_DATA); //czytamy dane
		//printf("Dane: %ld\n", offsetData);
		if (offsetData == -1)
		{
			if (input->hole == 0)
			{
				printf("Hole with size %d\n", size-i);
				return;
			}
			else if(input->hole ==1)
			{
				printf("0 : %d\n",size -i);
				return;
			}
		}
		offsetHole = lseek(inputFd,offsetData,SEEK_HOLE); // czytamy dziure ale od momentu daty
		//printf("Dziura: %ld\n", offsetHole);
		if (offsetHole == -1)
		{
			perror("Error in seek hole!\n");
			return;
		}
		if (offsetData-i>0) // jezeli do momentu napotkania danych byla jakas dziura to ja wypisujemy
		{
			if (input->hole == 0) { printf("Hole with size %ld\n", offsetData-i); }
			else if (input->hole == 1) { printf("0 : %ld\n",offsetData -i); }
		}
		lseek(inputFd,offsetData,SEEK_SET);
		off_t bytesToRead = offsetHole - offsetData;
		char* buff = (char*)calloc(bytesToRead,sizeof(char));
		if (read(inputFd,buff,bytesToRead) != bytesToRead)
		{
			perror("Error in write!\n");
			exit(EXIT_FAILURE);
		}
		printTab(buff,bytesToRead);
		i = offsetHole; // ustawiam szukanie od kolejnej dziury
		free(buff);
	}
	close(inputFd);

}

void printTab(char* buff, int size)
{
	int occurrence = 1;
	for (int i=1; i<size; ++i)
	{
		if (buff[i-1] != buff[i])
		{
			if (buff[i-1] >= 32 && buff[i-1] < 127) { printf("%c : %d\n", buff[i-1], occurrence); }
			else { printf("0 : %d\n", occurrence); }
			occurrence = 1;
			continue;
		}
		++occurrence;
	}
	if ( occurrence >= 2 )
	{
		if (buff[size-1] >= 32 && buff[size-1] < 127) { printf("%c : %d\n", buff[size-1], occurrence); }
		else { printf("0 : %d\n", occurrence); }
	}
}