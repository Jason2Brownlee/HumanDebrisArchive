
// herbivore_agent_perceptron.c


/**
 * Description: Perceptron functions for the herbivore
 * Manages goal seection and aquisition - related to the herbivores memory
 * and perceptron
 *
 *
 * Author: Jason Brownlee 
 *
 * Change History
 * Who			When		Description
 * --------------------------------------------------------------------
 * 
 *
 */

#include "generic_agent.h"
#include "generic_perceptron.h"

#define	GOAL_SELECTION_DEBUG				FALSE


// external
edict_t *getClosestAgent(edict_t *self);
edict_t *getRandomAgent(edict_t *self);
edict_t *getClosestPlant(edict_t *self);
edict_t *getRandomPlant();
edict_t *getRandomPlantFromClosest(edict_t *self);
int getNClosestPlants(edict_t *plantList[], int maxPlants, edict_t *self);
qboolean isPoisonGeneActive(float gene);

// entry points: 
void herbivore_prepare_perceptrion(edict_t *self, edict_t *parent);
void remember_current_feedgoal(edict_t *self, qboolean feedResult);
edict_t *select_food_goal(edict_t *self);

// helper fucntion prototypes
int doRememberPlant(generic_agent_t *aAgent, long plantID);
void populateKnowledgeStructure(herbivore_knowledge_t *source, herbivore_knowledge_t *dest);
void storeNewFeedMemory(herbivore_knowledge_t *feedMemoryQueue[], herbivore_knowledge_t *newMemory);
qboolean isPerceptronFoodGoal(double weights[], herbivore_knowledge_t *knowledge);
void populateKnowledgeFromPlant(edict_t *self, edict_t *plant, herbivore_knowledge_t *knowledge);
edict_t *selectAndRememberFoodGoal(edict_t *self, edict_t *array[], int numGoals);
void printKnowledgeStructure(herbivore_knowledge_t *knowledge);
void trainFromMemory(double *weights, herbivore_knowledge_t *feedMemoryQueue[]);
void printInputVector(double inputVector[]);
void prepareinitializeWeights(double weights[]);



/**
 * Selects a food goal for the herbivore from a list of possible food goals
 *
 * @param edict_t : self
 * 
 * @return edict_t : a selected good goal, or null if no food goals are good
 */
edict_t *select_food_goal(edict_t *self)
{
	edict_t	*selectedGoal = NULL;
	edict_t	*possibleFoodGoals[HERBIVORE_PERCEPTRON_MAX_GOALS];	// an array of pointers to entities	
	int numGoals = 0;

	// get a collection of possible food targets that are close
	numGoals = getNClosestPlants(possibleFoodGoals, HERBIVORE_PERCEPTRON_MAX_GOALS, self);

	// check that there are was some plants
	if(numGoals)
	{
		// use perceptron 
		selectedGoal = selectAndRememberFoodGoal(self, possibleFoodGoals, numGoals); 
	}
	else
	{
		writeMessage("GS: No plants to select a goal from. \n" ,GOAL_SELECTION_DEBUG);
		selectedGoal = NULL;
	}

	return selectedGoal;
}



/**
 * Remember the results of a goal selection whether the selection
 * was a good or bad selection. If the plant already exists in memory,
 * update the details to refect the new (or the same) understanding of the plant.
 *
 * A copy is made of the resulting details (because we have a pointer to it). A pointer
 * of this locally created structure is then stored in the herbivores memory
 *
 * @param edict_t: a herbivore
 * @param qboolean : success if eaten from, otherwise failure
 */
void remember_current_feedgoal(edict_t *self, 
							   qboolean feedResult)
{
	generic_agent_t		   *aAgent = castAgent(self->agent);
	int	                   plantsLocation = 0;
	herbivore_knowledge_t  *currentFeedGoal = &(aAgent->herbivoreAgent->currentFeedGoalKnowledge);

/* // jason brownlee - does not fit in with the model
	// can only create memories and apply knowledge when a child
	if(aAgent->herbivoreAgent->state == HERBIVORE_CHILD_STATE)
	{
*/
		// store the result - the final position in the array
		currentFeedGoal->inputs[HERBIVORE_PERCEPTRON_RESULT_INDEX] = ((feedResult) ? PERCEPTRON_FIRE : PERCEPTRON_NOT_FIRE);

		// check if this is a new memory
		plantsLocation = doRememberPlant(aAgent, currentFeedGoal->plantID);
		if(plantsLocation != -1)
		{
			//writeMessage("GS: Updating knowledge of a plant. \n" ,GOAL_SELECTION_DEBUG);

			// update the herbivores knowledge of this plant
			populateKnowledgeStructure(currentFeedGoal, aAgent->herbivoreAgent->foodSelectionResultQueue[plantsLocation]);
		}	
		else
		{
			// make a new memory for this plant
			storeNewFeedMemory(aAgent->herbivoreAgent->foodSelectionResultQueue, currentFeedGoal);		
		}

		// always retrain from memory
		trainFromMemory(aAgent->herbivoreAgent->perceptron_weights, aAgent->herbivoreAgent->foodSelectionResultQueue);									

/*		
	}
*/
}



