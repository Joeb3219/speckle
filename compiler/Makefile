CC := gcc
EXE_NAME := speckle

all: clean build

build: speckle.c
	$(CC) -g -o $(EXE_NAME) speckle.c lexer.c parser.c codegen.c hashmap.c error.c 

clean:
	rm -rf *.o $(EXE_NAME)
