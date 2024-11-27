#ifndef CANTRAIN_H
#define CANTRAIN_H

#include "../unirail/CAN/canLinux.h"
#include "../unirail/CAN/loco_Parametres.h"

#include "../odometrie/odometrie.h"
#include "../common.h"

#include <pthread.h>
#include <stdio.h>

#define DISTANCE_SECU 20 

t_TrainInfo* create_train_can();
void initCan();
int writeVitesseLimite(float vitesseLimite);
int writeVitesseConsigne(unsigned int vitesse, unsigned char sense);
void *lectureCan(void * args);

#endif