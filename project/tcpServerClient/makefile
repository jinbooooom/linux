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
clean:
	rm -f *.o server client