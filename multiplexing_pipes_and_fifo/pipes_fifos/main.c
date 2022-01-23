#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <float.h>
#include <sys/ioctl.h>
#include <time.h>
#include <poll.h>
#include <memory.h>

#define MAX_NUM_OF_PIPES 50
#define MAX_NUM_OF_FIFOS 50
#define MAX_LENGTH 50
#define MAX_APOC_VALUES 20

struct unparsedInputData {

	char delayInMuxPmp[MAX_LENGTH];
	char valuesForApoc[MAX_APOC_VALUES][MAX_LENGTH];

};

//float delayForMuxPmp = 0;
int numberOfParametersToApoc;
int fifoNumber = 0;

struct pipesAndFifoData {
	int numberOfFifos;
	int fifoTab[MAX_NUM_OF_FIFOS];
	int numberOfPipes;
	int pipeTab[MAX_NUM_OF_PIPES];
};

struct dataForApoc {
	const char dWait[50];
	const char tRead[50];
	const char cRead[50];
};


void readData(int argc, char* argv[], struct unparsedInputData* unparsedData);
void parseInputData(struct unparsedInputData* unparsedData, struct dataForApoc* apocData);
//float parseFloat(char* buff);
int parseInt(const char* buff);
void parseFifoAndPipes(struct pipesAndFifoData* pipeAndFifoStruct);

void doJob(struct dataForApoc* apocData, struct pipesAndFifoData pipeAndFifoStruct, char* delayForMuxpmp);

void createApocFifosAndPipes(int numberOfPipes, int numberOfFifos, int (*pipeDesc)[2], int (*fifoDesc)[2]);
int findDesc(int** fifoDesc, int** pipeDesc, int numberOfFifos, int numberOfPipes);


int main(int argc, char* argv[])
{
	struct unparsedInputData unparsedData = {0};
	readData(argc, argv, &unparsedData);
	struct dataForApoc* apocData = calloc(numberOfParametersToApoc, sizeof(struct dataForApoc));
	parseInputData(&unparsedData, apocData);
//	printf("%f\n", delayForMuxPmp);
	for (int i=0; i<numberOfParametersToApoc; ++i)
	{
		printf("%s\t%s\t%s\n", apocData[i].dWait, apocData[i].tRead, apocData[i].cRead);
	}
	struct pipesAndFifoData pipeAndFifoStruct = {0};
	parseFifoAndPipes(&pipeAndFifoStruct);
	printf("Number of pipes: %d\n", pipeAndFifoStruct.numberOfPipes);
	for (int i=0; i<pipeAndFifoStruct.numberOfPipes; ++i)
	{
		 printf("Pipe value: %d\n", pipeAndFifoStruct.pipeTab[i]);
	}

	printf("Number of fifos: %d\n", pipeAndFifoStruct.numberOfFifos);
	for (int i=0; i<pipeAndFifoStruct.numberOfFifos; ++i)
	{
		printf("Fifo value: %d\n", pipeAndFifoStruct.fifoTab[i]);
	}
	doJob(apocData, pipeAndFifoStruct, unparsedData.delayInMuxPmp);
	free(apocData);
	return 0;
}


