#include "odometrie.h"

/**
 * @brief Create a odometrie object
 * 
 * @param ttyFile ttyFile to use for the Marvelmind
 * @param anyInputPacketCallback Function to use when a packet is received
 * @return odometrie* NULL if an error occured, the odometrie object otherwise
 */
odometrie* create_odometrie(char * ttyFile, void (*anyInputPacketCallback)()) {
    // Check if ttyFile is NULL
    if (ttyFile == NULL) ttyFile = DEFAULT_TTY_FILENAME;
    // Allocate memory for odometrie object
    odometrie * odo = malloc(sizeof(odometrie));
    // Create a marvelmind hedge object
    odo->hedge = createMarvelmindHedge();
    // Check if marvelmind hedge object is NULL
    if (odo->hedge == NULL) {
        free(odo);
        return NULL;
    }
    // Configure marvelmind hedge object
    odo->hedge->ttyFileName = ttyFile;
    odo->hedge->verbose = true;
    odo->hedge->anyInputPacketCallback = anyInputPacketCallback;
    // Start the marvelmind
    startMarvelmindHedge(odo->hedge);

    // Initialize odometrie object
    odo->actual_position = (Position_t) {0, 0, 0};
    odo->last_beacon_update = (Position_t) {0, 0, 0};
    odo->distance_last_update = 0;
    odo->total_distance_last_beacon = 0;

    return odo;
}

/**
 * @brief Update the odometrie actual position with the marvelmind position.
 * 
 */
void update_odometrie_position(odometrie* odo) {
    struct PositionValue position;
    if (getPositionFromMarvelmindHedge(odo->hedge, &position)) {
        odo->actual_position.x = position.x;
        odo->actual_position.y = position.y;
        odo->actual_position.z = position.z;
    }   
}

/**
 * @brief Delete an odometrie object
 * 
 * @param odo Odometrie object
 * @return int 0 if everything went well
 */
int delete_odometrie(odometrie* odo) {
    // Stop the marvelmind
    stopMarvelmindHedge(odo->hedge);
    // Destroy the marvelmind
    destroyMarvelmindHedge(odo->hedge);
    // Free the odometrie object
    free(odo);
    return 0;
}

/**
 * @brief Update the odometrie distance
 * 
 * @param odo Odometrie object
 */
void update_odometrie_distance(odometrie* odo) {
    odo->distance_last_update = sqrt(pow(odo->actual_position.x - odo->last_beacon_update.x, 2) + pow(odo->actual_position.y - odo->last_beacon_update.y, 2));
    odo->total_distance_last_beacon += odo->distance_last_update;
}

/**
 * @brief Reset the odometrie when passing a beacon
 * 
 * @param odo Odometrie object
 */
void reset_odometrie(odometrie * odo) {
    odo->total_distance_last_beacon= 0;
    odo->last_beacon_update = odo->actual_position;
}

/**
 * @brief Thread function for the odometrie
 * 
 * @param arg Argument for the thread
 */
void * thread_odometrie(void * arg) {
    odometrie * odo = (odometrie *) arg;
    while (odo->is_running) {
        // Wait for the mutex
        sem_wait(&odo->mutex);

        // Update the odometrie
        update_odometrie_position(odo);
        update_odometrie_distance(odo);
        
        // Release the mutex
        sem_post(&odo->mutex);
        
        // Sleep for 100ms
        usleep(100000);
    }
}