/**
 * Prepare a newly born herbivores perceptron details
 *
 * @param edict_t : a herbivore
 * @param herbivore_perceptron_t : pointer to a perceptron to copy, or NULL if 
 *							it is to be populated from random values
 *
 */
void herbivore_prepare_perceptrion(edict_t *self,
								   edict_t *parent)
{
	generic_agent_t *selfAgent = castAgent(self->agent);
	generic_agent_t *parentAgent = NULL;

	// check for no parent
	if(parent == NULL)
	{
		selfAgent->herbivoreAgent->hadParent = false;
		
		// initialise weights to random/good values - ensure bias is also initialised
		prepareinitializeWeights(selfAgent->herbivoreAgent->perceptron_weights);
	}
	// the herbivore does have a parent
	else
	{		
		selfAgent->herbivoreAgent->hadParent = true;

		// make a copy of the weights - ensure bias is also copied
		parentAgent = castAgent(parent->agent);
		vectorCopy(parentAgent->herbivoreAgent->perceptron_weights, 
			       selfAgent->herbivoreAgent->perceptron_weights, 
				   HERBIVORE_PERCEPTRON_NUM_INPUTS + 1);
	}
}

/**
 * set weights to sensible initial values
 */
void prepareinitializeWeights(double weights[])
{
	if(HERBIVORE_PERCEPTRON_INIT_RANDOM)
	{
		initializeWeights(weights, HERBIVORE_PERCEPTRON_NUM_INPUTS + 1);
	}
	else
	{
		// tained from 87.5% - achieved 50.0 % accuracy (7/8 correct)
		weights[W_POISON1] = 1.8076074635437456;
		weights[W_POISON2] = 3.7087329397852065;
		weights[W_POISON3] = 1.7715825137023657;
		weights[W_BIAS]	   = -1.9835396858336072;
	}
}


//
//
// below here are all utlity functions to help with the above entry points
//
//




/**
 * Determine whether the provided plant already exists in the
 * provided herbivores memory 
 *
 * returns the positive array index of the plants position in the queue
 * or (-1) if the plant is not in the herbivores memory
 */
int doRememberPlant(generic_agent_t *aAgent,
					long plantID)
{
	int i;

	// go through the herbivores memory and determine if they remember ths provided plant id
	for(i=0; i<HERBIVORE_PERCEPTRON_MAX_PATTERNS; i++)
	{
		// if memory is NULL then its not a result
		if(aAgent->herbivoreAgent->foodSelectionResultQueue[i] == NULL)
		{			
			return -1; // not all brain cells have been populated with memories yet
		}
		// check if the herbivore already knows the plant
		else if(aAgent->herbivoreAgent->foodSelectionResultQueue[i]->plantID == plantID)
		{			
			return i; // herbivore does remember this plant
		}
	}

	// ran out memory to check 
	return -1; // do not know plant
}


/**
 * Copy the values in the source structure to values in the destination structure.
 */
void populateKnowledgeStructure(herbivore_knowledge_t *source, 
							    herbivore_knowledge_t *dest)
{
	// copy the inputs - as well as the result
	vectorCopy(source->inputs, dest->inputs, HERBIVORE_PERCEPTRON_NUM_INPUTS + 1);

	// copy the plant id
	dest->plantID = source->plantID;
}



/**
 * store a new memory for in a herbivores memory structure
 *
 * The herbivore stores memorys in a FIFO Queue structure. This
 * meaning older memories are replaced with newer memories.
 *
 * This function manages the queue strucutre - but really
 * an external libaray using a linked list should be used. - future improvement
 */
