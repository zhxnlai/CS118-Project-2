//
//  Server.h
//  server
//
//  Created by Zhixuan Lai on 6/2/14.
//  Copyright (c) 2014 Zhixuan Lai. All rights reserved.
//

#ifndef __server__Server__
#define __server__Server__

#include <iostream>
#include "Host.h"




class Server : public Host
{
public:

    int windowSize;


    Server(int portno,
           int fd,
           struct sockaddr_in myaddr,
           struct sockaddr_in remaddr
           ) : Host (portno, fd, myaddr, remaddr)
    {
        this->windowSize = 5;
    }
    
    Server(int portno,
           int fd,
           struct sockaddr_in myaddr,
           struct sockaddr_in remaddr, int windowSize, float lossProb, float corrProb
           ) : Host (portno, fd, myaddr, remaddr)
    {
        this->windowSize = windowSize;
        this->lossProb = lossProb;
        this->corrProb = corrProb;
    }

    void startListening();

    
    
    //initialization
    int initializePacketsACKAndSEQ(std::string filename, int ack, int seq);

    // return 1 if reached the last packet
    int recieveExpectedSEQ();
    
    //outgoing list
    void addAndSendPacketAtIndex(int i);
    void addAndSendPacketsInRange(int from, int to);
};



#endif /* defined(__server__Server__) */
