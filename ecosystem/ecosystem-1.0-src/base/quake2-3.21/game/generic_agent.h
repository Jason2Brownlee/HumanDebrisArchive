
// generic_agent.h


/**
 * Description: Contains various generic agent definitions.
 *				This file should be included by all c files that make use of 
 *				agent code. This file should be included instead of q_local.h
 *
 * Author: Jason Brownlee 
 *
 * Change History
 * Who		When		Description
 * --------------------------------------------------------------------
 *
 *
 *
 */

#ifndef GENERIC_AGENT_H
#define GENERIC_AGENT_H

//
// includes
//
#include "generic_util.h"
#include "g_local.h"

//
// constants
//

// ini file
#define INI_FILENAME				"ecosystem/agents.ini"
#define INI_GENERIC					"Generic"
#define INI_PLANT					"Plant"
#define INI_HERBIVORE				"Herbivore"

// generic
#define ECOSYSTEM_AUTHOR			"Jason Brownlee"
#define ECOSYSTEM_CONTACT			"hop_cha@hotmail.com"
#define ONE_SECOND					+1

#define ALL_AGENT_TYPE				+0 
#define PLANT_AGENT_TYPE			+1 
#define HERBIVORE_AGENT_TYPE		+2

#define MAX_PLANT_POPULATION		50
#define MAX_HERBIVORE_POPULATION	15
#define MAX_AGENT_POPULATION		(MAX_PLANT_POPULATION + MAX_HERBIVORE_POPULATION)

#define AGENT_MODEL_SCALE			1.0
#define AGENT_MIN_HEALTH			15		// agent intelligence will try to ensure, that its actions do not push its health below this value 

// herbivore constants
#define HERBIVORE_PERCEPTRON_MAX_GOALS			+5	// max closest goals that can be evaluated by percepton
#define HERBIVORE_PERCEPTRON_MAX_PATTERNS		+6	// maximum number of training patterns that can be collected
#define HERBIVORE_PERCEPTRON_NUM_INPUTS			+3	// number of inputs to the perceptron (remember the +1 for the bias)
#define HERBIVORE_PERCEPTRON_RESULT_INDEX		+3	// index in input vectors of result (pos 3 (4th element))

// generic
extern int SHOW_AGENT_EFFECTS;
extern int STARTER_KIT_NUM_PLANTS;
extern int STARTER_KIT_NUM_HERBIVORES;

// plants
extern int PLANT_GA_USE_CROSSOVER;
extern int PLANT_GA_USE_MUTATION;
extern int PLANT_GA_USE_ELITISM;
extern float PLANT_GA_MUTATION_PROBABILITY;
extern int PLANT_SEED_MAX_LIFE_SECONDS;
extern int PLANT_SPROUT_MAX_LIFE_SECONDS;
extern int PLANT_ESTABLISHED_MAX_LIFE_SECONDS;
extern int PLANT_MIN_REPRODUCE_WAIT_SECONDS;
extern int PLANT_MAX_RESOURCE_SPROUT;
extern int PLANT_MAX_RESOURCE_ESTABLISHED;
extern int PLANT_SPROUT_REGENERATION_RATE;
extern int PLANT_ESTABLISHED_REGENERATION_RATE;
extern int PLANT_REPRODUCE_RESOURCE_HIT;
extern int PLANT_INITAL_RESOURCE;
extern float PLANT_STICKY_THRESHOLD;
extern int PLANT_MIN_SEEDS;
extern int PLANT_MAX_SEEDS;
extern int PLANT_MIN_REPRODUCE;
extern int PLANT_MAX_REPRODUCE;

// herbivore
extern int HERBIVORE_PERCEPTRON_ENABLED;
extern int HERBIVORE_PERCEPTRON_MAX_EPOCHS;
extern int HERBIVORE_PERCEPTRON_EARLY_STOP;
extern int HERBIVORE_PERCEPTRON_INIT_RANDOM;
extern int HERBIVORE_BIRTH_TIMEOUT_SECONDS;
extern int HERBIVORE_BIRTH_START_RESOURCE;
extern int HERBIVORE_CHILD_RESOURCE_CONSUME_RATE;
extern int HERBIVORE_CHILD_DEGEN_RATE;
extern int HERBIVORE_CHILD_MAX_RESOURCE;
extern float HERBIVORE_HUNGRY_UPPER_THRESHOLD;
extern float HERBIVORE_HUNGRY_LOWER_THRESHOLD;
extern int HERBIVORE_GOAL_TIMEOUT_SECONDS;
extern int HERBIVORE_NEW_GOAL_WAIT_SECONDS;
extern int HERBIVORE_ADULT_TIME_SECONDS_THRESHOLD;
extern int HERBIVORE_ADULT_MAX_RESOURCE;
extern int HERBIVORE_ADULT_MAX_LIFE_SECONDS;
extern int HERBIVORE_ADULT_RESOURCE_CONSUME_RATE;
extern int HERBIVORE_ADULT_DEGEN_RATE;
extern int HERBIVORE_REPRODUCE_MIN_WAIT_SECONDS;
extern int HERBIVORE_REPRODUCE_NUM_CHILDREN;
extern int HERBIVORE_REPRODUCE_RESOURCE_HIT;
extern int HERBIVORE_MAX_REPRODUCE;
extern float HERBIVORE_BITE_PROXIMITY;
extern float HERBIVORE_LAZINESS;
extern int HERBIVORE_SHOW_GOAL_SELECTION;
extern int HERBIVORE_MAX_BITES;

//
// type definitions
//

