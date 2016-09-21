#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

//-----required structure--------

pthread_mutex_t* station_mutexs;
int n_stations;

struct client{
	int fd;
	uint16_t sid;
	struct station* station; //point to father station
	struct sockaddr_in udpaddr;
	struct sockaddr_in sockaddr;
	//work as a node in a linked list 
	struct client* next;
}

struct station{
	int udpfd;
	int id;
	const char* songname;
	FILE* file;
	//list clinets connect to the station
	struct client* clients;
}
struct station* stations;

struct command{
	//0: hello, 1: set station
	uint8_t type;
	uint16_t number;
}

struct welcome_msg{
	uint8_t type;
	uint16_t number;
};

struct song_msg{
    uint8_t type;
    uint8_t strsize;
    char* string;
};

struct error_msg{
    uint8_t type;
    uint8_t strsize;
    char* string;
};

//-----help functions-----
char* get_sock_ip(struct sockaddr_in* addr, char* str, size_t len){
	inet_ntop(AF_INET, &(addr->sin_addr), str, len);
	return str;
}

//-----send information----

void send_welcome(int fd, int n_stations){
	
}

void send_songname(int fd, const char* songname){

}

void send_invalid(int fd, int error_type){
	// 0: too many hello
	// 1: too many set_station
	// 2: unknow
}

//------receive information--

int get_int8 (unsigned char* buffer, uint8_t *number){
    *number = *buffer;
    return 1;
}

int get_int16 (unsigned char* buffer, uint16_t *number){
    memcpy(number, buffer, sizeof(uint16_t));
    *number = ntohs(*number);
    return 2;
}

int get_message(unsigned char* buffer, int buf_len, struct welcome_msg* welcome){
    int info = 0;
    info += get_int8(buffer, &command->type);
    if (welcome->type != 0){
    	perror("Not Welcome msg");
    }    
    info += get_int16(buffer + info, &command->number);
    //error return -1
    return 0;
}

int recv_message(int fd, struct command* cmd){
	int buf_len = sizeof(uint8_t) + sizeof(uint16_t);
	unsigned char buffer[buf_len];
	memset(buffer, 0, buf_len);
	int bytes = recv(fd, buffer, buf_len, 0);

	if (bytes < 0){
		perror("Receive Welcome Error");
		return -1;
	}
	if (bytes == 0){
		printf("Client closed \n");
		return -1;
	}
	if (bytes < buf_len){
		printf("Length of the msg is wrong. \n");
		return -1;
	}
	get_message(buffer,buf_len,cmd);
	return 0;
}

//------client_station interaction-------

int station_del_client(struct client* client, int sid){
	if (client->station == NULL){
		return 0;
	}
	struct station* station = &stations[sid];

	pthread_mutex_lock(&station_mutexs[sid]);

	struct client* previous = station->clients;
	struct client* current = station->clients;

	while (current){
		if (current == client){
			if (current == station->clients){
				station->clients = client->next;
			}else{
				previous->next = current->next;
			}
		}
		previous = current;
		current = current->next
	}
	client->station = NULL;
	client->sid = 0;

	pthread_mutex_unlock(&station_mutexs[sid]);
	return 0;
}

int station_add_client(struct client* client, int new_sid){
	struct station* new_station = &stations[new_sid];
	if (new_station == client->station){
		return 0;
	}
	station_del_client(client,client->sid);

	pthread_mutex_lock(&station_mutexs[new_sid]);
	struct client *tmp_client;
	tmp_client = new_station->client;
	client->next = tmp_client;

	client->sid = new_sid;
	client->station = new_station;

	pthread_mutex_lock(&station_mutexs[new_sid]);
	return 0;
}

//------client_command------

void kill_client(struct client* client){
	station_del_client(client, client->sid);
	close(client->fd);
	free(client);
	pthread_exit(NULL);
}

