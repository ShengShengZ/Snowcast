#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/*	Structure
main thread:
	create the other threads

client thread:
	number of client
	recv:
		hello: get udpport
		set_station
	send:
		welcome
		send announce
		send invalid_command

station thread:
	number of stations
	send:
		song data
		announce when replay
*/
struct client{
	int fd;
	uint16_t sid;
	struct station* station; //point to father
	struct sockaddr_in udpaddr;
	struct sockaddr_in sockaddr;
	struct client* next;
}

struct station{
	int udpfd;
	int id;
	const char* songname;
	FILE* file;
	struct client* next;
}

void send_message(int fd, int type, const char* songname){

}

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
				send_message(client->fd, 1, station->songname);
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
		stations[i].udpfd = open_udpfd();
	}

	//set thread for each station
	pthread_t station_threads[n_stations];
	station_mutexs = (pthread_mutex_t)malloc(n_stations * sizeof(pthread_mutex_t));
	for (i = 0; i < n_stations; i++){
		pthread_mutex_init(&station_mutexs[i],NULL);
		pthread_create(&station_threads[i], NULL, intial_station, &stations[i]);
	}
	return 0;
}






















