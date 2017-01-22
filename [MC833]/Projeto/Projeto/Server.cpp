//
//  Server.cpp
//  Projeto
//
//  Created by Jean Thomé on 6/16/16.
//  Copyright © 2016 Jean Thomé. All rights reserved.
//

#include "Server.hpp"
#include <sys/types.h>

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>
#include "json.hpp"

using json = nlohmann::json;

Server::Server( int serverPort ){
    
    this->users = map<string, User >();
    this->messages = map<size_t, Message>();
    this->groups = map<string, Group>();
    this->connectionsFd = set<int>();
    FD_ZERO(&(this->readSet));
    

    /*Inicializa estrutura do endereço*/
    bzero( &serverAddress, sizeof(serverAddress) );
    serverAddress.sin_family        = AF_INET;
    serverAddress.sin_addr.s_addr   = htonl( INADDR_LOOPBACK );
    serverAddress.sin_port          = htons( serverPort );    
}

int Server::serverSocket(){
    
    
    listenSocket = socket( AF_INET, SOCK_STREAM, 0 );
    
    printf("Socket %d\n", listenSocket );
    if ( listenSocket != -1) {
        FD_ZERO(&allSet);
        FD_SET(listenSocket, &allSet);
    }
    
    return listenSocket;
}

int Server::serverBind(){
    
    printf("Bind()\n");
    return ::bind(listenSocket, (struct sockaddr*)&serverAddress, sizeof( struct sockaddr) );
}

int Server::serverListen(){
    
    printf("Listen()\n");
    return ::listen( listenSocket, MAX_PENDING );
}

void Server::closeSocket(){
    close( this->listenSocket );
}



void Server::insertUser( User userToBeInserted ) {
    
    this->users.insert( make_pair( userToBeInserted.getName(), userToBeInserted ) );
    printf("Usuário $[%s] inserido com sucesso.\n", userToBeInserted.getName().c_str() );
    
}

void Server::insertGroup( Group groupToBeInserted ) {

    printf("Grupo %s [%zu] inserido com sucesso!\n", groupToBeInserted.getName().c_str(), groupToBeInserted.nextGroupId );
    printf("Grupo com %s usuarios.\n", groupToBeInserted.getUsersName().front().c_str() );
    this->groups.insert( make_pair( groupToBeInserted.getName() , groupToBeInserted ) );

}
void Server::insertMessage( Message messageToBeInserted ) {
    
    this->messages.insert( make_pair( messageToBeInserted.getId(), messageToBeInserted ) ) ;
}

void Server::insertConnectionFd( int fdToBeInserted ){

    FD_SET(fdToBeInserted, &allSet);
    this->connectionsFd.insert( fdToBeInserted );
    
    
    set<int>::iterator it;
    
    
    printf("[ ");
    for ( it = connectionsFd.begin(); it != connectionsFd.end(); it++) {
        printf("%d, ", (*it) );
    }
    printf("]\n");
}

bool Server::insertUserOnGroup( User userToBeInserted, string groupName ){

    if ( this->groups.find( groupName)->second.hasUserWithName( userToBeInserted.getName() ) ) {
        return false;
    }

    this->groups.find( groupName )->second.insertUser( userToBeInserted );
    printf("Grupo %s com %d usuarios.\n", groupName.c_str(), (int)groups.find( groupName)->second.getUsersName().size() );
    return true;
    
}

void Server::insertNewMessageOnUser( User receiverUser, Message messageToBeInserted ){
    
    this->users.find( receiverUser.getName() )->second.insertNewMessageIdOnQueue( messageToBeInserted.getId() );
    
    printf("Usuario %s com %d mensagens na fila.\n", receiverUser.getName().c_str(), this->users.find( receiverUser.getName() )->second.hasPendingMessages() );
}

void Server::insertNewResponseOnUser( User receiverUser, string response ){
    this->users.find( receiverUser.getName() )->second.insertNewResponseOnQueue( response );
}

void Server::setUserOnLine( User userToBeOnline, int connectionFd ){
    
    this->users.find(userToBeOnline.getName())->second.setConnectionFd( connectionFd );
}

void Server::setUserOffLine( User userToBeOffline ){
    printf("Offline");
    
    this->users.find( userToBeOffline.getName() )->second.setConnectionFd( -1 );
    //users[ userToBeOffline.getName() ].setConnectionFd( -1 );
}

void Server::deleteMessage( Message messageToBeDeleted ){
    
    this->messages.erase( messageToBeDeleted.getId() );
}

void Server::removeConnectionFd(int fdToBeRemoved ){
    FD_CLR(fdToBeRemoved, &allSet);
    this->connectionsFd.erase( fdToBeRemoved );
}

bool Server::hasUserWithName( string name ){
    
    return ( (users.count( name ) == 1 )? true: false );
}

int Server::hasPendingClient(){
    
    return FD_ISSET( this->listenSocket, &(this->readSet) );
}

bool Server::hasGroupWithName( string name ){
    return ( (groups.count( name ) == 1 )? true: false );
}

