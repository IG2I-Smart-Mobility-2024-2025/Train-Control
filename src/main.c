/**
 * @file main.c
 * @author Teo Treny
 * @brief Programme principal permettant la gestion d'un train.
 * Dans un premier temps, le train récupère sa trajectoire enregistrée dans un fichier texte.
 * En suite, un thread est créé pour gérer les déplacements du train.
 * - Faire des demandes d'autorisation au RBC
 * - Transmettre son itinéraire au RBC
 * - Odométrie + transmettre au RBC
 * - Recaliber l'odométrie au passage sur une balise
 * - Gérer sa vitesse (autorisation)
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
#include "../libs/train/train.h"
#include "../libs/communication/communication.h"
#include "../libs/odometrie/odometrie.h"
#include "../libs/can/canTrain.h"

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
    int res = load_train_course(train, fichier_trajectoire, 1);
    // Gestion des erreurs
    switch(res) {
        case -1:
            printf("Erreur lors du chargement de la trajectoire\n");
            return 1;
        case -2:
            printf("La trajectoire doit etre une boucle (la derniere etape doit etre la meme que la premiere)\n");
            return 1;
    }

    // Test de debug
    debug_train(train);
    print_train_course(train);

    // On envoie une requete au RBC
    printf("J'envoie une requete au RBC pour me deplacer\n");

    // On configure la socket
    int socket = init_client_socket("192.168.1.200");
    if (!socket) {
        printf("Erreur lors de la creation de la socket\n");
        return 1;
    }

    // Structure train info pour connard
    train_info * ti = create_train_info(
        train->id,
        train->course.steps_code[train->course.current_step],
        0,
        0,
        train->course.steps_code,
        train->course.size
    );

    print_train_info(ti);

    // On envoie au RBC
    printf("Fail or pass ? %d\n", send_train_info(socket, ti));

    train_mov_auth* tma = create_train_mov_auth();

    printf("Fail or pass ? %d\n", recv_train_mov_auth(socket, tma));
    printf("Length authorized : %2.f\n", tma->length);

    // On lance le thread d'odométrie
    pthread_t thread_odo;
    odometrie* odo = create_odometrie(NULL, NULL);
    pthread_create(&thread_odo, NULL, thread_odometrie, (void*) odo);

    // On lance le thread du CAN
    pthread_t thread_can;
    thread_args args = {train->train_can, odo};
    pthread_create(&thread_can, NULL, lectureCan, (void*) &args);

    while(1);

    // On attend la fin des threads
    pthread_join(thread_odo, NULL);
    pthread_join(thread_can, NULL);

    // Suppression du train
    // delete_train(train);
    delete_train_info(ti);
    delete_train_mov_auth(tma);
    close(socket);

    return 0;
}