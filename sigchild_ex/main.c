#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include <memory.h>
#include <float.h>
#include <limits.h>
#include <sys/ioctl.h>

#define clrscr() printf("\e[1;1H\e[2J");
#define gotoxy(x,y) printf("\033[%d;%dH", (y), (x))


struct inputData
{
	float interval;
	int numberOfIntervals;
	int x;
	int y;
};

siginfo_t sigInfo;
struct timespec timeStruct;
struct timespec timeEnd;
struct timespec difference;
int counterForChild = 0;
struct winsize window;
const char* redColor = "\033[0;31m";
const char* greenColor = "\033[0;32m";

void parseData(char** argv, struct inputData* data);
struct timespec parseTime(float interval);
void countDown(struct inputData* data);
void createChild(struct inputData* data);
void observeChild(pid_t pid);
void childFunction(struct inputData* data);
void signalFunction(int signo, siginfo_t* SI, void* data);
void handleChangeTerminalSizeSignal();
void takeWindowSize();
void ignoreChild();
void checkSize(struct inputData* data, unsigned int outputLength);
struct timespec calculateTime(struct timespec firstTime, struct timespec secondTime);


int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		perror("Invalid input!\n Usage ./whip %f:%d:%d:%d");
		exit(EXIT_FAILURE);
	}
	takeWindowSize();
	struct inputData data;
	parseData(argv, &data);
	handleChangeTerminalSizeSignal();
	countDown(&data);
	ignoreChild();
	createChild(&data);
	return 0;
}

void countDown(struct inputData *data)
{
	struct timespec sleepTime = parseTime(data->interval);
	struct timespec resTime = {0};
	for (int i = data->numberOfIntervals; i > 0; --i)
	{
		gotoxy(data->x, data->y);
		checkSize(data, strlen("%sodliczanie #%d %d%d\n"));
		printf("%sodliczanie #%d %d%d\n", redColor, i, window.ws_row, window.ws_col);
		errno = 0;
		if (nanosleep(&sleepTime, &resTime) == -1)
		{
			if (errno == EINTR)
			{
				nanosleep(&resTime, NULL);
			} else
			{
				perror("Error in nanosleep!\n");
				exit(EXIT_FAILURE);
			}
		}
	}
	clrscr();
	if (fflush(stdout) == EOF)
	{
		perror("Error in flush!\n");
		exit(EXIT_FAILURE);
	}
}

void createChild(struct inputData* data)
{
	pid_t p;

	p = fork();
	switch(p)
	{
		case -1:
		{
			perror("Error in fork!\n");
			exit(EXIT_FAILURE);
		}
		case 0:
		{
			childFunction(data);
		}
		default:
		{
			observeChild(p);
		}
	}
}

void observeChild(pid_t pid)
{
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = signalFunction;
	if (sigaction(SIGCHLD, &sa, NULL) == -1)
	{
		perror("Error in sigaction!\n");
		exit(EXIT_FAILURE);
	}
	//---- for blocking
	sigset_t set = {0};
	if (sigaddset(&set, SIGCHLD) == -1)
	{
		perror("Error in sigaddset!\n");
		exit(EXIT_FAILURE);
	}
	while (1)
	{
		if (pause() == -1)
		{
			if (errno == EINTR)
			{
				if (sigprocmask(SIG_BLOCK, &set, NULL) == -1)
				{
					perror("Error in sigprocmask!\n");
					exit(EXIT_FAILURE);
				}
				//czytanko z siginfo
				if (sigInfo.si_status == SIGSTOP)
				{
//					sleep(1);
					kill(pid, SIGCONT);
				}
				if (sigInfo.si_status == SIGCONT)
				{
					if (clock_gettime(CLOCK_REALTIME, &timeStruct) == -1)
					{
						perror("Error in gettime!\n");
						exit(EXIT_FAILURE);
					}
				}
				if( sigInfo.si_code == CLD_EXITED)
				{

					if (clock_gettime(CLOCK_REALTIME, &timeEnd) == -1)
					{
						perror("Error in timeEnd!\n");
						exit(EXIT_FAILURE);
					}
					break;

				}
				if (sigprocmask(SIG_UNBLOCK, &set, NULL) == -1)
				{
					perror("Error in sigprocmask2!\n");
					exit(EXIT_FAILURE);
				}
			}
		}
	}
	difference = calculateTime(timeStruct, timeEnd);
	printf("Difference: %ld seconds\t%ld nanoseconds\n", difference.tv_sec, difference.tv_nsec);
}