void storeNewFeedMemory(herbivore_knowledge_t *feedMemoryQueue[], 
						herbivore_knowledge_t *newMemory)
{	
	herbivore_knowledge_t *newMemoryLocation = NULL; // structure where new memory will be stored
	int i;

	// look for a spare place
	for(i=0; i<HERBIVORE_PERCEPTRON_MAX_PATTERNS; i++)
	{
		// if memory is NULL then its not a result
		if(feedMemoryQueue[i] == NULL)
		{
			// allocate system memory
			newMemoryLocation = (herbivore_knowledge_t*) malloc(sizeof(herbivore_knowledge_t));

			// copy the memory into the new memory location
			populateKnowledgeStructure(newMemory, newMemoryLocation);

			// store a copy to the structure 
			feedMemoryQueue[i] = newMemoryLocation;

			//writeMessage("GS: Allocated new memory for a plant. \n" ,GOAL_SELECTION_DEBUG);
			
			return; // finished
		}		
	}

	// no spare slot, need to create one

	// this will forget the first element in the list, and allow
	// the new result to be remembered at the end (creating a queue)
	for(i=1; i<HERBIVORE_PERCEPTRON_MAX_PATTERNS; i++)
	{
		// copy current element into previous element - element [0] is lost and all others are shifted down
		populateKnowledgeStructure(feedMemoryQueue[i], feedMemoryQueue[i-1]);
	}

	//writeMessage("GS: Forgot old memory for new plant memory. \n" ,GOAL_SELECTION_DEBUG);

	// store the new element at the end of the queue
	populateKnowledgeStructure(newMemory, feedMemoryQueue[HERBIVORE_PERCEPTRON_MAX_PATTERNS-1]);
}


/**
 * determine if the provided knowledge about a plant is determined
 * by the herbivores perceptron to be a food goal or not
 */
qboolean isPerceptronFoodGoal(double weights[], 
							  herbivore_knowledge_t *knowledge)
{	
	double perceptronResult;
		
	// get the perceptrons opinion
	perceptronResult = test(weights, 
							knowledge->inputs, 
							HERBIVORE_PERCEPTRON_NUM_INPUTS);

	// translate into something useable
	if(perceptronResult == PERCEPTRON_FIRE)
	{
		return true;
	}

	return false;
}

/**
 * populate the provided knolwedge structure with details derived from the
 * provided plant entity
 */
void populateKnowledgeFromPlant(edict_t *self, 
								edict_t *plant, 
								herbivore_knowledge_t *knowledge)
{
	generic_agent_t		*selfAgent = castAgent(self->agent);
	generic_agent_t		*plantAgent = castAgent(plant->agent);

	//Com_Printf("GS: Populate knowledge structure: Herbivore ID[%d], Plant ID[%d]\n", selfAgent->identity, plantAgent->identity);

	// store the plant id
	knowledge->plantID = plantAgent->identity;

	// set input values
	knowledge->inputs[W_POISON1] = isPoisonGeneActive(plantAgent->plantAgent->dna.poison1);
	knowledge->inputs[W_POISON2] = isPoisonGeneActive(plantAgent->plantAgent->dna.poison2);
	knowledge->inputs[W_POISON3] = isPoisonGeneActive(plantAgent->plantAgent->dna.poison3);

	// NOTE: this is not known at this stage - populate with a false value
	knowledge->inputs[HERBIVORE_PERCEPTRON_RESULT_INDEX] = -1.0; // invalid - should NEVER be fed into perceptron with this value
}


/**
 * displays the provided knowledge structure to the console
 */
void printKnowledgeStructure(herbivore_knowledge_t *knowledge)
{
	if(GOAL_SELECTION_DEBUG)
	{
		// print plant id
		Com_Printf("ID[%d] ", knowledge->plantID);

		// print all inputs
		printInputVector(knowledge->inputs);
	}
}

/**
 * print an input vector
 */
void printInputVector(double inputVector[])
{
	Com_Printf("Poison1[%g], Poison2[%g], Poison3[%g], Rs[%g].\n",
				inputVector[0],
				inputVector[1],
				inputVector[2],
				inputVector[3]); // rs
}


/**
 * from a listing of plant entities, select a food entity
 */
