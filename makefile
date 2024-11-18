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
DIR_LIBS_COMM = $(DIR_LIBS)/communication
DIR_LIBS_ODO = $(DIR_LIBS)/odometrie
DIR_LIBS_CAN = $(DIR_LIBS)/can
DIR_LIBS_UNIRAIL = $(DIR_LIBS)/unirail
DIR_LIBS_UNIRAIL_CAN = $(DIR_LIBS)/unirail/CAN

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
$(DIR_BIN)/train_control.e : $(DIR_BIN)/train.o $(DIR_BIN)/tcp_interface.o $(DIR_BIN)/canTrain.o $(DIR_BIN)/odometrie.o $(DIR_LIBS_UNIRAIL_CAN)/canLinux.o $(DIR_LIBS_UNIRAIL_CAN)/MESCAN1_Utilitaire.o $(DIR_LIBS_UNIRAIL_CAN)/MESCAN1_VarStatusTrain.o $(DIR_LIBS_UNIRAIL_CAN)/MESCAN1_VarTrain.o $(DIR_SRC)/main.c
	@echo "--- Compiling train_control ---"
	$(CC) $(CFLAGS) $^ -o $@ -lpthread

# train
$(DIR_BIN)/train.o : $(DIR_LIBS_TRAIN)/train.c $(DIR_LIBS_TRAIN)/train.h
	@echo "--- Compiling train ---"
	$(CC) $(CFLAGS) -c $< -o $@

# tcp_interface
$(DIR_BIN)/tcp_interface.o : $(DIR_LIBS_COMM)/tcp_interface.c $(DIR_LIBS_COMM)/communication.h
	@echo "--- Compiling tcp_interface ---"
	$(CC) $(CFLAGS) -c $< -o $@

# odometrie
$(DIR_BIN)/odometrie.o : $(DIR_LIBS_ODO)/odometrie.c $(DIR_LIBS_ODO)/odometrie.h
	@echo "--- Compiling odometrie ---"
	$(CC) $(CFLAGS) -c $< -o $@

# can
$(DIR_BIN)/canTrain.o : $(DIR_LIBS_CAN)/canTrain.c $(DIR_LIBS_CAN)/canTrain.h
	@echo "--- Compiling can ---"
	$(CC) $(CFLAGS) -c $< -o $@

