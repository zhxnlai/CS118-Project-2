//
//  utils.c
//  cs118p2
//
//  Created by Zhixuan Lai on 5/30/14.
//  Copyright (c) 2014 Zhixuan Lai. All rights reserved.
//

#include "utils.h"

int make_nonblocking( int sock )
{
    /* get existing socket flags */
    int flags = fcntl (sock, F_GETFL, 0 );
    /* switch socket to non-blocking mode */
    int res = fcntl( sock, F_SETFL, flags | O_NONBLOCK );
    if ( res == -1 )
    {
        printf("failed to make socket %d non-blocking\n",sock);
        return 0;
    }
    else
        return 1;
}
void error(char *msg)
{
    perror(msg);
    exit(1);
}


void handleHTTPRequest(char*buffer, int sock)
{
    int n;
    int file_size; char *content_type;
    char *file_data = fileRequestedByHTTPHeader(buffer, &file_size, &content_type);
    if (file_data)
    {
        //generate header
        
        int MAX_HEADER_SIZE = 1024;
        
        char header[MAX_HEADER_SIZE];
        bzero((char *) &header, sizeof(header));
        
        snprintf(header, MAX_HEADER_SIZE, "HTTP/1.1 200 OK\nConnection: close\nContent-Length: %i\nContent-Type: %s\n\n", file_size, content_type);
        
        fprintf(stderr,"%s", header);
        
        
        int reply_len = strlen(header)+file_size+1;
        char reply[reply_len];
        bzero((char *) &reply, sizeof(reply));
        
        
        memcpy(reply, header, strlen(header));
        memcpy(reply+strlen(header), file_data, file_size);
        
        
        //printf("reply len: %i\n", reply_len);
        
        n = write(sock,reply,reply_len);
        if (n < 0) error("ERROR writing to socket");
        
    }
    else
    {
        char *message = "<h1>File Not Found :(</h1>";
        n = write(sock,message,strlen(message));
        if (n < 0) error("ERROR writing to socket");
    }

}


char *fileRequestedByHTTPHeader(char *header, int *size, char **ct)
{
    char *ret = NULL;
    
    
    char *rawsubstr = substrWithRegex(header, "^GET .* HTTP");
    //    fprintf(stderr, "%s\n", rawsubstr);
    
    char substr[1024];
    strncpy(substr, rawsubstr+4, strlen(rawsubstr)-9);
    //    fprintf(stderr, "%s\n", substr);
    //    char *substr = substrWithRegex(buffer, "(?<=^GET ).*(?= HTTP)");
    
    if (substr) {
        
        char address[sizeof(substr)+1];
        snprintf(address, sizeof(address), "%s%s", ".", substr);
        
        //printf("File address: %s\n", address);
        
        int ret_size;
        
        char *file_data = readFile(address, &ret_size);
        *size = ret_size;
        
        if (file_data) {
            ret = file_data;
            //free(file_data);
            
            char *ctt = substrWithRegex(substr, "\\..*$");
            
            if (ctt)
            {
                *ct = contentTypeFromString(ctt);
            }
        }
        else {
            fprintf(stderr,"requested file not found.\n");
            
        }
        
    }
    return ret;
}

char *substrWithRegex(char *buffer, char *reg)
{
    char *ret=NULL;
    
    regex_t regex;
    int reti;
    char msgbuf[100];
    
    regmatch_t match[1];
    /* Compile regular expression */
    
    reti = regcomp(&regex, reg, 0);
    if( reti ){ fprintf(stderr, "Could not compile regex\n"); exit(1); }
    
    /* Execute regular expression */
    reti = regexec(&regex, buffer, 1, match, 0);
    if( !reti ){
        //        fprintf(stderr, "match 1: %lli 2:%lli ", match[0].rm_so, match[0].rm_eo);
        
        char ctt[1024];
        snprintf(ctt, 1024, "%.*s", match[0].rm_eo-match[0].rm_so, buffer+match[0].rm_so);
        ret = strdup(ctt);
        
        //        fprintf(stderr,  "matched string: %s\n", ctt);
    } else if( reti == REG_NOMATCH ){
        puts("No match\n");
    }
    else{
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex match failed: %s\n", msgbuf);
        exit(1);
    }
    
    /* Free compiled regular expression if you want to use the regex_t again */
    regfree(&regex);
    
    return ret;
}

char* readFile(char *filename, int *size)
{
    char *buffer = NULL;
    int string_size,read_size;
    FILE *handler = fopen(filename,"rb");
    
    if (handler)
    {
        //seek the last byte of the file
        fseek(handler,0,SEEK_END);
        //offset from the first to the last byte, or in other words, filesize
        string_size = ftell (handler);
        //go back to the start of the file
        
        *size = string_size;
        rewind(handler);
        //fseek(handler,0,SEEK_SET);
        //allocate a string that can hold it all
        buffer = (char*) malloc (sizeof(char) * (string_size + 1) );
        //read it all in one operation
        read_size = fread(buffer,sizeof(char),string_size,handler);
        //fread doesnt set it so put a \0 in the last position
        //and buffer is now officialy a string
        buffer[string_size+1] = '\0';
        
        if (string_size != read_size) {
            //something went wrong, throw away the memory and set
            //the buffer to NULL
            free(buffer);
            buffer = NULL;
        }
    }
    
    return buffer;
}

char *contentTypeFromString(char *s)
{
    char *ret = NULL;
    if (!strcmp(s, ".html"))
    {
        ret = "text/html";
    } else if (!strcmp(s, ".jpg"))
    {
        ret = "image/jpeg";
    } else if (!strcmp(s, ".png"))
    {
        ret = "image/png";
    } else if (!strcmp(s, ".gif"))
    {
        ret = "image/gif";
    }
    
    return ret;
}