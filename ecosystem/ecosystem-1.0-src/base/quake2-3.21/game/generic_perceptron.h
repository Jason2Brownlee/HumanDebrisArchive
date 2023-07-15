/**
 * Generic Perceptron : generic_perceptron.h
 *
 * Author: Jason Brownlee - hop_cha@hotmail.com
 *
 * Models a single neuron with a linear transfer function - as basic as it gets.
 * Supports any number of inputs + bias, and a single output
 * This structure is designed to to be reused, as it maintains no state.
 * 
 * To use:
 *
 * Initialize Weights : initializeWeights(double weights[], int totalWeights)
 * Train Weights      : int train(double weights[], double trainingVectors[][], int, totalVectors, int totalInputs, int maxEpochs, int stopOnSuccess)
 * Get Classification : double test(double weights[], double vector[], int totalInputs);
 * 
 */

#ifndef GENERIC_PERCEPTRON_H
#define GENERIC_PERCEPTRON_H


#include "generic_util.h"


// must be > this for a neuron to fire
#define TRANSFER_FUNCTION_THRESHOLD			+0.0

// value when the neuron fires
#define PERCEPTRON_FIRE                     +1.0

// value when neuron does not fire
#define PERCEPTRON_NOT_FIRE					+0.0

// the constant value always input from the bias
#define BIAS_INPUT							+1.0


// prototypes
double transferFunction(double activation);
double activationFunction(double weights[],double vector[],int numInputs);
void initializeWeights(double weights[], int totalWeights);
double test(double weights[], double vector[], int totalInputs);
void adaptWeights(double weights[], double inputVector[], int totalInputs, double generatedOutput);
int meetStopCondition(int stopOnSuccess, int allCorrect, int currentEpoch, int maxEpochs);
int train(double weights[], double *trainingVectors[], int totalVectors, int totalInputs, int maxEpochs, int stopOnSuccess);


#endif