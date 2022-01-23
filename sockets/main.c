#define _GNU_SOURCE
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <memory.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "CircularBuffer.h"
#include <sys/types.h>
#include <poll.h>
#include <sys/timerfd.h>
//#include <sys/wait.h>

#define BACKLOG 5
#define BUF_SIZE 100
#define PARENT_POLL_SIZE 2
#define CHILD_POLL_SIZE 3

struct inputData
{
	char address[256];
	uint16_t port;
	int simultaneousConnections;
	float timestamp;

};

void readData(int argc, char** argv, struct inputData* data);
float parseFloat(const char* buff);
int parseInt(const char* buff);
int parseUint16(const char* buff);
int createServer(char* address, uint16_t port);
void handleConnections(struct inputData* data, struct pollfd* pollFd, int serverSocket);
void handleSocket(int clientSocket, struct pollfd* pollFd, struct circularBuffer* buffer, float timestamp, int serverSocket);
void prepareParentPoll(struct pollfd* pollFd, int serverSocket);
void childFunction(int clientSocket, int parentSocket, float timestamp);
int createTimerForChildren(float timestamp);
void prepareChildrenPoll(struct pollfd* pollFd, int timerFd, int clientSocket, int parentSocket);
struct timespec parseTime(float interval);
void getTimeAndWrite(int socketToWrite);
void parseTimeToStringAndSend(struct timespec time, int clientSocket);
void ignoreChildren();

/*
 * Nie wiem dlaczego wyswietlaja sie te artefakty po zakonczeniu wszystkich polaczen
 */

int main(int argc, char* argv[])
{
	ignoreChildren();
	printf("%d\n", getpid());
	signal(SIGCHLD, SIG_IGN);
	struct inputData data = { .address = "127.0.0.1", .port = 5566, .simultaneousConnections=5 };
	readData(argc, argv, &data);
	printf("Address: %s\nPort: %d\nMax number of connections: %d\nTimestamp: %f\n", data.address, data.port, data.simultaneousConnections, data.timestamp);
	int serverSocket = createServer(data.address,data.port);
	struct pollfd pollFd[PARENT_POLL_SIZE];
	handleConnections(&data, pollFd, serverSocket);
//	while (wait(NULL) > 0);
	return 0;

}

void handleConnections(struct inputData* data, struct pollfd* pollFd, int serverSocket)
{
	int time = -1; // we want to wake up only when poll find desc
	int newConnection = -1; // temp variable to store descrptor for our new connection
	struct circularBuffer* buffer = create(data->simultaneousConnections); // we are creating our circular buffer structure
	char buff[BUF_SIZE];
	prepareParentPoll(pollFd, serverSocket); // preparing parent poll structure
	ssize_t numRead;
	while (poll(pollFd,2,time) != -1)
	{
		if (pollFd[0].revents & POLLIN)
		{
			printf("New connection!\n");
			newConnection = accept4(serverSocket, NULL, NULL, SOCK_NONBLOCK);
			if (buffer->currentSize == buffer->size)
			{
				perror("Reached max connection limits!\n");
				if (close(newConnection) == -1)
				{
					perror("Error in close newConnection!\n");
					exit(EXIT_FAILURE);
				}
				continue;
			}
			handleSocket(newConnection, pollFd, buffer, data->timestamp, serverSocket);// clean code XD
		}
		if (pollFd[1].revents & POLLHUP) // our child is dead
		{
			pop(buffer);
			while (read(pollFd[1].fd,buff,BUF_SIZE) > 0); // we are cleaning buffer? idk if we need this
//			if (shutdown(pollFd[1].fd, SHUT_RDWR) == -1)
//			{
//				perror("Error in shutdown!\n");
//				exit(EXIT_FAILURE);
//			}
			if (close(pollFd[1].fd) == -1) // we still need to close to free resources
			{
				perror("Error in close!\n");
				exit(EXIT_FAILURE);
			}
			pollFd[1].fd = buffer->buffer[buffer->first]; // we are taking next child to cooperate
			getTimeAndWrite(pollFd[1].fd);
			printf("\nUser deleted!\n");
			continue;
		}
		if (pollFd[1].revents & POLLIN) // our child is asking us for new timespec struct
		{
			numRead = read(pollFd[1].fd,buff,BUF_SIZE);
			if (numRead == -1)
			{
				perror("Error in read!\n");
				exit(EXIT_FAILURE);
			}
			getTimeAndWrite(pollFd[1].fd);
		}
	}
	destroy(buffer);
}

