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
#include <math.h>
#include "../can/canTrain.h"
#include "../odometrie/odometrie.h"

// Structs

typedef struct {
    int size;           // Number of steps
    char* filename;     // Filename of the course
    char** steps;       // Steps of the course in "C..." format
    int * steps_code;   // Steps of the course in "0x..." format
    int current_step;   // Current step of the course
    int repeat;         // Number of times the course is repeated
} Course_t;

typedef struct {
    char id;                        // Train ID
    char flag_init;                 // Flag for init sequence
    char is_running;                 // Flag for when the train is running
    Course_t course;                // Train course
    can_train_t* can_train;         // Train can interface (contains another odometrie)
    odometrie_t* odometrie;         // Train odometrie
} Train_t;

typedef struct {
    Train_t* train;
    float max_speed;
    float alpha;
    float distance;
    float security_distance;
    int is_running;
    pthread_t thread_id;
    pthread_mutex_t mutex;
} deceleration_params_t;

// MACROS
#define COURSE_SIZE(a) (a)->course.size
#define COURSE_FIRST_STEP(a) (a)->course.steps[0]
#define COURSE_LAST_STEP(a) (a)->course.steps[COURSE_SIZE((a))-1]

// Functions

/**
 * @brief Initialize a train
 * 
 * @param id Train id
 * @return Train_t* NULL if an error occured, a pointer to the train otherwise
 */
Train_t * train_init(char id);

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

/**
 * @brief Fonction de callback appelée lorsqu'une balise est passée
 * 
 * @note On passe le train par argument. Pour un système temps réel critique, ce n'est pas optimal.
 * Il faudrait utiliser une variable globale dans notre cas.
 * 
 * @param arg Argument de la fonction (train_t*)
 */
void on_beacon_passed(void * arg);

/**
 * @brief Fonction de callback appelée lorsqu'une balise est passée
 * 
 * @note On passe le train par argument. Pour un système temps réel critique, ce n'est pas optimal.
 * Il faudrait utiliser une variable globale dans notre cas.
 * 
 * @param arg Argument de la fonction (train_t*)
 */
void on_beacon_passed(void * arg);

/**
 * @brief Fonction exécutée par le thread de décélération
 * 
 * @param arg Pointeur vers la structure deceleration_params_t
 * @return void* NULL
 */
void* deceleration_thread(void* arg);

/**
 * @brief Lance le processus de décélération
 * 
 * @param train Train à décélérer
 * @param max_speed Vitesse maximale autorisée
 * @param alpha Coefficient de décélération (m/s^2)
 * @param distance Distance à parcourir avant l'arrêt
 * @param security_distance Distance de sécurité
 * @return deceleration_params_t* Paramètres de décélération
 */
deceleration_params_t* start_deceleration(Train_t* train, float max_speed, float alpha, float distance, float security_distance);

/**
 * @brief Coupe le processus de décélération
 * 
 * @param params Paramètres de décélération
 * @return int 0 si tout s'est bien passé, -1 sinon
 */
int stop_deceleration(deceleration_params_t* params);

#endif // "TRAIN_H"