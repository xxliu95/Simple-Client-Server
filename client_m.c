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
 * Format ./client ip max
 */

#define PORT 5000
#define BUFFSIZE 1536

int main (int argc, char *argv[]) {

	/* Variables */
	int max = 1;
	struct hostent *hp;
	struct sockaddr_in server;
	char buff[BUFFSIZE];

	if (argv[2]) {
		max = atoi(argv[2]);
	}
	int i, sock[max];

	hp = gethostbyname(argv[1]);
	if (hp == 0) {
		perror("gethostbyname failed");
		return 1;
	}

	for (i = 0; i < max; i++) {
		/* Creation of sockets */
		sock[i] = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock[i] < 0) {
			perror("socket failed");
			return 1;
		}

		server.sin_family = AF_INET;
		memcpy(&server.sin_addr, hp->h_addr, hp->h_length);
		server.sin_port = htons(PORT);

		/* Connection */
		if (connect(sock[i], (struct sockaddr *) &server, sizeof(server)) < 0) {
			perror("connect failed");
			close(sock[i]);
			return 1;
		}
	}

	/* Infinite loop */
	printf("%d connections to %s\n", max, inet_ntop(AF_INET, &server.sin_addr, buff, sizeof(buff)));

	while (1) {
		for (i = 0; i < max; i++){
			if (send(sock[i], buff, sizeof(buff), 0) < 0) {
				perror("send failed");
				close(sock[i]);
				return 1;
			}
		}
		sleep(1e-3);
	}

	for (i = 0; i < max; i++)
		close(sock[i]);
	return 0;
}
