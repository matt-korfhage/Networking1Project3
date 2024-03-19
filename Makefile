CC=gcc


all: client server

client:
	$(CC) -g ./src/UDP_Echo_Client.c -o ./build/client

server:
	$(CC) -g ./src/UDP_Echo_Server.c -o ./build/server

.PHONY= clean

clean:
	rm -rf ./build/*