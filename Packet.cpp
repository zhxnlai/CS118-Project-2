//
//  Packet.cpp
//  CS118 project 2
//
//  Created by Zhixuan Lai on 6/1/14.
//  Copyright (c) 2014 Zhixuan Lai. All rights reserved.
//

#include "Packet.h"
#include <iostream>
#include <sstream>
#include <string.h>
#include <string>
#include <stdio.h>
#include <fstream>

#include "json11.hpp"

const int PACKET_SIZE = 1024;

using namespace std;

Packet* Packet::packetStreamer(ifstream &is, int fileLength){
    Packet* p = new Packet;
    
    int BUFSIZE = 1025;
    char buf[BUFSIZE];
    is.read(buf, BUFSIZE-1);
    buf[BUFSIZE-1]='\0';
    
//    std::string buff(std::istreambuf_iterator<char>(is), {});
//    std::cout << buff << std::endl;

    int contentLength = 0;

    contentLength = BUFSIZE-1;
    if(is.eof()){
        contentLength = fileLength % (BUFSIZE-1); //the last packet has size of fileLength % BUFSIZE
        
    }

    p->setContentAndChecksum(string(buf, contentLength));
    
    p->content_length = contentLength;
    
    
//    cerr<<"contentLength: "<<p->content_length<<" contentsize: "<<p->content.size()<<" content: %s###\n"<<  p->content.c_str()<<endl;

    return p;
}


vector<Packet*>* Packet::packetsFromFile(string  filename, int seq)

{
    //ack, seq
    
    vector<Packet*> *packet_vector = new vector<Packet*>();
    int bytes_read = 0;
    ifstream istream(filename.c_str(), ifstream::in | ifstream::binary);
    if (istream) {
        // get length of file:
        istream.seekg (0, istream.end);
        int length = istream.tellg();
        istream.seekg (0, istream.beg);
        
        //read file packet by packet until eof
        while(!istream.eof()){
            Packet *p = packetStreamer(istream, length);
            
            
//            ????//p->seq = bytes_read +seq ; // SEQ is the start of the next buffer
            
            bytes_read += istream.gcount();
            packet_vector->push_back(p);
        }
        cerr<<"File length: "<<length<<endl;
        cerr<<"Num Packets: "<<packet_vector->size()<<endl;

        
        istream.close();
    }
    else {
        cerr << "cannot construct the ifstream from file \"" << filename << "\"." << endl;
        delete packet_vector;
        return NULL;
    }
    
    return packet_vector;
}


int Packet::responseACK(int length)
{
    return (this->SEQ + length);
}

int Packet::responseSEQ()
{
    return (this->ACK);
}

char* Packet::bserialize(Packet* p)
{
    char *s = new char[PACKET_SIZE+1]();
    bzero(s, PACKET_SIZE);
    
    memcpy(s,  &(p->SEQ),			 	4);
    
    memcpy(s+4, &p->ACK, 			 4);
    
    memcpy(s+8, &p->FIN, 			 4);
    
    memcpy(s+12, &p->SYN, 			 4);
    memcpy(s+16, &p->checksum, 		 2);
    
    
    memcpy(s+18, &p->content_length, 	 4);
    
    
    memcpy(s+22, p->bcontent, 	p->content_length);
    fprintf(stderr, "string is : %s\n", s);

    for (int j = 0; j < PACKET_SIZE; j++ )
    {
        fprintf(stderr, "%d: %c",j,s[j] );
    }
    fprintf(stderr, "\n" );

//    s[22+p->content_length]=0;
    return s;

}

Packet* Packet::bdeserialize(char *s)
{
    Packet *p = new Packet();
    
    memcpy(&p->SEQ, s, 4);
    memcpy(&p->ACK, s+4, 4);
    memcpy(&p->FIN, s+8, 4);
    memcpy(&p->SYN, s+12, 4);
    memcpy(&p->checksum, s+16, 2);
    memcpy(&p->content_length, s+18, 4);
    memcpy(&p->bcontent, s+22, p->content_length);
    
    cout<<"SEQ "<<p->SEQ
    <<"ACK "<<p->ACK
    <<"FIN "<<p->FIN
    <<"SYN "<<p->SYN
    <<"checksum "<<p->checksum
    <<"content_length "<<p->content_length
        <<"bcontent "<<p->bcontent<<endl;
    return p;

}


