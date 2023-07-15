
/**
 * Generic Perceptron : generic_perceptron.c
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

#include "generic_perceptron.h"


// sample problems
// ----------------------------------------------------------

// and problem

static double AND_PROBLEM[4][3] = {
									{1.0, 1.0, 1.0},
									{1.0, 0.0, 0.0},
									{0.0, 1.0, 0.0},
									{0.0, 0.0, 0.0}
								  };

// or problem
double static OR_PROBLEM[4][3] = {
									{1.0, 1.0, 1.0},
									{1.0, 0.0, 1.0},
									{0.0, 1.0, 1.0},
									{0.0, 0.0, 0.0}
								 };
// ----------------------------------------------------------




/**
 * simple function used for unit testing the code with a sample a problem
 */
void runTestProblem(double *problem[], 
					int totalPatterns, 
					int totalInputs,
					double weights[],
					int maxEpochs,
					int stopOnSuccess)
{
	int trainingResult;
	double testResult;
	int i;
	int correct;

	// print initial weights
	printf("Initial Weights\n");
	printf("------------------------------------------------\n");	
	printVector(weights, totalInputs+1);
	printf("------------------------------------------------\n");
	printf("\n");

	// print all patterns to screen
	printf("All Patterns:\n");
	printf("------------------------------------------------\n");	
	printVectorList(problem, totalPatterns, totalInputs+1);
	printf("------------------------------------------------\n");
	printf("\n");

	// train the weights on selected problem
	trainingResult = train(weights, problem, totalPatterns, totalInputs, maxEpochs, stopOnSuccess);
	printf("Completed training after %d epochs.\n", trainingResult);
	printf("\n");

	
	printf("Results:\n");
	printf("------------------------------------------------\n");
	// test on all vectors
	for(i=0; i<totalPatterns; i++)
	{
		testResult = test(weights, problem[i], totalInputs);
		correct = (testResult == problem[i][totalInputs]);
		printf("Test pattern[%2d], Expected[%1.0f], Actual[%1.0f], Correct[%s].\n", 
			   i,
			   problem[i][totalInputs],
			   testResult,
			   ((correct)?"True":"False"));
	}
	printf("------------------------------------------------\n");
	printf("\n");

	// print initial weights
	printf("Final Weights\n");
	printf("------------------------------------------------\n");	
	printVector(weights, totalInputs+1);
	printf("------------------------------------------------\n");
	printf("\n");
}



/**
 * prepare the AND sample problem
 */
void prepareAndProblem(double *problem[])
{
	int i;
	for(i=0;i<4; i++)
	{
		// pointer i points to the i'th array 
		problem[i] = AND_PROBLEM[i];
	}
}

/**
 * prepare the OR sample problem
 */
void prepareOrProblem(double *problem[])
{
	int i;
	for(i=0;i<4; i++)
	{
		// pointer i points to the i'th array 
		problem[i] = OR_PROBLEM[i];
	}
}

/**
 * Example of how to use this structure to 
 * train a perceptron on the AND & OR binary problems
 *
 * Provides an effective unit testing platform
 */
int main()
{
	//
	// prepare generic properties
	//
	double **problem;
	int totalPatterns, inputs;
	int maxEpochs = 1000;
	int stopOnSuccess = TRUE;
	

	//
	// prepare a specific sample problem
	//


	// prepare the AND problem
	///*
	double *and_problem[4];
	double weights[3]; // inputs + 1
	prepareAndProblem(and_problem);	
	problem = and_problem;
	inputs = 2;
	totalPatterns = 4;
	//*/

	
	// prepare the OR problem
	/*
	double *or_problem[4];
	double weights[3]; // inputs + 1
	prepareOrProblem(or_problem);	
	problem = or_problem;
	inputs = 2;
	totalPatterns = 4;
	*/

	// initialise weights
	initializeWeights(weights, (inputs+1));

	//
	// run the test problem
	//
	runTestProblem(problem, totalPatterns, inputs, weights, maxEpochs, stopOnSuccess);

	return APPLICATION_SUCCESS;
}







/**
 * Trains the perceptron on the provided training patterns for maxEpochs number of epochs.
 * stopOnSuccess can be used for an early stop condition when the perceptron gets all patterns
 * correct earlier than expected. This is an online training approach where errors are used
 * to adapt ther weights after each patten, as opposed to Batch where updates occur
 * at the end of epochs.
 *
 * Weights must be [totalInputs + 1] in length to accomodate the BIAS constant input.
 *
 * The expected outputs for each input pattern MUST be the totalInputs'th element in each input
 * pattern vector, thus weights and a single inputvecor will be of the same length.
 *
 * Returns the total number of epochs executed.
 */
