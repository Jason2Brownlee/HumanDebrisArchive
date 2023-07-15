
// plant_agent_ga.c


/**
 * Description: The plant agent ga functions
 *
 * Author: Jason Brownlee
 *
 * Change History
 * Who			When		Description
 * --------------------------------------------------------------------
 * 
 */


#include "generic_agent.h"

#define PLANT_GA_DEBUG						FALSE


//entry points
void prepareDNA(plant_dna_t *parent1DNA, plant_dna_t *parent2DNA, plant_dna_t *childDNA, int seedNumber);
void cloneDNA(plant_dna_t *sourceDNA, plant_dna_t *destDNA);


// Function Prototypes for this file
void prepareRandomDNA(plant_dna_t *childDNA);
void mutateDNA(plant_dna_t *childDNA);
qboolean shouldMutate();
void crossoverDNA(plant_dna_t *parent1DNA, plant_dna_t *parent2DNA, plant_dna_t *childDNA);






/**
 * Prepare a new child dna from two parent dna's
 */
void prepareDNA(plant_dna_t *parent1DNA, 
				plant_dna_t *parent2DNA, 
				plant_dna_t *childDNA, 
				int seedNumber)
{
	// check for no parents
	if(parent1DNA==NULL || parent2DNA==NULL)
	{
		prepareRandomDNA(childDNA);
	}
	else
	{
		// check for elitism
		if((seedNumber==1 || seedNumber==2) && PLANT_GA_USE_ELITISM)
		{
			switch(seedNumber)
			{
			case 1:
				cloneDNA(parent1DNA, childDNA); // clone parent 1
				break;
			case 2:
				cloneDNA(parent2DNA, childDNA); // clone parent 2
				break;
			default:
				Com_Printf("PLANT:prepareDNA(): Inavlid seed number: [%d]\n",seedNumber);
				break;
			}
		}
		else
		{
			// check for crossover
			if(PLANT_GA_USE_CROSSOVER)
			{
				crossoverDNA(parent1DNA, parent2DNA, childDNA);
			}
			// perform direct copy of a random parent
			else
			{
				if(RANDOM <= 0.5)
				{
					cloneDNA(parent1DNA, childDNA); // clone parent 1
				}
				else
				{
					cloneDNA(parent2DNA, childDNA); // clone parent 2
				}
			}
		}

		// perform mutation on child
		if(PLANT_GA_USE_MUTATION)
		{
			mutateDNA(childDNA);
		}
	}
}



/**
 * populate the destination DNA with the source DNA
 */
void cloneDNA(plant_dna_t *sourceDNA, 
			  plant_dna_t *destDNA)
{	
	// poison data
	destDNA->poison1 = sourceDNA->poison1;
	destDNA->poison2 = sourceDNA->poison2;
	destDNA->poison3 = sourceDNA->poison3;

	// reproduction data
	destDNA->numSeeds           = sourceDNA->numSeeds;
	destDNA->reproduceFrequency = sourceDNA->reproduceFrequency;
	destDNA->seedStickiness     = sourceDNA->seedStickiness;
	destDNA->seedWeight         = sourceDNA->seedWeight;
}


/**
 * populate dna with random values
 */
void prepareRandomDNA(plant_dna_t *childDNA)
{
	// poison data
	childDNA->poison1 = RANDOM;
	childDNA->poison2 = RANDOM;
	childDNA->poison3 = RANDOM;

	// reproduction data
	childDNA->numSeeds           = getRandomInteger(PLANT_MIN_SEEDS,     PLANT_MAX_SEEDS,     FALSE);
	childDNA->reproduceFrequency = getRandomInteger(PLANT_MIN_REPRODUCE, PLANT_MAX_REPRODUCE, FALSE);
	childDNA->seedStickiness     = RANDOM;
	childDNA->seedWeight         = RANDOM;
}


/**
 * populate dna with random values
 */