string* Packet::serialize(Packet* p)
{
    
    

    json11::Json my_json = json11::Json::object {
        {"SEQ", p->SEQ},
        {"ACK", p->ACK},
        {"FIN", p->FIN},
        {"SYN", p->SYN},
        {"checksum", p->checksum},
        {"content_length", p->content_length},
        {"content", p->content},
    };
    std::string json_str = my_json.dump();

    string* newstring = new string(json_str);
    return newstring;
    

}

Packet* Packet::deserialize(string *str)
{
    
    
    string err="";
    json11::Json json = json11::Json::parse(*str, err);
    if (err!="") {
        cout<<"ERROR parsing json: "<<err<<endl;
        cout<<"String: "<<*str<<endl;
    }
    
    Packet *p = new Packet();

    p->SEQ = json["SEQ"].int_value();
    p->ACK = json["ACK"].int_value();
    p->FIN = json["FIN"].int_value();
    p->SYN = json["SYN"].int_value();
    p->checksum = json["checksum"].int_value();
    p->content_length = json["content_length"].int_value();
    p->content = json["content"].string_value();

    
    return p;
    
}

uint16_t Packet::computeChecksum(std::string content)
{
    int length = content.size();
    uint16_t temp;
    uint32_t sum = 0;
    
    // sums up 16 bit words made up of adjacent 8 bit words
    for (int i = 0; i < length; i += 2){
        temp = ((content[i] << 8) & 0xFF00) + (content[i+1] & 0xFF);
        sum += (uint32_t) temp;
    }
    // take 16 bits out of sum and add the carries
    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);
    
    //returns checksum as complement of sum
    return ((uint16_t) ~sum);
}


void Packet::setContentAndChecksum(std::string content)
{
    this->content = content;
//    cerr<<"Content: "<<content<< "This->Content: "<<this->content<<endl;
    this->checksum = Packet::computeChecksum(this->content);
    this->content_length = this->content.size();

}




void Packet::setBContentAndChecksum(char* content, int len)
{
    this->bcontent = new char[len+1]();
    memcpy(this->bcontent, content, len);
//        cerr<<"Content: "<<content<< "This->Content: "<<this->content<<endl;
    this->checksum = Packet::computeChecksum(this->content);
    this->content_length = len;
    
}



/*
 stringstream ss;
 ss << "source_port: " << p->source_port;
 ss << "\n";
 ss << "dest_port: " << p->dest_port;
 ss << "\n";
 
 ss << "SEQ: " << p->SEQ;
 ss << "\n";
 ss << "ACK: " << p->ACK;
 ss << "\n";
 ss << "FIN: " << p->FIN;
 ss << "\n";
 ss << "SYN: " << p->SYN;
 ss << "\n";
 ss << "content_length: " << p->content_length;
 ss << "\n";
 ss << "checksum: " << p->checksum;
 ss << "\n\n";
 ss << p->content;
 //    ss << string(p->content, p->content_length);
 
 //    cout<<"\ncontent: "<<p.content<<"\n content2: "<<string(p.content, p.content_length)<<endl;
 
 string *s_ptr = new string(ss.str());
 return s_ptr;
 */

/*
 Packet *p = new Packet();
 
 stringstream ss(*str);
 
 string buf;
 
 getline(ss, buf);
 sscanf(buf.c_str(), "source_port: %d", &(p->source_port));
 getline(ss, buf);
 sscanf(buf.c_str(), "dest_port: %d", &(p->dest_port));
 
 getline(ss, buf);
 sscanf(buf.c_str(), "SEQ: %d", &(p->SEQ));
 getline(ss, buf);
 sscanf(buf.c_str(), "ACK: %d", &(p->ACK));
 getline(ss, buf);
 sscanf(buf.c_str(), "FIN: %hd", &(p->FIN));
 getline(ss, buf);
 sscanf(buf.c_str(), "SYN: %hd", &(p->SYN));
 
 getline(ss, buf);
 sscanf(buf.c_str(), "content-length: %d", &(p->content_length));
 getline(ss, buf);
 sscanf(buf.c_str(), "checksum: %hd", &(p->checksum));
 getline(ss, buf);
 
 
 //copy message content
 
 istreambuf_iterator<char> buf_iteartor;
 string content = string(istreambuf_iterator<char>(ss), buf_iteartor);
 p->content = content;
 */



