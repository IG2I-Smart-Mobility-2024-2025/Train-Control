# Variables Compiler and Flags
CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lm -lpthread

# Variables Directories
DIR_BIN = bin
DIR_SRC = src
DIR_LIBS = libs

# Variables Directories Libs
DIR_LIBS_TRAIN = $(DIR_LIBS)/train
DIR_LIBS_COMM = $(DIR_LIBS)/communication
DIR_LIBS_ODO = $(DIR_LIBS)/odometrie
DIR_LIBS_CAN = $(DIR_LIBS)/can
DIR_LIBS_MARVELMIND = $(DIR_LIBS)/marvelmind
DIR_LIBS_UNIRAIL = $(DIR_LIBS)/unirail
DIR_LIBS_UNIRAIL_CAN = $(DIR_LIBS)/unirail/CAN

# Variables Files
ARCHIVE= Train_control

# RPI
RPI_USER = pi
RPI_IP = 192.168.1.166

# General Rules
all : directories $(DIR_BIN)/train_control.e

clear :
	rm -rf bin/*

archive : clear
	tar -czf $(ARCHIVE).tar.gz *

televerse : archive
	scp $(ARCHIVE).tar.gz $(RPI_USER)@$(RPI_IP):/home/$(RPI_USER)/IG2I-2425/Teo

directories :
	mkdir -p $(DIR_BIN)

# train_control
$(DIR_BIN)/train_control.e : $(DIR_BIN)/train.o $(DIR_BIN)/tcp_interface.o $(DIR_BIN)/canTrain.o $(DIR_BIN)/odometrie.o $(DIR_BIN)/canLinux.o $(DIR_BIN)/marvelmind.o $(DIR_BIN)/MESCAN1_Utilitaire.o $(DIR_BIN)/MESCAN1_VarStatusTrain.o $(DIR_BIN)/MESCAN1_VarTrain.o $(DIR_SRC)/main.c
	@echo "--- Compiling train_control ---"
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

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

# marvelmind
$(DIR_BIN)/marvelmind.o : $(DIR_LIBS_MARVELMIND)/marvelmind.c $(DIR_LIBS_MARVELMIND)/marvelmind.h
	@echo "--- Compiling marvelmind ---"
	$(CC) $(CFLAGS) -c $< -o $@

# can
$(DIR_BIN)/canTrain.o : $(DIR_LIBS_CAN)/canTrain.c $(DIR_LIBS_CAN)/canTrain.h
	@echo "--- Compiling can ---"
	$(CC) $(CFLAGS) -c $< -o $@

# canLinux
$(DIR_BIN)/canLinux.o : $(DIR_LIBS_UNIRAIL_CAN)/canLinux.c $(DIR_LIBS_UNIRAIL_CAN)/canLinux.h
	@echo "--- Compiling canLinux ---"
	$(CC) $(CFLAGS) -c $< -o $@

# MESCAN1_Utilitaire
$(DIR_BIN)/MESCAN1_Utilitaire.o : $(DIR_LIBS_UNIRAIL_CAN)/MESCAN1_Utilitaire.c $(DIR_LIBS_UNIRAIL_CAN)/MESCAN1_Utilitaire.h
	@echo "--- Compiling MESCAN1_Utilitaire"
	$(CC) $(CFLAGS) -c $< -o $@

# MESCAN1_VarStatusTrain.o
$(DIR_BIN)/MESCAN1_VarStatusTrain.o : $(DIR_LIBS_UNIRAIL_CAN)/MESCAN1_VarStatusTrain.c $(DIR_LIBS_UNIRAIL_CAN)/MESCAN1_VarStatusTrain.h
	@echo "--- Compiling MESCAN1_VarStatusTrain"
	$(CC) $(CFLAGS) -c $< -o $@

# MESCAN1_VarTrain.o
$(DIR_BIN)/MESCAN1_VarTrain.o : $(DIR_LIBS_UNIRAIL_CAN)/MESCAN1_VarTrain.c $(DIR_LIBS_UNIRAIL_CAN)/MESCAN1_VarTrain.h
	@echo "--- Compiling MESCAN1_VarTrain"
	$(CC) $(CFLAGS) -c $< -o $@
