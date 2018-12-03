#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>

/*
 * Format ./client ip port rate
 * rate needs to be in KBps
 */

//#define PORT 5000
#define BUFFSIZE 1536

int main(int argc, char *argv[]){

	/* Variables */
	int sock;
	struct hostent *hp;
	struct sockaddr_in server;
	struct timeval t, start, end;
	char buff[BUFFSIZE];

	int dt1 = 0;
	int sent = 0;
	int startcount = 1;
	double rate = 50000.0; //Default rate in Bps
	long t1, t2;

	if(argc < 4){
		printf("Not enough arguments\n");
		return 1;
	}
	/* Creation of the socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0){
		perror("socket failed");
		return 1;
	}

	hp = gethostbyname(argv[1]);
	if(hp == 0){
		perror("gethostbyname failed");
		close(sock);
		return 1;
	}

	server.sin_family = AF_INET;
	memcpy(&server.sin_addr, hp->h_addr, hp->h_length);
	if(argv[2]){
		server.sin_port = htons(atoi(argv[2]));
	}else{
		server.sin_port = htons(5000); //default port 5000
		printf("Default port\n");
	}

	if(argv[3]){
		rate = atoi(argv[3]) * 1000;
	}

	/* Connection */
	if(connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0){
		perror("connect failed");
		close(sock);
		return 1;
	}

	dt1 = sizeof(buff)*1000000/rate; // microseconds

	/* Infinite loop */
	printf("rate = %f KBps dt = %d us  buff size = %ld\n", rate/1000.0, dt1, sizeof(buff));

	while(1){
		gettimeofday(&t, NULL);
		t1 = t.tv_sec*1000000 + t.tv_usec;
		if(send(sock, buff, sizeof(buff), 0) < 0){
			perror("send failed");
			close(sock);
			return 1;
		}
		while(1){
			if(t2 >= t1 + dt1){
				break;
			}
			gettimeofday(&t, NULL);
			t2 = t.tv_sec*1000000 + t.tv_usec;
		}

	}
	printf("Disconnected\n");
	close(sock);
	return 0;
}