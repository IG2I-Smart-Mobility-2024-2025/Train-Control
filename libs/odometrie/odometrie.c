#include "odometrie.h"

/**
 * @brief Create a odometrie object
 * 
 * @param ttyFile ttyFile to use for the Marvelmind
 * @param anyInputPacketCallback Function to use when a packet is received
 * @return odometrie_t* NULL if an error occured, the odometrie object otherwise
 */
odometrie_t* create_odometrie(char * ttyFile, void (*anyInputPacketCallback)()) {
    // Check if ttyFile is NULL
    if (ttyFile == NULL) ttyFile = DEFAULT_TTY_FILENAME;
    // Allocate memory for odometrie object
    odometrie_t* odo = malloc(sizeof(odometrie_t));
    // Create a marvelmind hedge object
    odo->hedge = createMarvelmindHedge();
    // Check if marvelmind hedge object is NULL
    if (odo->hedge == NULL) {
        free(odo);
        return NULL;
    }
    // Initialize the mutex
    pthread_mutex_init(&odo->mutex, NULL);

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
void update_odometrie_position(odometrie_t* odo) {
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
int delete_odometrie(odometrie_t* odo) {
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
void update_odometrie_distance(odometrie_t* odo) {
    odo->distance_last_update = sqrt(pow(odo->actual_position.x - odo->last_beacon_update.x, 2) + pow(odo->actual_position.y - odo->last_beacon_update.y, 2));
    odo->total_distance_last_beacon += odo->distance_last_update;
}

/**
 * @brief Reset the odometrie when passing a beacon
 * 
 * @param odo Odometrie object
 */
void reset_odometrie(odometrie_t* odo) {
    printf("Reset odometrie\n");
    odo->total_distance_last_beacon= 0;
    odo->last_beacon_update = odo->actual_position;
}

/**
 * @brief Thread function for the odometrie
 * 
 * @param arg Argument for the thread
 */
void * thread_odometrie(void * arg) {
    odometrie_t* odo = (odometrie_t*) arg;
    while (odo->is_running) {
        // Wait for the mutex
        pthread_mutex_lock(&odo->mutex);

        // Update the odometrie
        update_odometrie_position(odo);
        update_odometrie_distance(odo);
        
        // Release the mutex
        pthread_mutex_unlock(&odo->mutex);

        // Debug
        // debug_odometrie(odo);
        
        // Sleep for 100ms
        usleep(100000);
    }
    return 0;
}

/**
 * @brief Debug the odometrie object
 * 
 * @param odo Odometrie object
 */
void debug_odometrie(odometrie_t* odo) {
    printf("Actual position : %d %d %d\n", odo->actual_position.x, odo->actual_position.y, odo->actual_position.z);
    printf("Last beacon update : %d %d %d\n", odo->last_beacon_update.x, odo->last_beacon_update.y, odo->last_beacon_update.z);
    printf("Distance last update : %2.f\n", odo->distance_last_update);
    printf("Total distance last beacon : %2.f\n", odo->total_distance_last_beacon);
}