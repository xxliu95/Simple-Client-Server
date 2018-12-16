#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFSIZE 100000
#define MAXCONNECTIONS 5
#define PORT 5000

void *connection_handler (void *data);

struct args {
	struct sockaddr_in client;
	int client_fd;
};

int main (int argc, char *argv[]) {

	/* Variables */
	int server_fd;
	int client_fd;
	struct sockaddr_in server;
	struct sockaddr_in client;
	pthread_t threads[MAXCONNECTIONS];
	char buff[BUFFSIZE];
	int rc;
	long t;

	/* Creation of sockets */
	server_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); // ipv4 using TCP
	if (server_fd < 0) {
		perror("Socket error");
		return 1;
	}

	memset(&server,0,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	/* Bind */
	if (bind(server_fd, (struct sockaddr *)&server, sizeof(server))) {
		perror("bind failed");
		return 1;
	}

	/* Listen */
	listen(server_fd, MAXCONNECTIONS);

	printf("ip,port,received,rate,dt\n");

	while (1) {

		pthread_t thread_id;
		int client_len;

		/* Accept */
		client_len = sizeof(client);
		if ((client_fd = accept(server_fd, (struct sockaddr *)(&client),
			(socklen_t *)(&client_len))) < 0) {
			perror("accept failed");
			return 1;
		} else {
			struct args *args = malloc(sizeof(struct args));
			args->client = client;
			args->client_fd = client_fd;

			if (pthread_create(&thread_id, NULL, (void *)(&connection_handler), args) < 0) {
				perror("thread creation failed");
				return 1;
			}
		}
	}
	return 0;
}

/*
 * Every thread handles one connection and
 * calculates the rate that it receives data
 */
void *connection_handler (void *data) {

	struct args *args = (struct args *)data;
	int client_fd = args->client_fd;

	struct sockaddr_in client = args->client;
	struct timeval t1, t2;
	char buff[BUFFSIZE];
	double dt = 0.0;
	double rate;
	int timeup = 1;
	int rcvd = 0;

	struct tcp_info tcp_info;
	int tcp_info_length;

	while (client_fd) {

		if (timeup) {
			gettimeofday(&t1, NULL);
			timeup = 0;
		}
		int rec = recv(client_fd, buff, sizeof(buff), 0);
		if (rec>0) {
			rcvd += rec; // counts received bytes
			gettimeofday(&t2, NULL);
			dt = ((t2.tv_sec - t1.tv_sec)*1000000.0 + (t2.tv_usec - t1.tv_usec)); // in microseconds
		}
		if (dt >= 1000000.0) {
			rate = rcvd*1000.0/dt;
			printf("src %s:%d received = %d B rate = %5.5f KBps dt = %7.1f us.\n",
				inet_ntop(AF_INET, &client.sin_addr, buff, sizeof(buff)),
				ntohs(client.sin_port),
				rcvd, rate, dt);
			/*
			printf("%s,%d,%d,%5.5f,%7.1f\n",
				inet_ntop(AF_INET, &client.sin_addr, buff, sizeof(buff)),
				ntohs(client.sin_port),
				rcvd, rate, dt);
			*/

			rcvd = 0;
			timeup = 1;
		}

	}
	printf("Client %d Disconnected\n",client_fd);
	pthread_exit(NULL);
}