void mutateDNA(plant_dna_t *childDNA)
{
	// poison data
	if(shouldMutate())
	{
		childDNA->poison1 = RANDOM;
	}
	if(shouldMutate())
	{
		childDNA->poison2 = RANDOM;
	}
	if(shouldMutate())
	{
		childDNA->poison3 = RANDOM;
	}
	if(shouldMutate())
	{
		childDNA->numSeeds = getRandomInteger(PLANT_MIN_SEEDS, PLANT_MAX_SEEDS, FALSE);
	}
	if(shouldMutate())
	{
		childDNA->reproduceFrequency = getRandomInteger(PLANT_MIN_REPRODUCE, PLANT_MAX_REPRODUCE, FALSE);
	}
	if(shouldMutate())
	{
		childDNA->seedStickiness = RANDOM;
	}
	if(shouldMutate())
	{
		childDNA->seedWeight = RANDOM;
	}
}

/**
 * whether or not a mutation action can occur
 */
qboolean shouldMutate()
{
	return (RANDOM <= PLANT_GA_MUTATION_PROBABILITY);
}


/**
 * perform single point crossover on parent dna to populate child dna
 */
void crossoverDNA(plant_dna_t *parent1DNA, 
				  plant_dna_t *parent2DNA, 
				  plant_dna_t *childDNA)
{
	// select one of 6 split points
	int splitPoint = getRandomInteger(1, 6, FALSE);

	switch(splitPoint)
	{
	case 1:
		childDNA->poison1            = parent1DNA->poison1; // parent 1
		childDNA->poison2			 = parent2DNA->poison1; // parent 2
		childDNA->poison3			 = parent2DNA->poison3;
		childDNA->numSeeds			 = parent2DNA->numSeeds;
		childDNA->reproduceFrequency = parent2DNA->reproduceFrequency;
		childDNA->seedStickiness	 = parent2DNA->seedStickiness;
		childDNA->seedWeight		 = parent2DNA->seedWeight;
		break;
	case 2:
		childDNA->poison1            = parent1DNA->poison1; // parent 1
		childDNA->poison2			 = parent1DNA->poison1; 
		childDNA->poison3			 = parent2DNA->poison3; // parent 2
		childDNA->numSeeds			 = parent2DNA->numSeeds;
		childDNA->reproduceFrequency = parent2DNA->reproduceFrequency;
		childDNA->seedStickiness	 = parent2DNA->seedStickiness;
		childDNA->seedWeight		 = parent2DNA->seedWeight;
		break;
	case 3:
		childDNA->poison1            = parent1DNA->poison1; // parent 1
		childDNA->poison2			 = parent1DNA->poison1; 
		childDNA->poison3			 = parent1DNA->poison3; 
		childDNA->numSeeds			 = parent2DNA->numSeeds; // parent 2
		childDNA->reproduceFrequency = parent2DNA->reproduceFrequency;
		childDNA->seedStickiness	 = parent2DNA->seedStickiness;
		childDNA->seedWeight		 = parent2DNA->seedWeight;
		break;
	case 4:
		childDNA->poison1            = parent1DNA->poison1; // parent 1
		childDNA->poison2			 = parent1DNA->poison1; 
		childDNA->poison3			 = parent1DNA->poison3; 
		childDNA->numSeeds			 = parent1DNA->numSeeds; 
		childDNA->reproduceFrequency = parent2DNA->reproduceFrequency; // parent 2
		childDNA->seedStickiness	 = parent2DNA->seedStickiness;
		childDNA->seedWeight		 = parent2DNA->seedWeight;
		break;
	case 5:
		childDNA->poison1            = parent1DNA->poison1; // parent 1
		childDNA->poison2			 = parent1DNA->poison1; 
		childDNA->poison3			 = parent1DNA->poison3; 
		childDNA->numSeeds			 = parent1DNA->numSeeds; 
		childDNA->reproduceFrequency = parent1DNA->reproduceFrequency; 
		childDNA->seedStickiness	 = parent2DNA->seedStickiness; // parent 2
		childDNA->seedWeight		 = parent2DNA->seedWeight;
		break;
	case 6:
		childDNA->poison1            = parent1DNA->poison1; // parent 1
		childDNA->poison2			 = parent1DNA->poison1; 
		childDNA->poison3			 = parent1DNA->poison3; 
		childDNA->numSeeds			 = parent1DNA->numSeeds; 
		childDNA->reproduceFrequency = parent1DNA->reproduceFrequency; 
		childDNA->seedStickiness	 = parent1DNA->seedStickiness; 
		childDNA->seedWeight		 = parent2DNA->seedWeight; // parent 2
		break;
	default:
		Com_Printf("PLANT : crossoverDNA() : Invalid split point [%d].\n", splitPoint);
		break;
	}
}

