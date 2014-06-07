//
//  Server.cpp
//  server
//
//  Created by Zhixuan Lai on 6/2/14.
//  Copyright (c) 2014 Zhixuan Lai. All rights reserved.
//

#include "Server.h"

#include <time.h>

using namespace std;


//use a non blocking socket

void Server::startListening()
{
    time_t timer = 0;
    bool timeoutMode = false;
    lastPacket = false;
    bool startedHandshaking = false;
    /* now loop, receiving data and printing what we received */
        
    
    cout<<"waiting for file request"<<endl;
    
    for (;;) {

//        cout<< "Timeout mode: "<<timeoutMode<<" timer: "<<timer<<" cur time: "<<time(NULL)<<endl;
        
        
        //if time out
        if (timeoutMode && time(NULL) > timer + TIMEOUT) {
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
        
        if (lastPacket&&p->FIN) {
            cerr<<"FINACK recieved seq#"<<p->SEQ<<", ACK#"<<p->ACK<<", FIN "<<p->FIN<<", content-length: "<<p->content_length<<endl;
            
        }else {
            cerr<<"ACK recieved  seq#"<<p->SEQ<<", ACK#"<<p->ACK<<", FIN "<<p->FIN<<", content-length: "<<p->content_length<<endl;
        }
        
        //compute probablities
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

    LastPacket:;
        if (lastPacket) {
            // recieving the FIN ack
            if (p->FIN) {
                Packet *responsePacket = Packet::finishingPacket();
                responsePacket->ACK = p->responseACK(1);
                responsePacket->SEQ = p->responseSEQ();

                addAndSendPacket(responsePacket);
                fprintf(stderr,"close connection\n");

                break;
            }
            continue;
        }
        
        //!startedHandshaking allows the first packet to have random SEQ
        if (!startedHandshaking || p->SEQ == expected_seq) {
            if (!handshaked) {
                if (p->SYN) {
                    if (!startedHandshaking) {
                        fprintf(stderr,"Started handshaking\n");
                        
                        Packet *responsePacket = Packet::handshakePacket();
                        responsePacket->ACK = p->responseACK(1);
                        responsePacket->SEQ = p->responseSEQ();
                        
                        //start timer
                        timeoutMode = true;
                        time(&timer);
                        addAndSendPacket(responsePacket);
                        
                        expected_seq = p->SEQ+responsePacket->content_length;

                        startedHandshaking = true;
                    }
                    else {
                        string filenamee = p->content;
                        handshaked=true;
                        
                        fprintf(stderr, "File requested is: %s\n", filenamee.c_str());
                        
                        popOutgoingPacket();

                        if (initializePacketsACKAndSEQ(filenamee, p->ACK, p->SEQ)==-1)
                        {
                            fprintf(stderr, "File not found! Terminating server\n");
                            lastPacket = true;
                            
                            // sending the FIN SEQ

                            Packet *responsePacket = Packet::finishingPacket();
                            responsePacket->ACK = p->responseACK(1);
                            responsePacket->SEQ = p->responseSEQ();
                            
                            addAndSendPacket(responsePacket);
                            time(&timer);

                            continue;
                        } else {
                            
                        expected_seq += (*packets)[currentPacketIndex]->content_length;

                        addAndSendPacketsInRange(currentPacketIndex,currentPacketIndex+windowSize);
                        time(&timer);
                        }
                    }
                } else {
                    error("recieved packet with no significant FIN bit during handshake\n");
                }
            } else {
                // recieved data packet
                if (recieveExpectedSEQ()) {
                    lastPacket = true;
                    continue;
                }
                
                //reset timer
                time(&timer);
            }

        }  else {
            // SEQ not matched:
            
            //ack 丢了怎么办?
            if (p->SEQ>expected_seq) {

                while (p->SEQ-1>expected_seq) {
                    if (recieveExpectedSEQ()) {
                        lastPacket = true;
                        break;
                    }
                }
                
                if (lastPacket) {
                    continue;
                }
                
            } else {

                if (outgoingPackets->front()) {
                    sendPacket(outgoingPackets->front());
                }
            }

//            cerr<<"********* SEQ not match: "<<p->SEQ<<" expected: "<<expected_seq<<endl;
            time(&timer);

        }
        
    }
}

int Server::recieveExpectedSEQ()
{
    currentPacketIndex++;
    popOutgoingPacket();
    
    if (currentPacketIndex>packetCount-1) {
        fprintf(stderr, "File transfer complete\n");
        return 1;;
    }
    
    expected_seq += (*packets)[currentPacketIndex]->content_length;
    
    fprintf(stderr, "Sliding window, current index: %d\n", currentPacketIndex);
    
    int index = currentPacketIndex+windowSize-1;
    if(index<packetCount) {
        addAndSendPacketAtIndex(index);
    }
    
    return 0;
}



void Server::addAndSendPacketsInRange(int from, int to)
{
    for (int i=from; i<to; i++) {
        addAndSendPacketAtIndex(i);
    }
}
void Server::addAndSendPacketAtIndex(int i)
{
    if (i>=0&&i<packets->size()) {
        Packet *p = (*packets)[i];
        if (addAndSendPacket(p))
            ;
//            fprintf(stderr, "Insering packet # %d to the list\n", i);
    }
}


int Server::initializePacketsACKAndSEQ(string filename, int ack, int seq)
{
    packets = Packet::packetsFromFile(filename, 0);
    if (packets==NULL) {
        return -1;
    }
    packetCount = packets->size();

    for (int i=0; i<packets->size(); i++) {
        (*packets)[i]->SEQ = ack;
        ack++; // +1 is arbitrary?
        
        seq+=(*packets)[i]->content_length;
        (*packets)[i]->ACK = seq;
    }
    (*packets)[packets->size()-1]->FIN = 1;
    
    return 0;
}


