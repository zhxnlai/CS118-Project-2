//
//  Packet.h
//  CS118 project 2
//
//  Created by Zhixuan Lai on 6/1/14.
//  Copyright (c) 2014 Zhixuan Lai. All rights reserved.
//

#ifndef __CS118_project_2__Packet__
#define __CS118_project_2__Packet__

#include <iostream>
#include <stdint.h>
#include <vector>
#include <string>
#include <fstream>


extern const int PACKET_SIZE;

class Packet
{
public:
    int ACK;
    int SEQ;
    int FIN;
    int SYN;
    uint16_t checksum;
    int content_length;
    std::string content;
    char* bcontent;

    
    //constructor
    Packet()
    {
        ACK=0; SEQ=0;
        FIN=0;
        SYN=0;checksum=0;content_length=0;content="";
    }
    
    //class function
    static std::vector<Packet*>* packetsFromFile(std::string filename, int seq);
    static Packet* packetStreamer(std::ifstream &is, int fileLength);

    static char* bserialize(Packet* p);
    static Packet* bdeserialize(char *s);

    static std::string* serialize(Packet* p);
    static Packet* deserialize(std::string *s);
    static uint16_t computeChecksum(std::string content);

    static Packet* handshakePacket()
    {
        Packet *p = new Packet();
        p->SYN=1;
        p->setContentAndChecksum("H");
        return p;
    }
    static Packet* finishingPacket()
    {
        Packet *p = new Packet();
        p->FIN=1;
        p->setContentAndChecksum("F");
        return p;
    }
    
    int bPacketSize()
    {
        return sizeof(int)*5+sizeof(uint16_t)+512;
    }

    void setContentAndChecksum(std::string content); //compute checksum
        
    void setBContentAndChecksum(char* content, int len);

    int responseACK(int length);
    int responseSEQ();
    
};




#endif /* defined(__CS118_project_2__Packet__) */
