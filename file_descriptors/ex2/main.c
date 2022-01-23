#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#define MAX_C 5
#define BUF_SIZE 1024

struct inputStruct
{
	char pathToInputFile[40];
	char pathToOutputFile[40];
	char locationOfCData[MAX_C][40];
	char locationOfDData[40];

};

//struct locationOfSourceDataAndPlaceToResult
//{
//	long beginningInSourceFile;
//	long numberOfBytesToCopy;
//	long beginningInResultingFile;
//
//};

int validation(const char *, const char *);

int readInput(int argc, char *argv[], struct inputStruct *inputData);
void parseCInput(long (*parsedLocation)[MAX_C][3], char locationOfCData[MAX_C][40], int numberOfC);
void parseDInput(long* dataD, char* locationOfDData);
void copyData(struct inputStruct inputData, long parsedLocation[MAX_C][3], int numberOfC);

int main(int argc, char *argv[])
{
	struct inputStruct inputData = {0};
	int numberOfC = readInput(argc, argv, &inputData);
//    printf("%s\n%s\n%s\n%s\n", inputData.pathToInputFile, inputData.pathToOutputFile,
//                                inputData.locationOfSourceData, inputData.constantValue);
	long dataC[MAX_C][3] = {0};
	long dataD[2] = {0};
	parseCInput(&dataC, inputData.locationOfCData, numberOfC);
	//printf("%ld\t %ld\t %ld\t", dataC[0][0], dataC[0][1], dataC[0][2]);
	parseDInput(dataD, inputData.locationOfDData);
//	printf("%ld\t%ld\t",dataD[0],dataD[1]);
//	printf("%s\n",inputData.locationOfDData);
	copyData(inputData,dataC,numberOfC);

	return 0;
}

//int validation(const char *argument, const char *endPointer)
//{
//	if (*argument != '\0' && *endPointer == '\0') { return 1; }
//	else { return 0; }
//}

int readInput(int argc, char *argv[], struct inputStruct *inputData)
{
	int opt = 0;
	int i = 0;
	while ((opt = getopt(argc, argv, "I:O:c:d:")) != -1)
	{
		switch (opt)
		{
			case 'I':
				memcpy(inputData->pathToInputFile, optarg, strlen(optarg) + 1);
				break;
			case 'O':
				memcpy(inputData->pathToOutputFile, optarg, strlen(optarg) + 1);
				break;
			case 'c':
				memcpy(inputData[i].locationOfCData, optarg, strlen(optarg) + 1);
				++i;
				break;
			case 'd':
				memcpy(inputData->locationOfDData, optarg, strlen(optarg) + 1);
				break;
			default:
				perror("Invalid input data!\n");
				exit(EXIT_FAILURE);
		}
	}
	return i;
}

