/**
 * Generic Utilities : generic_util.h
 *
 * Author: Jason Brownlee - hop_cha@hotmail.com
 *
 * Generic utility functions
 * 
 */

#ifndef GENERIC_UTIL_H
#define GENERIC_UTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>

// macro for getting a random double between 0 and 1
// rand() is between [0 - 32767] constrained to [0 - 1]
#define RANDOM (((double)rand()) / ((double)RAND_MAX))

#define TRUE		+1
#define FALSE		+0


#define SUCCESS		+1
#define FAILURE		-1

#define APPLICATION_SUCCESS		+0
#define APPLICATION_FAILURE		+1


// random numbers
double getRandomDouble(double lower, double upper, int allowMinus);
float getRandomFloat(float lower, float upper,int allowMinus);
int getRandomInteger(int lower, int upper, int allowMinus);

// vectors
void printVectorList(double *vectorList[], int totalVectors, int vectorLength);
void printVector(double *vector, int vectorLength);
void vectorCopy(double source[], double dest[], int totalElements);

void init_pointer_array(void *array[], int length);

#endif
