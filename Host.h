//
//  Host.h
//  CS118 project 2
//
//  Created by Zhixuan Lai on 6/1/14.
//  Copyright (c) 2014 Zhixuan Lai. All rights reserved.
//

#ifndef __CS118_project_2__Host__
#define __CS118_project_2__Host__

#include <iostream>
#include <vector>
#include "Packet.h"

#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <strings.h>
#include <sys/wait.h> /* for the waitpid() system call */
#include <signal.h> /* signal name macros, and the kill() prototype */
#include <string.h>
#include "utils.h"

#include <cstdlib>
#include <time.h>
#include <list>
#include <vector>

#define TIMEOUT 1

#define PRINT_MSG 0

class Host {
public:
    
    struct sockaddr_in myaddr;	/* our address */
	struct sockaddr_in remaddr;	/* remote address */
	socklen_t addrlen;		/* length of addresses */
	int fd;				/* our socket */
	int msgcnt = 0;			/* count # of messages we received */
    int portno = 0;         /* port number to listen to */
    int BUFSIZE = 4096;
    
    float lossProb;
    float corrProb;
    
    std::vector<Packet*>* packets;
    std::list<Packet*>* outgoingPackets;

    Host(int portno,
               int fd,
               struct sockaddr_in myaddr,
               struct sockaddr_in remaddr
               )
    {
        this->portno = portno;
        this->fd = fd;
        this->myaddr = myaddr;
        this->remaddr = remaddr;
        this->addrlen = sizeof(remaddr);
        handshaked = false;
        handshakePhase = 0;
        currentPacketIndex = 0;
        packetCount = 0;
        expected_seq = 0;
        this->lossProb = 0;
        this->corrProb = 0;
        
        packets = new std::vector<Packet*>();
        outgoingPackets = new std::list<Packet*>();
        srand (static_cast <unsigned> (time(0)));

    }
    
    bool lastPacket;    //for server
    bool hasFinished;   //for client
    bool handshaked;
    int handshakePhase;
    
    int currentPacketIndex;
    int packetCount;
    int expected_seq;
    
    
    //send & recieve
    std::string* recv();
    void send(std::string* msg);
    
    char* recv(int* len);
    void send(char* msg, int len);

    void sendPacket(Packet* p);

    void popOutgoingPacket();
    void resendOutgoingPackets();
    bool addAndSendPacket(Packet *p);

    
    virtual void startListening() {};

    bool tryProbability(float prob)
    {
        
        float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
//        std::cout<<"r: " <<r<<" prob: "<<prob<<std::endl;
        if (r<prob)
            return true;
        return false;
    }
    
};

#endif /* defined(__CS118_project_2__Host__) */
