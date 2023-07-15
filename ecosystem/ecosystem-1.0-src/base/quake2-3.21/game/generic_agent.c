
// generic_agent.c


/**
 * Description: Contains various generic agent functions.
 *
 * Author: Jason Brownlee
 *
 * Change History
 * Who			When		Description
 * --------------------------------------------------------------------
 * 
 *
 *
 */


#include "generic_agent.h"
#include <math.h>
#include <time.h>


// external function prototypes
void Killed (edict_t *targ, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

// external prototypes - agents
void spawn_plant(int delaySeconds, vec3_t spawn_origin, vec3_t spawn_angles);
void spawn_herbivore(int delaySeconds, vec3_t spawn_origin, vec3_t spawn_angles);
qboolean isPlantPoisonous(edict_t *plant);
void initAllAgentConfig();


// project globals
IniFile ini_file = {0,0,0,0,0,NULL,NULL,NULL,NULL};


// file globals

// count total agents created - used for unique agent id's
static int agent_creation_count = 1;


/**
 * get time function called by all agents
 */
long getTime(void)
{
	return level.time;	// time in the game
}


/**
 * Determine the distnace between two origins
 */
float UtilDistanceBetweenPoints(vec3_t p1, 
								vec3_t p2)
{
	float returnValue = 0.0;
	vec3_t v;

	VectorSubtract(p1, p2, v); // non-destructive
	returnValue = VectorLength(v); // returns sqrt of the sum of the squares (+)

	return returnValue; 
}

/**
 * distance between two entities
 */
float UtilDistanceBetweenEntities(edict_t *p1, 
								  edict_t *p2)
{
	if(!p1->s.origin || !p2->s.origin)
	{
		Com_Printf("UtilDistanceBetweenEntities - ERROR: something has no origin: P1[%d], P2[%d]\n", p1->s.origin, p2->s.origin);
		return 999999;
	}

	return UtilDistanceBetweenPoints(p1->s.origin, p2->s.origin);
}


/**
 * draw a line between two origins
 */
void UtilDrawLine(vec3_t p1, 
				  vec3_t p2)
{
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_BFG_LASER);
	gi.WritePosition(p1);
	gi.WritePosition(p2);
	gi.multicast(p1, MULTICAST_PHS);
}



/**
 * get a listing of all spawn points in the world
 */
int getSpawnPointList(edict_t *list[], int maxSpawnPoints)
{	
	edict_t	*current = NULL;
	int count = 0;

	// get all player starts
	while((current = G_Find(current, FOFS(classname), "info_player_start")) != NULL)
	{
		list[count++] = current;

		if(count >= maxSpawnPoints)
		{
			break;
		}
	}

	return ((count<maxSpawnPoints) ? count : maxSpawnPoints);
}


/**
 * Spawn a starter kit 
 *
 * This function is designed to kick start a simulation by introducing 
 * a collection of all three species. Plants will be introduced first, then
 * herbivores. 
 *
 */
void spawn_starter_kit()
{
	edict_t *spawnList[10];
	vec3_t spawn_origin, spawn_angles;
	int i;
	int numSpawnPoints = 0;
	int currentPos = 0;
	int delay = 1; // dealy between spawns

	// get a listing of all spawn points in the world (cut off at 10)
	numSpawnPoints = getSpawnPointList(spawnList, 10);

	// work on a rotor system through the list

	// plants
	for(i=0; i<STARTER_KIT_NUM_PLANTS; i++)
	{
		VectorCopy(spawnList[currentPos]->s.origin, spawn_origin);
		spawn_origin[2] += 9;
		VectorCopy(spawnList[currentPos]->s.angles, spawn_angles);

		spawn_plant(i*delay, spawn_origin, spawn_angles); // delayed release

		if(++currentPos >= numSpawnPoints)
		{
			currentPos = 0; // cycle back around
		}
	}

	// herbivores
	for(i=0; i<STARTER_KIT_NUM_HERBIVORES; i++)
	{
		VectorCopy(spawnList[currentPos]->s.origin, spawn_origin);
		spawn_origin[2] += 9;
		VectorCopy(spawnList[currentPos]->s.angles, spawn_angles);

		spawn_herbivore(((STARTER_KIT_NUM_PLANTS*delay) + 5) + (i*delay), spawn_origin, spawn_angles); // delayed release

		if(++currentPos >= numSpawnPoints)
		{
			currentPos = 0; // cycle back around
		}
	}
}