edict_t *selectAndRememberFoodGoal(edict_t *self,
									edict_t *array[], 
									int numGoals)
{
	generic_agent_t	*selfAgent = castAgent(self->agent);
	edict_t *selectedGoal = NULL;
	herbivore_knowledge_t knowledge; // reused - but remeber its local	
	edict_t *perceptronGoalList[HERBIVORE_PERCEPTRON_MAX_GOALS];
	int totalPerceptronGoals = 0;
	int i;

	// use perceptron
	if(HERBIVORE_PERCEPTRON_ENABLED)
	{			
		// build a listing of all entities the perceptron things are good food goals
		for(i=0; i<numGoals; i++)
		{				
			// populate the structure with knowledge about the current plant
			populateKnowledgeFromPlant(self, array[i], &knowledge);

			// use perceptron to classify the plant
			if(isPerceptronFoodGoal(selfAgent->herbivoreAgent->perceptron_weights, &knowledge))
			{
				perceptronGoalList[totalPerceptronGoals++] = array[i];
			}
		}

		// check that there was at least one goal the perceptron liked
		if(totalPerceptronGoals)
		{
			// select one from the array at random - do not want the perceptron to select the same one all the time
			selectedGoal = perceptronGoalList[getRandomInteger(0, totalPerceptronGoals, FALSE)];
			//writeMessage("GS: Perceptron selected new feed goal\n", GOAL_SELECTION_DEBUG);
		}
		else
		{
			writeMessage("GS: Perceptron failed to locate a suitable food goal\n", GOAL_SELECTION_DEBUG);
		}
	}

	// not using Perceptron for selection, or perceptron failed to find a target
	if(selectedGoal == NULL)
	{	
		// select a closest goal
		selectedGoal = array[getRandomInteger(0, numGoals, FALSE)]; 
		//writeMessage("GS: Selected random goal from closest list\n", GOAL_SELECTION_DEBUG);		
	}	

	// remember the selected goal
	populateKnowledgeFromPlant(self, selectedGoal, &(selfAgent->herbivoreAgent->currentFeedGoalKnowledge));
	return selectedGoal;
}






/**
 * train the perceptron weights on the current feed queue
 */
void trainFromMemory(double *weights, 
					 herbivore_knowledge_t *feedMemoryQueue[])
{
	double *inputVectorList[HERBIVORE_PERCEPTRON_MAX_PATTERNS];
	int totalUsableVectors = 0;
	int i = 0;
	int totalEpochs = 0;
	double testingResult = 0.0;

	// prepare a list of training vecors
	for(totalUsableVectors=0; totalUsableVectors<HERBIVORE_PERCEPTRON_MAX_PATTERNS; totalUsableVectors++)
	{
		if(feedMemoryQueue[totalUsableVectors] == NULL)
		{
			break;
		}
		
		inputVectorList[totalUsableVectors] = feedMemoryQueue[totalUsableVectors]->inputs;		
	}

	// check for no patterns in memory - should always have something in memory when running this
	if(!totalUsableVectors)
	{
		Com_Printf("GS:ERROR Unable to train from memory - no memory to train from.\n");
		return;
	}

	// dump list of all training patterns
	if(GOAL_SELECTION_DEBUG)
	{
		//Com_Printf("GS: About to train with [%d] usable input vectors.\n", totalUsableVectors);		

		//Com_Printf("Weights Before: ");
		//printInputVector(weights);
/*
		// print all vectors to screen
		Com_Printf("---------------------------------------------\n");
		for(i=0; i<totalUsableVectors; i++)
		{
			Com_Printf("%d).", i);
			printInputVector(inputVectorList[i]);
		}
		Com_Printf("---------------------------------------------\n");
*/
	}


	// NOTE: training will stop after [totalUsableVectors] so do not worry about garbage if the
	// queue is not full

	// perform training	
	totalEpochs = train(weights, 
						inputVectorList, 
						totalUsableVectors, 
						HERBIVORE_PERCEPTRON_NUM_INPUTS, 
						HERBIVORE_PERCEPTRON_MAX_EPOCHS, 
						HERBIVORE_PERCEPTRON_EARLY_STOP);

	// dump test results of training patterns
	if(GOAL_SELECTION_DEBUG)
	{
		//Com_Printf("Completed training after [%d] epochs\n", totalEpochs);

		//Com_Printf("Weights After: ");
		//printInputVector(weights);

/*
		Com_Printf("GS: Results after training\n");
		
		// print all vectors to screen
		Com_Printf("---------------------------------------------\n");
		for(i=0; i<totalUsableVectors; i++)
		{
			testingResult = test(weights, inputVectorList[i], HERBIVORE_PERCEPTRON_NUM_INPUTS);
			Com_Printf("%d). Expected[%g], Actual[%g].\n", 
				       i, 
					   inputVectorList[i][HERBIVORE_PERCEPTRON_RESULT_INDEX], 
					   testingResult);
		}
		Com_Printf("---------------------------------------------\n");
*/
	}
}
