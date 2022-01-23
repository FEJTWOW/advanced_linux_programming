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

struct inputData {

	int numberOfDescriptors;
	float timeout;
	double mux_delay;
	int* tableOfDescriptors;
};


void readData(int argc, char* argv[], struct inputData* data);
void readEnvData(struct inputData* data);
void checkDescriptors(struct inputData* data);
void shiftTable(int* tab, int numberOfDescriptors, int positionToErase);
void prepareForPoll(struct inputData* data, struct pollfd* pollFd);
int parseInt(const char* buff);
double parseDouble(char* buff);
float parseFloat(char* buff);
void checkControlDescriptor(int controlDescriptor);
int checkNormalDescriptors(int normalDescriptor);

void doPoll(struct inputData* data, struct pollfd* pollFd);


int main(int argc, char* argv[])
{
	struct inputData data = {.numberOfDescriptors=1024, .timeout = 0, .mux_delay = 7.5};
	readData(argc, argv, &data); // segmentation fault dunno why
	readEnvData(&data);
	data.numberOfDescriptors -= 2; // we dont need 1 and 2 descriptors
	data.tableOfDescriptors = (int*)calloc(data.numberOfDescriptors, sizeof(int));
	data.tableOfDescriptors[0] = 0; // under 0 index we have our control pipe
	for (int i=1; i<data.numberOfDescriptors; ++i)
		data.tableOfDescriptors[i] = i+2;
	printf("%d\n %f\n %f\n", data.numberOfDescriptors, data.timeout, data.mux_delay);
	checkDescriptors(&data);
	struct pollfd* pollFd = (struct pollfd*)calloc(data.numberOfDescriptors, sizeof(struct pollfd));
	prepareForPoll(&data, pollFd);
	doPoll(&data, pollFd);
	free(data.tableOfDescriptors);
	free(pollFd);
	return 0;
}

void readData(int argc, char* argv[], struct inputData* data)
{
	if (argc != 2 && argc !=3)
	{
		perror("Wrong program arguments!\n");
		exit(EXIT_FAILURE);
	}
	data->timeout = parseFloat(argv[1]);
	if (argc==3)
	{
		int valI = 0;
		valI = parseInt(argv[2]);
		if (valI < 3)
		{
			perror("Invalid descriptors number!\n");
			exit(EXIT_FAILURE);
		}
		data->numberOfDescriptors = valI;
	}

}

void readEnvData(struct inputData* data)
{
	char* temp = getenv("MUX_DELAY");
	if (temp != NULL)
	{
		data->mux_delay = parseDouble(temp);
	}
}

void checkDescriptors(struct inputData* data)
{
	// check control descriptor!
	checkControlDescriptor(data->tableOfDescriptors[0]);
	//--------------------------
	for (int i=1; i<data->numberOfDescriptors; ++i)
	{
		if (checkNormalDescriptors(data->tableOfDescriptors[i]) == 0)
		{
			if (data->numberOfDescriptors == 1)
			{
				perror("Not enough data descriptors!\n");
				exit(EXIT_FAILURE);
			}
			close(data->tableOfDescriptors[i]);
			shiftTable(data->tableOfDescriptors, (int)data->numberOfDescriptors, i);
			--data->numberOfDescriptors;
			continue;
		}
	}

}

void shiftTable(int* tab, int numberOfDescriptors, int positionToErase)
{
	for (int i=positionToErase; i<numberOfDescriptors-1; ++i)
	{
		tab[i] = tab[i+1];
	}
}

void prepareForPoll(struct inputData* data, struct pollfd* pollFd)
{
	pollFd[0].fd = data->tableOfDescriptors[0];
	pollFd[0].events = POLLIN | POLLERR;
	pollFd[0].revents = 0;
	for (int i=1; i<data->numberOfDescriptors; ++i)
	{
		pollFd[i].fd = data->tableOfDescriptors[i];
		pollFd[i].events = POLLOUT | POLLERR;
		pollFd[i].revents = 0;
	}
}