void doJob(struct dataForApoc* apocData, struct pipesAndFifoData pipeAndFifoStruct, char* delayForMuxpmp)
{
	int numberOfApocs = pipeAndFifoStruct.numberOfFifos > pipeAndFifoStruct.numberOfPipes ?
			pipeAndFifoStruct.numberOfFifos : pipeAndFifoStruct.numberOfPipes;

//	int totalNumberOfFifos = 0;
//	int totalNumberOfPipes = 0;

	int controlPipeFd[2] = {0};

	if (pipe(controlPipeFd) == -1)
	{
		perror("Error in creating pipes!\n");
		exit(EXIT_FAILURE);
	}

	// Creating muxpmp file

	pid_t pmux = fork();
	pid_t papoc = 0;
	switch(pmux)
	{
		case -1:
		{
			perror("Error in fork for muxpmp!\n");
			exit(EXIT_FAILURE);
		}
		case 0:
		{
			int fifoDescriptors[MAX_NUM_OF_FIFOS][2] = {0}; // it's ok to overwrite values here
			int pipeDescriptors[MAX_NUM_OF_PIPES][2] = {0}; // here also
			for (int i=0; i<numberOfApocs; ++i) // we are going through all apocs
			{
				createApocFifosAndPipes(pipeAndFifoStruct.pipeTab[i], pipeAndFifoStruct.fifoTab[i],
					  pipeDescriptors, fifoDescriptors);

				papoc = fork();
				switch(papoc)
				{
					case -1:
					{
						perror("Error in forking apoc!\n");
						exit(EXIT_FAILURE);
					}
					case 0:
					{
						if (setenv("D_WAIT", (const char *)apocData[i % numberOfApocs].dWait, 1) == -1)
						{
							perror("Error in setting D_WAIT env value!\n");
							exit(EXIT_FAILURE);
						}
						if (setenv("T_READ", (const char *)apocData[i % numberOfApocs].tRead, 1) == -1)
						{
							perror("Error in setting T_READ env value!\n");
							exit(EXIT_FAILURE);
						}
						if (setenv("C_READ", (const char *)apocData[i % numberOfApocs].cRead, 1) == -1)
						{
							perror("Error in setting C_READ env value!\n");
							exit(EXIT_FAILURE);
						}

						char* argumentList[1024][10];
						sprintf(argumentList[0][0],"apoc%d",i);
						sprintf(argumentList[1][0], "-c");
						close(controlPipeFd[0]); // we dont want to read from control pipe in 1st program
						if (dup2(controlPipeFd[1],0) == 1)
						{
							perror("Error in dup2!\n");
							exit(EXIT_FAILURE);
						}
						sprintf(argumentList[2][0], "0");
						int currentIndex = 3; // our argList right know look like apoc -c 0 so our next free index is 3

						for (int k=0; k< pipeAndFifoStruct.pipeTab[i]; ++k)
						{
							if (close(pipeDescriptors[k][1]) == -1) // we are closing write descriptors in apoc
							{
								perror("Error in closing writing pipe desc!\n");
								exit(EXIT_FAILURE);
							}
							sprintf(argumentList[currentIndex++][0]," %d", pipeDescriptors[k][0]);
						}
						for ( int k=0; k < pipeAndFifoStruct.fifoTab[i]; ++k)
						{
							if (close(fifoDescriptors[k][1]) == -1)
							{
								perror("Error in closing write fifo desc!\n");
								exit(EXIT_FAILURE);
							}
							sprintf(argumentList[currentIndex++][0], " %d", fifoDescriptors[k][0]);
						}
						argumentList[currentIndex][0] = NULL;
						if (execv("apoc", argumentList[i]) == -1)
						{
							perror("Error in execv during apoc!\n");
							exit(EXIT_FAILURE);
						}
					}
					default:
					{
						for (int k=0; k<pipeAndFifoStruct.pipeTab[i]; ++k)
						{
							if (close(pipeDescriptors[k][0]) == -1)
							{
								perror("Error in closing pipes reading desc in muxmp!\n");
								exit(EXIT_FAILURE);
							}
							if (close(fifoDescriptors[k][0]) == -1)
							{
								perror("Errror in closing fifos reading desc in muxmp!\n");
								exit(EXIT_FAILURE);
							}
						}
					}
				}
			}
			if (close(controlPipeFd[1]) == -1)
			{
				perror("Error in closing writing side in muxmp in control fd!\n");
				exit(EXIT_FAILURE);
			}
			if (dup2(controlPipeFd[0],0) == -1)
			{
				perror("Error in dup2 in muxmp!\n");
				exit(EXIT_FAILURE);
			}
			if (close(controlPipeFd[0]) == -1)
			{
				perror("Error in closing unused control desc!\n");
				exit(EXIT_FAILURE);
			}
			char numberOfAllDesc[100];
			// I have to find the biggest descriptor
			int theBiggestDescriptor = findDesc((int**)fifoDescriptors, (int**)pipeDescriptors,
									   pipeAndFifoStruct.numberOfFifos, pipeAndFifoStruct.numberOfPipes);
			/*
			 *  Szukam maksymalnej wartosc deskryptora, poniewaz program 3ci pisalem jako ostatni i logike w drugim
			 *  programie oparłem jakby na tym ze drugi parametr przekazuje jakby górna granice deskryptorów czyli
			 *  że mam sprawdzać deskryptory od 0 do tego parametru. Nie jest to najlepsze rozwiazanie bo lepiej
			 *  bylo dzialac tak jak funkcja select ale juz nie chcialo mi się zmieniać drugiego programu
			 */
			//-------------------------------------
			sprintf(numberOfAllDesc,"%d",theBiggestDescriptor);
			char* argList[] = { "muxpmp", delayForMuxpmp, numberOfAllDesc };
			if (execv("muxpmp", argList) == -1)
			{
				perror("Error in creating muxpmp process!\n");
				exit(EXIT_FAILURE);
			}
		}
		default:
		{
			printf("Muxpmp created!\n");
			close(controlPipeFd[0]);
			close(controlPipeFd[1]);
		}
	}

	//---------------------

}


