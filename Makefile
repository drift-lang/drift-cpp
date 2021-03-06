DIR_SRC = ./src
DIR_TMP = ./tmp
CC = g++

SRC = $(wildcard ${DIR_SRC}/*.cpp)
TMP = $(wildcard $(DIR_TMP)/*.o)

all:
	${CC} -c $(foreach i, $(SRC), $(i))

install:
	$(shell mkdir ${DIR_TMP})
	$(shell mv *.o ${DIR_TMP})

	${CC} $(foreach i, $(TMP), $(i)) -o drift

	@echo "\n\t\033[41;37m<TARGET FILE GENERATED>: ./drift\033[0m 🐇🐰🍻 \n"

run: 
	./test/run.sh

clean:
	rm -f *.o
	rm -rf ${DIR_TMP}
	rm -f drift