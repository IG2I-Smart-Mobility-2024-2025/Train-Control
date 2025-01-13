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

int main(int argc, char* argv[]) {
    // Vérification du nombre d'arguments
    if (argc != 3) {
        printf("Usage: %s <id_train> <fichier_trajectoire>\n", argv[0]);
        return 1;
    }

    // Récupération de l'id du train
    int id_train = atoi(argv[1]);

    // Récupération du nom du fichier de trajectoire
    char* fichier_trajectoire = argv[2];

    // Initialisation du train
    Train_t * train = train_init(id_train);
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
    // debug_train(train);
    print_train_course(train);

    // On configure la socket
    int socket = init_client_socket("192.168.1.200");
    if (!socket) {
        printf("Erreur lors de la creation de la socket\n");
        return 1;
    }

    // Structure train info pour interface TCP
    train_info * ti = create_train_info(
        train->id,
        train->course.steps_code[train->course.current_step],
        0,
        0,
        train->course.steps_code,
        train->course.size
    );

    // On crée la structure de mouvement autorisé
    train_mov_auth* tma = NULL;

    // On lance le thread d'odométrie
    pthread_t thread_odo;
    pthread_create(&thread_odo, NULL, thread_odometrie, (void*) train->odometrie);
    // On lance l'odometrie à l'aide du flag
    train->odometrie->is_running = 1;

    // On lance le thread du CAN
    pthread_t thread_can;
    pthread_create(&thread_can, NULL, lectureCan, (void*) train->can_train);

    // Routine bete pour aller jusqu'à la première balise avant de lancer le déplacement
    while(!(train->flag_init))
        writeVitesseConsigne(10, 1);

    // Commande de déplacement
    tma = create_train_mov_auth();
    int flag = 1;

    while(flag) {
        // On demande au RBC la distance autorisee et la vitesse max
        // En plus on lui envoie les informations du train (train_info)
        send_train_info(socket, ti);
        recv_train_mov_auth(socket, tma); // TODO : si le serveur met trop de temps à répondre et que le train roule, potentiel accident. Solution : appel non bloquant ?

        // Affichage des informations (DEBUG)
        // printf("Troncons\tMax speeds\n");
        // for (int i = 0 ; i < train->course.size ; i++)
            // printf("%s\t%2.f\n", train->course.steps[i], tma->max_speeds[i]);

        printf("+--------------------------+\n");
        printf("| LA : %2.f\n", tma->length);
        printf("| Dist : %2.f\n", train->can_train->can_odometrie.distance);
        printf("| Max speed : %2.f\n", tma->max_speeds[train->course.current_step]);
        printf("| Current edge : %s\n", train->course.steps[train->course.current_step]);
        printf("| Current Step : %d\n", train->course.current_step);

        // Si la distance autorisée n'est pas dépassée
        pthread_mutex_lock(&(train->can_train->can_odometrie.can_odometrie_mutex));
        if (train->can_train->can_odometrie.distance < tma->length) {
            // On se déplace en avant
            train->is_running = 1;
            // writeVitesseLimite((int)tma->max_speeds[train->course.current_step]);
            writeVitesseLimite(30); // TODO : Ya du 0 dans ce que m'envoie louis, bizarre
            writeVitesseConsigne(20, 1); // TODO : 20 n'est pas la bonne solution, il faut asservir la vitesse
            
            // MAJ distance de ti
            ti->train_position = train->can_train->can_odometrie.distance;
            // MAJ vitesse de ti
            ti->train_velocity = (float) train->can_train->can_odometrie.vit_mesuree;
            // MAJ troncon actuel de ti
            ti->current_edge_id = train->course.steps_code[train->course.current_step];
            // MAJ des troncons à parcourir de ti
            if (train->course.current_step == 0) {
                ti->next_edges = train->course.steps_code;
                ti->next_edges_count = train->course.size;
            }
            // Pour avoir aussi le troncon précédent
            else {
                ti->next_edges = &(train->course.steps_code[train->course.current_step-1]);
                ti->next_edges_count = train->course.size - train->course.current_step + 1;
            }
        }
        // Sinon
        else {
            // // On arrête le train
            // train->is_running = 0;
            // // TODO : faut il arreter l'odometrie ?
            // // On demande au moteur de stop
            // writeVitesseConsigne(0, 1);
            // flag = 0; // on sort de la boucle

            // Démarre la décélération
            deceleration_params_t* params = start_deceleration(train, tma->max_speeds[train->course.current_step], 1, tma->length, 0);

            // On attend la fin de la décélération
            stop_deceleration(params);
        }
        // On déverrouille la mutex
        pthread_mutex_unlock(&(train->can_train->can_odometrie.can_odometrie_mutex));

        usleep(100);
    };

    // On attend la fin des threads
    pthread_join(thread_odo, NULL);
    pthread_join(thread_can, NULL);

    // Suppression du train
    delete_train(train);
    delete_train_info(ti);
    delete_train_mov_auth(tma);
    close(socket);

    return 0;
}