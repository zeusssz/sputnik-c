CC = gcc
CFLAGS = -Wall -Wextra -O2 -pthread

all: server

server: main.o server.o client.o
	$(CC) $(CFLAGS) -o server main.o server.o client.o

main.o: main.c server.h client.h
	$(CC) $(CFLAGS) -c main.c

server.o: server.c server.h client.h
	$(CC) $(CFLAGS) -c server.c

client.o: client.c client.h
	$(CC) $(CFLAGS) -c client.c

clean:
	rm -f *.o server
