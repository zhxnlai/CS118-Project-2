//
//  utils.h
//  cs118p2
//
//  Created by Zhixuan Lai on 5/30/14.
//  Copyright (c) 2014 Zhixuan Lai. All rights reserved.
//

#ifndef cs118p2_utils_h
#define cs118p2_utils_h

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
#include <fcntl.h>

//const int MAX_REGEX_MATCH_LENGTH = 1024;
int make_nonblocking( int sock );

void error(char *msg);

void handleHTTPRequest(char*buffer, int sock);

char *fileRequestedByHTTPHeader(char *header, int *size, char **ct);
char *substrWithRegex(char *buffer, char *reg);

char *contentTypeFromString(char *s);
char* readFile(char *filename, int *size);



#endif
