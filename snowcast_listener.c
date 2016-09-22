#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h> 
#include <unistd.h>

int main(int argc, char * argv[]){

	if (argc != 2){
		printf("Correct Input Format: snowcast_listener <udpport> \n");
		return 0;
	}

    int clientfd;
    
    //hints part follows stackoverflow
    struct addrinfo hints, *addr_info;
    memset( &hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
	getaddrinfo( NULL, argv[1], &hints, &addr_info);
	
	clientfd = socket(addr_info->ai_family,addr_info->ai_socktype,addr_info->ai_protocol);
   	bind(clientfd,addr_info->ai_addr, addr_info->ai_addrlen);
    freeaddrinfo(addr_info);

	struct sockaddr_in  server_address;
	socklen_t server_address_length;

	int buf_len = 1024; //defined by music rate
	char buffer[buf_len];

	struct timespec waittime;
	waittime.tv_sec = 0;
	waittime.tv_nsec = 1000000000/16; //calculated by nanosec/freq 

	while(1){
		int bytes = recvfrom(clientfd,buffer,buf_len,0,(struct sockaddr*)&server_address,&server_address_length);
		if (bytes == 0){
			printf("Conncection closed \n");
			continue;
		}
		write(fileno(stdout),buffer,sizeof(buffer));
		nanosleep(&waittime, NULL);
	}	
	return 0;
}
