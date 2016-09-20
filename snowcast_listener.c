#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>

int open_client(const char* serverport){
	//same as snowcast_control
	int client_fd;
	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, serverport, &hints, &servinfo);

	if ((client_fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) < 0){
		perror("Socket Error");
		exit(1);
	}
	bind(client_fd, servinfo->ai_addr, servinfo->ai_addrlen);
	freeaddrinfo(servinfo);
	return client_fd;
}

void snowcast_listener(const char* udpport){
	int fd;
	if ((fd = open_client(udpport)) <0){
		printf("Conncection failed \n");
		return;
	}
	struct sockaddr_in  server_address;
	socklen_t server_address_length;

	int buf_len = 1024; //defined by music rate
	char buffer[buf_len];

	struct timespec waittime;
	waittime.tv_sec = 0;
	waittime.tv_nsec = 62500000; //calculated by sec/freq

	int byte = 0;
	while(1){
		byte = recvfrom(fd,buffer,buf_len,0,(struct sockaddr*)&server_address,&server_address_length);
		if (byte < 0){
			printf("Conncection failed \n");
			return;
		}
		if (byte == 0){
			("Conncection failed \n");
			return;
		}
		write(fileno(stdout),buffer,sizeof(buffer));
		nanosleep(&waittime, NULL);
	}
}

int main(int argc, char * argv[]){

	if (argc != 2){
		printf("Correct Input Format: snowcast_listener <udpport> \n");
		exit(0);
	}
	char* udpport = argv[1];

	snowcast_listener(udpport);
	return 0;
}