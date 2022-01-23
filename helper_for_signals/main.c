#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <float.h>

void readData(int argc, char** argv);
float parseFloat(char* buff);
int parseInt(const char* buff);
void sentSignal();

float timestamp = 0;
float timestamp2 = 0;
int pid = 0;


int main(int argc, char* argv[])
{
	readData(argc, argv);
	printf("%f\n%d\n", timestamp, pid);
	sentSignal();
	return 0;
}

void readData(int argc, char** argv)
{
	int opt = 0;
	while ((opt = getopt(argc, argv, "1:3")) != -1)
	{
		switch(opt)
		{
			case '1':
			{
				timestamp = parseFloat(optarg);
				break;
			}
			case '3':
			{
				timestamp2 = parseFloat(optarg);
			}
			default:
			{
				perror("Invalid input data!\n");
				exit(EXIT_FAILURE);
				break;
			}
		}
	}
	if (argc == optind +1)
	{
		pid = parseInt(argv[optind]);
	}
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
void sentSignal()
{
	int number = 0;
	while(1)
	{
		printf("Number of signals sent %d\n", ++number);
		union sigval value = {0};
		value.sival_int = 5;
		if (sigqueue(pid, SIGUSR1, value) == 0) {
			printf("signal sent successfully!!\n");
		} else {
			perror("SIGSENT-ERROR:");
		}
		if ()
		sleep(timestamp);
	}
}