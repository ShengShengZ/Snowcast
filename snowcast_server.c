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
int main(int argc, char** argv){
	if (argc < 2){
		//error info
		continue;
	}
	int tcp_port = 0;
	char* tcp_port_input = argv[1];
	int number_song = argc - 2;

	check = str2int(tcp_port_input, &tcp_port);
	if (check){
		continue;
	}
	//check tcp port range

	char* songs[number_song];
	for (int i = 0; i< argc - 2; i++){
		songs[i] = argv[i+2];
	}
	






















}