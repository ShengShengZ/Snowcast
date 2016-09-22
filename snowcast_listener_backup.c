#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>

int open_client(const char* port){

    int clientfd;

    int ai_err;
    struct addrinfo hints, *info;

    memset( &hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if( (ai_err = getaddrinfo( NULL, port, &hints, &info)) != 0){
        fprintf(stderr, "Error: getaddrinfo: %s\n", 
                gai_strerror(ai_err));
        return -1;
    }

    if( (clientfd = socket( info->ai_family, 
                            info->ai_socktype,
                            info->ai_protocol) ) < 0){
        perror("Error: socket():");
        return -1;
    }

    if( bind(clientfd, info->ai_addr, info->ai_addrlen) < 0){
        perror("Error: bind():");
        return -1;
    }

    freeaddrinfo(info);
    return clientfd;
}

void snowcast_listener(const char* udpport){
	int fd;
	if ((fd = open_client(udpport)) <0){
		printf("Open Client failed \n");
		exit(1);
	}
	struct sockaddr_in  server_address;
	socklen_t server_address_length;

	int buf_len = 1024; //defined by music rate
	char buffer[buf_len];

	struct timespec waittime;
	waittime.tv_sec = 0;
	waittime.tv_nsec = 1000000000/16; //calculated by sec/freq

	while(1){
		int bytes = recvfrom(fd,buffer,buf_len,0,(struct sockaddr*)&server_address,&server_address_length);
		if (bytes < 0){
			printf("Receive failed \n");
			exit(1);
		}
		if (bytes == 0){
			("Conncection closed \n");
			continue;
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
