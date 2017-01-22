//
//  Group.cpp
//  Projeto
//
//  Created by Jean Thomé on 6/15/16.
//  Copyright © 2016 Jean Thomé. All rights reserved.
//

#include "Group.hpp"
#include "User.hpp"
#include <string>
#include <algorithm>

Group::Group( string name ) {
    
    _id = this->nextGroupId++;
    _name = name;
    usersName = list<string>();
    
}

string Group::getName(){
    return _name;
}

list<string> Group::getUsersName(){
    return this->usersName;
}

bool Group::hasUserWithName( string name ){
    int countList = count( usersName.begin(), usersName.end(), name );
    
    if ( countList > 0) {
        return true;
    }else{
        return false;
    }
}

size_t Group::nextGroupId = 0;

void Group::insertUser( User userToBeInserted ) {
    
    printf("Usuário %s inserido no grupo %s com sucesso.\n", userToBeInserted.getName().c_str(), this->getName().c_str() );
    usersName.push_back( userToBeInserted.getName() );
}

//TODO imprimir a lista dos usuários com seus respectivos Status.
//Arrumar um jeito de deixar a coluna com largura fixa.
void Group::printUsersOfGroup(){
    list<string> names;
    list<string>::iterator it;
    
    names = this->getUsersName();
    
    for ( it = names.begin(); it !=  names.end(); it++) {
        printf("%s\n", (*it).c_str( ) );
    }
    
}
