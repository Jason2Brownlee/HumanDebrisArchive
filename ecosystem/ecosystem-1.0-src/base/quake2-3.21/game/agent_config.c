
// agent_config.c


/**
 * Description: Agent config
 *
 * Author: Jason Brownlee 
 *
 * Change History
 * Who			When		Description
 * --------------------------------------------------------------------
 *
 */

#include "generic_agent.h"

// prototypes
int Ini_GetIntegerValue(char *section, char *entry);
float Ini_GetFloatValue(char *section, char *entry);
void initAllAgentConfig();


// generic
int SHOW_AGENT_EFFECTS;
int	STARTER_KIT_NUM_PLANTS;
int	STARTER_KIT_NUM_HERBIVORES;

// plants
int PLANT_GA_USE_CROSSOVER;
int PLANT_GA_USE_MUTATION;
int PLANT_GA_USE_ELITISM;
float PLANT_GA_MUTATION_PROBABILITY;
int PLANT_SEED_MAX_LIFE_SECONDS;
int PLANT_SPROUT_MAX_LIFE_SECONDS;
int PLANT_ESTABLISHED_MAX_LIFE_SECONDS;
int PLANT_MIN_REPRODUCE_WAIT_SECONDS;
int PLANT_MAX_RESOURCE_SPROUT;
int PLANT_MAX_RESOURCE_ESTABLISHED;
int PLANT_SPROUT_REGENERATION_RATE;
int PLANT_ESTABLISHED_REGENERATION_RATE;
int PLANT_REPRODUCE_RESOURCE_HIT;
int PLANT_INITAL_RESOURCE;
float PLANT_STICKY_THRESHOLD;
int PLANT_MIN_SEEDS;
int PLANT_MAX_SEEDS;
int PLANT_MIN_REPRODUCE;
int PLANT_MAX_REPRODUCE;

// herbivore
int HERBIVORE_PERCEPTRON_ENABLED;
int HERBIVORE_PERCEPTRON_MAX_EPOCHS;
int HERBIVORE_PERCEPTRON_EARLY_STOP;
int HERBIVORE_PERCEPTRON_INIT_RANDOM;
int HERBIVORE_BIRTH_TIMEOUT_SECONDS;
int HERBIVORE_BIRTH_START_RESOURCE;
int HERBIVORE_CHILD_RESOURCE_CONSUME_RATE;
int HERBIVORE_CHILD_DEGEN_RATE;
int HERBIVORE_CHILD_MAX_RESOURCE;
float HERBIVORE_HUNGRY_UPPER_THRESHOLD;
float HERBIVORE_HUNGRY_LOWER_THRESHOLD;
int HERBIVORE_GOAL_TIMEOUT_SECONDS;
int HERBIVORE_NEW_GOAL_WAIT_SECONDS;
int HERBIVORE_ADULT_TIME_SECONDS_THRESHOLD;
int HERBIVORE_ADULT_MAX_RESOURCE;
int HERBIVORE_ADULT_MAX_LIFE_SECONDS;
int HERBIVORE_ADULT_RESOURCE_CONSUME_RATE;
int HERBIVORE_ADULT_DEGEN_RATE;
int HERBIVORE_REPRODUCE_MIN_WAIT_SECONDS;
int HERBIVORE_REPRODUCE_NUM_CHILDREN;
int HERBIVORE_REPRODUCE_RESOURCE_HIT;
int HERBIVORE_MAX_REPRODUCE;
float HERBIVORE_BITE_PROXIMITY;
float HERBIVORE_LAZINESS;
int HERBIVORE_SHOW_GOAL_SELECTION;
int HERBIVORE_MAX_BITES;

/**
 * initalise all config values
 */
