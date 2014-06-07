/*
 demo-udp-03: udp-send: a simple udp client
 send udp messages
 This sends a sequence of messages (the # of messages is defined in MSGS)
 The messages are sent to a port defined in SERVICE_PORT
 
 usage:  udp-send
 
 Paul Krzyzanowski
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in

#include <sys/wait.h> /* for the waitpid() system call */
#include <signal.h> /* signal name macros, and the kill() prototype */
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "utils.h"
#include "Packet.h"
#include "Client.h"
#include "json11.hpp"
#define BUFLEN 2048
#define MSGS 5	/* number of messages to send */


void printPacket (Packet * p)
{
    std::cout<<"\n ********* \n "<<
    "SEQ: "<<p->SEQ<<" ACK: "<<p->ACK <<" SYN: "<<p->SYN<<" length: "<<p->content_length<<" content: " << p->content
    <<"\n ********* \n\n";
}

void test()
{
    
    std::string c = R"({"ACK": 512, "FIN": 0, "SEQ": 108, "SYN": 0, "checksum": 4006, "content": "tess.txt\nhao qi guai a !!! hao qi guai a !!! hao qi guai a !!! hao qi guai a !!! hao qi guai a !!!\nhao qi guai a !!! hao qi guai a !!! hao qi guai a !!! hao qi guai a !!! hao qi guai a !!!\nhao qi guai a !!! hao qi guai a !!! hao qi guai a !!! hao qi guai a !!! hao qi guai a !!!\nhao qi guai a !!! hao qi guai a !!! hao qi guai a !!! hao qi guai a !!! hao qi guai a !!!\nhao qi guai a !!! hao qi guai a !!! hao qi guai a !!! hao qi guai a !!! hao qi guai a !!!\nhao qi guai a !!! hao qi guai a !!! hao qi guai a !!", "content_length": 511, "dest_port": 0, "source_port": 0})";
    
    Packet * p = Packet::deserialize(&c);
    
    printPacket(p);
    
    
    Packet testp;
    testp.ACK =100;
    testp.SEQ =100;
    testp.SYN =100;
    testp.setContentAndChecksum("kals;dfja;sdkfj;asjdf;waefjkwq;lj fiownjq hfowqpjioehqwoheiopefoiapijfwepqi");

    std::string* d = Packet::serialize(&testp);
    fprintf(stderr, "%s\n", d->c_str());
    
    Packet * p2 = Packet::deserialize(d);

    printPacket(p2);


    
}

void test2()
{
    std::string a = "AB\0AB";
    
    std::cout<<a<<" size: "<<a.size()<<" c_size: "<<strlen(a.c_str())<<std::endl;
    
//    std::cout<<a<<" sizeof: "<<sizeof(&a)<<std::endl;

}


int main(int argc, char **argv)
{
//    test2();
//    return 0;

    
    if (argc < 4) {
        fprintf(stderr,"ERROR, no address or port or filename provided\n");
        exit(1);
    }
	char *server = argv[1];	/* change this to use a different server */
    int portno = atoi(argv[2]);
    char *filename = argv[3];
    
	struct sockaddr_in myaddr, remaddr;
    int fd;
	/* create a socket */
    
	if ((fd=socket(AF_INET, SOCK_DGRAM, 0))==-1)
		printf("socket created\n");
    
	/* bind it to all local addresses and pick any port number */
    
	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(0);
    
	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return 0;
	}
    
	/* now define remaddr, the address to whom we want to send messages */
	/* For convenience, the host address is expressed as a numeric IP address */
	/* that we will convert to a binary format via inet_aton */
    
	memset((char *) &remaddr, 0, sizeof(remaddr));
	remaddr.sin_family = AF_INET;
	remaddr.sin_port = htons(portno);
	if (inet_aton(server, &remaddr.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}
    
    make_nonblocking(fd);
    
    if (argc==6) {
        float LOSS_PROBABILITY = atof(argv[4]);
        float CORRUPTION_PROBABILITY = atof(argv[5]);
        
        Client client(portno, fd, myaddr, remaddr, server, filename, LOSS_PROBABILITY, CORRUPTION_PROBABILITY);
        client.startListening();
    } else {
        Client client(portno, fd, myaddr, remaddr, server, filename);
        client.startListening();
    }
    

    
	/* now let's send the messages */
    
    
    close(fd);
	return 0;
}
