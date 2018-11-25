#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>

#define BUFFSIZE 15360
#define MAXCONNECTIONS 5
#define PORT 5000

void *connection_handler(void *client_fd);

int main(int argc, char *argv[]){

	/* Variables */
	int server_fd;
	int client_fd;
	struct sockaddr_in server;
	struct sockaddr_in client;
	pthread_t threads[MAXCONNECTIONS];
	int rc;
	long t;

	/* Creation of sockets */
	server_fd = socket(AF_INET, SOCK_STREAM, 0); // ipv4 using TCP
	if(server_fd < 0){
		perror("Socket error");
		return 1;
	}

	memset(&server,0,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	/* Bind */
	if(bind(server_fd, (struct sockaddr *)&server, sizeof(server))){
		perror("bind failed");
		return 1;
	}

	/* Listen */
	listen(server_fd, MAXCONNECTIONS);

	//printf("received,rate,dt\n");

	while(1){
		pthread_t thread_id;
		int client_len;

		/* Accept */
		client_len = sizeof(client);
		if((client_fd = accept(server_fd,(struct sockaddr *)(&client), (socklen_t *)(&client_len))) < 0){
			perror("accept failed");
			return 1;
		}
		if(pthread_create(&thread_id,NULL,(void *)(&connection_handler),(void *)(&client_fd)) < 0){
			perror("thread creation failed");
			return 1;
		}
	}
	return 0;
}

/*
 * Every thread handles one connection and
 * calculates the rate that it receives data
 */
void *connection_handler(void *client_fd){

	int fd = *((int *)client_fd);
	struct timeval t1, t2;
	char buff[BUFFSIZE];
	double dt = 0.0;
	double rate;
	int timeup = 1;
	int rcvd = 0;

	while(client_fd){
		if(timeup){
			gettimeofday(&t1, NULL);
			timeup = 0;
		}
		int rec = recv(fd, buff, sizeof(buff), 0);
		if(rec>0){
			rcvd += rec; // counts received bytes
			gettimeofday(&t2, NULL);
			dt = ((t2.tv_sec - t1.tv_sec)*1000000.0 + (t2.tv_usec - t1.tv_usec)); // in microseconds
		}
		if(dt >= 1000000.0){
			rate = rcvd*1000.0/dt;
			printf("client = %d received = %d B rate = %5.5f KBps dt = %7.1f us.\n", fd, rcvd, rate, dt);
			//printf("%d,%5.2f,%7.4f\n", rcvd, rate, dt);
			rcvd = 0;
			timeup = 1;
		}
	}
	printf("Client %d Disconnected\n",fd);
	pthread_exit(NULL);
}