int train(double weights[], 
		  double *trainingVectors[], 
		  int totalVectors, 
		  int totalInputs, 
		  int maxEpochs, 
		  int stopOnSuccess)
{
	int allCorrect = FALSE;	
	int epochNumber;
	int i;
	double *inputVector;
	double output;

	// each iteration is a single epoch
	// consists of two stop conditions - totalEpochs, and if stopOnSuccess and all patterns are classified correctly
	for(epochNumber=0; !meetStopCondition(stopOnSuccess,allCorrect,epochNumber,maxEpochs); epochNumber++)
	{
		// optimistic - expect all correct until determined otherwise
		allCorrect = TRUE;

		for(i=0; i<totalVectors; i++)
		{
			// get a single input vector
			inputVector = trainingVectors[i];

			// test the input
			output = test(weights, inputVector, totalInputs);
			
			// check for error
			if(output != inputVector[totalInputs])
			{
				allCorrect = FALSE;
				adaptWeights(weights, inputVector, totalInputs, output); // update weights to learn error in ways
			}
		} // end of epoch
	}

	// total number of epochs completed
	return epochNumber;
}


/**
 * models the stop condition for training the perceptron
 * looks less messy than putting it all in the condition of the for loop
 */
int meetStopCondition(int stopOnSuccess, 
					  int allCorrect, 
					  int currentEpoch, 
					  int maxEpochs)
{
	// check if maximum epochs has been reached
	if(currentEpoch >= maxEpochs)
	{
		return TRUE;
	}
	// check if all patterns were classified correctly and the user wants an eary exit
	else if(stopOnSuccess && allCorrect)
	{
		return TRUE;
	}

	return FALSE;
}



/**
 * Given the set of weights, determine the perceptron output for a 
 * given input vector. The total number of weights MUST be 
 * [totalInputs + 1] in length to support to BIAS (constant)
 * input. The value returned is the perceptron output for
 * the given input pattern.
 */
double test(double weights[], 
			double vector[], 
			int totalInputs)
{
	// get activation based on provided input vector
	double activation = activationFunction(weights, vector, totalInputs);

	// determine output using transfer function and activation
	double output = transferFunction(activation);

	// return the result to the user
	return output;
}


/**
 * Initialize a set of weights to random values. Total weights
 * must be [(num inputs) + 1] == to accomodate the bias
 */
void initializeWeights(double weights[], 
					   int totalWeights)
{
	int i;

	for(i=0; i<totalWeights; i++)
	{
		// initialize to small random values between -0.3 and +0.3
		weights[i] = getRandomDouble(0.0, 0.3, TRUE);
	}
}




/**
 * This is the learning function responsible for adapting weights based on the amount of error
 * It is assumed that the expected output is the totalInputs'th element in the input vector,
 * thus both weights and inputVector are of the same length. The generated output is the output 
 * returned from the test() function.
 *
 * Note: this is function is for an online learning approach (adapt after each pattern)
 */
void adaptWeights(double weights[], 
				  double inputVector[], 
				  int totalInputs, 
				  double generatedOutput) 
{
	// determine the perceptrons error
	double errorInOutput = (inputVector[totalInputs] - generatedOutput); // expected - actual
	int i;

	// adjust all weights
	for(i=0; i<totalInputs; i++)
	{
		// Weight = Weight + ((Expected - Output) * Input)
		weights[i] += (errorInOutput * inputVector[i]);
	}

	// adjust the weight on the bias input
	weights[totalInputs] += (errorInOutput * BIAS_INPUT);
}



/**
 * Neurons internal activation based on inputs.
 * takes a constant bias into accounr
 */
double activationFunction(double weights[], 
						  double vector[], 
						  int numInputs)
{
	double sum = 0.0;
	int i;

	// sum weighted inputs
	for(i=0; i<numInputs; i++)
	{
		// input * weight
		sum += (vector[i] * weights[i]);
	}

	// add the constant bias input
	sum += (BIAS_INPUT * weights[numInputs]);

	return sum;
}



/**
 * Simple linear transfer function 
 * Used to determine the neurons output based on its activation
 */
double transferFunction(double activation)
{
	double output;

	// check for a neuron fire
	if(activation > TRANSFER_FUNCTION_THRESHOLD)
	{
		output = PERCEPTRON_FIRE;
	}
	else
	{
		output = PERCEPTRON_NOT_FIRE;
	}

	//printf("Activation got[%g], output: [%g]\n", activation, output);

	return output;
}

