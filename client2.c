#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PORT 5001
#define BUFFSIZE 1536

int main(int argc, char *argv[]){

	int sock;
	struct sockaddr_in server;
	struct hostent *hp;
	struct timeval t, start, end;
	char buff[BUFFSIZE];
	int dt1 = 0;
	int sent = 0;
	int startcount = 1;
	double rate = 20000.0; //Bps
	long t1, t2;

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
	server.sin_port = htons(PORT);

	if(connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0){
		perror("connect failed");
		close(sock);
		return 1;
	}

	dt1 = sizeof(buff)*1000000/rate; // microseconds

	printf("dt = %d us  buff size = %ld\n", dt1, sizeof(buff));

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
	close(sock);
	return 0;
}