void* initial_client(void* client_for_fundation){
	struct client* client = (strcut client*) client_for_fundation;
	int fd = client->fd;
	struct command cmd;

	int status = 0;

	while (1){
		if (recv_message(fd,&cmd) < 0){
			kill_client(client);
		}
		if (cmd.type == 0){
			if (status != 0){
				send_invalid(fd,0);
				kill_client(fd);
				break;
			}

			uint16_t udpport = cmd.content;
			printf("udpport: %d\n",udpport);
			client->udpaddr = client->sockaddr;
			client->udpaddr.sin_port = htons(udpport);

			send_welcome(fd, n_stations);
			status = 1;
		}else if (cmd.type == 1){
			if (status != 1){
				send_invalid(fd,1);
				kill_client(fd);
				break;
			}
			station_add_client(client, station);
			send_songname(fd, stations[client->sid].songname);
		}else{
			send_invalid(fd,2);
			kill_client(fd);
			break;			
		}
	}
	return NULL;
}

//------station command------

void kill_station(void* struct_station){
	int sid = station->id;

	pthread_mutex_lock(&station_mutexs[sid]);

	struct client* client = station->clients;
	while (client != NULL){
		struct client* next = client->next; //i hate c
		kill_client(client);
		client = next;
	}

	pthread_mutex_unlock(&station_mutexs[sid]);

	close(station->udpfd);
	fclose(station->songfile);
	pthread_exit(NULL); //why?
}

void* intial_station(void* struct_station){
	struct station* station = (struct station*)struct_station;
	struct client* client;

	FILE* file = fopen(station->songname, "r");
	if (file == NULL){
		printf("Fail to open %s \n", station->songname);
		kill_station(station);
		return;
	}
	station->songfile = file;

	struct timespec waittime;
	int bytes;
	int buf_len = 1024;
	unsigned char buffer[buf_len];

	while(1){
		waittime.tv_sec = 0;
		waittime.tv_nsec = 62500000;


		bytes = fread(buffer, 1, buf_len, file);

		//file end
		if (feof(file)){
			rewind(file);

			pthread_mutex_lock(&station_mutexs[station->id]);
			client = station->clients;
			while (client != NULL){
				send_songname(client->fd, station->songname);
				client = client->next;
			}
			pthread_mutex_unlock(&station_mutexs[station->id]);
			continue;
		}

		//send song
		pthread_mutex_lock(&station_mutexs[station->id]);
		client = station->clients;
		while (client != NULL){
			bytes = sendto(station->udpfd, buffer, buf_len, 0, (struct sockaddr*)&client->udpaddr, sizeof(struct sockaddr));
			client = client->next;
		}		
		pthread_mutex_unlock(&station_mutexs[station->id]);
		nanosleep(waittime,NULL);
	}
}

//-----server command------

void accept_server(int client_fd, struct sockaddr_in client_addr, socklen_t client_addrlen){
	pthread_t client_thread;
	struct client* client;
	int addrstrlen = INET_ADDRSTRLEN;
	char addrstr[addrstrlen];

	printf("Connection from %s \n",get_sock_ip(&client_addr, addrstr, addrstrlen));

	client = (struct client*)calloc(1,sizeof(struct client));
	client.fd = client_fd;
	client.sid = 0;
	client.station = NULL; 
	client.udpaddr = client_addr;
	client.sockaddr = client_addr;
	client.next = NULL;

	pthread_create(&client_thread, NULL, initial_client, client);
}

void quit_server(int n_stations, struct station* stations, int receiver_fd){
	for (int i = 0; i < n_stations; i++){
		pthread_mutex_lock(&station_mutexs[i]);
		struct client* client = stations[i].clients;
		while (client != NULL){
			struct client* next = client->next; //i hate c
			
			close(client.fd);
			free(client);

			client = next;
		}
		fclose(station[i].songfile);
		pthread_mutex_unlock(&station_mutexs[i]);
	}
	free(stations);
	free(station_mutexs);
	exit(0);
}