void createApocFifosAndPipes(int numberOfPipes, int numberOfFifos, int (*pipeDesc)[2], int (*fifoDesc)[2])
{
	char buff[100];
	for (int i=0; i<numberOfFifos; ++i)
	{
		sprintf(buff, "/tmp/myfifo%d", fifoNumber);
		++fifoNumber;
		if (mkfifo(buff, S_IRUSR | S_IWUSR | S_IWGRP) == -1)
		{
			perror("Error in mkfifo!\n");
			exit(EXIT_FAILURE);
		}
		fifoDesc[i][0] = open(buff, O_RDONLY | O_NONBLOCK);
		fifoDesc[i][1] = open(buff, O_WRONLY);
		if (pipeDesc[i][0] == -1 || pipeDesc[i][1] == -1)
		{
			perror("Error in opening fifo!\n");
			exit(EXIT_FAILURE);
		}
		if (unlink(buff) == -1)
		{
			perror("Error in unlink fifo!\n");
			exit(EXIT_FAILURE);
		}
	}
	for (int i=0; i<numberOfPipes; ++i)
	{
		if(pipe(pipeDesc[i]) == -1) {
			perror("Error in pipe\n"); exit(EXIT_FAILURE);
		}
		if(fcntl(pipeDesc[i][0], F_SETFL, O_NONBLOCK) == -1)
			perror("Error in fcntl\n");
	}


}

void readData(int argc, char* argv[], struct unparsedInputData* unparsedData)
{
	int opt = 0;
	int val = 0;
	while ((opt = getopt(argc, argv, "m:a:")) != -1)
	{
		switch(opt)
		{
			case 'm':
			{
				memcpy(unparsedData->delayInMuxPmp, optarg, strlen(optarg)+1);
				break;
			}
			case 'a':
			{
				if (val==50)
				{
					perror("Too many -a parameters!\n");
					exit(EXIT_FAILURE);
				}
				memcpy(unparsedData->valuesForApoc[val++], optarg, strlen(optarg)+1);
				break;
			}
			default:
			{
				perror("Error in parameters!\n");
				exit(EXIT_FAILURE);
			}
		}
	}
	numberOfParametersToApoc = val;
}

