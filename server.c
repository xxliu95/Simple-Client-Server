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

#define BUFFSIZE 1024
#define MAXCONNECTIONS 100
#define PORT 5000

void *connection_handler (void *data);

struct args {
	struct sockaddr_in client;
	int client_fd;
	int handle;
	char iface[10];
};

int main (int argc, char *argv[])
{
	/* Variables */
	int server_fd;
	int client_fd;
	struct sockaddr_in server;
	struct sockaddr_in client;
	pthread_t threads[MAXCONNECTIONS];
	char buff[BUFFSIZE];

	int handle = 10;
	//char iface[10] = "eth0";
	char iface[10] = "lo";

	/* Delete previous qdisc */
	char command[200];
	sprintf(command, "tc qdisc del dev %s root 2>/dev/null", iface);
	system(command);

	/* Creation of sockets */
	server_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); // ipv4 using TCP
	if (server_fd < 0) {
		perror("Socket error");
		exit(1);
	}

	memset(&server,0,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	/* Bind */
	if (bind(server_fd, (struct sockaddr *)&server, sizeof(server))) {
		perror("bind failed");
		exit(1);
	}

	/* Listen */
	listen(server_fd, MAXCONNECTIONS);

	/* Script */

	if (argv[1]) {
		strcpy(iface, argv[1]);
	}
	sprintf(command, "tc qdisc add dev %s root handle 1: htb && \
			tc class add dev %s parent 1: classid 1:1 htb rate 1Gbps ceil 1Gbps",
			iface, iface);
	system(command);
	//printf("%s\n", command);


	while (1) {

		pthread_t thread_id;
		int client_len;

		/* Accept */
		client_len = sizeof(client);
		if ((client_fd = accept(server_fd, (struct sockaddr *)(&client),
			(socklen_t *)(&client_len))) < 0) {
			perror("accept failed");
			exit(1);
		} else {
			struct args *args = malloc(sizeof(struct args));

			if (read(client_fd,buff,BUFFSIZE) >= 0) {

				char req_rate [10];
				sprintf(req_rate,"%s",buff);
				sprintf(args->iface,"%s",iface);
				args->handle = handle;
				args->client = client;
				args->client_fd = client_fd;

				sprintf(command, "tc class add dev %s parent 1:1 classid 1:%d htb rate %s ceil %s &&\
				 		tc filter add dev %s protocol ip parent 1:0 prio 1 u32 \
				 		match ip dst %s match ip dport %d 0xffff flowid 1:%d",
						iface, handle, req_rate, req_rate, iface,
						inet_ntop(AF_INET, &client.sin_addr, buff, sizeof(buff)),
					 	ntohs(client.sin_port), handle);
				system(command);
				//printf("%s\n", command);

				printf("Client %s:%d Connected. Rate %s handle 1:%d\n",
					inet_ntop(AF_INET, &client.sin_addr, buff, sizeof(buff)),
					ntohs(client.sin_port),
					req_rate, handle);

				handle++;

				if (pthread_create(&thread_id, NULL, (void *)(&connection_handler), args) < 0) {
					perror("thread creation failed");
					exit(1);
				}
			}
		}
	}
	return 0;
}

/*
 * Every thread handles one connection
 */
void *connection_handler (void *data)
{
	pthread_detach(pthread_self());

	struct args *args = (struct args *)data;
	int client_fd = args->client_fd;
	int handle = args->handle;
	char iface[10];
	sprintf(iface, "%s", args->iface);
	struct sockaddr_in client = args->client;

	char command[200];
	char buff[BUFFSIZE];

	free(data);

	while (client_fd > 0) {
		if (send(client_fd, buff, sizeof(buff), MSG_NOSIGNAL) < 0) {
			printf("Client %s:%d Disconnected\n",inet_ntop(AF_INET,
				&client.sin_addr, buff, sizeof(buff)), ntohs(client.sin_port));

			sprintf(command, "tc filter del dev %s protocol ip parent 1:0 prio 1 u32 \
					match ip dst %s match ip dport %d 0xffff flowid 1:%d && \
					tc class del dev %s parent 1:1 classid 1:%d",
					iface, inet_ntop(AF_INET, &client.sin_addr, buff, sizeof(buff)),
					ntohs(client.sin_port), handle, iface, handle);
			printf("%s\n", command);
			
			close(client_fd);
			break;
		}
	}
	close(client_fd);
	pthread_exit(NULL);
	return 0;
}
