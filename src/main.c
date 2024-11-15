/**
 * @file main.c
 * @author Teo Treny
 * @brief Programme principal permettant la gestion d'un train.
 * Dans un premier temps, le train récupère sa trajectoire enregistrée dans un fichier texte.
 * En suite, un thread est créé pour gérer les déplacements du train.
 * @version 0.1
 * @date 2024-11-15
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "libs/train-utils/train-utils.h"

int main(int argc, char* argv[]) {
    // Vérification du nombre d'arguments
    if (argc != 2) {
        printf("Usage: %s <fichier_trajectoire>\n", argv[0]);
        return 1;
    }

    // Récupération du nom du fichier de trajectoire
    char* fichier_trajectoire = argv[1];

    // Initialisation du train
    Train_t* train = train_init(1, (Position_t) {0, 0, 0});
    if (train == NULL) {
        printf("Erreur lors de l'initialisation du train\n");
        return 1;
    }

    // Chargement de la trajectoire du train
    if (load_train_course(train, fichier_trajectoire) != 0) {
        printf("Erreur lors du chargement de la trajectoire du train\n");
        return 1;
    }

    // Test de debug
    debug_train(train);

    return 0;
}