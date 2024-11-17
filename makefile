# Variables Compiler and Flags
CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lm

# Variables Directories
DIR_BIN = bin
DIR_SRC = src
DIR_LIBS = libs

# Variables Directories Libs
DIR_LIBS_TRAIN = $(DIR_LIBS)/train

# Variables Files
ARCHIVE= Train_control

# General Rules
all : directories $(DIR_BIN)/train_control.e

clear :
	rm -rf bin/*

archive :

directories :
	mkdir -p $(DIR_BIN)

# train_control
$(DIR_BIN)/train_control.e : $(DIR_BIN)/train.o $(DIR_SRC)/main.c
	@echo "--- Compiling train_control ---"
	@echo "gcc $(CFLAGS) $^ -o $@"
	@$(CC) $(CFLAGS) $^ -o $@

# train
$(DIR_BIN)/train.o : $(DIR_LIBS_TRAIN)/train.c $(DIR_LIBS_TRAIN)/train.h
	@echo "--- Compiling train ---"
	@echo "gcc -c $(CFLAGS) $< -o $@"
	@$(CC) $(CFLAGS) -c $< -o $@
