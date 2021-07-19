#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>


int client_number;
pthread_mutex_t mutex;

void *connection_handler(void *);


int main(int argc, char *argv[])
{
	if (argc != 5){
		printf("run program: \n./socket-server -h listenaddress -p portnumber\n");
		return 1;
	}
	if (strcmp(argv[1],"-h") != 0){
		printf("please set -h and write host after that.");
		return 1;
	}
	if (strcmp(argv[3],"-p") != 0){
		printf("please set -p and write port after that.");
		return 1;
	}

	const int PORT = atoi(argv[4]);
	int server_fd, new_socket, *new_sock;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[8192];
	
	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	
	// Forcefully attaching socket to the port
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(argv[2]);
	address.sin_port = htons( PORT );
	
	// Forcefully attaching socket to the port 8080
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 3) == -1)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
    puts("Waiting for incoming connections...");

	while( new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))
	{
		pthread_mutex_lock(&mutex);
		client_number++;
		pthread_mutex_unlock(&mutex);

		printf("Connection accepted. %d client\n", client_number);
		printf("socket fd: %d, ip: %s, port: %d\n", new_socket, inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

		
		pthread_t socket_thread;
		new_sock = malloc(1);
		*new_sock = new_socket;
		
		pthread_create(&socket_thread, NULL, connection_handler, (void*) new_sock);
	}
	
	if (new_socket < 0)
	{
		perror("accept failed");
		return 1;
	}
}


void *connection_handler(void *socket_desc)
{
	int sock = *(int*)socket_desc;
	int read_size;
	char *message , client_message[8192];
	
	read_size = read( sock , client_message, 8192);
	client_message[read_size] = '\0';
	printf("from client: %s\n", client_message );
	send(sock , "message received." , strlen("I've recieved your message") , 1);
	puts("client disconnected\n------------\n");
	
	pthread_mutex_lock(&mutex);
	client_number--;
	pthread_mutex_unlock(&mutex);

	free(socket_desc);
	
	return 0;
}