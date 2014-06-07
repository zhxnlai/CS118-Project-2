/* A simple server in the internet domain using TCP
 The port number is passed as an argument
 This version runs forever, forking off a separate
 process for each connection
 */
#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <strings.h>
#include <sys/wait.h> /* for the waitpid() system call */
#include <signal.h> /* signal name macros, and the kill() prototype */

#include <unistd.h>

#include <regex.h>
#include <string.h>

#include "utils.h"
#include "Packet.h"
#include "Server.h"

int WINDOW_SIZE=5 ;
float LOSS_PROBABILITY=0 ;
float CORRUPTION_PROBABILITY=0;

void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

void dostuff(int); /* function prototype */

void test()
{
    char *teststring = "POST /test/demo_form.asp HTTP/1.1\nHost: w3schools.com\nname1=value1&name2=value2";
    
    fprintf(stderr, "%s\n", substrWithRegex(teststring, "."));
    fprintf(stderr, "%s\n", substrWithRegex(teststring, "(?:POST) "));
    
};

//using namespace std;



int main(int argc, char *argv[])
{
    //======= insert unit test
    //    test();
    //=======
    
    
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    
    int portno = atoi(argv[1]);
    
    struct sigaction sa;          // for signal SIGCHLD
    /****** Kill Zombie Processes ******/
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    /*********************************/
    
    
    
    struct sockaddr_in myaddr;	/* our address */
	struct sockaddr_in remaddr;	/* remote address */
	int fd;				/* our socket */
    
    
	/* create a UDP socket */
    
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create socket\n");
		return 0;
	}
    
	/* bind the socket to any valid IP address and a specific port */
    
	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(portno);
    
	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return 0;
	}
    
    make_nonblocking(fd);
    
    if (argc == 5) {
        //set window size... maybe validate?
        WINDOW_SIZE = atoi(argv[2]);
        LOSS_PROBABILITY = atof(argv[3]);
        CORRUPTION_PROBABILITY = atof(argv[4]);
        Server server(portno, fd, myaddr, remaddr, WINDOW_SIZE, LOSS_PROBABILITY, CORRUPTION_PROBABILITY);
        server.startListening();
    } else {
        Server server(portno, fd, myaddr, remaddr);
        server.startListening();
    }
    
}


//
//void sendPacket(int i, int sock, vector<Packet*>* packets)
//{
//    //resend packets every recieving a packet
//
//    if (i>packets->size()) {
//        fprintf(stderr, "Index out of bound!: %d\n", i);
//        return;
//    }
//    Packet * p = (*packets)[i];
//    fprintf(stderr, "Sending packet # %d\n", i);
//    writePacket(sock, p);
//
//}
//
//
//void sendPackets(int currentPacketIndex, int packetCount, int sock, vector<Packet*>* packets)
//{
//    //resend packets every recieving a packet
//    int i_max = min(packetCount, currentPacketIndex+WINDOW_SIZE);
//    for (int i=currentPacketIndex; i<i_max; i++) {
//        Packet * p = (*packets)[i];
//        fprintf(stderr, "Sending packet # %d\n", i);
//        writePacket(sock, p);
//    }
//}

/******** DOSTUFF() *********************
 There is a separate instance of this function
 for each connection.  It handles all communication
 once a connnection has been established.
 *****************************************/
