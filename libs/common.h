#ifndef COMMON_H
#define COMMON_H

#include "./marvelmind/marvelmind.h"

typedef struct {
    int x;              // X position
    int y;              // Y position
    int z;              // Z position
} Position_t;

typedef struct {
	float distance;  // en cm
	float vit_consigne;
	int vit_mesuree;
	int nb_impulsions;
}canOdo;

typedef struct {
    pthread_mutex_t trainInfo_mutex;
    canOdo train;
} t_TrainInfo;

typedef struct {
    Position_t actual_position;
    Position_t last_beacon_update;
    float distance_last_update;
    float total_distance_last_beacon;
    struct MarvelmindHedge * hedge;
    pthread_mutex_t mutex;
    bool is_running;
} odometrie;

typedef struct {
	t_TrainInfo * train_info;
	odometrie * odo;
} thread_args;

typedef struct {
    int size;           // Number of steps
    char* filename;     // Filename of the course
    char** steps;       // Steps of the course in "C..." format
    int * steps_code;   // Steps of the course in "0x..." format
    int current_step;   // Current step of the course
    int repeat;         // Number of times the course is repeated
} Course_t;

typedef struct {
    char id;                    // Train ID
    int speed;                  // Train speed
    Position_t position;        // Train position
    float distance;             // Train distance [cm]
    Course_t course;            // Train course
    t_TrainInfo * train_can;    // Train information
} Train_t;

#endif