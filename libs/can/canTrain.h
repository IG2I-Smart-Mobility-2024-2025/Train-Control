#ifndef CANTRAIN_H
#define CANTRAIN_H

#include "../unirail/CAN/canLinux.h"
#include "../unirail/CAN/loco_Parametres.h"
#include <pthread.h>
#include <stdio.h>

#define DISTANCE_SECU 20 

typedef struct canOdo
{
	float distance;  // en cm
	float vit_consigne;
	int vit_mesuree;
	int nb_impulsions;
}canOdo;

typedef struct s_TrainInfo{
    pthread_mutex_t trainInfo_mutex;
    struct canOdo train;
} t_TrainInfo;

void initCan();
int writeVitesseLimite(float vitesseLimite);
int writeVitesseConsigne(unsigned int vitesse, unsigned char sense);
void *lectureCan();

#endif