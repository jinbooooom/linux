OBJ_LIB = oddSum.o sum.o
OBJ_UTILS = isOdd.o show.o
OBJS = main.o ${UTILS}isOdd.o ${UTILS}show.o ${LIB}oddSum.o ${LIB}sum.o

CC = g++
STD = -std=c++11
INCLUDE = -I ./utils -I ./lib/
LIB = ./lib/
UTILS = ./utils/

main:${OBJS}
	${CC} ${OBJS} -o $@ ${STD} ${INCLUDE}
lib:${LIB}oddSum.o ${LIB}sum.o
	${CC} -c ${LIB}*.cpp ${STD} 
utils:${UTILS}isOdd.o ${UTILS}show.o
	${CC} ${STD} -c ${UTILS}*.cpp
${UTILS}: #去掉 .o 也可以编译
.PHONY : clean
clean:# -rm 中 - 的意思是也许会出问题，但是不要管，继续执行后面的命令
	find . -name "*.o" | xargs rm -rf
	-rm main 	