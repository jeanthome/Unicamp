//
//  main.cpp
//  Projeto
//
//  Created by Jean Thomé on 6/15/16.
//  Copyright © 2016 Jean Thomé. All rights reserved.
//

#include <iostream>
#include "json.hpp"
#include <functional>
#include <string>
#include <map>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>
#include "User.hpp"
#include "Server.hpp"
#include "Group.hpp"
#include <sstream>


using namespace std;

using json = nlohmann::json;

size_t msg_hash( string sender, string receiver, string msg );

int main(int argc, const char * argv[]) {
    
    struct sockaddr_in	cliaddr;
    int nReady, connFd, socketFd, maxFd;
    socklen_t clilen;
    set<int>::iterator listIt;
    ssize_t nBytes;
    int serverPort;
    
    vector<int> temp;
    
    
    if ( argc != 2) {
        printf("Erro nos parametros.\n");
        return 0;
    }
    
    Server servidor = Server( atoi( argv[1] ));
    
    if ( servidor.serverSocket() < 0) {
        perror("socket error");
        return 1;
    }
    
    if ( servidor.serverBind() < 0  ) {
        perror("bind error");
        servidor.closeSocket();
        return 1;
    }
    
    if ( servidor.serverListen() < 0) {
        perror("listen error");
        servidor.closeSocket();
        return 1;
    }
    
    maxFd = servidor.getListenSocket();
    
    while (1) {
        
        servidor.readSet = servidor.allSet;
        
        printf("Escutando....\n");
        nReady = select( maxFd + 1 , &(servidor.readSet), NULL, NULL, NULL);
        if (nReady == 0)
            perror("Select timeout");
        else if (nReady < 0)
            perror("Select error");

        if ( servidor.hasPendingClient() ) { /*Nova conexão*/
            
            clilen = sizeof(cliaddr);
            connFd = accept( servidor.getListenSocket(), (struct sockaddr *) &cliaddr, &clilen);
            if (connFd < 0)
                perror("Accept error");
            
            if ( servidor.getConectionsFd().size() == FD_SETSIZE ) {
                perror("too many clients");
                exit(1);
            }
            
            printf("\tNova conexao com fd: %d\n", connFd);
            servidor.insertConnectionFd( connFd );
            temp.push_back( connFd );
            
            maxFd = max(maxFd, connFd );
            
            if (--nReady <= 0) /*Não ha mais descritores para ser lido*/
                continue;
        }
            
        set<int> copyConnectionsFd = servidor.getConectionsFd();
        
        for ( listIt = copyConnectionsFd.begin(); listIt != copyConnectionsFd.end(); listIt++) { /*Analisa os fd abertos*/
            
            socketFd = (*listIt);
            printf("Verificando  " );
            printf("%d ...", socketFd );
            
            if ( FD_ISSET( socketFd, &(servidor.readSet) ) ) {
                printf("..Alterado\n");
                
                nBytes = read( socketFd , servidor.buffer, MAXLINE );
                
                printf("\tNovo pacote com %zd bytes\n", nBytes);
                if ( nBytes == 0) { /*Cliente fechou conexão*/
                    
                    printf("Fechou conexão\n");
                    servidor.removeConnectionFd( socketFd );
                    close( socketFd );
                }else if ( nBytes < 0){
                    perror("Read Error");
                    
                }else{

                    /*
                     AQUI DEVE-SE PROCESSAR A MENSAGEM [Deserializar o JSON]
                     
                     
                     */
                    printf("------------------------------------\n");
                    
                    printf("\n");

                    json receivedJSON = json::parse( servidor.buffer );
                    
                    // special iterator member functions for objects
                    for (json::iterator it = receivedJSON.begin(); it != receivedJSON.end(); ++it) {
                        std::cout << it.key() << " : " << it.value() << "\n";
                    }
                    printf("\n");
                    
                    string userName = receivedJSON[USERNAME];
                    
                    if ( receivedJSON["CMD"] == IDENTIFICATION ) {
                        if ( servidor.hasUserWithName( userName ) ) {
                            
                            printf("Reconexão do usuário: $[%s] com fd %d\n", userName.c_str(), socketFd );
                            User userTemp = servidor.getUserWithName(userName);
                            servidor.setUserOnLine( userTemp, socketFd );
                            servidor.sendMessagesToUser( servidor.getUserWithName(userName) );
                            
                        }else{
                            printf("Novo usuário: $[%s]\n", userName.c_str() );
                            
                            User newUser = User( socketFd, userName );
                            servidor.insertUser( newUser );
                        }
                        
                    }else if ( receivedJSON["CMD"] ==  EXIT ){
                        
                        printf("Usuário $[%s] saiu...\n", userName.c_str() );
                        User userTemp = servidor.getUserWithName( userName );
                        servidor.setUserOffLine( userTemp );
                    
                    }else if ( receivedJSON["CMD"] == WHO ){
                    
                        map<string, bool> users = servidor.getUsersStatus();
                        json j_map(users);
                        j_map[WHO] = true;
                        servidor.sendStringToUser( j_map.dump(), servidor.getUserWithName( userName ) );
                        
                    }else if ( receivedJSON["CMD"] == CREATEG ){

                        string groupName = receivedJSON["GROUPNAME"];
                        
                        json result;
                        
                        if ( servidor.hasGroupWithName( groupName ) ) {
                            result[MSG] = "Já existe um grupo com o nome " + groupName + ".";
                        }else{
                            Group newGroup = Group( groupName );
                            newGroup.insertUser( servidor.getUserWithName( userName ) ) ;
                            servidor.insertGroup( newGroup );
                            result[MSG] = "Grupo " + groupName + " criado com sucesso.";
                        }

                        printf("%s\n", result.dump().c_str() );
                        
                        servidor.sendStringToUser( result.dump(), servidor.getUserWithName( userName ) );
                        
                    }else if ( receivedJSON["CMD"] == JOING ){
                        
                        string groupName = receivedJSON["GROUPNAME"];
                        json result;
                        
                        if ( servidor.hasGroupWithName( groupName )) {
                            
                            if ( servidor.insertUserOnGroup( servidor.getUserWithName( userName ), groupName ) ) {
                                result[MSG] = userName + " entrou no grupo " + groupName + " com sucesso.";
                            }else{
                                result[MSG] = "Usuário " + userName + " já presente no grupo " + groupName + ".";
                            }
  
                        }else{
                            result[MSG] = "Grupo " + groupName + " não encontrado.";
                        }
 
                        printf("%s\n", result.dump().c_str() );

                        servidor.sendStringToUser( result.dump(), servidor.getUserWithName( userName ) );
                    
                    }else if ( receivedJSON["CMD"] == SEND ){

                        string receiverName = receivedJSON["RECEIVER"];
                        string sender = receivedJSON["USERNAME"];
                        string msg = receivedJSON[MSG];
                        size_t messageId = receivedJSON[MSGID];
                        
                        json result;
                        if ( servidor.hasUserWithName( receiverName ) ) {
                            
                            list<string> receiver;
                            receiver.push_back( receiverName );
                            
                            Message newMessage = Message( messageId, sender, msg, "", receiver );

                            servidor.insertMessage( newMessage );
                            servidor.insertNewMessageOnUser( servidor.getUserWithName( receiverName ), newMessage );
                            
                            json msgReceived;
                            msgReceived[MSG] = to_string( messageId%MSGID_DIVISOR) + " enfileirada.";
                            
                            servidor.sendStringToUser( msgReceived.dump(), servidor.getUserWithName( userName ) );
                            
                            servidor.sendMessagesToUser( servidor.getUserWithName(receiverName) );
                            
                            
                        }else{
                            
                            result[MSG] = "Usuário de destino inexistente.";
                            servidor.sendStringToUser( result.dump(), servidor.getUserWithName( userName ) );

                        }
                    }else if( receivedJSON["CMD"] == SENDG ){
                        
                        string groupName = receivedJSON["RECEIVER"];
                        string sender = receivedJSON["USERNAME"];
                        string msg = receivedJSON[MSG];
                        size_t messageId = receivedJSON[MSGID];
                        
                        json result;
                        
                        if ( servidor.hasGroupWithName( groupName) ) {
                            
                            list<string> receivers = servidor.getGroupWithName( groupName ).getUsersName();
                            Message newMessage = Message( messageId, sender, msg, groupName, receivers );
                            
                            servidor.insertMessage( newMessage );
                            
                            json msgReceived;
                            msgReceived[MSG] = to_string( messageId%MSGID_DIVISOR) + " enfileirada.";
                            servidor.sendStringToUser( msgReceived.dump(), servidor.getUserWithName( userName ) );
                            
                            list<string>::iterator it;
                            
                            for ( it = receivers.begin(); it != receivers.end(); it++) {
                                
                                string tempReceiverName = (*it);
                                
                                printf("ENVIANDO PARA %s\n", tempReceiverName.c_str() );
                                
                                servidor.insertNewMessageOnUser( servidor.getUserWithName( tempReceiverName ), newMessage );
                                servidor.sendMessagesToUser( servidor.getUserWithName(tempReceiverName) );
                            }
                            
                        }else{
                            result[MSG] = "Grupo de destino inexistente.";
                            servidor.sendStringToUser( result.dump(), servidor.getUserWithName( userName ) );
                            
                        }
                        
                    
                    
                    }
                    
                    printf("\n------------------------------------\n\n");

                }

                if (--nReady <= 0)
                    break;				/* no more readable descriptors */
            }else{
                printf("..Não alterado\n");
            }
        }
    }
    return 0;
}

size_t msg_hash( string sender, string receiver, string msg ){
    
    ostringstream oss;
    oss << receiver << sender << msg;
    hash<string> str_hash;
    return str_hash( oss.str() );

}

