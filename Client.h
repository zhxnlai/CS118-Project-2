//
//  Client.h
//  server
//
//  Created by Zhixuan Lai on 6/2/14.
//  Copyright (c) 2014 Zhixuan Lai. All rights reserved.
//

#ifndef __server__Client__
#define __server__Client__

#include <iostream>
#include "Host.h"

class Client : public Host
{
public:
    std::string server;
    std::string filename;
    std::string fileData;
    
    Client(int portno,
           int fd,
           struct sockaddr_in myaddr,
           struct sockaddr_in remaddr,
           std::string server,
           std::string filename
           ) : Host (portno, fd, myaddr, remaddr)
    {
        this->server = server;
        this->filename = filename;
        fileData = "";
    }
    Client(int portno,
           int fd,
           struct sockaddr_in myaddr,
           struct sockaddr_in remaddr,
           std::string server,
           std::string filename,  float lossProb, float corrProb
           ) : Host (portno, fd, myaddr, remaddr)
    {
        this->server = server;
        this->filename = filename;
        this->lossProb = lossProb;
        this->corrProb = corrProb;
        fileData = "";
    }
    
    void startListening();
    
    void saveFileData(std::string d)
    {
        fileData.append(d);
    }
    void addToPackets(Packet *p)
    {
        packets->push_back(p);
    }
    void savePacketsToFileWithFilename(std::string filename);

};

#endif /* defined(__server__Client__) */
