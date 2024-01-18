CC = gcc
CFLAGS = -Wall

all: server client other

server: server.o cmd_server.o util.o strutture.o
	$(CC) $(CFLAGS) -o $@ $^

client other: client.o cmd_client.o util.o
	$(CC) $(CFLAGS) -o $@ $^

client.o: client.c include.h util.h cmd_client.h
	$(CC) $(CFLAGS) -c $<

server.o: server.c include.h util.h cmd_server.h strutture.h
	$(CC) $(CFLAGS) -c $<

cmd_client.o: cmd_client.c include.h util.h cmd_client.h
	$(CC) $(CFLAGS) -c $<

cmd_server.o: cmd_server.c include.h util.h cmd_server.h strutture.h
	$(CC) $(CFLAGS) -c $<

util.o: util.c include.h util.h
	$(CC) $(CFLAGS) -c $<

strutture.o: strutture.c include.h strutture.h
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm -f server client *.o