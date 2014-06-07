//
//  Host.cpp
//  CS118 project 2
//
//  Created by Zhixuan Lai on 6/1/14.
//  Copyright (c) 2014 Zhixuan Lai. All rights reserved.
//

#include "Host.h"

using namespace std;

//return null if failed
std::string* Host::recv()
{
    char buf[BUFSIZE];
    int recvlen;            /* # bytes received */
    recvlen = recvfrom(fd, buf, BUFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
    if (recvlen > 0) {
        buf[recvlen] = 0;
        if (PRINT_MSG) {
            printf("----------\nreceived message: \"%s\" (%d bytes)\n", buf, recvlen);
        }

        
        string bufString(buf);
        string *bbb = new string(bufString);
        return bbb;
    }
    else if (recvlen==-1) {
//        printf("No msg available\n");
        return NULL;

    }else {
        printf("recv: uh oh - something went wrong!\n");
        return NULL;
    }
}

char* Host::recv(int* len)
{
    char *buf = new char[BUFSIZE]();
    int recvlen;            /* # bytes received */
    recvlen = recvfrom(fd, buf, BUFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
    if (recvlen > 0) {
        buf[recvlen] = 0;
        if (PRINT_MSG) {
            printf("----------\nreceived message: \"%s\" (%d bytes)\n", buf, recvlen);
        }
        
        *len = recvlen;
        return buf;
    }
    else if (recvlen==-1) {
        //        printf("No msg available\n");
        return NULL;
        
    }else {
        printf("recv: uh oh - something went wrong!\n");
        return NULL;
    }
}

void Host::send(char* msg, int len)
{
    if (sendto(fd, msg, len, 0, (struct sockaddr *)&remaddr, addrlen) < 0)
        perror("ERROR: sendto");
    else {
        if (PRINT_MSG) {
            printf("===========\nsent message: \"%s\" (%d bytes)\n", msg, len);
        }
    }
}

void Host::send(std::string* msg)
{
    char buf[msg->size()+1];
    strcpy(buf, msg->c_str());
    
    if (sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&remaddr, addrlen) < 0)
        perror("ERROR: sendto");
    else {
        if (PRINT_MSG) {
            printf("===========\nsent message: \"%s\" (%d bytes)\n", buf, strlen(buf));
        }
    }

}

void Host::sendPacket(Packet* p)
{

    cerr<<"DATA sent seq#"<<p->SEQ<<", ACK#"<<p->ACK<<", FIN "<<p->FIN<<", content-length: "<<p->content_length<<endl;
    
    string *serializedResponse = Packet::serialize(p);
    send(serializedResponse);
    
    
}

void Host::popOutgoingPacket()
{
    delete outgoingPackets->front();
    outgoingPackets->pop_front();
}



void Host::resendOutgoingPackets()
{
    std::list<Packet*>::const_iterator iterator;
    for (iterator = outgoingPackets->begin(); iterator != outgoingPackets->end(); ++iterator) {
        //        std::cout << *iterator;
        sendPacket(*iterator);
    }
}
bool Host::addAndSendPacket(Packet *p)
{
    std::list<Packet *>::iterator findIter = std::find(outgoingPackets->begin(), outgoingPackets->end(), p);
    
    if (findIter==outgoingPackets->end()) {
        outgoingPackets->push_back(p);
        sendPacket(p);
        return true;
    } else {
        return false;
    }
    
}




