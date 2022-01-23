#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>

#define NUMBER_OF_CLOCKS 11
#define NUMBER_OF_ITERATION 100

char* clockNameTable[NUMBER_OF_CLOCKS] = {"CLOCK_REALTIME",
                                      "CLOCK_MONOTONIC",
                                      "CLOCK_PROCESS_CPUTIME_ID",
                                      "CLOCK_THREAD_CPUTIME_ID",
                                      "CLOCK_REALTIME_COARSE",
                                      "CLOCK_MONOTONIC_COARSE",
                                      "CLOCK_MONOTONIC_RAW",
                                      "CLOCK_BOOTTIME",
                                      "CLOCK_REALTIME_ALARM",
                                      "CLOCK_BOOTTIME_ALARM",
                                      "CLOCK_TAI"
									};


clockid_t clockIdTable[NUMBER_OF_CLOCKS] = {CLOCK_REALTIME,
                                      CLOCK_MONOTONIC,
                                      CLOCK_PROCESS_CPUTIME_ID,
                                      CLOCK_THREAD_CPUTIME_ID,
                                      CLOCK_REALTIME_COARSE,
                                      CLOCK_MONOTONIC_COARSE,
                                      CLOCK_MONOTONIC_RAW,
                                      CLOCK_BOOTTIME,
                                      CLOCK_REALTIME_ALARM,
                                      CLOCK_BOOTTIME_ALARM,
                                      CLOCK_TAI
};

struct timespec calculateTime(struct timespec firstTime, struct timespec secondTime);

int main()
{
	struct timespec resTime;
	struct timespec timeDeltaOne;
	struct timespec timeDeltaTwo;

	struct timespec timeToWait = { .tv_sec=2, .tv_nsec =750000000};
	struct timespec timeDifference = {0};
	for (int i=0; i<NUMBER_OF_CLOCKS; ++i)
	{
		if (clock_getres(clockIdTable[i],&resTime) == -1)
		{
			fprintf(stderr,"There is no clock: %s\n", clockNameTable[i]);
			continue;
		}
		else
		{
			printf("Resolution of a clock %s: %ld sec, %ld nsec\n", clockNameTable[i], resTime.tv_sec, resTime.tv_nsec);

			printf("CHECKING DELTA FOR %s without waiting!\n", clockNameTable[i]);
			long seconds = 0;
			long nanoseconds = 0;
			for (int j=0; j<NUMBER_OF_ITERATION; ++j)
			{
				clock_gettime(clockIdTable[i], &timeDeltaOne);
				clock_gettime(clockIdTable[i], &timeDeltaTwo);
				timeDifference = calculateTime(timeDeltaOne, timeDeltaTwo);
				seconds += timeDifference.tv_sec;
				nanoseconds += timeDifference.tv_nsec;

			}
			printf("Time difference in seconds: %ld\n", seconds/NUMBER_OF_ITERATION);
			printf("Time difference in nanoseconds: %ld\n", nanoseconds/NUMBER_OF_ITERATION);

			printf("CHECKING DELTA FOR %s with waiting!\n", clockNameTable[i]);

			clock_gettime(clockIdTable[i], &timeDeltaOne);
			nanosleep(&timeToWait, NULL);
			clock_gettime(clockIdTable[i], &timeDeltaTwo);
			timeDifference = calculateTime(timeDeltaOne, timeDeltaTwo);
			printf("Time difference in seconds: %ld\n", timeDifference.tv_sec);
			printf("Time difference in nanoseconds: %ld\n", timeDifference.tv_nsec);
			printf("\n\n");

		}
	}

	return 0;
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