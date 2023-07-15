
/**
 * Generic Utilities : generic_util.c
 *
 * Author: Jason Brownlee - hop_cha@hotmail.com
 *
 * Generic utility functions
 * 
 */


#include "generic_util.h"


/**
 * Return random double between lower and upper bounds
 * if allowminus, range is extended to (-upper and +upper)
 */
double getRandomDouble(double lower, 
					   double upper, 
					   int allowMinus)
{
	double value;

	if(lower == 0.0 && upper == 1.0)
	{
		value = RANDOM;
	}
	else
	{
		// constrain the random number
		value = (lower + (RANDOM * (upper - lower)));
	}

	if(allowMinus)
	{
		// make + / -
		if(RANDOM > 0.50)
		{
			value = -value;
		}
	}

	return value;
}

/**
 * Return random float between lower and upper bounds
 * if allowminus, range is extended to (-upper and +upper)
 */
float getRandomFloat(float lower, 
					 float upper, 
					 int allowMinus)
{
	return (float) getRandomDouble(lower, upper, allowMinus);
}


/**
 * Return random integer between lower and upper bounds
 * if allowminus, range is extended to (-upper and +upper)
 */
int getRandomInteger(int lower, 
					 int upper, 
					 int allowMinus)
{
	int value;

	if(lower == 0)
	{
		return (rand() % upper);
	}
	else
	{
		// constrain the random number
		value = (int) (((double)lower) + (RANDOM * ((double)(upper - lower))) );
	}

	if(allowMinus)
	{
		// make + / -
		if(RANDOM > 0.50)
		{
			value = -value;
		}
	}

	return value;
}


/**
 * print a list of vectors to screen
 */
void printVectorList(double *vectorList[], 
					 int totalVectors, 
					 int vectorLength)
{
	int i;

	for(i=0; i<totalVectors; i++)
	{
		printf("%2d). ", i);
		printVector(vectorList[i], vectorLength);
	}
}



/**
 * Print a vector on a line
 */
void printVector(double *vector, 
				 int vectorLength)
{
	int i;

	for(i=0; i<vectorLength-1; i++)
	{		
		printf("%1.1f, ", vector[i]);
	}

	printf("%1.1f\n", vector[vectorLength-1]);
}


/**
 * copy source into destination
 */
void vectorCopy(double source[], 
				double dest[], 
				int totalElements)
{
	int i;
	for(i=0; i<totalElements; i++)
	{
		dest[i] = source[i];
	}

}

/** 
 * initialise an array of pointers to null
 */
void init_pointer_array(void *array[], int length)
{
	int i;

	for(i=0; i<length; i++)
	{
		array[i] = NULL;
	}
}
