#ifndef ODOMETRIE_H
#define ODOMETRIE_H

#include <semaphore.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h>
#include "../train/train.h"
#include "../marvelmind/marvelmind.h"

typedef struct {
    Position_t actual_position;
    Position_t last_beacon_update;
    float distance_last_update;
    float total_distance_last_beacon;
    struct MarvelmindHedge * hedge;
    pthread_mutex_t mutex;
    bool is_running;
} odometrie;

/**
 * @brief Create a odometrie object
 * 
 * @param ttyFile ttyFile to use for the Marvelmind
 * @param anyInputPacketCallback Function to use when a packet is received
 * @return odometrie* NULL if an error occured, the odometrie object otherwise
 */
odometrie* create_odometrie(char * ttyFile, void (*anyInputPacketCallback)());

/**
 * @brief Update the odometrie actual position with the marvelmind position.
 * 
 */
void update_odometrie_position(odometrie* odo);

/**
 * @brief Delete an odometrie object
 * 
 * @param odo Odometrie object
 * @return int 0 if everything went well
 */
int delete_odometrie(odometrie* odo);

/**
 * @brief Update the odometrie distance
 * 
 * @param odo Odometrie object
 */
void update_odometrie_distance(odometrie* odo);

/**
 * @brief Reset the odometrie when passing a beacon
 * 
 * @param odo Odometrie object
 */
void reset_odometrie(odometrie * odo);

/**
 * @brief Thread function for the odometrie
 * 
 * @param arg Argument for the thread
 */
void * thread_odometrie(void * arg);

/**
 * @brief Debug the odometrie object
 * 
 * @param odo Odometrie object
 */
void debug_odometrie(odometrie* odo);
#endif