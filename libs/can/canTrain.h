/**
 * @file canTrain.h
 * @author Urban Prevost
 * @brief Collection de fonctions pour la gestion du train sur le bus CAN
 * @version 0.1
 * @date 2024-12-02
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef CANTRAIN_H
#define CANTRAIN_H

#include "../unirail/CAN/canLinux.h"
#include "../unirail/CAN/loco_Parametres.h"
#include <pthread.h>
#include <stdio.h>

#define DISTANCE_SECU 20 

// Callback function type
typedef void (*callback_function)(void* arg);

// Structs

typedef struct {
	pthread_mutex_t can_odometrie_mutex;
	float distance;  // en cm
	float vit_consigne;
	int vit_mesuree;
	int nb_impulsions;
} can_odometrie_t;

typedef struct {
    can_odometrie_t can_odometrie;
	callback_function callback;
	void* callback_arg;
} can_train_t;

// Macros
#define CAN_ODO(a) (a)->can_odometrie

/**
 * @brief Create a can train object
 * 
 * @param port Port can
 * @param callback Function to call when a beacon is passed
 * @return can_train_t* The can train object
 */
can_train_t* create_can_train(char * port, callback_function callback);

/**
 * @brief Initialise le port CAN
 * 
 * @param port Port can (can0 par defaut)
 */
void initCan(char * port);

/**
 * @brief Ecrit la vitesse limite du train sur le bus can
 * 
 * @param vitesseLimite 
 * @return int 1 si l'envoi a reussi, 0 sinon
 */
int writeVitesseLimite(float vitesseLimite);

/**
 * @brief Ecrit la vitesse consigne du train sur le bus can
 * 
 * @param vitesse Vitesse consigne
 * @param sense Sens de deplacement
 * @return int 1 si l'envoi a reussi, 0 sinon
 */
int writeVitesseConsigne(unsigned int vitesse, unsigned char sense);

/**
 * @brief Met à jour l'odométrie du train
 * 
 * @param can_odo Odométrie du train
 * @param recCanMsg Message can pour tirer les informations
 */
void updateCanOdometrie(can_odometrie_t* can_odo, uCAN1_MSG recCanMsg);

/**
 * @brief Thread de lecture de CAN
 * 
 * @param args Structure de type can_train_t
 */
void* lectureCan(void* args);

/**
 * @brief Delete a can train object
 * 
 * @param can_train 
 * @return int 0 if the can train was deleted successfully, -1 otherwise
 */
int delete_can_train(can_train_t * can_train);

#endif