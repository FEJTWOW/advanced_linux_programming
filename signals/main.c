#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include <memory.h>

siginfo_t sigInfo;
struct timespec volatile timeStruct;

void signalFunction(int signo, siginfo_t* SI, void* data);
struct timespec calculateTime(struct timespec firstTime, struct timespec secondTime);


int main(int argc, char* argv[])
{
	printf("%d\n", getpid());
	struct timespec timeInMain;
	struct timespec deltaTime;
	struct timespec timeToWaitInShow = { .tv_sec = 1, .tv_nsec = 500000000};
	struct sigaction sa;
	memset(&sa,'\0', sizeof(sa));
	sa.sa_sigaction = signalFunction;
	sa.sa_flags = SA_SIGINFO;
	if ( sigaction(SIGUSR1, &sa, NULL) == -1)
	{
		perror("Error in setting signal handling!\n");
		exit(EXIT_FAILURE);
	}
	if (sigaction(SIGTERM+3, &sa, NULL) == -1)
	{
		perror("Error in setting sigterm signal handling\n");
		exit(EXIT_FAILURE);
	}
	errno = 0;
	int counter = 0;
	sigemptyset(&sa.sa_mask);
	sigset_t set = {0};
	sigaddset(&set, SIGUSR1);
	sigaddset(&set, SIGTERM+3);
	while(1)
	{
		if (pause() == -1)
		{

			if (errno == EINTR)
			{
				printf("\nNumber of signals handled: %d\n", ++counter);
				if (sigprocmask(SIG_BLOCK, &set, NULL) == -1)
				{
					perror("Error in sigprocmask!\n");
					exit(EXIT_FAILURE);
				}
				printf("%d\n", sigInfo.si_signo);
				printf("%d\n", sigInfo.si_code);
				printf("%d\n", sigInfo.si_value.sival_int);

				nanosleep(&timeToWaitInShow,NULL);
				if (sigprocmask(SIG_UNBLOCK, &set, NULL) == -1)
				{
					perror("Error in sigprocmask2!\n");
					exit(EXIT_FAILURE);
				}
			}

		}
		clock_gettime(CLOCK_MONOTONIC, &timeInMain);
		deltaTime = calculateTime(timeStruct, timeInMain);
		printf("Sec: %ld Nanosec: %ld\n", deltaTime.tv_sec, deltaTime.tv_nsec);
	}
	return 0;
}

void signalFunction(int signo, siginfo_t* SI, void* data)
{
	struct timespec temp = {0};
	clock_gettime(CLOCK_MONOTONIC, &temp);
	timeStruct = temp;
	sigInfo = *SI;
	write(1,"signal ",7);
}

struct timespec calculateTime(struct timespec firstTime, struct timespec secondTime)
{
	struct timespec deltaTime = {0};
	deltaTime.tv_sec = secondTime.tv_sec - firstTime.tv_sec;
	deltaTime.tv_nsec = secondTime.tv_nsec - firstTime.tv_nsec;
	if (deltaTime.tv_nsec < 0)
	{
		deltaTime.tv_sec -= 1; // nsec cannot be higher than 1 sec
		deltaTime.tv_nsec = 1000000000 + deltaTime.tv_nsec; // timeToSleep.tv_nsec is negative
	}
	return deltaTime;
}
