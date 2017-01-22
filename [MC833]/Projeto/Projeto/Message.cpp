//
//  Message.cpp
//  Projeto
//
//  Created by Jean Thomé on 6/15/16.
//  Copyright © 2016 Jean Thomé. All rights reserved.
//

#include "Message.hpp"
#include <iostream>

using namespace std;

Message::Message( size_t id,  string senderName, string content , string groupName, list<string> receiversName ){
    
    _id = id;
    _senderName = senderName;
    _content = content;
    _receiversName = receiversName;
    _groupName = groupName;
}
void insertReceiver( string receiverToBeInserted );

size_t Message::getId(){
    return _id;
}

string Message::getContent(){
    return _content;
}

void Message::removeReceiver( string receiverToBeRemoved ){
    
    printf("Removendo %s ", receiverToBeRemoved.c_str() );
    
    this->_receiversName.remove( receiverToBeRemoved );
    
    printf(" sobraram %d\n", (int)this->_receiversName.size() );
}

void Message::insertReceiver( string receiverToBeInserted ){
    
    this->_receiversName.push_back( receiverToBeInserted );
}

bool Message::hasPendingReceiver(){
    
    if ( this->_receiversName.size() > 0) {
        return true;
    }
    return false;
}

void Message::printContent(){
    printf("C[%s]\n", this->_content.c_str() );
    
    for (list<string>::iterator it = _receiversName.begin(); it != _receiversName.end(); it++) {
        printf("\t[%s]\n", (*it).c_str() );
    }
}

string Message::getSenderName(){
    return this->_senderName;
}

bool Message::messageFromGroup(){
    if ( this->_groupName.length() > 0) {
        printf("EH MENSAGEM DE GRUPO\n");
        return true;
    }
    printf("NAO EH MENSAGEM DE GRUPO\n");
    return false;
}

string Message::getGroupName(){
    return this->_groupName;
}
