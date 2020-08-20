OBJ_LIB = oddSum.o sum.o
OBJ_UTILS = isOdd.o show.o
OBJS = main.o ${UTILS}isOdd.o ${UTILS}show.o ${LIB}oddSum.o ${LIB}sum.o
#OBJS = $(patsubst %.cpp, %.o, $(wildcard */*.cpp))

CC = g++
STD = -std=c++11
INCLUDE = -I ./utils -I ./lib/
LIB = ./lib/
UTILS = ./utils/

main:${OBJS}
	${CC} ${OBJS} -o $@ ${STD} ${INCLUDE}
lib:#${LIB}oddSum.o ${LIB}sum.o
	${CC} -c ${LIB}*.cpp ${STD} 
utils:#${UTILS}isOdd.o ${UTILS}show.o
	${CC} ${STD} -c ${UTILS}*.cpp
clean:
	find . -name "*.o" | xargs rm -rf
	rm main