void doPoll(struct inputData* data, struct pollfd* pollFd)
{
	int ready;
	int time = data->timeout*1000;
	struct timespec mux_delay = {0};
	struct timespec rem = {0};
	mux_delay.tv_sec = data->mux_delay/100;
	mux_delay.tv_nsec = (data->mux_delay - (data->mux_delay/100)*100)*10000000;
	while ((ready = poll(pollFd,data->numberOfDescriptors,time)) != -1)
	{
		if (ready==0) // timeout
		{
			for (int i=1; i<data->numberOfDescriptors; ++i)
			{
				// we are setting up descriptors
				if (pollFd[0].events == 0)
				{
					long numberOfBytes = 0;
					if (ioctl(pollFd[i].fd, FIONREAD, &numberOfBytes) == -1)
					{
						perror("Error in icotl!\n");
						exit(EXIT_FAILURE);
					}
					if (numberOfBytes == 0) { pollFd[i].events = POLLOUT | POLLERR; }
				}

			}
		}
		else if ( ready == 1 && (pollFd[0].revents & POLLIN))
		{
			if (pollFd[0].revents & POLLERR)
			{
				perror("Error in control stream!\n");
				exit(EXIT_FAILURE);
			}
			char buff[5];
			buff[4] = '\0';
			if (read(pollFd[0].fd, buff, 4) != 4)
			{
				perror("We didn't read 4 bytes which are represent number of descriptor to close!\n");
				exit(EXIT_FAILURE);
			}
			int val = parseInt(buff);
			for (int i=1; i<data->numberOfDescriptors; ++i)
			{
				if (pollFd[i].fd == val)
				{
					pollFd[i].events = 0;
				}
			}
			pollFd[0].revents = 0;
			continue;
		}
		else
		{
			if (pollFd[0].revents & POLLIN) // one of returned descriptor is our control descriptor
				--ready;

			for (int i=1; i<data->numberOfDescriptors; ++i)
			{
				if (ready==0) { continue; }
				if (pollFd[i].revents & POLLERR)
				{
					perror("Error in descriptor!\n");
					exit(EXIT_FAILURE);
				}
				if (pollFd[i].revents & POLLOUT)
				{
					if (write(pollFd[i].fd, &pollFd[i].fd, sizeof(int32_t)) != 4)
					{
						perror("Unable to write 4 bytes to pipe!\n");
						exit(EXIT_FAILURE);
					}
					--ready;
				}
			}
		}
		errno = 0;
		if (nanosleep(&mux_delay,&rem) == -1)
		{
			if (errno == EINTR)
			{
				if (nanosleep(&rem, NULL) == -1)
				{
					perror("Error in rem nanosleep!\n");
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
}



void checkControlDescriptor(int controlDescriptor)
{
	int flags = 0;
	int accessMode = 0;
	struct stat info = {0};
	flags = fcntl(controlDescriptor, F_GETFL);
	if (flags == -1)
	{
		perror("Error in control pipe!\n");
		exit(EXIT_FAILURE);
	}
	accessMode = flags & O_ACCMODE;
	if (accessMode != O_RDONLY)
	{
		perror("Error in access mode in control pipe!\n");
		exit(EXIT_FAILURE);
	}
	if (fstat(controlDescriptor, &info) == -1)
	{
		perror("Error in fstat!\n");
		exit(EXIT_FAILURE);
	}
	if ((info.st_mode & S_IFMT) != S_IFIFO) // we are checking if our descriptors leads to fifo/pipe
	{
		perror("Our control pipe is not fifo or pipe!\n");
		exit(EXIT_FAILURE);
	}
}

int checkNormalDescriptors(int normalDescriptor)
{
	int flags = 0;
	int accessMode = 0;
	struct stat info = {0};
	flags = fcntl(normalDescriptor, F_GETFL);
	if (flags == -1)
	{
		perror("Error in fcntl while checking descriptors!\n");
		exit(EXIT_FAILURE);
	}
	accessMode = flags & O_ACCMODE;
	if (accessMode != O_WRONLY) // we are checking if our descriptors are write only
	{
		return 0;
	}
	if (fstat(normalDescriptor, &info) == -1)
	{
		perror("Error in fstat!\n");
		exit(EXIT_FAILURE);
	}
	if ((info.st_mode & S_IFMT) != S_IFIFO) // we are checking if our descriptors leads to fifo/pipe
	{
		return 0;
	}
	return 1;
}


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

double parseDouble(char* buff)
{
	char* endptr = NULL;
	double valD = 0;
	errno = 0;
	valD = strtod(buff, &endptr);
	if ((errno == ERANGE && (valD == DBL_MAX || valD == DBL_MIN))
	    || (errno != 0 && valD == 0))
	{
		perror("Error in strtod!\n");
		exit(EXIT_FAILURE);
	}

	if (endptr == buff)
	{
		fprintf(stderr, "No digits were found\n");
		exit(EXIT_FAILURE);
	}
	return valD;
}

float parseFloat(char* buff)
{
	char* endptr = NULL;
	float val = 0;
	errno = 0;
	val = strtof(buff, &endptr);
	if ((errno == ERANGE && (val == FLT_MAX || val == FLT_MIN))
	    || (errno != 0 && val == 0)) {
		perror("strtof");
		exit(EXIT_FAILURE);
	}

	if (endptr == buff) {
		fprintf(stderr, "No digits were found\n");
		exit(EXIT_FAILURE);
	}
	if ( (int)(val*1000)%125 != 0)
	{
		perror("Float value should be provided by one eight of a second value");
		exit(EXIT_FAILURE);
	}
	return val;
}