void print_server(int n_stations,struct station* stations){
	int addrstrlen = INET_ADDRSTRLEN;
	char addrstr[addrstrlen];

	for (int i = 0; i < n_stations; i++){
		pthread_mutex_lock(&station_mutexs[i]);
		printf("Station %d plays %s. \n Listener: ", i, station[i].songname);

		struct client* client;
		client = stations[i].clients;
		while (client){
			printf("%s:%d \n",get_sock_ip(&client->sockaddr,addrstr,addrstrlen));
			client = client->next;
		}
		pthread_mutex_unlock(&station_mutexs[i]);
	}
}

int open_udp_fd(){
	int fd;
	fd = socket( AF_INET, SOCK_DGRAM, 0);
	return fd;
}

int open_receiver(const char* tcpport){
	int receiver_fd;

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, serverport, &hints, &servinfo);

	if ((receiver_fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) < 0){
		perror("Socket Error");
		exit(1);
	}
	bind(client_fd, servinfo->ai_addr, servinfo->ai_addrlen);
	listen(receiver_fd, SOMAXCONN);

	printf("My address: %s:%d\n",
        get_sock_ip( (struct sockaddr_in*)servinfo->ai_addr, addrstr,addrstr_len),
        ntohs(((struct sockaddr_in*)servinfo->ai_addr)->sin_port));
	freeaddrinfo(servinfo);
	return receiver_fd;
}

void snowcast_server(const char*tcpport, int n_stations, (struct station*) stations){

	int receiver_fd = open_server(tcpport);

	fd_set read_fds;
	int fd_limit;
	while(1){
        FD_ZERO(&read_fds);
        FD_SET( fileno(stdin), &read_fds);
        FD_SET( receiver_fd, &read_fds);

        fd_max = fileno(stdin);
        if (receiver_fd > fd_max ){
            fd_max = receiver_fd;
        }

        select(fd_max+1, &read_fds, NULL, NULL, NULL);

        if (FD_ISSET( fileno(stdin), &read_fds )){
            char input_char = fgetc(stdin);
            if( input_char == 'p' ){
                print_station(n_stations, stations);
            }
            if( input_char == 'q' ){
                quit_server(n_stations, stations, receiver_fd);
            }
        }
        else if (FD_ISSET(receiver_fd, &read_fds)) {
        	int client_fd;
        	struct sockaddr_in client_addr;
        	socklen_t client_addrlen;
        	client_fd = accept( receiver_fd, (struct sockaddr*)&client_addr, &client_addrlen);
            accept_server(client_fd, client_addr, client_addrlen);
        }
	}
}

int main(int argc, char** argv){
	int i;

	if (argc < 2){
		printf("Format: snowcast_server <tcpport> <file1> ... <filen> \n");
		return 0;
	}

	char* tcp_port_input = argv[1];
	
	int n_songs = argc - 2;
	if (n < 0){
		printf("Need one song at least \n");
		return 0;
	}
	char* songs[n_songs];

	for (i = 0; i< argc - 2; i++){
		songs[i] = argv[i+2];
	}

	//initialize each station
	n_stations = n_songs;
	stations = (struct station*)calloc(n_stations,sizeof(struct station));
	for (i = 0; i < n_songs; i++){
		stations[i].id = i;
		stations[i].songname = files[i];
		stations[i].songfile = NULL;
		stations[i].clients = NULL;
		stations[i].udpfd = open_udp_fd();
	}

	//set thread for each station
	pthread_t station_threads[n_stations];
	station_mutexs = (pthread_mutex_t)malloc(n_stations * sizeof(pthread_mutex_t));
	for (i = 0; i < n_stations; i++){
		pthread_mutex_init(&station_mutexs[i],NULL);
		pthread_create(&station_threads[i], NULL, intial_station, &stations[i]);
	}

	snowcast_server(tcpport, n_stations, stations);
	return 0;
}






