void parseCInput(long (*parsedLocation)[MAX_C][3], char locationOfCData[MAX_C][40], int numberOfC)
{
	const char s[2] = ",";
	char* token;
	long val = 0;
	char* endPointer;
	for (int i=0; i<numberOfC; ++i)
	{
		int j = 0;
		token = strtok(locationOfCData[i], s);
		while( token != NULL )
		{
			errno = 0;
			val = strtol(token, &endPointer, 10);
			if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
			    || (errno != 0 && val == 0))
			{
				perror("strtol");
				exit(EXIT_FAILURE);
			}
			if (endPointer == token)
			{
				fprintf(stderr, "No digits were found\n");
				exit(EXIT_FAILURE);
			}
			//*parsedLocation[i][j] = val; // i odpowiada za ilość flag "c" a j za D C W
			//char* tabOfC[6] = {"B", "k", "M", "K", "Ki", "Mi"};
//			if (*endPointer != '\0')
//			{
//
//				switch( *endPointer )
//				{
//					case 'B':
//						break;
//					case 'k':
//						parsedLocation[i][j] *= 1000;
//						break;
//					case 'M':
//						if ((*endPointer)+1 == 'i')
//						{
//							parsedLocation[i][j] *= 1048576;
//						}
//						parsedLocation[i][j] *= 1000000000;
//						break;
//					case 'K':
//						parsedLocation[i][j] *= 1024;
//						printf("%c",(*endPointer)+1);
//						if ((*endPointer) + 1 == 'i' || (*endPointer) + 1 == '\0')
//							break;
//						perror("Invalid char after K\n");
//						exit(EXIT_FAILURE);
//					default:
//						perror("Invalid values after flags\n");
//						exit(EXIT_FAILURE);
//
//				}
//			}
		    if (strcmp(endPointer,"") == 0 || strcmp(endPointer,"B") == 0)
		    {
			    (*parsedLocation)[i][j] = val;
		    } else if (strcmp(endPointer, "k") == 0)
		    {
			    (*parsedLocation)[i][j] = val*1000;
		    } else if (strcmp(endPointer, "M") == 0)
		    {
			    (*parsedLocation)[i][j] = val*1000000000;
		    } else if (strcmp(endPointer, "K") == 0 || strcmp(endPointer,"Ki") == 0)
		    {
			    (*parsedLocation)[i][j] = val*1024;
		    }
		    else if (strcmp(endPointer,"Mi") == 0)
		    {
			    (*parsedLocation)[i][j] = val*1048576;
		    } else
		    {
		    	perror("Invalid values after c flag\n");
		    	exit(EXIT_FAILURE);
		    }
		    ++j;
			token = strtok(NULL, s);
		}
	}
}
void parseDInput(long* dataD, char* locationOfDData)
{
	const char s[2] = ",";
	char* token;
	long val = 0;
	char* endPointer;
	int j = 0;
	char buff[40] = {0};
	token = strtok(locationOfDData, s);
	if (token != NULL) { memcpy(buff,token,strlen(token)+1); } // chyba konieczne ze względu na to jak jest napisany strtok
	token = strtok(NULL, s);
	while( token != NULL )
	{
		errno = 0;
		val = strtol(token, &endPointer, 10);
		if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
		    || (errno != 0 && val == 0))
		{
			perror("strtol");
			exit(EXIT_FAILURE);
		}
		if (endPointer == token)
		{
			fprintf(stderr, "No digits were found\n");
			exit(EXIT_FAILURE);
		}
		if (strcmp(endPointer,"") == 0 || strcmp(endPointer,"B") == 0)
		{
			dataD[j] = val;
		} else if (strcmp(endPointer, "k") == 0)
		{
			dataD[j] = val*1000;
		} else if (strcmp(endPointer, "M") == 0)
		{
			dataD[j] = val*1000000000;
		} else if (strcmp(endPointer, "K") == 0 || strcmp(endPointer,"Ki") == 0)
		{
			dataD[j] = val*1024;
		}
		else if (strcmp(endPointer,"Mi") == 0)
		{
			dataD[j] = val*1048576;
		} else
		{
			perror("Invalid values after c flag\n");
			exit(EXIT_FAILURE);
		}
		++j;
		token = strtok(NULL, s);
	}
	memcpy(locationOfDData,buff,sizeof(buff)+1);
}

void copyData(struct inputStruct inputData, long parsedLocation[MAX_C][3], int numberOfC)
{
	int inputFile = 0;
	int outputFile = 0;
	char buf[BUF_SIZE] = {0};
	inputFile = open(inputData.pathToInputFile, O_RDONLY);
	if (inputFile == -1)
	{
		perror("Error in input file!\n");
		exit(EXIT_FAILURE);
	}
	outputFile = open(inputData.pathToOutputFile, O_RDWR);
	if (outputFile == -1)
	{
		perror("Error in output file!\n");
		exit(EXIT_FAILURE);
	}
	for (int i=0; i<numberOfC; ++i)
	{
		lseek(inputFile,parsedLocation[i][0],SEEK_SET);
		lseek(outputFile,parsedLocation[i][2],SEEK_SET);
		if (read(inputFile,&buf,parsedLocation[i][1]) != parsedLocation[i][1])
		{
			perror("Error in read\n");
			exit(EXIT_FAILURE);
		}
		if (write(outputFile,&buf,parsedLocation[i][1]) != parsedLocation[i][1])
		{
			perror("Error in write file\n");
			exit(EXIT_FAILURE);
		}
	}
	if (close(inputFile) == -1)
	{
		perror("Error in close inputFile\n");
		exit(EXIT_FAILURE);
	}
	if (close(outputFile) == -1)
	{
		perror("Error in close outputFile\n");
		exit(EXIT_FAILURE);
	}
}

// lseek na hole i potem lseek na data i lewa i prawa i elo ez 