//void dostuff (int sock)
//{
//    int n, packet_count=0;
//    char buffer[PACKET_SIZE];
//
//    bool handshaked = false;
//    int handshakePhase = 0;
//
//    vector<Packet*>* packets = new vector<Packet*>();
//    int currentPacketIndex = 0;
//    int packetCount = 0;
//
//    int prevACK = 0;
//
//    //enter a loop
//    while (1) {
//        bzero(buffer,PACKET_SIZE);
//        n = read(sock,buffer, PACKET_SIZE);
//        if (n < 0) {
//            error("ERROR reading from socket");
//            return;
//        }
//        packet_count++;
//        fprintf(stderr, "Recieving the # %d packet\n", packetCount);
//
//        fprintf(stderr, "Here is the message: %s\n",buffer);
//        string buffer_string(buffer);
//        Packet *p = Packet::deserialize(&buffer_string);
//
//
//        if (!handshaked) {
//            if (p->SYN!=0) {
//                error("Handshaking packet does not have significant SYN bit");
//            }
//            if (handshakePhase==0) {
//                Packet *response = Packet::handshakePacketPhase2(p->source_port, p->dest_port);
//                string *serializedResponse = Packet::serialize(response);
//                n = write(sock,serializedResponse,serializedResponse->size());
//                if (n < 0) error("ERROR writing to socket");
//                prevACK = p->ACK;
//
//                handshakePhase++;
//            }
//            else if (handshakePhase==1) {
//                int fileSize = 0;
//                string fileName = p->content;
//                char *fileName_cstr = new char[fileName.length()+1];
//                strcpy(fileName_cstr, fileName.c_str());
//                char *fileData = readFile(fileName_cstr, &fileSize);
//                string fileDataString(fileData);
//
////                fprintf(stderr, "File Data: %s\n", fileData);
//
//                packets = Packet::packetsFromFile(fileDataString, p->SEQ);
//                packetCount = packets->size();
//                handshaked=true;
//
//                prevACK = p->ACK;
//
//                fprintf(stderr, "Handshake successful!");
//
////                sendPackets(currentPacketIndex, packetCount, sock, packets);
//
//            }
//        } else {
//            if (currentPacketIndex==packetCount) {
//                fprintf(stderr, "File is sent");
//                break;
//            }
//            if (p->ACK==prevACK+1) {
//                currentPacketIndex++;
//                fprintf(stderr, "sliding the window, current index: %d\n", currentPacketIndex);
//                int index = currentPacketIndex+WINDOW_SIZE;
//                if(index<packetCount) {
//                    sendPacket(index, sock, packets);
//
//                }
//
//            }
//            else {
//                //ack not as expected
//                sendPackets(currentPacketIndex, packetCount, sock, packets);
//            }
//
//
//        }
//
//
//    }
//
//}







//int main(int argc, char *argv[])
//{
//    //======= insert unit test
//    //    test();
//    //=======
//
//
//    int sockfd, newsockfd, portno, pid;
//    socklen_t clilen;
//    struct sockaddr_in serv_addr, cli_addr;
//    struct sigaction sa;          // for signal SIGCHLD
//
//    if (argc < 2) {
//        fprintf(stderr,"ERROR, no port provided\n");
//        exit(1);
//    }
//
//    if (argc == 5) {
//        //set window size... maybe validate?
//        WINDOW_SIZE = atoi(argv[2]);
//        LOSS_PROBABILITY = atoi(argv[3]);
//        CORRUPTION_PROBABILITY = atoi(argv[4]);
//    }
//
//    sockfd = socket(AF_INET, SOCK_STREAM, 0);
//    if (sockfd < 0)
//        error("ERROR opening socket");
//    bzero((char *) &serv_addr, sizeof(serv_addr));
//    portno = atoi(argv[1]);
//    serv_addr.sin_family = AF_INET;
//    serv_addr.sin_addr.s_addr = INADDR_ANY;
//    serv_addr.sin_port = htons(portno);
//
//    if (bind(sockfd, (struct sockaddr *) &serv_addr,
//             sizeof(serv_addr)) < 0)
//        error("ERROR on binding");
//
//    listen(sockfd,5);
//
//    clilen = sizeof(cli_addr);
//
//    /****** Kill Zombie Processes ******/
//    sa.sa_handler = sigchld_handler; // reap all dead processes
//    sigemptyset(&sa.sa_mask);
//    sa.sa_flags = SA_RESTART;
//    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
//        perror("sigaction");
//        exit(1);
//    }
//    /*********************************/
//
//    while (1) {
//        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
//
//        if (newsockfd < 0)
//            error("ERROR on accept");
//
//        pid = fork(); //create a new process
//        if (pid < 0)
//            error("ERROR on fork");
//
//        if (pid == 0)  { // fork() returns a value of 0 to the child process
//            close(sockfd);
//            dostuff(newsockfd);
//            exit(0);
//        }
//        else //returns the process ID of the child process to the parent
//            close(newsockfd); // parent doesn't need this
//    } /* end of while */
//    return 0; /* we never get here */
//}