/**
 * Remove all agents from the environment
 *
 */
void remove_all_agents()
{
	int			i;
	edict_t		*current = NULL;
	int			agentCount = 0;

	// enumerate all entities in the world
	// if the entity is an agent, kill them, otherwise 
	// if the entity has pollen, remove it

	// start from 1, because 0 is the world (level)
	for(i=1; i<globals.num_edicts; i++)
	{
		current = &g_edicts[i];
		
		if(current->inuse && current->health>0)
		{
			if(current->agent)
			{
				// kill the agent
				Killed(current, current, current, -999, current->s.origin);
				agentCount++;
			}
		}
	}

	// print a little stock take
	Com_Printf("Removed a total of %d agents.\n", agentCount, globals.num_edicts);
}


/**
 * determine if an agent type is within its quota
 */
qboolean withinPopulationQuota(int agentType)
{
	int totalAgents = getCountOfAgentType(agentType);

	switch(agentType)
	{
	case PLANT_AGENT_TYPE:
		if(totalAgents >= MAX_PLANT_POPULATION)
		{
			return FALSE;
		}
		break;

	case HERBIVORE_AGENT_TYPE:
		if(totalAgents >= MAX_HERBIVORE_POPULATION)
		{
			return FALSE;
		}
		break;

	default: // ALL_AGENT_TYPE
		if(totalAgents >= MAX_AGENT_POPULATION)
		{
			return FALSE;
		}
		break;
	}

	return TRUE; // NOT over quota
}

/**
 * determine the population size of an agent 
 * 0 == ALL AGENTS
 * 1 == PLANT
 * 2 == HERBIVORE
 */
int getCountOfAgentType(int agentType)
{
	int			i;
	edict_t		*current = NULL;
	int			agentCount = 0;

	// JB [17-04-2003]
	// FIXME: need to account for pending (dely spawn)
	//  -- agent memory structure is not alloated until entry to world

	// start from 1, because 0 is the world (level)
	for(i=1; i<globals.num_edicts; i++)
	{
		current = &g_edicts[i];		
		
		if(current->inuse && 
		   current->deadflag != DEAD_DEAD && 
		   current->health && 
		   current->agent)
		{
			// count all agents
			if(agentType == ALL_AGENT_TYPE)
			{
				agentCount++;
			}
			// count specific agent type
			else if(castAgent(current->agent)->agentType == agentType)
			{
				agentCount++;
			}
		}
	}

	//Com_Printf("getCountOfAgentType:: located %d agents of type: %d.\n", agentCount, agentType);

	return agentCount;
}



/**
 * populate the provided array with a listing of all agents that meet the type criterion
 * that are in the game world
 * ENSURE: that you have enough space in the provided structure to store all possible entities
 *
 * returns the total number of agents in the collection
 */
int getAllAgentsOfType(edict_t *allAgents[], int agentType, int maximumResults)
{
	edict_t		*current = NULL;	
	int			agentCount = 0;
	int			i;

	// JB [17-04-2003]
	// FIXME: need to account for pending (dely spawn)
	//  -- agent memory structure is not alloated until entry to world

	// start from 1, because 0 is the world (level)	
	for(i=1; i<globals.num_edicts && agentCount<maximumResults; i++) // enforce maximum results to be safe
	{
		current = &(g_edicts[i]);
		
		// ensure inuse, alive and an agent
		if(current->inuse && 
		   current->deadflag != DEAD_DEAD && 
		   current->health && 
		   current->agent)
		{
			// collect all agents
			if(agentType == ALL_AGENT_TYPE)
			{
				allAgents[agentCount++] = current;
			}
			// collect specific agent type
			else if(castAgent(current->agent)->agentType == agentType)
			{
				allAgents[agentCount++] = current;
			}
		}
	}

	//Com_Printf("getAllAgentsOfType:: located %d agents of type: %d.\n", agentCount, agentType);

	return agentCount; // last pos is at (agentCount-1)
}



/**
 * Utility function to cast the void pointer in the edict_t structure
 * to a useful type.
 *
 */
generic_agent_t* castAgent(void *aStructure)
{
	return (generic_agent_t*) aStructure;
}

/**
 * Utility function to print debug messages to the console
 *
 */
void writeMessage(char *message, 
				  int toDisplay)
{
	if(toDisplay == TRUE)
	{
		gi.bprintf (PRINT_HIGH, message);
	}
}


