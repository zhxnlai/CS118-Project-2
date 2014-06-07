//
//  Client.cpp
//  server
//
//  Created by Zhixuan Lai on 6/2/14.
//  Copyright (c) 2014 Zhixuan Lai. All rights reserved.
//

#include "Client.h"
#include <time.h>

using namespace std;

void Client::startListening()
{
    time_t timer = 0;
    bool timeoutMode = true;

    //random ACK and sequence?
    int ack = 100;
    int seq = 0;
    Packet * firstHandshakePacket = Packet::handshakePacket();
    firstHandshakePacket->ACK = ack;
    firstHandshakePacket->SEQ = seq;
    addAndSendPacket(firstHandshakePacket);
    
    
    hasFinished = false;
    
    expected_seq = firstHandshakePacket->ACK;
    fprintf(stderr,"Expected SEQ: %d\n", expected_seq);

    for (;;) {
        
        if (timeoutMode && time(NULL) > timer + TIMEOUT) {
            
            if (hasFinished) {
                fprintf(stderr, "Finished\n");
                break;
            }
            
            //reset timer
            time(&timer);
            
            resendOutgoingPackets();
        }
        
        string *receivedString = recv();
        if (!receivedString) {
            continue;
        }
        
        /*

        int len;
        char *receivedString = recv(&len);
        if (!receivedString) {
            continue;
        }
         */

        Packet *p = Packet::deserialize(receivedString);
        if (p==NULL) {
            fprintf(stderr,"Cannot deserialize packet\n");
            continue;
        }
        
        if (hasFinished&&p->FIN) {
            cerr<<"FINACK recieved seq#"<<p->SEQ<<", ACK#"<<p->ACK<<", FIN "<<p->FIN<<", content-length: "<<p->content_length<<endl;
            
        }else {
        cerr<<"ACK recieved  seq#"<<p->SEQ<<", ACK#"<<p->ACK<<", FIN "<<p->FIN<<", content-length: "<<p->content_length<<endl;
        }
        
        //compute probablity
        if (tryProbability(lossProb)) {
            cerr<<"********** Packet Lost: ACK: "<<p->ACK<<" SEQ: "<<p->SEQ<<" *********"<<endl;
            continue;
        }
        
        if (tryProbability(corrProb)) {
            cerr<<"********** Packet Corrupted: ACK: "<<p->ACK<<" SEQ: "<<p->SEQ<<" *********"<<endl;
            resendOutgoingPackets();
            continue;
        }
        
        if (Packet::computeChecksum(p->content)!=p->checksum) {
            cerr<<"********** Real!! Packet Corrupted: ACK: "<<p->ACK<<" SEQ: "<<p->SEQ<<" *********"<<endl;
        }
        
        if (p->SEQ == expected_seq) {
            
            if (hasFinished) {
                fprintf(stderr, "Finished\n");
                break;
            }
            
            
            if (!this->handshaked) {
                fprintf(stderr,"here");
                //wait for the second handshake packet
                if (p->SYN) {
                    Packet * secondHandshakePacket = Packet::handshakePacket();
                    secondHandshakePacket->setContentAndChecksum(this->filename);
                    secondHandshakePacket->ACK = p->responseACK(secondHandshakePacket->content_length);
                    secondHandshakePacket->SEQ = p->responseSEQ();
                    
                    popOutgoingPacket();
                    addAndSendPacket(secondHandshakePacket);
                    
                    expected_seq += secondHandshakePacket->content_length;
//                    fprintf(stderr,"Expected SEQ: %d\n", expected_seq);

                    //TODO: add a timer
                    fprintf(stderr,"Sent filename: %s, waiting for data transfer\n", this->filename.c_str());
                    this->handshaked=true;
                } else {
                    fprintf(stderr,"recieved packet with no significant SYN bit during handshake\n");
                }
            
            } else {
                saveFileData(p->content);

                if (p->FIN) {
                    hasFinished = true;
//                    fprintf(stderr, "Last packet recieved\n");
                    savePacketsToFileWithFilename(filename);
                }

                Packet *responsePacket = Packet::handshakePacket();
                responsePacket->ACK = p->responseACK(1);
                responsePacket->SEQ = p->responseSEQ();
                ack = responsePacket->ACK;
                seq = responsePacket->SEQ;
                
                if (hasFinished) {
                    responsePacket->FIN = 1;
                }
                
                popOutgoingPacket();
                addAndSendPacket(responsePacket);
                

                expected_seq += responsePacket->content_length;
//                fprintf(stderr,"Expected SEQ: %d\n", expected_seq);


            }
        }
        else {
            cerr<<"********* received seq#"<<p->SEQ<<", expect seq#"<<expected_seq<<" *********"<<endl;
            resendOutgoingPackets();
        }
    
        delete p;
    }

}

void Client::savePacketsToFileWithFilename(std::string filename)
{
    ofstream myfile;
    myfile.open (filename);
    
    if (myfile.is_open())
        myfile << fileData;

    myfile.close();
}