bool Server::sendMessagesToUser( /*Message messageToBeSenT,*/ User user ){
    
    printf("Comecou a enviar...\n");
    string userName = user.getName();

    printf("Responses...\n");
    while ( this->getUserWithName( userName).hasPendingResponses() && this->getUserWithName(userName).isOnline() ) {
        
        string responseMSG = this->getUserWithName(userName).getFrontResponse();
        json response;
        response[MSG] = responseMSG;
        
        printf("ResOnQ: %s \t", responseMSG.c_str() );
        if ( this->sendStringToUser( response.dump(), user) ) {
            printf("\t\tEnviou\n");
            this->users.find( userName )->second.popFrontResponse();
        }
    }
    
    printf("\nMSGS...\n");
    while ( this->getUserWithName(userName).hasPendingMessages() && this->getUserWithName(userName).isOnline() ) {
        
        size_t nextMessageId = this->getUserWithName(userName).getFrontMessageId();
        
        printf("H[%zu]: ", nextMessageId );
        
        Message msgCopy = this->messages.find( nextMessageId )->second;
        msgCopy.printContent();
        
        string msgFinal;
        if ( !msgCopy.messageFromGroup() ) {
            msgFinal = "[" + msgCopy.getSenderName() + ">] " + msgCopy.getContent();
        }else{
            msgFinal = "[" + msgCopy.getSenderName() + "@" + msgCopy.getGroupName() +">] " + msgCopy.getContent();
        }
            
        json responseMSG;
        responseMSG[MSG] = msgFinal;
        
        if ( this->sendStringToUser( responseMSG.dump(), user ) ) {
            
            printf("\t1\n");
            this->users.find( userName )->second.popFrontMessage();
            this->messages.find( nextMessageId )->second.removeReceiver( userName );
            

            if ( !this->messages.find( nextMessageId)->second.hasPendingReceiver() ) {
                
                json msgDelivered;
                string msg = to_string( nextMessageId%MSGID_DIVISOR ) + " entregue!";
                msgDelivered[MSG] = msg;
                
                
                string sender = msgCopy.getSenderName();
                if ( this->getUserWithName(sender).isOnline() ) {
                    
                    this->sendStringToUser( msgDelivered.dump(), this->getUserWithName( sender ) );
                    
                    printf("Response entregue.\n");
                } else {
                    printf("Response on Queue\n");
                    this->users.find( sender )->second.insertNewResponseOnQueue( msg );
                }
                
                
                /*
                  PENSAR NO CASO DE O DESTINO ESTAR OFFLINE NA HORA DO ENVIO E, QUANDO O DESTINO FICAR ONLINE, A ORIGEM ESTÁ OFFILINE
                  
                  */
            }
        }else{
            
            printf("\t2\n");
        }
    }
    
    return true;
}

int Server::sendMessageToGroup( Message messageToBeSent, Group group ){
    
//    list<string>names = group.getUsersName();
//    list<string>::iterator it;
    int nMessagesSent = 0;
//    
//        for ( it = names.begin(); it != names.end(); it++) {
//
//            printf("Enviando para %s", users.find( (*it) )->second.getName().c_str() );
//            if ( users.find( (*it) )->second.isOnline()  ) {
//                
//                printf("-> Online\n");
//                if ( sendMessagesToUser( users.find( (*it) )->second ) ) { /*Conseguiu inviar a mensagem*/
//                    
//                    messages.find( messageToBeSent.getId() )->second.removeReceiver( (*it) );
//                    nMessagesSent++;
//                }
//                
//            }else{
//                printf("-> Offline\n");
//                users.find( (*it) )->second.insertNewMessageIdOnQueue( messageToBeSent.getId() );
//            }
//        }
    return nMessagesSent;
}

bool Server::sendStringToUser( string stringToBeSent, User user ){
    
    printf("\tSendStringToUser com fd: %d\n", user.getConnectionFd() );
    char buf[MAXLINE];
    buf[MAXLINE-1] = '\0';
    
    strcpy( buf, stringToBeSent.c_str() );
    
    printf("[%s]\n", buf);
    int len = strlen(buf) + 1;
    if ( send( user.getConnectionFd(), buf, len, 0 ) != -1) {
        return  true;
    }
    
    return false;

}

User Server::getUserWithName( string name ){
    
    return users.find(name)->second;
}

int Server::getListenSocket(){
    return listenSocket;
}

set<int>  Server::getConectionsFd() {
    
    set<int>::iterator it;
    return this->connectionsFd;
}

map<string, bool> Server::getUsersStatus(){

    map<string, User >::iterator it;
    map<string, bool > usersStatus;

    for ( it = users.begin(); it != users.end(); it++) {
        
        string userName = it->second.getName();
        bool status = it->second.isOnline();
        usersStatus.insert(make_pair(userName, status) );
        
        printf("---%*s%*s---\n",10+ (int)strlen( userName.c_str() )/2,userName.c_str(),10- (int)strlen(userName.c_str())/2,"");
        printf("\n");
        
    }
    
    return usersStatus;
}

Group Server::getGroupWithName( string name ){
    return groups.find( name )->second;
}

Message Server::getMessageWithId( size_t messageId ){
    return this->messages.find( messageId )->second;
}
