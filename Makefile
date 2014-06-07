CC=g++
CFLAGS= -std=c++11 -I.
all: server client

%.o: %.cpp 
	$(CC) -O -c -o $@ $< $(CFLAGS) 

client: ClientMain.o Client.o 
	$(CC) -o client ClientMain.o Client.o Packet.o json11.o Host.o utils.o; mkdir server_folder client_folder; mv server server_folder; mv client client_folder;

server: ServerMain.o Server.o Host.o Packet.o utils.o json11.o 
	$(CC) -o server ServerMain.o Server.o Packet.o json11.o Host.o utils.o

clean:
	rm -rf *o server_folder client_folder