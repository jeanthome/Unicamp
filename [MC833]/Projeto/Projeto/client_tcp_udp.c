//
//  client_tcp_udp.c
//  Task7
//
//  Created by Jean Thomé on 6/9/16.
//  Copyright © 2016 Jean Thomé. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#define SERVER_PORT 56789
#define MAX_PENDING 5
#define MAX_LINE 64
#define TCP 1
#define UDP 2


int main(int argc, char * argv[]){
    
    struct hostent *hp;
    struct sockaddr_in sin;
    int s_len = sizeof(sin);
    char *host;
    int protocolId;
    char buf[MAX_LINE];
    int s;
    int len;
    
    if (argc == 4) {
        host = argv[1];
        protocolId =  atoi( argv[2] );
        
        if ( protocolId != TCP && protocolId != UDP) {
            fprintf(stderr, "Invalid protocolID\n");
            fprintf(stderr, "usage: ./client host protocolID\nUse '1' to TCP and '2' to UDP\n");
            exit(1);
        }

    }else {
        fprintf(stderr, "usage: ./client host protocolID\nUse '1' to TCP and '2' to UDP\n");
        exit(1);
    }
    
    
    /* translate host name into peer’s IP address */
    hp = gethostbyname(host);
    if ( !hp ) {
        fprintf(stderr, "simplex-talk: unknown host: %s\n", host);
        exit(1);
    }
    
    /* build address data structure */
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    sin.sin_port = htons(SERVER_PORT);
    
        
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("simplex-talk: socket");
        exit(1);
    }

    
    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("simplex-talk: connect");
        close(s);
        exit(1);
    }

    /* main loop: get and send lines of text */
    while (fgets(buf, sizeof(buf), stdin)) {
        
        buf[MAX_LINE-1] = '\0';
        len = strlen(buf) + 1;
        send(s, buf, len, 0);
        
        if(recv(s, buf, sizeof(buf), 0) ){
            fputs(buf, stdout);
        }
    }


}