void handleSocket(int clientSocket, struct pollfd* pollFd, struct circularBuffer* buffer, float timestamp, int serverSocket)
{
	int sockfd[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) == -1)
	{
		perror("Error in socketpair!\n");
		exit(EXIT_FAILURE);
	}

	pid_t pid = -1;
	pid = fork();

	switch(pid)
	{
		case -1:
		{
			perror("Error in fork!\n");
			exit(EXIT_FAILURE);
		}
		case 0:
		{
			close(sockfd[1]); // bombeleks descriptor
			close(serverSocket);
			childFunction(clientSocket, sockfd[0], timestamp);
			exit(EXIT_SUCCESS);
		}
		default:
		{
			close(sockfd[0]);
			close(clientSocket);
			break;
		}
	}
	if (buffer->currentSize == 0)
	{
		pollFd[1].fd = sockfd[1];
		getTimeAndWrite(pollFd[1].fd);
	}
	if (add(buffer,sockfd[1]) == -1)
	{
		perror("Reached maximum number of elements in circular buffer!\nDid you forget to pop element?\n");
		exit(EXIT_FAILURE);
	}
}
int createServer(char* address, uint16_t port)
{
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1)
	{
		perror("Error in creating socket!\n");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	int res = inet_aton(address, &addr.sin_addr);
	if (res == -1)
	{
		perror("Error in inet_aton!\n");
		exit(EXIT_FAILURE);
	}
	if (bind(sfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) == -1)
	{
		perror("Error in binding socket!\n");
		exit(EXIT_FAILURE);
	}
	if (listen(sfd, BACKLOG) == -1)
	{
		perror("Error in listen!\n");
		exit(EXIT_FAILURE);
	}
	return sfd;
}

void prepareParentPoll(struct pollfd* pollFd, int serverSocket)
{
	pollFd[0].fd = serverSocket;
	pollFd[0].events = POLLIN;
	pollFd[0].revents = 0;

	pollFd[1].fd = -1;
	pollFd[1].events = POLLIN;
	pollFd[1].revents = 0;

}

void childFunction(int clientSocket, int parentSocket, float timestamp) // this is the oldest child function
{

	struct timespec timeToSend = {0};
	ssize_t numRead = 0;
	numRead = read(parentSocket,&timeToSend, sizeof(struct timespec));
	if (numRead == -1)
	{
		perror("Error in reading from parent socket!\n");
		exit(EXIT_FAILURE);
	}
	char buf[BUF_SIZE] = {0};

	struct pollfd pollFd[CHILD_POLL_SIZE];
	int timerFd = createTimerForChildren(timestamp); // we are creating timer for child
	int time = -1;
	prepareChildrenPoll(pollFd, timerFd, clientSocket, parentSocket);
	while (poll(pollFd,3,time) != -1)
	{
		if (pollFd[0].revents & POLLIN)
		{
			uint64_t dataFromTimer = 0;
			numRead = read(pollFd[0].fd, &dataFromTimer, sizeof(uint64_t));
			if (numRead == -1)
			{
				perror("Error in reading from timerfd desc!\n");
				exit(EXIT_FAILURE);
			}
			printf("We are sending data to our client!\n");
			parseTimeToStringAndSend(timeToSend, clientSocket);

		}
		if (pollFd[1].revents & POLLIN)
		{
			printf("We received message from our client!\n");
			while ((numRead = read(clientSocket,buf,BUF_SIZE)) > 0)
			{
				if (write(parentSocket, buf, numRead) != numRead)
				{
					perror("Error in writing client message to parent!\n");
					exit(EXIT_FAILURE);
				}
			}
			if (numRead == 0)
			{
				close(clientSocket);
//				printf("Our client END connection!\n");
				break;
			}
		}
		if (pollFd[2].revents & POLLIN)
		{
			numRead = read(parentSocket,&timeToSend, sizeof(struct timespec));
			if (numRead != sizeof(struct timespec))
			{
				perror("Error in reading from parent!\n");
			}
		}
	}
}

