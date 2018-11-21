#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PORT 5001
#define BUFFSIZE 15360

int main(int argc, char *argv[]){

	int server_fd;
	int client_fd;
	int timeup = 1;
	int rcvd = 0;
	struct sockaddr_in server;
	struct timeval t1, t2;
	char buff[BUFFSIZE];
	double dt = 0.0;
	double rate;

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(server_fd < 0){
		perror("Socket error");
		return 1;
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	if(bind(server_fd, (struct sockaddr *)&server, sizeof(server))){
		perror("bind failed");
		return 1;
	}

	listen(server_fd, 10);

	if((client_fd = accept(server_fd,(struct sockaddr *)NULL, NULL)) < 0){
		perror("accept failed");
		return 1;
	}
	printf("received,rate,dt\n");
	while(1){
		if(timeup){
			gettimeofday(&t1, NULL);
			timeup = 0;
		}
		int rec = recv(client_fd, buff, sizeof(buff), 0);
		if(rec>0){
			rcvd += rec; // counts received bytes
			gettimeofday(&t2, NULL);
			dt = ((t2.tv_sec - t1.tv_sec)*1000000.0 + (t2.tv_usec - t1.tv_usec)); // in microseconds
		}
		if(dt >= 1000000.0){
			rate = rcvd*1000.0/dt;
			//printf("received = %d B rate = %5.5f KBps dt = %7.1f us.\n", rcvd, rate, dt);
			printf("%d,%5.2f,%7.4f\n", rcvd, rate, dt);
			rcvd = 0;
			timeup = 1;
		}
	}
	return 0;
}
