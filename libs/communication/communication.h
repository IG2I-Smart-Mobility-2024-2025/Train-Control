/**
 * @file communication.h
 * @author Téo Trény
 * @brief 
 * @version 0.1
 * @date 2024-11-18
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Consts
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

// Structs for communication

typedef struct {                    // ne pas envoyer de demande d'authorisation de mouvement quand le train est dans un aiguillage. ou succession d'aiguillage
    char train_id;               // Identifiant unique du train
    int current_edge_id;         // Identifiant unique de la derniere balise.
    float train_position;        // Position du train depuis la derniere balise.
    float train_velocity;        // Vitesse du train
    int *next_edges;             // Tableau des tronçons futurs demandés
    int next_edges_count;        // Nombre de tronçons futurs demandés
} train_info;

typedef struct {
    float length;                // Longueur validée par le RBC 
    float *max_speeds;           // Vitesse max pour chaque section du parcours validé (troncons et aiguillages)
    int max_speeds_count;        // Nombre de vitesses max définies (troncons et aiguillages)
} train_mov_auth;

// Functions
/**
 * @brief Configure a client socket in stream mode
 *  
 * @param server_ip Server IP
 * @return int Client socket descriptor
 */
int init_client_socket(const char *server_ip);

/**
 * @brief Send train information to the server
 * 
 * @param socket socket descriptor of the socket
 * @param train train information (=!= train.h)
 * @return int -1 if error, 0 otherwise
 */
int send_train_info(int socket, train_info *train);

/**
 * @brief Create a train mov auth object
 * 
 * @return train_mov_auth* NULL if error
 */
train_mov_auth* create_train_mov_auth();

/**
 * @brief Delete a train movement authorisation
 * 
 * @param tma The object
 * @return int -1 if error, 0 otherwise
 */
int delete_train_mov_auth(train_mov_auth* tma);

/**
 * @brief Receives movement authorisation
 * 
 * @param socket socket descriptor of the socket
 * @param auth response from server
 * @return int -1 if error, 0 otherwise
 */
int recv_train_mov_auth(int socket, train_mov_auth *auth);

/**
 * @brief Create a train info struct
 * 
 * @param id Train id
 * @param current_edge The train current edge 
 * @param position Train distance from last beacon
 * @param velocity Train actual velocity
 * @param planned_edges Train planned edges
 * @param edge_count Number of edges from the train route
 * @return train_info* 
 */
train_info* create_train_info(char id, int current_edge, float position, float velocity, int* planned_edges, int edge_count);

/**
 * @brief Free train_info from memory
 * 
 * @param train_info train_info to delete
 * @return int -1 if error, otherwise 0
 */
int delete_train_info(train_info * train_info);

/**
 * @brief Debug train info
 * 
 * @param train Train to debug
 */
void print_train_info(const train_info* train);

#endif