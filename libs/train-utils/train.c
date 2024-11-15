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
#include "train-utils.h"

/**
 * @brief Initialize a train
 * 
 * @param id Train id
 * @param position Train initial position
 * @return Train_t* NULL if an error occured, a pointer to the train otherwise
 */
Train_t * train_init(int id, Position_t position) {
    // Allocate memory for the train
    Train_t * train = (Train_t *) malloc(sizeof(Train_t));
    // Check
    if (train == NULL) return NULL;
    
    // Initialize the train
    train->id = id;
    train->position = position;
    train->speed = 0;
    train->distance = 0;
    train->course.size = 0;
    train->course.filename = NULL;
    train->course.steps = NULL;
    train->course.current_step = 0;
    train->course.repeat = 0;
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
int load_train_course(Train_t * train, char* filename, int repeat = 0) {
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
        train->course.steps = (char *) realloc(train->course.steps, (train->course.size + 1) * sizeof(char));
        if (train->course.steps == NULL) return -1;
        train->course.steps[train->course.size] = line[0];
        train->course.size++;
    }
    
    // Set current step (first step)
    train->course.current_step = 0;

    // Close the file
    fclose(file);

    // Set the repeat
    if (repeat && strcomp(COURSE_FIRST_STEP(train), COURSE_LAST_STEP(train)) == 0)
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
    if (train->course.steps != NULL) free(train->course.steps);
    if (train->course.filename != NULL) free(train->course.filename);
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
    printf("Position: (%d, %d, %d)\n", train->position.x, train->position.y, train->position.z);
    printf("Speed: %d\n", train->speed);
    printf("Distance: %.2f\n", train->distance);
    printf("Course: %s\n", train->course.steps);
    printf("Course current step: [%d] --> %s\n", train->course.current_step, train->course.steps[train->course.current_step]);
    printf("Course size: %d\n", train->course.size);
    printf("Course repeat: %d\n", train->course.repeat);
    printf("Course filename: %s\n", train->course.filename);
}