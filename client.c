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
 * Format ./client ip
 */

#define PORT 5000
#define BUFFSIZE 1024

int main (int argc, char *argv[]) {

	/* Variables */
	int sock;
	struct hostent *hp;
	struct sockaddr_in client;
	struct sockaddr_in server;
	struct timeval t, start, end;
	char buff[BUFFSIZE];

	struct timeval t1, t2;
	double dt = 0.0;
	double rate;
	int rec, rcvd = 0;

	/* Creation of the socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket failed");
		exit(1);
	}

	hp = gethostbyname(argv[1]);
	if (hp == 0) {
		perror("gethostbyname failed");
		close(sock);
		exit(1);
	}

	memset(&server, '0', sizeof(server));

	server.sin_family = AF_INET;
	memcpy(&server.sin_addr, hp->h_addr, hp->h_length);
	server.sin_port = htons(PORT);

	/* Connection */
	if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
		perror("connect failed");
		close(sock);
		exit(1);
	}

	if (send(sock, buff, sizeof(buff), 0) < 0) {
		perror("send failed");
		close(sock);
		exit(1);
	}

	/* Infinite loop */

	while (1) {

		gettimeofday(&t1, NULL);

		again:
		rec = recv(sock, buff,BUFFSIZE, 0);
		if (rec > 0) {
			rcvd += rec;
			gettimeofday(&t2, NULL);
			dt = ((t2.tv_sec - t1.tv_sec)*1000000.0 + (t2.tv_usec - t1.tv_usec)); // in microseconds
			//printf("%7.1f\n", dt);
		}
		if (dt >= 1000000.0) {
			rate = rcvd*1000.0/dt;

			printf("%d Bytes from %s rate = %5.5f KBps dt = %7.1f us.\n",
				rcvd, inet_ntop(AF_INET, &server.sin_addr, buff, sizeof(buff)),
				rate, dt);

			rcvd = 0;
		} else {
			goto again;
		}
	}
	printf("Disconnected\n");
	close(sock);
	return 0;
}
