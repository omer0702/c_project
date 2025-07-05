all: server client

server: server.c server.h
	gcc server.c -o server -lpthread

client: client.c client.h
	gcc client.c -o client -lpthread

clean:
	rm -f server client
