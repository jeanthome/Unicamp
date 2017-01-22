/*
 * Code from http://www.unpbook.com/
 *
 *
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <string.h>

#define LISTENQ 5
#define MAXLINE 64
#define SERV_PORT 56789

int main(int argc, char **argv)
{
    int					i, maxi, maxfd, listenfd, connfd, sockfd;
    int					nready, client[FD_SETSIZE];
    ssize_t				n;
    fd_set				rset, allset;
    char				buf[MAXLINE];
    socklen_t			clilen;
    struct sockaddr_in	cliaddr, servaddr;
    
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        return 1;
    }
    
  //  printf("FDSIZE: %d\n", FD_SETSIZE);
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(SERV_PORT);
    
    
    
    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)  {
        perror("bind error");
        close(listenfd);
        return 1;
    }
    
    if (listen(listenfd, LISTENQ) < 0) {
        perror("listen error");
        close(listenfd);
        return 1;
    }
    
    maxfd = listenfd;			/* initialize */
    
    //printf("Listen FD: %d\n", listenfd);
    maxi = -1;					/* index into client[] array */
    for (i = 0; i < FD_SETSIZE; i++)
        client[i] = -1;			/* -1 indicates available entry */
    
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    

    
    while (1) {
        
        rset = allset;		/* structure assignment */
        nready = select(maxfd+1, &rset, NULL, NULL, NULL);
		if (nready == 0)
			perror("Select timeout");
		else if (nready < 0)
			perror("Select error");
	
        
       // printf("%d\n", nready);
        //If something happened on the master socket , then its an incoming connection
        if (FD_ISSET(listenfd, &rset)) {	/* new client connection */
            
            printf("Nova Conexão\n");
            clilen = sizeof(cliaddr);
            connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
			if (connfd < 0)
				perror("Accept error");
            
            for (i = 0; i < FD_SETSIZE; i++)
                if (client[i] < 0) {
                    client[i] = connfd;	/* save descriptor */
                   // printf("DesF: %d\n", connfd);
                    break;
                }
            if (i == FD_SETSIZE) {
                perror("too many clients");
                exit(1);
            }
            
            FD_SET(connfd, &allset);	/* add new descriptor to set */
            if (connfd > maxfd)
                maxfd = connfd;			/* for select */
            
            if (i > maxi)
                maxi = i;				/* max index in client[] array */
            
            if (--nready <= 0)
                continue;				/* no more readable descriptors */
        }
        
        for (i = 0; i <= maxi; i++) {	/* check all clients for data */
            
           // printf("%d ", i);
            
            if ( (sockfd = client[i]) < 0)
                continue;
            
            //printf("Activity on %d\n", sockfd);
            if (FD_ISSET(sockfd, &rset)) {
				n = read(sockfd, buf, MAXLINE);
                if ( n == 0) {
                    
                   ///printf("Fechou\n");
                    /* connection closed by client */
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                } else if(n < 0) {
					perror("Read error");
				} else {
                    
                   //printf("Recebeu:\n");
                    fputs(buf, stdout);
                    if (send(sockfd, buf, n, 0) < 0)
						perror("Send error");
                    
                    
                }
                
                if (--nready <= 0)
                    break;				/* no more readable descriptors */
            }
        }
    }
}
