# Variables Compiler and Flags
CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lm

# Variables Directories
DIR_BIN = bin
DIR_SRC = src
DIR_LIBS = libs

# Variables Files
ARCHIVE= Train_control

all :

clear :
	rm -rf bin/*

archive :