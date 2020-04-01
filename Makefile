# Makefile for HW1, ecs36b, s2020
#

CC = gcc
CFLAGS = -g -Wall -Wstrict-prototypes

# CFLAGS = -O3

# rules.
all: ecs36b_hw1_server ecs36b_hw1_client

#
#
socket_client_hw1.o:	socket_client_hw1.c
			$(CC) -c $(CFLAGS) socket_client_hw1.c

socket_server_hw1.o:	socket_server_hw1.c
			$(CC) -c $(CFLAGS) socket_server_hw1.c

ecs36b_hw1_server:	socket_server_hw1.o
			$(CC) -o ecs36b_hw1_server socket_server_hw1.o

ecs36b_hw1_client:	socket_client_hw1.o
			$(CC) -o ecs36b_hw1_client socket_client_hw1.o

clean:
	rm -f *.o *~ core ecs36b_hw1_server ecs36b_hw1_client