// plant states
typedef enum
{
	PLANT_SEED_STATE,
	PLANT_SPROUT_STATE,
	PLANT_ESTABLISHED_STATE,
	PLANT_DEATH_STATE
} plant_state_t;

// herbivore states
typedef enum
{
	HERBIVORE_BIRTH_STATE,
	HERBIVORE_CHILD_STATE,
	HERBIVORE_ADULT_STATE,
	HERBIVORE_DEATH_STATE
} herbivore_state_t;

typedef enum
{
	HERBIVORE_IDLE,
	HERBIVORE_FLEE,
	HERBIVORE_FEED
} herbivore_mode_t;


// easy way to refer to perceptron weights/inputs
typedef enum
{
	W_POISON1, 
	W_POISON2, 
	W_POISON3,
	W_BIAS	
} herbivore_weight_names;



// dna data for a plant
typedef struct plant_dna_s
{
	// poison data 
	float					poison1;
	float					poison2;
	float					poison3;

	// how sticky the plant seed is (1==very, 0==not sticky)
	float					seedStickiness;

	// the weight of the plant seed
	float					seedWeight;

	// the number of seeds created when the plant reproduces
	int						numSeeds;

	// the number of times the plant is allowed to reproduce over its life time
	int						reproduceFrequency;

} plant_dna_t;


// plant agent data structure
typedef struct plant_agent_s
{
	// the current state the plant is in
	plant_state_t			state;

	// last time some resource was regenerated
	long					lastRegen;

	// the plants dna information
	plant_dna_t				dna;

} plant_agent_t;


// perceptron result set
typedef struct herbivore_knowledge_s
{
	// stores all inputs for the perceptron 
	// the result is at position "HERBIVORE_PERCEPTRON_NUM_INPUTS"
	double     inputs[HERBIVORE_PERCEPTRON_NUM_INPUTS + 1];

	// the id of the plant that this knowledge is modeled on
	long		plantID;

} herbivore_knowledge_t;


// herbivore data structure
typedef struct herbivore_agent_s
{
	// the state the herbivore is in birth, child, adult, death
	herbivore_state_t			state;

	// the mode the herbivore is in idle, feed, flee
	herbivore_mode_t			mode;

	// last time the herbivore got a goal
	long						lastModeChange;

	// last time some resource was degenerated
	long						lastDegen;

	// the herbivores perceptron weights (used for food selection)
	// this is what is passed down from generation to generation - the core of the perceptron
	double						perceptron_weights[HERBIVORE_PERCEPTRON_NUM_INPUTS + 1];	
	
	// whether or not the herbivore had a parent or was spawned in my magic
	qboolean					hadParent;

	// the last few results from the food selection mechanism
	// all structures in thes array must be freed manually!!!
	herbivore_knowledge_t		*foodSelectionResultQueue[HERBIVORE_PERCEPTRON_MAX_PATTERNS];

	// the currently selected feed goal - not yet stored in memory
	herbivore_knowledge_t		currentFeedGoalKnowledge;

	// the number of bites taken from current food goal (plant)
	int							numPlantBites;

} herbivore_agent_t;



// generic agent data structure
typedef struct generic_agent_s
{
	// may be one of three types: PLANT_AGENT, HERBIVORE_AGENT
	int						agentType;

	// an agent will only have one of the following data structures depending on agent type
	plant_agent_t			*plantAgent;
	herbivore_agent_t		*herbivoreAgent;

	// the system datetime in millis when the agent was created - provides a unique id
	long					identity; 

	// the time in milliseconds when the agent was born
	long					birthDate;

	// the time in milliseconds when the agent last reproduced
	long					lastReproduceTime;

	// time of last state change
	long					lastStateChangeTime;

	// keep track of the number of times the agent has repoduced
	int						reproduceCount;

	// amount of resource awarded per server frame (dependent on state)
	int						regenerationRate; 
	int						degenerationRate;	// taken per server frame

	// function pointer to the agent specific find target function
	qboolean				(*find_target)(edict_t *self);

	// the agent can be provided with CPU cycles every server frame direclty if required
	// this is executed after the currentmove from  monster_think
	void					(*agent_server_frame)(edict_t *self);

	// run agent effects
	void					(*agent_effects)(edict_t *self);

	// whether or not the entity is carrying pollen
	qboolean		hasPollen;

	// a copy of the plants dna - the pollen data 
	plant_dna_t		pollen;

	// the unique plant id where this pollen came from
	long			pollenPlantID;
} generic_agent_t;


// function prototypes


generic_agent_t* castAgent(void *aStructure);
void allocateAgentMemory(edict_t *self, int type);
void reclaimAgentMemory(edict_t *self);

void writeMessage(char *message, int toDisplay);

void align_entity_to_world(edict_t *self, edict_t *other);
void alignEntityToPlane(edict_t *self, cplane_t *plane);

void UtilDrawLine(vec3_t p1, vec3_t p2);
float UtilDistanceBetweenPoints(vec3_t p1, vec3_t p2);
float UtilDistanceBetweenEntities(edict_t *p1, edict_t *p2);

long getTime(void);

void spawn_starter_kit(void);
void remove_all_agents(void);
int getSpawnPointList(edict_t *list[], int maxSpawnPoints);

// agent state
void generic_agent_effects(edict_t *self);
qboolean isAgentHealthy(edict_t *self);
qboolean isAgentUnhealthy(edict_t *self);
int getCountOfAgentType(int agentType);
qboolean withinPopulationQuota(int agentType);
int getAllAgentsOfType(edict_t *allAgents[], int agentType,int maximumResults);

#endif