void parseInputData(struct unparsedInputData* unparsedData, struct dataForApoc* apocData)
{
	//delayForMuxPmp = parseFloat(unparsedData->delayInMuxPmp);
	const char s[2] = ",";
	char* token;

	for (int i=0; i<numberOfParametersToApoc; ++i)
	{
		// zapomniałem że do setenva przekazuje sie const chary :C
//		token = strtok(unparsedData->valuesForApoc[i], s);
//		if (token != NULL) // parsing D_WAIT
//		{
//			apocData[i].dWait = parseFloat(token);
//			token = strtok(NULL, s);
//		}
//
//		if (token != NULL)
//		{
//			apocData[i].tRead = parseInt(token);
//			token = strtok(NULL, s);
//		}
//
//		if (token != NULL)
//		{
//			apocData[i].cRead = parseInt(token);
//			token = strtok(NULL,s);
//		}
//
//		if (token != NULL)
//		{
//			perror("Wrong string to parse!\n");
//			exit(EXIT_FAILURE);
//		}
		token = strtok(unparsedData->valuesForApoc[i], s);
		if (token != NULL) // parsing D_WAIT
		{
			memcpy((void*)&(apocData[i].dWait), token, strlen(token)+1);
			token = strtok(NULL,s);
		}

		if (token != NULL) // parsing T_READ
		{
			memcpy((void*)&(apocData[i].tRead), token, strlen(token)+1);
			token = strtok(NULL,s);
		}
		if (token != NULL) // parsing C_READ
		{
			memcpy((void*)&(apocData[i].cRead), token, strlen(token)+1);
			token = strtok(NULL,s);
		}
		if (token != NULL)
		{
			perror("Wrong string to parse!\n");
			exit(EXIT_FAILURE);
		}
	}
}


void parseFifoAndPipes(struct pipesAndFifoData* pipeAndFifoStruct)
{
	char* temp = getenv("PIPES");

	if (temp == NULL)
	{
		perror("PIPES env value is not set!\n");
		exit(EXIT_FAILURE);
	}

	const char s[2] = ",";
	char *token;

	token = strtok(temp,s);
	int i=0;
	while (token != NULL)
	{
		pipeAndFifoStruct->pipeTab[i] = parseInt(token);
		token = strtok(NULL,s);
		++i;
	}
	pipeAndFifoStruct->numberOfPipes = i;

	temp = getenv("FIFOS");

	if (temp == NULL)
	{
		perror("FIFOS env value is not set!\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(temp,s);
	i = 0;
	while (token != NULL)
	{
		pipeAndFifoStruct->fifoTab[i] = parseInt(token);
		token = strtok(NULL,s);
		++i;
	}
	pipeAndFifoStruct->numberOfFifos = i;

}

//float parseFloat(char* buff)
//{
//	char* endptr = NULL;
//	float val = 0;
//	errno = 0;
//	val = strtof(buff, &endptr);
//	if ((errno == ERANGE && (val == FLT_MAX || val == FLT_MIN))
//	    || (errno != 0 && val == 0)) {
//		perror("strtof");
//		exit(EXIT_FAILURE);
//	}
//
//	if (endptr == buff) {
//		fprintf(stderr, "No digits were found\n");
//		exit(EXIT_FAILURE);
//	}
//	return val;
//}

int parseInt(const char* buff)
{
	int val = 0;
	char* endptr = NULL;
	errno = 0;
	val = (int)strtol(buff, &endptr, 10);
	if ((errno == ERANGE && (val == INT_MAX || val == INT_MIN))
	    || (errno != 0 && val == 0)) {
		perror("strtol");
		exit(EXIT_FAILURE);
	}

	if (endptr == buff)
	{
		fprintf(stderr, "No digits were found\n");
		exit(EXIT_FAILURE);
	}
	return val;
}

int findDesc(int** fifoDesc, int** pipeDesc, int numberOfFifos, int numberOfPipes)
{
	int maxValue = 0;
	for (int i=0; i<numberOfFifos; ++i)
	{
		if (maxValue < fifoDesc[i][1]) { maxValue = fifoDesc[i][1]; }
	}
	for (int i=0; i<numberOfPipes; ++i)
	{
		if (maxValue < pipeDesc[i][1]) { maxValue = pipeDesc[i][1]; }
	}
	return maxValue;
}
