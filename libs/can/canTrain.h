#ifndef CANTRAIN_H
#define CANTRAIN_H

#include "../unirail/CAN/canLinux.h"
#include "../unirail/CAN/loco_Parametres.h"

#include "../odometrie/odometrie.h"
#include "../common.h"

#include <pthread.h>
#include <stdio.h>

#define DISTANCE_SECU 20 

/**
 * @brief Crée et initialise une structure t_TrainInfo pour le train.
 *
 * Cette fonction alloue dynamiquement une structure t_TrainInfo, initialise
 * ses membres à des valeurs par défaut, configure le mutex associé et
 * appelle la fonction d'initialisation CAN.
 *
 * @return Un pointeur vers la structure t_TrainInfo initialisée. NULL si l'allocation échoue.
 */
t_TrainInfo* create_train_can();

/**
 * @brief Initialise la configuration pour l'utilisation du bus CAN.
 *
 * Cette fonction configure l'interface CAN de la Raspberry Pi, initialise des filtres CAN 
 * pour restreindre les messages à lire, et met en place les structures nécessaires 
 * pour la communication sur le bus CAN.
 *
 * @note Vérifiez que le nom de l'interface CAN est correct au niveau du système 
 * avant d'utiliser cette fonction.
 */
void initCan();

/**
 * @brief Envoie une consigne de vitesse limite sur le bus CAN.
 *
 * Cette fonction prépare et envoie un message CAN contenant une consigne 
 * de vitesse limite. Si la vitesse dépasse la limite maximale autorisée, 
 * elle est automatiquement ramenée à cette limite.
 *
 * @param vitesseLimite La vitesse limite souhaitée, en cm/s.
 *                      Elle sera plafonnée à MAX_CONSIGNE_VITESSE_AUTORISEE si nécessaire.
 *
 * @return Un entier indiquant le résultat de la transmission.
 *         0 en cas de succès, une valeur négative en cas d'erreur.
 */
int writeVitesseLimite(float vitesseLimite);

/**
 * @brief Envoie une consigne de vitesse et de sens de déplacement sur le bus CAN.
 *
 * Cette fonction prépare et envoie un message CAN contenant une consigne de vitesse 
 * et le sens de déplacement associé. Si la vitesse dépasse la limite maximale autorisée,
 * elle est automatiquement ramenée à cette limite.
 *
 * @param vitesse La vitesse souhaitée, en cm/s. Elle sera plafonnée à MAX_CONSIGNE_VITESSE_AUTORISEE si nécessaire.
 * @param sense Le sens de déplacement (0 pour un sens, 1 pour l'autre).
 *
 * @return Un entier indiquant le résultat de la transmission.
 *         0 en cas de succès, une valeur négative en cas d'erreur.
 */
int writeVitesseConsigne(unsigned int vitesse, unsigned char sense);

/**
 * @brief Thread de lecture des messages CAN.
 *
 * Cette fonction lit en continu les messages reçus sur le bus CAN et effectue des
 * actions en fonction des identifiants des messages. Elle met à jour les informations
 * de train et l'odométrie en conséquence.
 *
 * @param args Pointeur vers une structure contenant les arguments pour le thread.
 *             - `train_info` : Informations du train à mettre à jour.
 *             - `odo` : Structure d'odométrie à réinitialiser si nécessaire.
 *
 * @return NULL. Cette fonction est exécutée dans un thread et ne retourne rien directement.
 */
void *lectureCan(void * args);

#endif