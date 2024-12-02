/**
 * @file train.c
 * @author Téo Trény (teo.treny@ig2i.centralelille.fr)
 * @brief 
 * @version 0.1
 * @date 2024-11-15
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "train.h"

/**
 * @brief Initialize a train
 * 
 * @param id Train id
 * @return Train_t* NULL if an error occured, a pointer to the train otherwise
 */
Train_t * train_init(char id) {
    // Allocate memory for the train
    Train_t * train = (Train_t *) malloc(sizeof(Train_t));
    if (train == NULL) return NULL;

    // Initialize the train
    train->id = id;
    train->course.size = 0;
    train->course.filename = NULL;
    train->course.steps = NULL;
    train->course.steps_code = NULL;
    train->course.current_step = 0;
    train->course.repeat = 0;

    // Initialize the can train with the default port
    train->can_train = create_can_train(NULL, on_beacon_passed);
    train->can_train->callback_arg = train;

    // Initialize the odometrie 
    train->odometrie = create_odometrie(NULL, NULL);

    return train;
}

/**
 * @brief Load a train course from a file
 * 
 * @param train     Train to load the course into
 * @param filename  Filename of the course
 * @param repeat    Number of times the course is repeated
 * @return int      0 if the course was loaded successfully
 *                  -1 if an error occured
 *                  -2 if the repeat is invalid (the first and last steps are different)
 */
int load_train_course(Train_t * train, char* filename, int repeat) {
    // Check
    if (train == NULL || filename == NULL) return -1;
    
    // Open the file
    FILE * file = fopen(filename, "r");
    if (file == NULL) return -1;

    // Read the file line by line
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Remove the newline character
        line[strcspn(line, "\n")] = 0;
        // Add the step to the course


        // train->course.steps is a char **
        train->course.steps = (char **) realloc(train->course.steps, (train->course.size + 1) * sizeof(char *));
        if (train->course.steps == NULL) return -1;
        // train->course.steps[train->course.size] is a char *
        train->course.steps[train->course.size] = (char *) malloc(sizeof(line));
        if (train->course.steps[train->course.size] == NULL) return -1;
        strcpy(train->course.steps[train->course.size], line);
        
        // train->course.steps_code is a int *
        train->course.steps_code = (int *) realloc(train->course.steps_code, (train->course.size + 1) * sizeof(int));
        train->course.steps_code[train->course.size] = (int) string_to_code(line);
        
        train->course.size++;
    }
    
    // Set current step (first step)
    train->course.current_step = 0;

    // Close the file
    fclose(file);

    // Set the repeat (last step must be the same as the first step)
    if (repeat && strcmp(COURSE_FIRST_STEP(train), COURSE_LAST_STEP(train)) == 0)
        train->course.repeat = repeat;
    else
        return -2;

    // Set the filename
    train->course.filename = filename;

    return 0;
}

/**
 * @brief Delete a train
 * 
 * @param train Train to delete
 * @return int 0 if the train was deleted successfully, -1 otherwise
 */
int delete_train(Train_t * train) {
    if (train == NULL) return -1;

    // Delete the course
    for (int i = 0; i < train->course.size; i++) {
        free(train->course.steps[i]);
    }
    free(train->course.steps);
    free(train->course.steps_code);
    free(train->course.filename);

    // Delete the can train
    delete_can_train(train->can_train);

    // Delete the odometrie
    delete_odometrie(train->odometrie);

    // Free the train
    free(train);

    return 0;
}

/**
 * @brief To debug a train
 * 
 * @param train Train to debug
 */
void debug_train(Train_t * train) {
    printf("Train id: %d\n", train->id);
    printf("Speed (can): %d\n", train->can_train->can_odometrie.vit_mesuree);
    printf("Distance (can): %.2f\n", train->can_train->can_odometrie.distance);
    printf("Course current step: [%d] --> %s\n", train->course.current_step, train->course.steps[train->course.current_step]);
    printf("Course size: %d\n", train->course.size);
    printf("Course repeat: %d\n", train->course.repeat);
    printf("Course filename: %s\n", train->course.filename);
}

/**
 * @brief From "0x..." to "C..."
 * 
 * @param code 
 * @return const char* 
 */
const char* code_to_string(unsigned char code) {
    switch(code) {
        case 0x051: return "C1";
        case 0x052: return "C2";
        case 0x053: return "C3";
        case 0x054: return "C4";
        case 0x055: return "C5";
        case 0x056: return "C6";
        case 0x057: return "C7";
        case 0x058: return "C8";
        case 0x059: return "C9";
        case 0x05A: return "C10";
        case 0x05B: return "C11";
        case 0x05C: return "C12";
        case 0x05D: return "C13";
        case 0x05E: return "C14";
        case 0x05F: return "C15";
        case 0x060: return "C16";
        case 0x061: return "C17";
        case 0x062: return "C18";
        case 0x063: return "C19";
        case 0x064: return "C20";
        case 0x065: return "C21";
        case 0x066: return "C22";
        case 0x067: return "C23";
        case 0x068: return "C24";
        case 0x069: return "C25";
        case 0x06A: return "C26";
        case 0x06B: return "C27";
        case 0x06C: return "C28";
        case 0x06D: return "C29";
        default: return "Unknown";
    }
}

/**
 * @brief From "C..." to "0x..."
 * 
 * @param str 
 * @return unsigned char 
 */
unsigned char string_to_code(const char* str) {
    if (str == NULL || str[0] != 'C') {
        return 0x00;  // Invalid input
    }

    // Get the numeric part of the string
    int num = atoi(str + 1);  // Skip 'C' prefix

    switch(num) {
        case 1: return 0x051;
        case 2: return 0x052;
        case 3: return 0x053;
        case 4: return 0x054;
        case 5: return 0x055;
        case 6: return 0x056;
        case 7: return 0x057;
        case 8: return 0x058;
        case 9: return 0x059;
        case 10: return 0x05A;
        case 11: return 0x05B;
        case 12: return 0x05C;
        case 13: return 0x05D;
        case 14: return 0x05E;
        case 15: return 0x05F;
        case 16: return 0x060;
        case 17: return 0x061;
        case 18: return 0x062;
        case 19: return 0x063;
        case 20: return 0x064;
        case 21: return 0x065;
        case 22: return 0x066;
        case 23: return 0x067;
        case 24: return 0x068;
        case 25: return 0x069;
        case 26: return 0x06A;
        case 27: return 0x06B;
        case 28: return 0x06C;
        case 29: return 0x06D;
        default: return 0x00;  // Unknown canton
    }
}

/**
 * @brief Print the course of a train
 * 
 * @param train Train to print the course of
 */
void print_train_course (Train_t * train) {
    for (int i = 0; i < train->course.size; i++) {
        printf("Step %d: %s\n", i, train->course.steps[i]);
    }
}

/**
 * @brief Fonction de callback appelée lorsqu'une balise est passée
 * 
 * @note On passe le train par argument. Pour un système temps réel critique, ce n'est pas optimal.
 * Il faudrait utiliser une variable globale dans notre cas.
 * 
 * @param arg Argument de la fonction (train_t*)
 */
void on_beacon_passed(void * arg) {
    printf("Beacon passed\n");
    // On récupère le train grâce à un passe passe avec des void*
    Train_t * train = (Train_t *) arg;

    // On reset l'odométrie
    reset_odometrie(train->odometrie);
}