void initAllAgentConfig()
{
	// generic
	SHOW_AGENT_EFFECTS					= Ini_GetIntegerValue(INI_GENERIC, "SHOW_AGENT_EFFECTS");
	STARTER_KIT_NUM_PLANTS				= Ini_GetIntegerValue(INI_GENERIC, "STARTER_KIT_NUM_PLANTS");
	STARTER_KIT_NUM_HERBIVORES			= Ini_GetIntegerValue(INI_GENERIC, "STARTER_KIT_NUM_HERBIVORES");

	// plants
	PLANT_GA_USE_CROSSOVER				= Ini_GetIntegerValue(INI_PLANT, "PLANT_GA_USE_CROSSOVER");
	PLANT_GA_USE_MUTATION				= Ini_GetIntegerValue(INI_PLANT, "PLANT_GA_USE_MUTATION");
	PLANT_GA_USE_ELITISM				= Ini_GetIntegerValue(INI_PLANT, "PLANT_GA_USE_ELITISM");
	PLANT_GA_MUTATION_PROBABILITY		= Ini_GetFloatValue  (INI_PLANT, "PLANT_GA_MUTATION_PROBABILITY");
	PLANT_SEED_MAX_LIFE_SECONDS			= Ini_GetIntegerValue(INI_PLANT, "PLANT_SEED_MAX_LIFE_SECONDS");
	PLANT_SPROUT_MAX_LIFE_SECONDS		= Ini_GetIntegerValue(INI_PLANT, "PLANT_SPROUT_MAX_LIFE_SECONDS");
	PLANT_ESTABLISHED_MAX_LIFE_SECONDS	= Ini_GetIntegerValue(INI_PLANT, "PLANT_ESTABLISHED_MAX_LIFE_SECONDS");
	PLANT_MIN_REPRODUCE_WAIT_SECONDS	= Ini_GetIntegerValue(INI_PLANT, "PLANT_MIN_REPRODUCE_WAIT_SECONDS");
	PLANT_MAX_RESOURCE_SPROUT			= Ini_GetIntegerValue(INI_PLANT, "PLANT_MAX_RESOURCE_SPROUT");
	PLANT_MAX_RESOURCE_ESTABLISHED		= Ini_GetIntegerValue(INI_PLANT, "PLANT_MAX_RESOURCE_ESTABLISHED");
	PLANT_SPROUT_REGENERATION_RATE		= Ini_GetIntegerValue(INI_PLANT, "PLANT_SPROUT_REGENERATION_RATE");
	PLANT_ESTABLISHED_REGENERATION_RATE = Ini_GetIntegerValue(INI_PLANT, "PLANT_ESTABLISHED_REGENERATION_RATE");
	PLANT_REPRODUCE_RESOURCE_HIT		= Ini_GetIntegerValue(INI_PLANT, "PLANT_REPRODUCE_RESOURCE_HIT");
	PLANT_INITAL_RESOURCE				= Ini_GetIntegerValue(INI_PLANT, "PLANT_INITAL_RESOURCE");
	PLANT_STICKY_THRESHOLD				= Ini_GetFloatValue  (INI_PLANT, "PLANT_STICKY_THRESHOLD");
	PLANT_MIN_SEEDS						= Ini_GetIntegerValue(INI_PLANT, "PLANT_MIN_SEEDS");
	PLANT_MAX_SEEDS						= Ini_GetIntegerValue(INI_PLANT, "PLANT_MAX_SEEDS");
	PLANT_MIN_REPRODUCE					= Ini_GetIntegerValue(INI_PLANT, "PLANT_MIN_REPRODUCE");
	PLANT_MAX_REPRODUCE					= Ini_GetIntegerValue(INI_PLANT, "PLANT_MAX_REPRODUCE");

	// herbivore
	HERBIVORE_PERCEPTRON_ENABLED		= Ini_GetIntegerValue(INI_HERBIVORE, "HERBIVORE_PERCEPTRON_ENABLED");
	HERBIVORE_PERCEPTRON_MAX_EPOCHS		= Ini_GetIntegerValue(INI_HERBIVORE, "HERBIVORE_PERCEPTRON_MAX_EPOCHS");
	HERBIVORE_PERCEPTRON_EARLY_STOP		= Ini_GetIntegerValue(INI_HERBIVORE, "HERBIVORE_PERCEPTRON_EARLY_STOP");	
	HERBIVORE_PERCEPTRON_INIT_RANDOM	= Ini_GetIntegerValue(INI_HERBIVORE, "HERBIVORE_PERCEPTRON_INIT_RANDOM");	
	HERBIVORE_BIRTH_TIMEOUT_SECONDS		= Ini_GetIntegerValue(INI_HERBIVORE, "HERBIVORE_BIRTH_TIMEOUT_SECONDS");
	HERBIVORE_BIRTH_START_RESOURCE		= Ini_GetIntegerValue(INI_HERBIVORE, "HERBIVORE_BIRTH_START_RESOURCE");
	HERBIVORE_CHILD_RESOURCE_CONSUME_RATE = Ini_GetIntegerValue(INI_HERBIVORE, "HERBIVORE_CHILD_RESOURCE_CONSUME_RATE");
	HERBIVORE_CHILD_DEGEN_RATE			= Ini_GetIntegerValue(INI_HERBIVORE, "HERBIVORE_CHILD_DEGEN_RATE");
	HERBIVORE_CHILD_MAX_RESOURCE		= Ini_GetIntegerValue(INI_HERBIVORE, "HERBIVORE_CHILD_MAX_RESOURCE");
	HERBIVORE_HUNGRY_UPPER_THRESHOLD	= Ini_GetFloatValue  (INI_HERBIVORE, "HERBIVORE_HUNGRY_UPPER_THRESHOLD");
	HERBIVORE_HUNGRY_LOWER_THRESHOLD	= Ini_GetFloatValue  (INI_HERBIVORE, "HERBIVORE_HUNGRY_LOWER_THRESHOLD");
	HERBIVORE_GOAL_TIMEOUT_SECONDS		= Ini_GetIntegerValue(INI_HERBIVORE, "HERBIVORE_GOAL_TIMEOUT_SECONDS");
	HERBIVORE_NEW_GOAL_WAIT_SECONDS		= Ini_GetIntegerValue(INI_HERBIVORE, "HERBIVORE_NEW_GOAL_WAIT_SECONDS");
	HERBIVORE_ADULT_TIME_SECONDS_THRESHOLD = Ini_GetIntegerValue(INI_HERBIVORE, "HERBIVORE_ADULT_TIME_SECONDS_THRESHOLD");
	HERBIVORE_ADULT_MAX_RESOURCE		= Ini_GetIntegerValue(INI_HERBIVORE, "HERBIVORE_ADULT_MAX_RESOURCE");
	HERBIVORE_ADULT_MAX_LIFE_SECONDS	= Ini_GetIntegerValue(INI_HERBIVORE, "HERBIVORE_ADULT_MAX_LIFE_SECONDS");
	HERBIVORE_ADULT_RESOURCE_CONSUME_RATE = Ini_GetIntegerValue(INI_HERBIVORE, "HERBIVORE_ADULT_RESOURCE_CONSUME_RATE");
	HERBIVORE_ADULT_DEGEN_RATE			= Ini_GetIntegerValue(INI_HERBIVORE, "HERBIVORE_ADULT_DEGEN_RATE");
	HERBIVORE_REPRODUCE_MIN_WAIT_SECONDS = Ini_GetIntegerValue(INI_HERBIVORE, "HERBIVORE_REPRODUCE_MIN_WAIT_SECONDS");
	HERBIVORE_REPRODUCE_NUM_CHILDREN	= Ini_GetIntegerValue(INI_HERBIVORE, "HERBIVORE_REPRODUCE_NUM_CHILDREN");
	HERBIVORE_REPRODUCE_RESOURCE_HIT	= Ini_GetIntegerValue(INI_HERBIVORE, "HERBIVORE_REPRODUCE_RESOURCE_HIT");
	HERBIVORE_MAX_REPRODUCE				= Ini_GetIntegerValue(INI_HERBIVORE, "HERBIVORE_MAX_REPRODUCE");
	HERBIVORE_BITE_PROXIMITY			= Ini_GetFloatValue  (INI_HERBIVORE, "HERBIVORE_BITE_PROXIMITY");
	HERBIVORE_LAZINESS					= Ini_GetFloatValue  (INI_HERBIVORE, "HERBIVORE_LAZINESS");
	HERBIVORE_SHOW_GOAL_SELECTION		= Ini_GetIntegerValue(INI_HERBIVORE, "HERBIVORE_SHOW_GOAL_SELECTION");
	HERBIVORE_MAX_BITES					= Ini_GetIntegerValue(INI_HERBIVORE, "HERBIVORE_MAX_BITES");
}






/**
 * get a floating point value from the ini file
 */
float Ini_GetFloatValue(char *section, char *entry)
{
	char *value = NULL;

	// get the value
	value = Ini_GetValue(&ini_file, section, entry);

	if(!value)
	{
		gi.error("Ini_GetFloatValue(): Failed to float value from ini file [%s] section[%s], entry[%s]", 
			INI_FILENAME, 
			section,
			entry);
	}

	return (float) atof(value);
}


/**
 * get a integer value from the ini file
 */
int Ini_GetIntegerValue(char *section, char *entry)
{
	char *value = NULL;

	// get the value
	value = Ini_GetValue(&ini_file, section, entry);

	if(!value)
	{
		gi.error("Ini_GetIntegerValue(): Failed to integer value from ini file [%s] section[%s], entry[%s]", 
			INI_FILENAME, 
			section,
			entry);
	}

	return atoi(value);
}




