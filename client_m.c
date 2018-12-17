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
#define BUFFSIZE 1536

int main (int argc, char *argv[]) {

	/* Variables */
	struct hostent *hp;
	struct sockaddr_in client;
	struct sockaddr_in server;
	char buff[BUFFSIZE];

	int max = 1;
	if (argv[2]) {
		max = atoi(argv[2]);
	}
	int i;
	int sock[max], rec[max], rcvd[max];
	struct timeval t1[max], t2[max];
	double dt[max];
	int timeup[max];
	double rate[max];

	/* Initialization of variables */
	for (i = 0; i < max; i++) {
		rcvd[0] = 0;
		dt[i] = 0.0;
		timeup[i] = 1;
		rate[i] = 0.0;
	}

	hp = gethostbyname(argv[1]);
	if (hp == 0) {
		perror("gethostbyname failed");
		exit(1);
	}

	memset(&server, '0', sizeof(server));

	server.sin_family = AF_INET;
	memcpy(&server.sin_addr, hp->h_addr, hp->h_length);
	server.sin_port = htons(PORT);

	for (i = 0; i < max; i++) {
		/* Creation of the socket */
		sock[i] = socket(AF_INET, SOCK_STREAM, 0);
		if (sock[i] < 0) {
			perror("socket failed");
			exit(1);
		}
		/* Connection */
		if (connect(sock[i], (struct sockaddr *) &server, sizeof(server)) < 0) {
			perror("connect failed");
			close(sock[i]);
			exit(1);
		}
	}

	/* Infinite loop */
	while (1) {
		for (i = 0; i < max; i++) {
			if (timeup[i]) {
				gettimeofday(&t1[i], NULL);
				timeup[i] = 0;
				rcvd[i] = 0;
			}
			rec[i] = recv(sock[i], buff,BUFFSIZE, 0);
			if (rec[i] > 0) {
				rcvd[i] += rec[i];
				gettimeofday(&t2[i], NULL);
				dt[i] = ((t2[i].tv_sec - t1[i].tv_sec)*1000000.0 + (t2[i].tv_usec - t1[i].tv_usec)); // in microseconds
			}
			if (dt[i] >= 1000000.0) {
				rate[i] = rcvd[i]*1000.0/dt[i];

				printf("handle: %d %d Bytes from %s rate = %5.5f KBps dt = %7.1f us.\n",
					i+10, rcvd[i], inet_ntop(AF_INET, &server.sin_addr, buff, sizeof(buff)),
					rate[i], dt[i]);

				timeup[i] = 1;
			}

		}
	}
	printf("Disconnected\n");
	for (i = 0; i < max; i++) {
		close(sock[i]);
	}

	return 0;
}