static void sigHandler(int sig)
{
	++counterForChild;
}

void childFunction(struct inputData* data)
{
	sigset_t set = {0};
	if (sigaddset(&set, SIGCONT) == -1)
	{
		perror("Error in sigaddset!\n");
		exit(EXIT_FAILURE);
	}

	siginfo_t info = {0};

	struct sigevent sev;
	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIGSTOP;
	struct timespec timeForTimer = parseTime(data->interval);
	struct itimerspec ts = { .it_interval = timeForTimer, .it_value = timeForTimer };
	timer_t timer;
	if (timer_create(CLOCK_REALTIME, &sev,&timer) == -1)
	{
		perror("Error in timer create!\n");
		exit(EXIT_FAILURE);
	}
	if( timer_settime(timer, 0, &ts, NULL) == -1)
	{
		perror("Error in timer_settime!\n");
		exit(EXIT_FAILURE);
	}
	struct timespec timeToWaitInLoop = { .tv_sec = 0, .tv_nsec = 100000000};
	struct timespec timeRes = {0};
	struct sigaction sa;
	memset(&sa,'\0',sizeof(sa));
	sa.sa_handler = sigHandler;
	if (sigaction(SIGCONT,&sa,NULL) == -1)
	{
		perror("Error in sigaction\n");
		exit(EXIT_FAILURE);
	}
	while (counterForChild < data->numberOfIntervals)
	{
		gotoxy(data->x + 1, data->y + 4);
		time_t t;
		if (time(&t) == -1)
		{
			perror("Error in time!\n");
			exit(EXIT_FAILURE);
		}
		checkSize(data, strlen(ctime(&t)));
		printf("%s%s", greenColor, ctime(&t));
		errno = 0;
		if (nanosleep(&timeToWaitInLoop, &timeRes) == -1)
		{

			if (errno == EINTR)
			{
//				sigwaitinfo(&set,&info);
//				printf("%d\n",info.si_signo);
				nanosleep(&timeRes, NULL);
			}
			else
			{
				perror("Error in nanosleep! dziecko\n");
				exit(EXIT_FAILURE);
			}
		}

	}
	if (timer_delete(timer) == -1)
	{
		perror("Error in timer_delete!\n");
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);

}


static void sigWidthHandler(int sig)
{
	takeWindowSize();
}


void handleChangeTerminalSizeSignal()
{
	struct sigaction sa;
	memset(&sa,'\0',sizeof(sa));
	sa.sa_handler = sigWidthHandler;
	if (sigaction(SIGWINCH,&sa,NULL) == -1)
	{
		perror("Error in sigaction\n");
		exit(EXIT_FAILURE);
	}

}
void signalFunction(int signo, siginfo_t* SI, void* data)
{
	sigInfo = *SI;
}

void parseData(char** argv, struct inputData* data)
{
	sscanf(argv[1], "%f:%d:%d,%d", &data->interval, &data->numberOfIntervals, &data->x, &data->y);
}

struct timespec parseTime(float interval)
{
	struct timespec temp;
	temp.tv_sec = interval/10;
	temp.tv_nsec = (interval - (interval/10)*10)*100000000;
	return temp;
}


void takeWindowSize()
{
	if (ioctl(0, TIOCGWINSZ, &window) == -1)
	{
		perror("Error in getting window size!\n");
		exit(EXIT_FAILURE);
	}
}

void ignoreChild()
{
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_NOCLDWAIT;
	if (sigaction(SIGCHLD, &sa, NULL) == -1)
	{
		perror("Error in sigaction!\n");
		exit(EXIT_FAILURE);
	}
}

void checkSize(struct inputData* data, unsigned int outputLength)
{
	time_t test;
	time(&test);
	if (window.ws_row < data->x+outputLength)
	{
		data->x = 1;
	}
	if (window.ws_col < data->y)
	{
		data->y = 1;
	}
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
