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

struct inputData {
	long controlDescriptor;
	long descriptorList[50];
	int numberOfDescriptors;
};

struct envData {

	double time;
	long numberOfReadings;
	long numberOfBytesToRead;
};

void readData(int argc, char** argv, struct inputData* data);
void checkDescriptors(struct inputData* data);
void readEnvData(struct envData* env, int capacityOfDataStream);
void doJob(struct inputData* data, struct envData* env, int capacityOfDataStream);
void shiftTable(long* tab, int numberOfDescriptors, int positionToErase);

int main(int argc, char* argv[])
{
	struct envData env = { .time = 0.5, .numberOfBytesToRead = 16, .numberOfReadings = 3};
	printf("%d\n", argc);
	struct inputData data = {0};
	readData(argc, argv, &data);
	int capacity = fcntl(data.controlDescriptor, F_GETPIPE_SZ);
//	printf("%ld\n", data.controlDescriptor);
//	for (int i=0; i<data.numberOfDescriptors; ++i)
//	{
//		printf("%ld\n", data.descriptorList[i]);
//	} // debug
	checkDescriptors(&data);
	readEnvData(&env, capacity);
	doJob(&data, &env, capacity);
//	printf("%f\n%ld\n%ld\n", env.time, env.numberOfReadings, env.numberOfBytesToRead);
	return 0;
}


void readData(int argc, char** argv, struct inputData* data) {

	int opt = 0;
	char* endptr = NULL;
	long val = 0;
	while ((opt = getopt(argc, argv, "c:")) != -1) {

		if (opt == 'c')
		{
			errno = 0;
			val = strtol(optarg, &endptr, 10);
			if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
			    || (errno != 0 && val == 0)) {
				perror("strtol");
				exit(EXIT_FAILURE);
			}

			if (endptr == optarg) {
				fprintf(stderr, "No digits were found\n");
				exit(EXIT_FAILURE);
			}
			data->controlDescriptor = val;
		}
	}
	int j = 0;
	for (int i=optind; i<argc; ++i)
	{
		errno = 0;
		val = strtol(argv[i], &endptr, 10);
		if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
		    || (errno != 0 && val == 0))
		{
			perror("Error in strtol!\n");
			exit(EXIT_FAILURE);
		}

		if (endptr == optarg)
		{
			fprintf(stderr, "No digits were found\n");
			exit(EXIT_FAILURE);
		}
		data->descriptorList[j++] = val;
	}
	data->numberOfDescriptors = j;
}


void checkDescriptors(struct inputData* data)
{
	struct stat info;
	int flags;
	int accessMode;

	flags = fcntl(data->controlDescriptor, F_GETFL);
	if (flags == -1)
	{
		perror("Error in fcntl!\n");
		exit(EXIT_FAILURE);
	}
	accessMode = flags & O_ACCMODE;
	if (accessMode != O_WRONLY)
	{
		perror("Control descriptor should be WRONLY!\n");
		exit(EXIT_FAILURE);
	}

	if (fstat(data->controlDescriptor, &info) == -1)
	{
		perror("Error in fstat");
		exit(EXIT_FAILURE);
	}

	if ((info.st_mode & S_IFMT) != S_IFIFO)
	{
		perror("Control descriptor is not FIFO!\n");
		exit(EXIT_FAILURE);
	}
	for (int i=0; i<data->numberOfDescriptors; ++i)
	{
		flags = fcntl(data->descriptorList[i], F_GETFL);
		if (flags == -1)
		{
			perror("Error in fcntl!\n");
			exit(EXIT_FAILURE);
		}
		accessMode = flags & O_ACCMODE;
		if (accessMode != O_RDONLY)
		{
			if (data->numberOfDescriptors == 0)
			{
				perror("Not enough data descriptors!\n");
				exit(EXIT_FAILURE);
			}
			close(data->descriptorList[i]); // TODO Shift table
			shiftTable(data->descriptorList, data->numberOfDescriptors, i);
			--data->numberOfDescriptors;
			continue;
		}
		if (fstat(data->descriptorList[i], &info) == -1)
		{
			perror("Error in fstat!\n");
			exit(EXIT_FAILURE);
		}
		if ((info.st_mode & S_IFMT) != S_IFIFO)
		{
			if (data->numberOfDescriptors == 0)
			{
				perror("Not enough data descriptors!\n");
				exit(EXIT_FAILURE);
			}
			close(data->descriptorList[i]);
			shiftTable(data->descriptorList, data->numberOfDescriptors, i);
			--data->numberOfDescriptors;
			continue;
		}

		if (fcntl(data->descriptorList[i], F_SETFL, flags | O_NONBLOCK) == -1)
		{
			perror("Error in setting non block flag!\n");
			exit(EXIT_FAILURE);
		}
	}
}

void readEnvData(struct envData* env, int capacityOfDataStream)
{

	char* endptr = NULL;
	long val = 0;

	char* temp = getenv("T_READ");
	if (temp != NULL)
	{
		errno = 0;
		val = strtol(temp, &endptr, 10);
		if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
		    || (errno != 0 && val == 0))
		{
			perror("Error in strtol!\n");
			exit(EXIT_FAILURE);
		}

		if (endptr == temp)
		{
			fprintf(stderr, "No digits were found\n");
			exit(EXIT_FAILURE);
		}
		env->numberOfReadings = val;
	}


	temp = getenv("C_READ");
	if (temp != NULL)
	{
		errno = 0;
		val = strtol(temp, &endptr, 10);
		if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
		    || (errno != 0 && val == 0))
		{
			perror("Error in strtol!\n");
			exit(EXIT_FAILURE);
		}

		if (endptr == temp)
		{
			fprintf(stderr, "No digits were found\n");
			exit(EXIT_FAILURE);
		}
		if (val <= 4 && val >= (double) capacityOfDataStream / 3);
		env->numberOfBytesToRead = val;
	}
	temp = getenv("D_WAIT");
	if (temp != NULL)
	{
		double valD = 0;
		errno = 0;
		valD = strtod(temp, &endptr);
		if ((errno == ERANGE && (valD == DBL_MAX || valD == DBL_MIN))
		    || (errno != 0 && valD == 0))
		{
			perror("Error in strtod!\n");
			exit(EXIT_FAILURE);
		}

		if (endptr == temp)
		{
			fprintf(stderr, "No digits were found\n");
			exit(EXIT_FAILURE);
		}
		if (valD <= 0.25 && valD >= 1.5)
		{
			perror("Time should be between 0.25 and 1.5!\n");
			exit(EXIT_FAILURE);
		}
		env->time = valD;
	}
}


