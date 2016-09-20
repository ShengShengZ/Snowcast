#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

//-----required structure--------

pthread_mutex_t* station_mutexs;

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
	struct client* next;
}

struct command{

}

//-----send information----

void send_welcome(int fd, int n_stations){

}

void send_songname(int fd, const char* songname){

}

void send_invalid(int fd, int error_type){

}

//------receive information--

int recv_message(int fd, struct command* cmd){

}

//------client_command------

void* initial_client(void* client){

}

void kill_client(struct client* client){

}

//------client_station interaction-------

int station_add_client(struct client* client, int sid){

}

int station_del_client(struct client* client, int sid){

}

//------station command------

void* intial_station(void* struct_station){
	struct station* station = (struct station*)struct_station;
	struct client* client;

	FILE* file = fopen(station->songname, "r");
	if (file == NULL){
		printf("Fail to open %s \n", station->songname);
		close_station(station);
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

void intial_station(void* struct_station){

}

//-----server command------

void accept_server(int client_fd, struct sockaddr_in client_addr, socklen_t client_addrlen){

}

void quit_server(int n_stations, struct station* stations, int receiver_fd){

}

void print_server(int n_stations,struct station* stations){

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
            new_client(client_fd, client_addr, client_addrlen);
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






















