OBJS = sort.o utils.o tcpServer.o
#OBJS = $(wildcard *.o)
SRC = $(wildcard *.c)

CC = gcc
#STD = -std=c++11

tcpServer:${OBJS}
	${CC} ${OBJS} -o $@ 
all:${SRC}
	${CC} -c ${SRC} 
client:tcpClient.c
	${CC} tcpClient.c -o tcpClient
clean:
	rm -f *.o tcpServer tcpClient