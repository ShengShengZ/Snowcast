CC = gcc
DEBUGFLAGS = -g -Wall
CFLAGS = -D_REENTRANT $(DEBUGFLAGS) -D_XOPEN_SOURCE=500
LDFLAGS = -lpthread 

#all: snowcast_listener snowcast_control snowcast_server

#snowcast_listener: snowcast_listener.c
#snowcast_control:  snowcast_control.c extrafile1.c extrafile2.c
#snowcast_server:   snowcast_server.c extrafile3.c extrafile4.c
#clean:
#	rm -f snowcast_listener snowcast_control snowcast_server

#for control
#snowcast_control:  snowcast_control.c
#clean:
#	rm -f snowcast_control

#for listener
#snowcast_listener:  snowcast_listener.c
#clean:
#	rm -f snowcast_listener

#for server
snowcast_server:   snowcast_server.c
clean:
	rm -f snowcast_server

