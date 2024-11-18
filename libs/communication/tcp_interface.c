/**
 * @file tcp_interface.c
 * @author Téo Trény
 * @brief 
 * @version 0.1
 * @date 2024-11-18
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "communication.h"

/**
 * @brief Configure a client socket in stream mode
 *  
 * @param server_ip Server IP
 * @return int Client socket descriptor
 */
int init_client_socket(const char *server_ip) {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Erreur lors de la création du socket");
        return -1;
    }
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur de connexion au serveur");
        close(client_socket);
        return -1;
    }
    return client_socket;
}

/**
 * @brief Send train information to the server
 * 
 * @param socket socket descriptor of the socket
 * @param train train information (=!= train.h)
 * @return int -1 if error, 0 otherwise
 */
int send_train_info(int socket, train_info *train) {
    int data_size = sizeof(char) + sizeof(int) + 2 * sizeof(float) + sizeof(int) + train->next_edges_count * sizeof(int);
    char *buffer = (char *)malloc(data_size);
    if (!buffer) {
        perror("Erreur d'allocation mémoire");
        return -1;
    }

    // Sérialisation des données dans le buffer
    int offset = 0;
    memcpy(buffer + offset, &train->train_id, sizeof(char));
    offset += sizeof(char);
    memcpy(buffer + offset, &train->current_edge_id, sizeof(int));
    offset += sizeof(int);
    memcpy(buffer + offset, &train->train_position, sizeof(float));
    offset += sizeof(float);
    memcpy(buffer + offset, &train->train_velocity, sizeof(float));
    offset += sizeof(float);
    memcpy(buffer + offset, &train->next_edges_count, sizeof(int));
    offset += sizeof(int);
    memcpy(buffer + offset, train->next_edges, train->next_edges_count * sizeof(int));

    int sent_bytes = send(socket, buffer, data_size, 0);
    free(buffer);
    return sent_bytes == data_size ? 0 : -1;
}

/**
 * @brief Create a train mov auth object
 * 
 * @return train_mov_auth* NULL if error
 */
train_mov_auth* create_train_mov_auth() {
    train_mov_auth* res = (train_mov_auth*) malloc(sizeof(train_mov_auth));
    return res;
}

/**
 * @brief Delete a train movement authorisation
 * 
 * @param tma The object
 * @return int -1 if error, 0 otherwise
 */
int delete_train_mov_auth(train_mov_auth* tma) {
    if (tma == NULL) return -1;
    if (tma->max_speeds != NULL) free(tma->max_speeds);
    free(tma);
    return 0;
}

/**
 * @brief Receives movement authorisation
 * 
 * @param socket socket descriptor of the socket
 * @param auth response from server
 * @return int -1 if error, 0 otherwise
 */
int recv_train_mov_auth(int socket, train_mov_auth *auth) {
    int received = recv(socket, &auth->length, sizeof(float), 0);
    if (received <= 0) return -1;

    received = recv(socket, &auth->max_speeds_count, sizeof(int), 0);
    if (received <= 0) return -1;

    auth->max_speeds = (float *)malloc(auth->max_speeds_count * sizeof(float));
    if (!auth->max_speeds) {
        perror("Erreur d'allocation mémoire");
        return -1;
    }

    received = recv(socket, auth->max_speeds, auth->max_speeds_count * sizeof(float), 0);
    return received > 0 ? 0 : -1;
}

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
train_info* create_train_info(char id, int current_edge, float position, float velocity, int* planned_edges, int edge_count) {
    train_info* train = (train_info*)malloc(sizeof(train_info));
    if (train == NULL) {
        printf("Memory allocation failed for train_info\n");
        return NULL;
    }

    // Initialize basic information
    train->train_id = id;
    train->current_edge_id = current_edge;
    train->train_position = position;
    train->train_velocity = velocity;
    train->next_edges_count = edge_count;

    // Allocate and copy the next_edges array
    train->next_edges = (int*)malloc(edge_count * sizeof(int));
    if (train->next_edges == NULL) {
        printf("Memory allocation failed for next_edges\n");
        free(train);
        return NULL;
    }
    memcpy(train->next_edges, planned_edges, edge_count * sizeof(int));

    return train;
}

/**
 * @brief Free train_info from memory
 * 
 * @param train_info train_info to delete
 * @return int -1 if error, otherwise 0
 */
int delete_train_info(train_info* train_info) {
    if (train_info != NULL) {
        free(train_info->next_edges);
        free(train_info);
        return 0;
    }
    return -1;
}

/**
 * @brief Debug train info
 * 
 * @param train Train to debug
 */
void print_train_info(const train_info* train) {
    printf("\nTrain Information:\n");
    printf("Train ID: %d\n", train->train_id);
    printf("Current Edge ID: %d\n", train->current_edge_id);
    printf("Position: %.2f meters\n", train->train_position);
    printf("Velocity: %.2f m/s\n", train->train_velocity);
    printf("Planned route (%d segments): ", train->next_edges_count);
    for (int i = 0; i < train->next_edges_count; i++) {
        printf("%d ", train->next_edges[i]);
    }
    printf("\n");
}