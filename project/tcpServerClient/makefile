OBJS = sort.o utils.o tcpServer.o
SRC = $(wildcard *.c)

CC = gcc
#STD = -std=c++11

server:${OBJS}
	${CC} ${OBJS} -o $@ 
all:${SRC}
	${CC} -c ${SRC} 
client:tcpClient.c
	${CC} tcpClient.c utils.h utils.o -o client
iterServer:iterServer.c
	${CC} iterServer.c utils.o sort.o -o iterServer
clean:
	rm -f *.o server client iterServer