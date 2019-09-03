INCLUDE=-Iinclude
MSGPK=-I/Users/alanyoung/Downloads/msgpack-c-master/include

%.o:src/%.cpp
	g++ -c $< ${INCLUDE} $(MSGPK) --std=c++17 

%.o:tests/%.cpp
	g++ -c $< ${INCLUDE} $(MSGPK) --std=c++17 

all: main client

main: feverrpc.o main.o threadmanager.o utils.o include/database/db-functions.hpp include/feverrpc/threadmanager.hpp
	g++ -o bin/server feverrpc.o main.o threadmanager.o utils.o -pthread -lpqxx

client: feverrpc.o clients.o utils.o 
	g++ -o bin/client $^ -pthread  -lpqxx

.PHONY:clean
clean:
	-rm *.o
	-rm -r bin
	-mkdir bin