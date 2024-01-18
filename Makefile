CC = gcc
CFLAGS = -Wall -pedantic -g #-fsanitize=address

all: client server

client: include.h util.h cmd_client.h cmd_client.c util.c client.c 
	$(CC) $(CFLAGS) -o $@ $^

server: include.h util.h strutture.h cmd_server.h cmd_server.c util.c strutture.c server.c 
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f server client *.o