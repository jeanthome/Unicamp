//
//  User.cpp
//  Projeto
//
//  Created by Jean Thomé on 6/15/16.
//  Copyright © 2016 Jean Thomé. All rights reserved.
//

#include "User.hpp"
#include <iostream>
#include <cstdlib>

using namespace std;
User::User( int connectionFd, string userName ){
    
    //_id = id;
    _name = userName;
    _connectionFd = connectionFd;
    pendingMessagesHashKeys = queue<size_t>();
    responsesMessages = queue<string>();
}

string User::getName(){
    return this->_name;
}

bool User::isOnline() {
    return ( (_connectionFd < 0)? false: true );
}

void User::printName() {
    cout << _name;
}

void User::printStatus() {
    ( isOnline()? printf("online") : printf("offline") );
}

bool User::hasPendingMessages() {
    printf("Q[%d]", (int)pendingMessagesHashKeys.size() );
    return !pendingMessagesHashKeys.empty();
}

bool User::hasPendingResponses(){
    printf("QR[%d]", (int)responsesMessages.size() );
    return !responsesMessages.empty();
}

void User::insertNewMessageIdOnQueue( size_t messageHashKey ){
    pendingMessagesHashKeys.push( messageHashKey );
}

void User::insertNewResponseOnQueue( string response ){
    this->responsesMessages.push( response );
}

void User::setConnectionFd( int connectionFd ){
    this->_connectionFd = connectionFd;
}

int User::getConnectionFd(){
    return this->_connectionFd;
}

size_t User::getFrontMessageId(){
    return this->pendingMessagesHashKeys.front();
}

void User::popFrontMessage(){
    this->pendingMessagesHashKeys.pop();
}

string User::getFrontResponse(){
    return this->responsesMessages.front();
}

void User::popFrontResponse(){
    this->responsesMessages.pop();
}
