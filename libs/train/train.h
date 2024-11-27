/**
 * @file train.h
 * @author Téo Trény
 * @brief 
 * @version 0.1
 * @date 2024-11-18
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef TRAIN_H
#define TRAIN_H

// Includes
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "../can/canTrain.h"
#include "../common.h"

// Structs



// MACROS
#define COURSE_SIZE(a) (a)->course.size
#define COURSE_FIRST_STEP(a) (a)->course.steps[0]
#define COURSE_LAST_STEP(a) (a)->course.steps[COURSE_SIZE((a))-1]

// Functions

/**
 * @brief Initialize a train
 * 
 * @param id Train id
 * @param position Train initial position
 * @return Train_t* NULL if an error occured, a pointer to the train otherwise
 */
Train_t * train_init(char id, Position_t position);

/**
 * @brief Load a train course from a file
 * 
 * @param train     Train to load the course into
 * @param filename  Filename of the course
 * @param repeat    Number of times the course is repeated
 * @return int      0 if the course was loaded successfully
 *                  -1 if an error occured
 *                  -2 if the repeat is invalid (the first and last steps are different)
 */
int load_train_course(Train_t * train, char* filename, int repeat);

/**
 * @brief Delete a train
 * 
 * @param train Train to delete
 * @return int 0 if the train was deleted successfully, -1 otherwise
 */
int delete_train(Train_t * train);

/**
 * @brief To debug a train
 * 
 * @param train Train to debug
 */
void debug_train(Train_t * train);

/**
 * @brief From "0x..." to "C..."
 * 
 * @param code 
 * @return const char* 
 */
const char* code_to_string(unsigned char code);

/**
 * @brief From "C..." to "0x..."
 * 
 * @param str 
 * @return unsigned char 
 */
unsigned char string_to_code(const char* str);

/**
 * @brief Print the course of a train
 * 
 * @param train Train to print the course of
 */
void print_train_course (Train_t * train);

#endif // "TRAIN_UTILS_H"