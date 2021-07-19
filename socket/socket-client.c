#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define BILLION  1000000000.0

int main(int argc, char *argv[])
{
	if (argc != 6){
		printf("run program: \n./socket-client -h listenaddress -p portnumber text\n");
		return 1;
	}
	if (strcmp(argv[1],"-h") != 0){
		printf("please set -h and write host after that.\n");
		return 1;
	}
	if (strcmp(argv[3],"-p") != 0){
		printf("please set -p and write port after that.\n");
		return 1;
	}
	struct timespec start, end;


	const int PORT = atoi(argv[4]);

	int socket_fd, valread;
	struct sockaddr_in address;

	char buffer[8192];
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("socket creation error \n");
		return -1;
	}

	address.sin_family = AF_INET;
	address.sin_port = htons(PORT);
	
	// Convert IPv4 and IPv6 addresses from text to binary form
	if(inet_pton(AF_INET, argv[2], &address.sin_addr)<=0)
	{
		printf("Invalid address/ Address not supported \n");
		return -1;
	}

	if (connect(socket_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		printf("Connection Failed \n");
		printf("please run server first\n");
		return -1;
	}
	clock_gettime(CLOCK_REALTIME, &start);
	send(socket_fd, argv[5], strlen(argv[5]), 0);
	printf("%s sent to server\n", argv[5]);

	while(valread = read(socket_fd, buffer, 1024))
	{
		clock_gettime(CLOCK_REALTIME, &end);
		printf("from server: %s\n", buffer);
		double time_spent = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / BILLION;
		printf("spent time: %f sec.\n", time_spent);
		puts("");
		break;
	}
	return 0;
}
