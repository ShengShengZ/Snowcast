#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>

//will modify into a general datastruct allowing extra saving
struct client_msg{
	uint8_t type;
	uint16_t number;
};

struct welcome_msg{
	uint8_t type;
	uint16_t number;
};

struct song_msg{
    uint8_t type;
    uint8_t strsize;
    char* string;
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

void send_hello(int fd, uint16_t udpport){
	struct client_msg hello;
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
}

void send_set_station(int fd, uint16_t station_number){
	struct client_msg set_station;
	set_station.type = 1;
	set_station.number = station_number;

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

int get_str(unsigned char* bufer, int buf_len, char* str, int len){
    if( bufl_en < len ){
        memcpy(str, buffer, buf_len);
        return buf_len;
    }else{
        memcpy(str, bufer, len);
        return len;
    }
}

int get_welcome(unsigned char* buffer, int buf_len, struct welcome_msg* welcome){
    int info = 0;
    info += get_int8(buffer, &welcome->type);
    if (song_msg->type != 1){
    	perror("receive error");
    }    
    info += get_int16(buffer + info, &welcome->number);
    //error return -1
    return 0;
}


int get_msg_str(buffer, buflen, struct song_msg* message, &stroff){
	int info = 0;
    info += get_int8(buffer, &message->type);
    if (message->type != 1){
    	perror("receive error");
    }
    info += get_int8(buffer + info, &message->strsize);
    message->string = (char*)malloc((message->strsize+1)*sizeof(char));
    *stroff = get_str( buffer + info, buf_len - info, message->string, message->strsize); 
    info += *stroff;
    message->string[*stroff] = '\0';    
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
	if (welcome.type != 0){
		return -1;
	}
	return welcome.number;
}

void receive_announce(int fd){
	int buf_len = 256;
	unsigned char buffer[buf_len];
	memset(buffer, 0, buf_len);
	int bytes;
	int stroff;
	struct song_msg message;
	do{
		bytes = recv(fd, buffer, buf_len, 0);
		if (bytes < 0){
			perror("receive error");
			close(fd);
			exit(1);
		}
		if (bytes == 0){
			close(fd);
		}
        if( get_msg_str(buffer, buflen, struct song_msg* message,&stroff) < 0 ){
            perror("receive error");
            close(fd);
            exit(1);
        }
	}while(bytes == buf_len);

	printf("'%s' is playing.\n",message.string);
}

void snowcast_control(const char* servername, const char* serverport, int udpport){

	int client_fd;
	int n_station = -1;
	int station_number;

	client_fd = open_client(servername, serverport);

	send_hello(client_fd, udpport);

    n_station = receive_welcome(client_fd);
    if (n_station < 0){
    	perror("receive error");
    	return;
    }
    if (n_station == 0){
    	printf("no station avaliable");
    	return;
    }
	printf("Station number: %d \n", n_station);
	
	while (1){
		station_number = fgets(stdin);
		if (station_number > n_station){
			printf("Station number out of range \n");
			continue;
		}
		if (station_number < 0){
			printf("Invalid input \n");
			continue;
		}
		break;
	}
	send_set_station(client_fd, station_number);

	receive_announce(client_fd);
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