void prepareChildrenPoll(struct pollfd* pollFd, int timerFd, int clientSocket, int parentSocket)
{
	pollFd[0].fd = timerFd;
	pollFd[0].events = POLLIN;
	pollFd[0].revents = 0;

	pollFd[1].fd = clientSocket;
	pollFd[1].events = POLLIN;
	pollFd[1].revents = 0;

	pollFd[2].fd = parentSocket;
	pollFd[2].events = POLLIN;
	pollFd[2].revents = 0;
}

int createTimerForChildren(float timestamp)
{
	int timerFd = timerfd_create(CLOCK_REALTIME,0);
	if (timerFd == -1)
	{
		perror("Error in timer_fd create!\n");
		exit(EXIT_FAILURE);
	}
	struct timespec timeForTimer = parseTime(timestamp);
	struct itimerspec ts = { .it_interval = timeForTimer, .it_value = timeForTimer };
	if (timerfd_settime(timerFd, 0, &ts, NULL) == -1)
	{
		perror("Error in timerfd_settime!\n");
		exit(EXIT_FAILURE);
	}
	return timerFd;
}

void getTimeAndWrite(int socketToWrite)
{
	struct timespec timeToSend = {0};
	if (clock_gettime(CLOCK_REALTIME, &timeToSend) == -1) //CLOCK_PROCESS_CPUTIME_ID dont work
	{
		perror("Error in clock_gettime!\n");
		exit(EXIT_FAILURE);
	}
	errno = 0;
	int bytesWrote = write(socketToWrite, &timeToSend, sizeof(struct timespec));
	if (bytesWrote == -1)
	{
		if (errno == EBADF)
		{
			printf("Client end connection before his turn came!\n");
			return;
		}
		perror("Error in write!\n");
		exit(EXIT_FAILURE);
	}
	if (bytesWrote != sizeof(struct timespec))
	{
		perror("Error in write!\n");
		exit(EXIT_FAILURE);
	}
}
void readData(int argc, char **argv, struct inputData *data)
{
	int opt = 0;
	while ((opt = getopt(argc, argv, "a:p:c:d:")) != -1)
	{
		switch(opt)
		{
			case 'a':
			{
				memcpy(data->address,optarg, strlen(optarg)+1);
				break;
			}
			case 'p':
			{
				data->port = parseUint16(optarg);
				break;
			}
			case 'c':
			{
				data->simultaneousConnections = parseInt(optarg);
				break;
			}
			case 'd':
			{
				data->timestamp = parseFloat(optarg);
				break;
			}
			default:
			{
				perror("Error while using getopt!\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}
float parseFloat(const char* buff)
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
int parseUint16(const char* buff)
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
	if (val < 0 || val > UINT16_MAX)
	{
		perror("Too large or negative number for port!\n");
		exit(EXIT_FAILURE);
	}
	return val;
}

struct timespec parseTime(float interval)
{
	struct timespec temp;
	temp.tv_sec = interval;
	temp.tv_nsec = (interval - (interval/10)*10)*100000000;
	return temp;
}

void parseTimeToStringAndSend(struct timespec time, int clientSocket)
{
	struct tm* my_tm = localtime(&time.tv_sec);
	char buff[BUF_SIZE];

	sprintf(buff,"%d-%d-%d %d:%d:%d:%ld\n", (1900+my_tm->tm_year), (1+my_tm->tm_mon), my_tm->tm_mday, my_tm->tm_hour, my_tm->tm_min, my_tm->tm_sec, time.tv_nsec/1000000);
	ssize_t numWrote = write(clientSocket, buff, strlen(buff));
	if (numWrote != strlen(buff))
	{
		perror("Error in write to client!\n");
		exit(EXIT_FAILURE);
	}
}

void ignoreChildren()
{
	if (signal(SIGCHLD, SIG_IGN) == SIG_ERR)
	{
		perror("Error in setting signal!\n");
		exit(EXIT_FAILURE);
	}
}