/**
 * Utility function to align an entity to the provided plane
 *
 */
void alignEntityToPlane(edict_t *self, 
						cplane_t *plane)
{
	vec3_t		normal_angles;
	vec3_t		forward, right, up;

	if(plane)
	{				
		vectoangles(plane->normal, normal_angles);		
		AngleVectors(normal_angles, forward, right, up);

		vectoangles(right, self->s.angles);
		//vectoangles(up, self->s.angles);
		//vectoangles(forward, self->s.angles);
	}
}

/**
 * Utility function to align an agent to the world
 *
 */
void align_entity_to_world(	edict_t *self, 
							edict_t *other)
{
	vec3_t		forward, right, up;

	AngleVectors(other->s.angles, forward, right, up);

	vectoangles(right, self->s.angles);
	//vectoangles(up, self->s.angles);
	//vectoangles(forward, self->s.angles);

}


/**
 * Utility function to allocate memory depending on the type of agent
 *
 * @param edict_t :
 * @param agent_type_t : 
 */
void allocateAgentMemory(edict_t *self, 
						 int type)
{
	generic_agent_t *aAgent = NULL;
	long			birthDate = getTime();
	//int				i;

	aAgent = (generic_agent_t *) malloc(sizeof(generic_agent_t));

	if(aAgent == NULL)
	{
		// error
		gi.error("allocateAgentMemory: Failed to allocate memory for generic_agent_t.");
		return;
	}

	// store in enitity structure
	self->agent = (generic_agent_t *) aAgent;

	switch(type)
	{
	case PLANT_AGENT_TYPE:		
		aAgent->herbivoreAgent	= NULL;
		aAgent->plantAgent		= (plant_agent_t *) malloc(sizeof(plant_agent_t));

		if(aAgent->plantAgent == NULL)
		{
			// error
			gi.error("allocateAgentMemory: Failed to allocate memory for plant_agent_t.");
			return;
		}		

		// initialise values		
		aAgent->plantAgent->dna.numSeeds = 0;
		aAgent->plantAgent->dna.poison1 = 0.0;
		aAgent->plantAgent->dna.poison2 = 0.0;
		aAgent->plantAgent->dna.poison3 = 0.0;
		aAgent->plantAgent->dna.reproduceFrequency = 0;
		aAgent->plantAgent->dna.seedStickiness = 0.0;
		aAgent->plantAgent->dna.seedWeight = 0.0;
		break;

	case HERBIVORE_AGENT_TYPE:
		aAgent->plantAgent		= NULL;
		aAgent->herbivoreAgent	= (herbivore_agent_t *) malloc(sizeof(herbivore_agent_t));

		if(aAgent->herbivoreAgent == NULL)
		{
			// error
			gi.error("allocateAgentMemory: Failed to allocate memory for herbivore_agent_t.");
			return;
		}

		// initialise values
		aAgent->herbivoreAgent->state						= 0;
		aAgent->herbivoreAgent->mode						= 0;
		aAgent->herbivoreAgent->lastModeChange				= 0;
		aAgent->herbivoreAgent->lastDegen					= 0;		
		
		// this array must have all entries feed when agent dies
		init_pointer_array(aAgent->herbivoreAgent->foodSelectionResultQueue, HERBIVORE_PERCEPTRON_MAX_PATTERNS);

		break;

	default:
		gi.error("allocateAgentMemory: Invalid agent type specified %d.", type);
		break;
	}

	//
	// Generic agent initialization
	//
	aAgent->agentType			= type; // remember the agent type

	// HEY - set these times when the agent *really* spawns into the world (especially if its a delayed spawn)
	aAgent->birthDate			= birthDate; // set birth day/time
	aAgent->lastStateChangeTime = birthDate; // time of last state change
	
	aAgent->lastReproduceTime	= 0;
	aAgent->reproduceCount		= 0;
	aAgent->degenerationRate	= 0;
	aAgent->regenerationRate	= 0;
	aAgent->find_target			= NULL; // not implemented by default
	aAgent->agent_server_frame	= NULL; // not implemented by default
	aAgent->agent_effects		= generic_agent_effects; // all agents run generic effects

	aAgent->identity			= agent_creation_count++; // unique id

	// no pollen by default
	aAgent->hasPollen = false;
	aAgent->pollenPlantID = -1;

	aAgent->pollen.numSeeds = 0;
	aAgent->pollen.poison1 = 0.0;
	aAgent->pollen.poison2 = 0.0;
	aAgent->pollen.poison3 = 0.0;
	aAgent->pollen.reproduceFrequency = 0;
	aAgent->pollen.seedStickiness = 0.0;
	aAgent->pollen.seedWeight = 0.0;
	
}



