//
//  Server.hpp
//  Projeto
//
//  Created by Jean Thomé on 6/16/16.
//  Copyright © 2016 Jean Thomé. All rights reserved.
//

#ifndef Server_hpp
#define Server_hpp

#include <set>
#include <stdio.h>
#include <map>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include "User.hpp"
#include "Group.hpp"
#include "Message.hpp"
#include "Common.hpp"


using namespace std;

class Server{

private:
    
    struct sockaddr_in serverAddress;
    int listenSocket;
    set<int> connectionsFd;
    map< string, User > users;
    map< size_t, Message > messages;
    map< string, Group > groups;
    
    
public:
    
    fd_set readSet, allSet;
    char buffer[MAXLINE];
    
    Server( int serverPort );
    
    int serverSocket();
    int serverBind();
    int serverListen();
    
    void closeSocket();
    
    void insertUser( User userToBeInserted );
    void insertGroup( Group groupToBeInserted );
    void insertMessage( Message messageToBeInserted );
    void insertConnectionFd( int fdToBeInserted );
    bool insertUserOnGroup( User userToBeInserted, string groupName );
    
    void insertNewMessageOnUser( User receiverUser, Message messageToBeInserted );
    
    void insertNewResponseOnUser( User receiverUser, string response );

    void setUserOnLine( User userToBeOnline, int connectionFd );
    void setUserOffLine( User userToBeOffline );
    
    void deleteMessage( Message messageToBeDeleted );
    void removeConnectionFd( int fdToBeRemoved );
    
    bool hasUserWithName( string name );
    int hasPendingClient();
    bool hasGroupWithName( string name );

    bool sendMessagesToUser( User user );
    //int sendMessageToGroup( Message messageToBeSent, Group group );
    bool sendStringToUser( string stringToBeSent, User user );
    
    User getUserWithName( string name );
    map<string, bool> getUsersStatus();
    
    Group getGroupWithName( string name );
    Message getMessageWithId( size_t messageId );
    
    int getListenSocket();
    set<int> getConectionsFd();
    
};


#endif /* Server_hpp */