void doJob(struct inputData* data, struct envData* env, int capacityOfDataStream)
{
	// STRUCTS FOR TIME
	struct timespec startTime = {0};
	struct timespec endTime = {0};
	struct timespec timeToSleep = {0};
	struct timespec rem = {0};
	struct timespec timeSliceSleep = {0};
	srand(time(NULL));
	// -----------------------
	int* flags = (int*)calloc(data->numberOfDescriptors, sizeof(int));
	int numOfBytes;
	int test = 0;
	char* buff = (char*)calloc(env->numberOfBytesToRead, sizeof(char));
	while(1)
	{
		test = (test+1)%(env->numberOfReadings+1);
		printf("%d\n", test);
		if (test==1) // first iteration ( start time count )
		{
			if (clock_gettime(CLOCK_REALTIME, &startTime) == -1)
			{
				perror("Error in clock_gettime!\n");
				exit(EXIT_FAILURE);
			}
		}
		for(int i=0; i<data->numberOfDescriptors; ++i)
		{
			switch(flags[i])
			{
				case 0:
				{
					if (test==0) // we are after T_READ iteration so we can check if we can start to clear buff
					{

						if (ioctl(data->descriptorList[i], FIONREAD, &numOfBytes) == -1)
						{
							perror("Error in ioctl!\n");
							exit(EXIT_FAILURE);
						}
						if (numOfBytes == capacityOfDataStream) { flags[i] = 1; }
					}
					break;
				}
				case 1:
				{
					if (read(data->descriptorList[i], buff, env->numberOfBytesToRead) == -1) // this is our first read so if an error occurs we have to stop
					{
						perror("Error in first read!\n"); // no need to check errno EAGAIN
						exit(EXIT_FAILURE);
					}
					if (write(data->controlDescriptor, buff, sizeof(int32_t)) == -1) // stop wrting to our descriptor!
					{
						perror("Error in write!\n");
						exit(EXIT_FAILURE);
					}
					flags[i] = 2;
					break;
				}
				case 2:
				{
					errno = 0;
					if (read(data->descriptorList[i],"/dev/null",env->numberOfBytesToRead) == -1)
					{
						if (errno==EAGAIN) { flags[i] = 0; } // TODO
						else
						{
							perror("Error in read case 2!\n");
							exit(EXIT_FAILURE);
						}
					}
					break;
				}
				default:
				{
					perror("Error in flags!\n");
					exit(EXIT_FAILURE);
				}
			}
			int randVal = (rand()%14)+3;
			timeSliceSleep.tv_nsec = ((double)1/randVal)*1000000000;
			errno = 0;
			if (nanosleep(&timeSliceSleep, &rem) == -1)
			{
				if (errno==EINTR)
				{
					if (nanosleep(&rem,NULL) == -1)
					{
						perror("Error in timeslice rem nanosleep!\n");
						exit(EXIT_FAILURE);
					}
				}
				else
				{
					perror("Error in nanosleep!\n");
					exit(EXIT_FAILURE);
				}
			}
		}

		if (test==0)
		{
			if (clock_gettime(CLOCK_REALTIME, &endTime) == -1)
			{
				perror("Error in clock_gettime!\n");
				exit(EXIT_FAILURE);
			}
//			timeToSleep.tv_sec = endTime.tv_sec - startTime.tv_sec;
//			timeToSleep.tv_nsec = endTime.tv_nsec - startTime.tv_nsec;
			timeToSleep.tv_sec = (time_t)(env->time / 10);
			timeToSleep.tv_nsec = (env->time - (double)(timeToSleep.tv_sec * 10))*100000000;

			timeToSleep.tv_sec -= (endTime.tv_sec - startTime.tv_sec);
			if (timeToSleep.tv_sec < 0) { continue; }
			timeToSleep.tv_nsec -= (endTime.tv_nsec - startTime.tv_nsec);
			if (timeToSleep.tv_nsec < 0)
			{
				if (timeToSleep.tv_sec==0) { continue; }
				timeToSleep.tv_sec -= 1; // nsec cannot be higher than 1 sec
				timeToSleep.tv_nsec = 1000000000 + timeToSleep.tv_nsec; // timeToSleep.tv_nsec is negative
			}
			errno = 0;
			if (nanosleep(&timeToSleep,&rem) == -1)
			{
				if (errno == EINTR)
				{
					if (nanosleep(&rem,NULL) == -1)
					{
						perror("Error in remaining nanosleep!\n");
						exit(EXIT_FAILURE);
					}
				}
				else
				{
					perror("Error in remaining nanosleep!\n");
					exit(EXIT_FAILURE);
				}
			}
		}
	}
}

void shiftTable(long* tab, int numberOfDescriptors, int positionToErase)
{
	for (int i=positionToErase; i<numberOfDescriptors-1; ++i)
	{
		tab[i] = tab[i+1];
	}
}
