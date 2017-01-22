//
//  User.hpp
//  Projeto
//
//  Created by Jean Thomé on 6/15/16.
//  Copyright © 2016 Jean Thomé. All rights reserved.
//

#ifndef User_hpp
#define User_hpp

#include <stdio.h>
#include <queue>
#include <string>

using namespace std;

class User{
private:

    string _name;
    int _connectionFd;
    queue< size_t > pendingMessagesHashKeys;
    queue< string > responsesMessages;
    
public:
    
    User( int connectionFd, string userName );
    //User( const User &obj);

    string getName();
    
    bool isOnline();
    
    void printName();
    
    void printStatus();
    
    bool hasPendingMessages();
    
    bool hasPendingResponses();
    
    void insertNewMessageIdOnQueue( size_t messageHashKey );
    
    void insertNewResponseOnQueue( string response );
    
    void setConnectionFd( int connectionFd );
    
    int getConnectionFd( );
    
    size_t getFrontMessageId();
    
    void popFrontMessage();
    
    string getFrontResponse();
    
    void popFrontResponse();
    
};

#endif /* User_hpp */
