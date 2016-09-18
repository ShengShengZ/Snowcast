#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>

//will modify into a general datastruct allowing extra saving
struct hello_msg{
	uint8_t type;
	uint16_t number;
};

struct welcome_msg{
	uint8_t type;
	uint16_t number;
};

int open_client(const char* servername, const char* serverport){
	int client_fd;
	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(servername, serverport, &hints, &servinfo);

	if ((client_fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) < 0){
		perror("Socket Error");
		exit(1);
	}
	if (connect(client_fd, servinfo->ai_addr, servinfo->ai_addrlen) <0){
		perror("Connect Error");
		exit(1);
	}
	return client_fd;
}

int set_int8 (unsigned char* buf, uint8_t n){
    *buf = n;
    return 1;
}

int set_int16(unsigned char* buf, uint16_t n){
    n = htons(n);
    memcpy( buf, &n, sizeof(int16_t));
    return 2;
}

int send_hello(int fd, uint16_t udpport){
		struct hello_msg hello;
		hello.type = 0;
		hello.number = udpport;

    int buf_len = sizeof(uint8_t) + sizeof(uint16_t);
    unsigned char buffer[buf_len];

    int off = 0;
    off += set_int8 (buffer, hello.type);
    off += set_int16(buffer+off,hello.number);

    if( send( fd, buffer, buf_len, 0)< 0){
        perror("Error: send():");
        close(fd);
        exit(1);
    }
    return 0;
}

int get_int8 (unsigned char* buffer, uint8_t *number){
    *number = *buffer;
    return 1;
}

int get_int16 (unsigned char* buffer, uint16_t *number){
    memcpy(number, buffer, sizeof(uint16_t));
    *number = ntohs(*number);
    return 2;
}

int get_welcome(unsigned char* buffer, int buf_len, struct welcome_msg* welcome){
    int info = 0;
    info += get_int8(buffer, &welcome->type);
    info += get_int16(buffer + info, &welcome->number);
    return 0;
}

int receive_welcome(int fd){
	struct welcome_msg welcome;
	int buf_len = sizeof(uint8_t) + sizeof(uint16_t);
	unsigned char buffer[buf_len];
	memset(buffer, 0, buf_len);

	int bytes;
	do{
		bytes = recv(fd, buffer, buf_len, 0);
		if (bytes < 0){
			perror("receive error");
			close(fd);
		}
		if (bytes == 0){
			close(fd);
		}
	}while(bytes < buf_len);

	get_welcome(buffer, buf_len, &welcome);
	return welcome.number;
}

void snowcast_control(const char* servername, const char* serverport, int udpport){

	int client_fd;
	int station_number;
	int n_station = -1;

	client_fd = open_client(servername, serverport);

	send_hello(client_fd, udpport);

	while(1){
        n_station = receive_welcome(client_fd);
				if (n_station >= 0){
					break;
				}
	}
	printf("Output the number of stations to prove the controller receive the welcome.\n %d \n", n_station);
	//set station here
}

int main(int argc, char * argv[]){

	if (argc != 4){
		printf("Correct Input Format: snowcast_control <servername> <serverport> <udpport> \n");
		exit(0);
	}

	char *servername;
	char *serverport;
	int udpport;

	servername = argv[1];
	serverport = argv[2];
	udpport =argv[3];

	snowcast_control(servername, serverport, udpport);
	return 0;
}
