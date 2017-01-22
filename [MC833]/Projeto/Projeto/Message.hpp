//
//  Message.hpp
//  Projeto
//
//  Created by Jean Thomé on 6/15/16.
//  Copyright © 2016 Jean Thomé. All rights reserved.
//

#ifndef Message_hpp
#define Message_hpp

#include <stdio.h>
#include <string>
#include <list>
#include <sstream>

using namespace std;

class Message{
private:
    size_t _id ;
    string _content, _senderName;
    list<string> _receiversName;
    string _groupName;
    
public:
    Message( size_t id, string senderName, string content, string groupName, list<string> receiversName );
    
    size_t getId();
    string getContent();
    
    void removeReceiver( string receiverToBeRemoved );
    void insertReceiver( string receiverToBeInserted );
    void printContent();
    bool hasPendingReceiver();
    string getSenderName();
    bool messageFromGroup();
    string getGroupName();
    
};


#endif /* Message_hpp */
