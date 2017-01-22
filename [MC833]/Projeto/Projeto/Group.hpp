//
//  Group.hpp
//  Projeto
//
//  Created by Jean Thomé on 6/15/16.
//  Copyright © 2016 Jean Thomé. All rights reserved.
//

#ifndef Group_hpp
#define Group_hpp

#include "User.hpp"
#include <stdio.h>
#include <string>
#include <list>
#include <iostream>

using namespace std;

class Group{
private:
    size_t _id;
    string _name;
    list<string> usersName;
    
public:
    
    static size_t nextGroupId;
    
    Group( string name );
    
    void insertUser( User userToBeInserted );
    bool hasUserWithName( string name );
    
    void printUsersOfGroup();
    
    string getName();
    
    list<string> getUsersName();
};


#endif /* Group_hpp */
