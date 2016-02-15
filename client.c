#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int easyConnect(const char* domain_or_ip, const char* port_string)
{
	struct addrinfo hints, *servinfo, *p;
	int rv, sockfd;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if((rv = getaddrinfo(domain_or_ip, port_string, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next)
	{
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			perror("client: socket");
			continue;
		}

		if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			perror("client: connect");
			continue;
		}
		break;
	}

	if(p == NULL)
	{
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}
	freeaddrinfo(servinfo); // all done with this structure
	
	return sockfd;
}

int main(int argc, char* argv[])
{
	if(argc < 3)
	{
		fprintf(stderr, "Usage: %s server_address server_port\n", argv[0]);
		exit(1);
	}

	//e.g. easyConnect("www.illinois.edu", "80");
	int socket_to_server = easyConnect(argv[1], argv[2]);

	char buffer[16];
	strcpy(buffer, "I'm the client!");

	if(send(socket_to_server, buffer, 15, 0) == -1)
	{
		perror("send");
		exit(1);
	}

	memset(buffer, 0, 16);
	int bytes_received;
	if((bytes_received = recv(socket_to_server, buffer, 15, 0)) == -1)
	{
		perror("recv");
		exit(1);
	}

	printf("Received from server: %s\n", buffer);

	close(socket_to_server);
	return 0;
}