/**
 * Reclaim memory provided to agent when it was initially created
 *
 * @param edict_t : 
 */
void reclaimAgentMemory(edict_t *self)
{
	generic_agent_t *aAgent = NULL;
	int i;

	if(self->agent == NULL)
	{
		writeMessage("reclaimAgentMemory(): Provided entity has no agent structure to reclaim.\n", TRUE);
	}

	aAgent = castAgent(self->agent);

	switch(aAgent->agentType)
	{
	case PLANT_AGENT_TYPE:
		free(aAgent->plantAgent);
		aAgent->plantAgent = NULL;
		break;

	case HERBIVORE_AGENT_TYPE:
		// free all remembered samples
		for(i=0; i<HERBIVORE_PERCEPTRON_MAX_PATTERNS; i++)
		{
			if(aAgent->herbivoreAgent->foodSelectionResultQueue[i] == NULL)
			{
				break; // no more samples
			}

			free(aAgent->herbivoreAgent->foodSelectionResultQueue[i]); //free memory
			aAgent->herbivoreAgent->foodSelectionResultQueue[i] = NULL;
		}

		free(aAgent->herbivoreAgent);
		aAgent->herbivoreAgent = NULL;
		break;	

	default:
		gi.error("allocateAgentMemory: Invalid agent type specified %d.", aAgent->agentType);
		break;
	}

	// free the actual agent structure
	free(self->agent);
	self->agent = NULL;
}


/**
 * determine if an agent is healthy
 */
qboolean isAgentHealthy(edict_t *self)
{
	// determine current agent health percentage
	float healthPercentage = ((float)(self->health) /(float)(self->max_health));

	if(healthPercentage >= 0.85)
	{
		return true;
	}

	return false;
}

/**
 * determine if an agent is unhealthy
 */
qboolean isAgentUnhealthy(edict_t *self)
{
	// determine current agent health percentage
	float healthPercentage = ((float)(self->health) /(float)(self->max_health));

	if(healthPercentage <= 0.15)
	{
		return true;
	}

	return false;
}


/**
 * show an agent effect
 */
void generic_agent_effects(edict_t *self)
{
	generic_agent_t *aAgent = castAgent(self->agent);

	if(SHOW_AGENT_EFFECTS)
	{
		// plant
		if(aAgent->plantAgent)
		{
			// poisonous
			if(isPlantPoisonous(self))
			{				
				self->s.effects |= EF_COLOR_SHELL;
				self->s.renderfx = RF_SHELL_BLUE; // only blue				
			}

			// plants can only show one effect
			return;
		}

		// very health
		if(isAgentHealthy(self))
		{
			// show green effect
			self->s.effects |= EF_COLOR_SHELL;
			self->s.renderfx |= RF_SHELL_GREEN;
			return;
		}

		// very sick
		if(isAgentUnhealthy(self))
		{
			// show red effect
			self->s.effects |= EF_COLOR_SHELL;
			self->s.renderfx |= RF_SHELL_RED;
			return;
		}		
	}
}



// below here are some wrapper ini file functions


/**
 * called when the dll is loaded
 */
void agentGameInit()
{
	int i;
	
	i = Ini_ReadIniFile(INI_FILENAME, &ini_file);

	if(!i)
	{
		gi.error("agentGameInit(): Failed to prepare ini file [%s] error: %d.", INI_FILENAME, i);
	}
	
	Com_Printf("\n-------- Agent Initialisation -------\n");
	Com_Printf("Ecosystem Mod : \n%s - %s.\n", ECOSYSTEM_AUTHOR, ECOSYSTEM_CONTACT);	
	Com_Printf("...%s\n", INI_FILENAME);
	Com_Printf("Agent Configuration loaded successfully.\n");
	Com_Printf("--------------------------\n\n");
	
	initAllAgentConfig();

	// seed random number generator
	srand(time(NULL));
}

/**
 * called when the game is shut down
 */
void agentGameShutdown()
{
	// free up memory associated with the ini file
	Ini_FreeIniFile(&ini_file);
}



