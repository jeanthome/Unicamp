//
//  Client.cpp
//  Projeto
//
//  Created by Jean Thomé and Gabriel Bueno on 6/18/16.
//  Copyright © 2016 Jean Thomé. All rights reserved.
//

#include <stdio.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "Common.hpp"
#include "json.hpp"
#include <string>
#include <ncurses.h>
#include <functional>
#include <ctime>
#include <regex>


using namespace std;
using json = nlohmann::json;


class Client{
    
private:
    string _name;
    int _sock;

public:
    
    Client( string name , int sock ) {
        this->_name = name;
        this->_sock = sock;
    }
    
    
    string getUserName() {
        return _name;
    }
    int getSock(){
        return  _sock;
    }
    
    size_t msg_hash( string sender, string receiver, string msg, time_t time ){
        
        ostringstream oss;
        oss << receiver << sender << msg << to_string( time );
        hash<string> str_hash;
        return str_hash( oss.str() );
        
    }
    
    void sendString( string stringToBeSent ){
        char buf[MAXLINE];
        int len;
        strcpy( buf, stringToBeSent.c_str() );
        buf[MAXLINE-1] = '\0';
        len = (int)strlen(buf) + 1;
        //printf("Enviando : %s\n", stringToBeSent.c_str() );
        send(this->_sock, buf, len, 0);
    }
    
    void sendIdentification(){
        
        json identification;
        identification["CMD"] = IDENTIFICATION;
        identification[USERNAME] = this->_name;
        string finalJSON = identification.dump();
        sendString( finalJSON );
    }
    
    void closeClientSocket(){
        close( this->_sock );
    }
    
};



