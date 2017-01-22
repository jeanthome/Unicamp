#g++ -c main.cpp -std=c++11
#g++ -c User.cpp -std=c++11
#g++ -c Message.cpp -std=c++11
#g++ -c Group.cpp -std=c++11
#g++ -c Server.cpp -std=c++11

#g++ -o main main.o User.o Message.o Group.o Server.o -std=c++11



# Makefile for Separate Compilation Example

# *****************************************************
# Parameters to control Makefile operation

CC = g++
CFLAGS = -std=c++11
CCFLAGS = -std=c++11 -lncurses

# ****************************************************
# Entries to bring the executable up to date

default: main

main: main.o User.o Message.o Group.o Server.o
	$(CC) $(CFLAGS) -o main main.o User.o Message.o Group.o Server.o

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp

User.o: User.cpp User.hpp
	$(CC) $(CFLAGS) -c User.cpp

Message.o: Message.cpp Message.hpp
	$(CC) $(CFLAGS) -c Message.cpp

Group.o: Group.cpp Group.hpp
	$(CC) $(CFLAGS) -c Group.cpp

Server.o: Server.cpp Server.hpp
	$(CC) $(CFLAGS) -c Server.cpp

cli: Client.o
	$(CC) $(CCFLAGS) -o client Client.o

Client.o: Client.cpp
	$(CC) $(CCFLAGS) -c Client.cpp
	
clear: 
	$(RM) *.o *~