int main(int argc, char * argv[]) {
    //./client <ip> <port> <name>
    struct hostent *he;
    struct in_addr addr;
    struct sockaddr_in sin;
    fd_set readSet;
    int serverPort, sock, len;
    char *host;
    char buf[MAXLINE];
    char cmdBuf[MAXLINE];
    char line;
    int irow,icol,col,row ,text_size = 7;
	int prow =1, shell = 1;
	WINDOW *msgs;
	WINDOW *text;
	initscr();
    
    
    /*================*/
    char *token;
    

	//cbreak();
	getmaxyx(stdscr,irow,icol);
	// Janela de mensgens		
	msgs = newwin(irow - text_size, icol, 0, 0);
	//box(msgs,0,0);
	scrollok(msgs, TRUE);
	wmove(msgs, 1, 1);
	
	//Janela de texto
	text = newwin(text_size,icol, irow-text_size,0 ); 	
	box(text,0,0);
	col = icol;
	row = irow;
	wrefresh(msgs);	
	wrefresh(text);	
    
    if ( argc != 4) {
		// janela de mensagens	
		wmove(msgs,prow,1);
		wprintw (msgs, "Erro nos parametros.\n", 1,1);
		prow++;
        //printf("Erro nos parametros.\n");
    }
    
    
    host = argv[1];
    serverPort = atoi( argv[2] );
    
    inet_aton(host, &addr);
    he = gethostbyaddr(&addr, sizeof(addr), AF_INET);

    /* build address data structure */
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy( he->h_addr, (char *)&sin.sin_addr, he->h_length);
    sin.sin_port = htons(serverPort);

    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("simplex-talk: socket");
        delwin(msgs);
		delwin(text);
		endwin();
        exit(1);
    }
    
    
    if (connect(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("simplex-talk: connect");
        close(sock);
        delwin(msgs);
		delwin(text);
		endwin();
        exit(1);
    }
    
    /*Instancia um novo cliente*/
    Client newClient = Client( string( argv[3]), sock);
    
    newClient.sendIdentification();
    
    /* main loop: get and send lines of text */
    
    FD_ZERO(&readSet);
    
    while ( /*getline(cin, line)*/1 ) {
        
        // janela de mensagens
        wmove(msgs,prow,1);
        //wprintw (msgs, "%s", line,1,1); //isso vai mudar quando as mensagens chegarem
        //prow++;
        wrefresh(msgs);
        
        //janela de textos
        wmove(text, 1,1);
//        wgetnstr(text,cmdBuf,MAXLINE);		// aqui eu pego a entrada do programa
        wrefresh(text);

        FD_SET( newClient.getSock(), &readSet );
        FD_SET( fileno(stdin), &readSet );
        
        select( max( newClient.getSock(), fileno(stdin) ) + 1, &readSet, NULL, NULL, NULL );
		
        
        if ( FD_ISSET( newClient.getSock(), &readSet) ) {

            //printf("1\n\n");
            if ( recv( newClient.getSock(), buf, sizeof(buf), 0) ) {
                
//                
//                regex TEMPREGEX("\\{(\\s*\"(.+?)\\s*\"\\s*:\\s*(.+?)\\s*)\\}");
//                
//                if ( !regex_match( buf, TEMPREGEX) ) {
//                    
////                    
////                    wmove(msgs,prow,1);
////                    wprintw (msgs, "NAO EH JSON\n", NULL, 1,1);// ta bugado a tela preciso arrumar
////                    prow++;
//                    continue;
//                } //else {
//                    //wmove(msgs,prow,1);
//                   // wprintw (msgs, "EH JSON\n", NULL, 1,1);// ta bugado a tela preciso arrumar
//                  //  prow++;
//                
                //}
                
                //printf("%s\n", buf);
                
                json result = json::parse( string(buf) );
                
                if ( result.count(WHO) ) {  /*Resposta à requisicao WHO*/
                    
                    //printf("2\n\n");
                    wmove(msgs,prow,1);
                    wprintw( msgs, "|%*s%*s|", 13, "USUARIO", 7, "", 1, 1 );
                    wprintw( msgs, "%*s%*s|", 13, "STATUS", 7, "", 1, 1 );
                    prow++;

                    json::iterator it;
                    for (  it = result.begin(); it != result.end(); it++) {
                        
                        string userName = it.key();
                        bool status = it.value();
                        
                        if ( userName != WHO) {

                            string sstatus = (status == true)? "Online": "Offline";
                            
                            wmove(msgs,prow,1);
                            
                            len = userName.length();
                            wprintw( msgs, "|%*s%*s|", 10+len/2, userName.c_str(), 10-len/2, "", 1, 1 );
                            
                            len = sstatus.length();
                            wprintw( msgs, "%*s%*s|\n", 10+len/2, sstatus.c_str(), 10-len/2, "", 1, 1 );
                            prow++;
                        }
                    }
                }else if( result.count(MSG) ){


                    //printf("3\n\n");
                    string response = result[MSG];
                    wmove(msgs,prow,1);
                    wprintw (msgs, "%s\n", response.c_str(), 1,1);// ta bugado a tela preciso arrumar
                    prow++;
                    
                }
            }
        }/*Fim do FD_ISSET (socket) */

        if( FD_ISSET( fileno(stdin ), &readSet) ){
            
            wgetnstr(text,cmdBuf,MAXLINE);		// aqui eu pego a entrada do programa
            wrefresh(text);

            token = strtok (cmdBuf," ");
            if ( token != NULL ) {
                
                json message;
                if ( !strcmp( token, "EXIT") ) {
                    
                    message["CMD"] = EXIT;
                    message[USERNAME] = newClient.getUserName();

                    newClient.sendString( message.dump() );
                    newClient.closeClientSocket();
                    
                    delwin(msgs);
                    delwin(text);
                    endwin();
                    
                    return 0;
                    
                }else if ( !strcmp( token, "WHO") ){
                    
                    message["CMD"] = WHO;
                    message[USERNAME] = newClient.getUserName();
                    
                    newClient.sendString( message.dump() );

                }else if ( !strcmp(token, "CREATEG") ) {
                    
                    token = strtok (NULL," ");
                    
                    if ( token != NULL) {
                        
                        message["CMD"] = CREATEG;
                        message[USERNAME] = newClient.getUserName();
                        message["GROUPNAME"] = token;

                        newClient.sendString( message.dump() );
                        
                    } else {
                        wmove(msgs,prow,1);
                        wprintw (msgs, "Erro de sintaxe.\n", NULL, 1,1);// ta bugado a tela preciso arrumar
                        prow++;
                    }
                
                } else if ( !strcmp( token, "JOING") ){
                    token = strtok (NULL," ");
                    
                    if ( token != NULL) {
                        
                        message["CMD"] = JOING;
                        message[USERNAME] = newClient.getUserName();
                        message["GROUPNAME"] = token;
                        
                        newClient.sendString( message.dump() );
 
                    } else {
                        wmove(msgs,prow,1);
                        wprintw (msgs, "Erro de sintaxe.\n", NULL, 1,1);// ta bugado a tela preciso arrumar
                        prow++;
                    }
                    
                }else if ( !strcmp( token, SEND ) || !strcmp(token, SENDG ) ){
                    
                    if ( !strcmp( token, SEND)) {
                        message["CMD"] = SEND;
                    }else{
                        message["CMD"] = SENDG;
                    }
                    
                    token = strtok( NULL, " ");

                    if ( token != NULL) {

                        message["RECEIVER"] = token;

                        token = strtok( NULL, "\"");

                        if ( token != NULL ) {

                            message[MSG] = token;
                            message[USERNAME] = newClient.getUserName();
                            size_t messageId = newClient.msg_hash( newClient.getUserName(), message["RECEIVER"], message[MSG], time(nullptr) );
                            message[MSGID] = messageId;

                            newClient.sendString( message.dump() );

                            wmove(msgs,prow,1);
                            wprintw (msgs, "%zu enviada!\n", messageId%MSGID_DIVISOR , 1,1);// ta bugado a tela preciso arrumar
                            prow++;
                        }
                    }
                }/*Fim do SEND*/
            }
        }

		wmove(text, 1,1);	
		wprintw (text, "                                                                                                                         ", line);

    }/*Fim do While principal*/
	delwin(msgs);
	delwin(text);
	endwin();
    return 0;
}



