
// plant_agent.c


/**
 * Description: The plant agent
 *
 * Author: Jason Brownlee 
 *
 * Change History
 * Who			When		Description
 * --------------------------------------------------------------------
 *
 */



#include "generic_agent.h"

#define DISPLAY_PLANT_FUNCTION_DEBUG		FALSE

#define PLANT_SEED_MODEL					"models/objects/grenade2/tris.md2"
#define PLANT_SPROUT_MODEL					"models/items/ammo/grenades/medium/tris.md2"
#define PLANT_ESTABLISHED_MODEL				"models/items/ammo/rockets/medium/tris.md2"


// external function prototypes
void Killed (edict_t *targ, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);


//
// Hooks into GA functionality
//
void prepareDNA(plant_dna_t *parent1DNA, plant_dna_t *parent2DNA, plant_dna_t *childDNA, int seedNumber);
void cloneDNA(plant_dna_t *sourceDNA, plant_dna_t *destDNA);


//
// plant function prototypes
//
void spawn_plant(int delaySeconds, vec3_t spawn_origin, vec3_t spawn_angles);
void spawn_plant_start(edict_t *self);
void spawn_plant_reproduce(edict_t *parent1Entity,plant_dna_t *parent1DNA,plant_dna_t *parent2DNA,int seedNumber);

void plant_flymove(edict_t *self);
void plant_flymove_timeoutcheck(edict_t *self);
void plant_generic_think(edict_t *self);
void plant_maturity_check(edict_t *self);

void plant_death_wrapper(edict_t *self);
void plant_dead(edict_t *self);

void plant_set_sprout_state(edict_t *self);
void plant_set_established_state(edict_t *self);
void plant_reproduce(edict_t *self, edict_t *other);
void plant_attach_pollen(edict_t *self, edict_t *other);

qboolean isPlantPoisonous(edict_t *plant);
qboolean isPoisonGeneActive(float gene);

// monster framework functions
void plant_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void plant_pain(edict_t *self, edict_t *other, float kick, int damage);
void plant_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void plant_stand(edict_t *self);
void plant_walk(edict_t *self);
void plant_run(edict_t *self);








//
// structure definitions for thinking and animation/AI patterns
//

// plant animation/AI cycle in seed state
mframe_t plant_move_seed_frames [] = 
{ 
	NULL, 0, plant_flymove, 	// use up 10 server frames, timeout check every second
	NULL, 0, plant_flymove,
	NULL, 0, plant_flymove,
	NULL, 0, plant_flymove,
	NULL, 0, plant_flymove,
	NULL, 0, plant_flymove,
	NULL, 0, plant_flymove,
	NULL, 0, plant_flymove,
	NULL, 0, plant_flymove
};
mmove_t plant_seed_move = {0, 0, plant_move_seed_frames, plant_flymove_timeoutcheck};


// generic plant animation/AI cycle
mframe_t plant_generic_frames [] = 
{ 
	NULL, 0, NULL, // number of server frames to use up, 10 == 1 second
	NULL, 0, NULL,
	NULL, 0, NULL,
	NULL, 0, NULL,
	NULL, 0, NULL,
	NULL, 0, NULL,
	NULL, 0, NULL,
	NULL, 0, NULL,
	NULL, 0, NULL
};
mmove_t plant_generic_think_move = {0, 0, plant_generic_frames, plant_maturity_check};


// plant death animation/AI cycle
mframe_t plant_death_frames [] = 
{ 
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t plant_death_think_move = {0, 0, plant_death_frames, plant_dead};




/**
 * Spawn function
 * Spawn a new plant agent into the environment
 *
 */
void spawn_plant(int delaySeconds, vec3_t spawn_origin, vec3_t spawn_angles)
{
	edict_t	*self = NULL;
	generic_agent_t	*seed = NULL;

	// never spawn in deathmatch game
	if(deathmatch->value)
	{
		return;
	}

	// check if can spawn
	if(!withinPopulationQuota(PLANT_AGENT_TYPE))
	{
		writeMessage("PLANT::Could not spawn - over population quota.\n", DISPLAY_PLANT_FUNCTION_DEBUG);
		return;
	}

	self = G_Spawn();
	
	VectorCopy(spawn_origin, self->s.origin); // inform the entity of the spawn location
	self->s.origin[2] += 1; // ensure the agent is spawned above the ground

	allocateAgentMemory(self, PLANT_AGENT_TYPE); // allocate memory and initialise agent structure
	seed = castAgent(self->agent);
	seed->plantAgent->state	= PLANT_SEED_STATE; // seed state

	// prepare dna - initialise to random
	prepareDNA(NULL, NULL, &seed->plantAgent->dna, 0);

	// have to wait a while before spawning
	self->think = spawn_plant_start; // initialize to seed state
	self->nextthink = level.time + delaySeconds; // think soon
}


/**
 * Create a new plant agent from two parent plants
 * Provided with dna to both parents
 * 
 */
void spawn_plant_reproduce(edict_t *parent1Entity,
						   plant_dna_t *parent1DNA, 
						   plant_dna_t *parent2DNA,
						   int seedNumber)
{
	edict_t				*self = NULL;
	generic_agent_t		*seed = NULL;	

	// check if can spawn
	if(!withinPopulationQuota(PLANT_AGENT_TYPE))
	{
		writeMessage("PLANT::Could not spawn - over population quota.\n", DISPLAY_PLANT_FUNCTION_DEBUG);
		return;
	}

	self = G_Spawn();

	// enter just above the parent plant 
	VectorCopy(parent1Entity->s.origin, self->s.origin);	// copy parents position
	self->s.origin[2] += 16; // ensure off the ground

	allocateAgentMemory(self, PLANT_AGENT_TYPE); // allocate memory and initialise agent structure
	seed = castAgent(self->agent);
	seed->plantAgent->state	= PLANT_SEED_STATE; // seed state

	// prepare dna
	prepareDNA(parent1DNA, parent2DNA, &seed->plantAgent->dna, seedNumber);

	// have to wait a while before spawning
	self->think = spawn_plant_start; // initialize to seed state
	self->nextthink = level.time + seedNumber; // seed number dictates the number of seconds to wait
}


/**
 * Initialise the plant into its seed state
 *
 * @param edict_t : plant entity instance
 */
void spawn_plant_start(edict_t *self)
{
	generic_agent_t		*aAgent = NULL;
	long				theTime = getTime();

	VectorSet(self->mins, -15, -15, -15);// plants have dimension, a cube
	VectorSet(self->maxs, 15, 15, 15);	

	self->s.modelindex		= gi.modelindex(PLANT_SEED_MODEL);	
	self->monsterinfo.scale = AGENT_MODEL_SCALE;
	self->s.skinnum			= 0;
	self->solid				= SOLID_BBOX;

	self->pain	= plant_pain; // generic sensors
	self->die	= plant_die;
	self->touch = plant_touch;

	self->monsterinfo.stand = plant_stand; // monster specifc think functions
	self->monsterinfo.walk	= plant_walk;
	self->monsterinfo.run	= plant_run;

	gi.linkentity(self); // link into the environment
	walkmonster_start(self); // initialize as a monster	

	aAgent = castAgent(self->agent);

	aAgent->birthDate				= theTime; // set birth day/time
	aAgent->lastStateChangeTime		= theTime;
	aAgent->plantAgent->lastRegen	= theTime;

	aAgent->agent_server_frame		= plant_generic_think;

	self->health		= PLANT_INITAL_RESOURCE;
	self->max_health	= PLANT_MAX_RESOURCE_SPROUT;
	self->gib_health	= -10;
	self->mass			= 10;
	self->gravity		= aAgent->plantAgent->dna.seedWeight;
	self->movetype		= MOVETYPE_BOUNCE;
	
	self->monsterinfo.currentmove = &plant_seed_move; // initialise current think/animation

	//
	// Send the seed off with a velocity and some crazy up angle, like a gib or grenade
	//	
	self->velocity[0] = 100.0 * crandom();
	self->velocity[1] = 100.0 * crandom();
	self->velocity[2] = 300;
}



/**
 * Function called each frame the plant seed is flying
 *
 * @param edict_t : plant entity instance
 */
void plant_flymove(edict_t *self)
{
	// perform any actions required when the plant seed is floating back to earth
	//writeMessage("Plant entity is flying...\n", DISPLAY_PLANT_FUNCTION_DEBUG);
}


/**
 * Function called each frame the plant seed is flying
 *
 * @param edict_t : plant entity instance
 */
void plant_flymove_timeoutcheck(edict_t *self)
{
	long currentTime = getTime();
	generic_agent_t	*aAgent = castAgent(self->agent);

	// check if the seed has not moved since the last time it's timeout was checked	
	if(self->groundentity)
	{
		//writeMessage("--> Not sticky, and stopped moving.\n", DISPLAY_PLANT_FUNCTION_DEBUG);

		// align the entity to the world
		align_entity_to_world(self, self->groundentity);
		
		// the seed has not moved, therefore it has stopped bouncing and it is ready to start growing		
		plant_set_sprout_state(self);
	}		
	// check if have been a seed for too long
	else if(currentTime > (aAgent->birthDate + PLANT_SEED_MAX_LIFE_SECONDS) )
	{
		// have been a seed for ages, remove self
		//writeMessage("--> Been a seed for too long, time to die.\n", DISPLAY_PLANT_FUNCTION_DEBUG);
		plant_death_wrapper(self);
	}
}

/**
 * Called when the plant entity tocuhes something
 *
 * @param edict_t : plant entity instance
 * @param edict_t : what was touched
 * @param cplane_t : the plane the thing was touched on (?)
 * @param csurface_t : the surface touched (?)
 *
 */
void plant_touch(edict_t *self, 
				 edict_t *other, 
				 cplane_t *plane, 
				 csurface_t *surf)
{
	generic_agent_t	*aAgent = castAgent(self->agent);

	// a touch in different states will do different things
	switch(aAgent->plantAgent->state)
	{
		case PLANT_SEED_STATE:
			if((other->svflags & SVF_MONSTER) || other->takedamage || other->client)
			{
				//writeMessage("--> I'm a seed and I hit another entity or monster, time to die.\n", DISPLAY_PLANT_FUNCTION_DEBUG);
				plant_death_wrapper(self); 
			}
			else if(surf && (/*(surf->flags & SURF_SKY) || */(surf->flags & SURF_NODRAW)) )
			{
				//writeMessage("--> I'm a seed and I hit a bad surface, time to die.\n", DISPLAY_PLANT_FUNCTION_DEBUG);
				plant_death_wrapper(self);		
			}			
			// check if the seed should stick, if not - bounce
			else if(aAgent->plantAgent->dna.seedStickiness > PLANT_STICKY_THRESHOLD)
			{				
				//writeMessage("I'm a seed and I'm sticky - I'm stuck to this surface.\n", DISPLAY_PLANT_FUNCTION_DEBUG);				
				alignEntityToPlane(self, plane); // align with the plane seed is sticking to
				plant_set_sprout_state(self); // sprout time
			}
			break;

		case PLANT_SPROUT_STATE:
			if(other && other->agent)
			{
				plant_attach_pollen(self, other); // attempt to attach pollen to the other agent
			}
			break;

		case PLANT_ESTABLISHED_STATE:
			if(other && other->agent)
			{
				if(castAgent(other->agent)->hasPollen)
				{
					plant_reproduce(self, other);	// attempt to reproduce
				}
				plant_attach_pollen(self, other); // attempt to attach pollen to the other agent
			}			
			break;

		case PLANT_DEATH_STATE:
			// do nothing, should not occur
			break;

		default: // unknown state, error
			break;
	}
}



/**
 * wrapper function for plant death
 *
 */ 
void plant_death_wrapper(edict_t *self)
{
	//writeMessage("Running plant death wrapper\n", DISPLAY_PLANT_FUNCTION_DEBUG);
	//self->health = -999; // no more health
	Killed(self, self, self, -999, self->s.origin);
}


/**
 * Function called when the plant dies, will remove the plant from the world
 * Based on g_misc.c ==> ThrowHead() function
 *
 * @param edict_t : plant entity instance
 * @param edict_t :
 * @param edict_t :
 * @param int : 
 * @param vec3_t : 
 * 
 */
void plant_die(edict_t *self, 
			   edict_t *inflictor, 
			   edict_t *attacker, 
			   int damage, 
			   vec3_t point)
{
	generic_agent_t	*aAgent = NULL;

	if(self->deadflag == DEAD_DEAD)
	{
		return; // already dead
	}

	aAgent = castAgent(self->agent);

	//
	// Dump debug on death
	//
	//print_plant_dna(self);

	// dont change mode, let the plant die in whatever model it was in
	//gi.setmodel(self, PLANT_DEATH_MODEL); // model change	

	// throw gibs	
	ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", 200, GIB_ORGANIC);

	self->deadflag = DEAD_DEAD; // agent is dead to the rest of the world

	//
	// Here is a gotcha - The agent needs to do some non-agent thinking whilst
	// getting ready to die. If this is not done, the game will crash due to the 
	// fact that the agent's think cycle may get some CPU cycles, and if it does
	// there will be no agent structure to run because the memory has been freed.
	// The fix was either run some dumb think cycle, or change all the existing
	// cycles. The approach taken is similar to a monster death where its body sticks
	// around, but here, the plant agent will disappear ASAP.
	//
	self->monsterinfo.currentmove = &plant_death_think_move;	

	aAgent->plantAgent->state = PLANT_DEATH_STATE; // plant dead state
	reclaimAgentMemory(self); // reclaim allocated system memory	

	self->takedamage	= DAMAGE_NO;
	self->touch			= NULL;		// no touching
	self->think			= G_FreeEdict;
	self->nextthink		= level.time; // ASAP

	gi.linkentity (self);
}

/**
 * Unused, not reached 
 */
void plant_dead(edict_t *self)
{
	//writeMessage("===> Plant is dead!!!\n", DISPLAY_PLANT_FUNCTION_DEBUG);
}


/**
 * walk function
 *
 * @param edict_t : plant entity instance
 */
void plant_walk(edict_t *self)
{
	//writeMessage("Plant running walk function\n", DISPLAY_PLANT_FUNCTION_DEBUG);
}

/**
 * stand function
 *
 * @param edict_t : plant entity instance
 */
void plant_stand(edict_t *self)
{
	//writeMessage("Plant running stand function\n", DISPLAY_PLANT_FUNCTION_DEBUG);
}

/**
 * run function
 *
 * @param edict_t : plant entity instance
 */
void plant_run(edict_t *self)
{
	//writeMessage("Plant running run function\n", DISPLAY_PLANT_FUNCTION_DEBUG);
}

/**
 * pain function
 * This is the mechanism that herbivores use to pollenate plants
 *
 * @param edict_t : plant entity instance
 * @param edict_t : 
 * @param float : 
 * @param int : 
 *
 */
void plant_pain(edict_t *self, 
				edict_t *other, 
				float kick, 
				int damage)
{
	generic_agent_t	*aAgent = NULL;
	
	// check self and other, and ENSURE that other is an agent
	if(self && other && other->agent)
	{
		aAgent = castAgent(self->agent);

		if(aAgent->plantAgent->state == PLANT_SPROUT_STATE)
		{			
			plant_attach_pollen(self, other); // attempt to attach pollen to the other agent
		}
		else if(aAgent->plantAgent->state == PLANT_ESTABLISHED_STATE)
		{			
			if(castAgent(other->agent)->hasPollen)
			{
				plant_reproduce(self, other);	// attempt to reproduce
			}

			plant_attach_pollen(self, other); // attempt to attach pollen to the other agent			
		}
	}
}


/**
 * change the plant into its sprout state
 *
 * @param edict_t : plant entity instance
 */
void plant_set_sprout_state(edict_t *self)
{
	generic_agent_t	*aAgent = castAgent(self->agent);

	//writeMessage("Changed to sprout state\n", DISPLAY_PLANT_FUNCTION_DEBUG);

	gi.setmodel(self, PLANT_SPROUT_MODEL); // model change

	self->movetype							= MOVETYPE_NONE; // sit still
	self->max_health						= PLANT_MAX_RESOURCE_SPROUT; // max resource that can be generated
	aAgent->lastStateChangeTime				= getTime(); // remember time of state change
	aAgent->plantAgent->state				= PLANT_SPROUT_STATE; // now entered the sprout state
	aAgent->regenerationRate				= PLANT_SPROUT_REGENERATION_RATE; //regen rate
	self->monsterinfo.currentmove			= &plant_generic_think_move; // use generic ai cycle
}


/**
 * change the plant into its established state
 *
 * @param edict_t : plant entity instance
 */
void plant_set_established_state(edict_t *self)
{
	generic_agent_t	*aAgent = castAgent(self->agent);

	//writeMessage("Changed to established state\n", DISPLAY_PLANT_FUNCTION_DEBUG);

	gi.setmodel(self, PLANT_ESTABLISHED_MODEL); // model change

	aAgent->lastReproduceTime				= getTime(); // must wait a while before can first reproduce
	self->max_health						= PLANT_MAX_RESOURCE_ESTABLISHED; // max resource that can be generated
	aAgent->lastStateChangeTime				= getTime(); // remember time of state change
	aAgent->plantAgent->state				= PLANT_ESTABLISHED_STATE; // now entered the sprout state
	aAgent->regenerationRate				= PLANT_ESTABLISHED_REGENERATION_RATE; //regen rate
	self->monsterinfo.currentmove			= &plant_generic_think_move; // use generic ai cycle
}



/**
 * generic think function called on the plant every sever frame
 *
 * @param edict_t : plant entity instance
 */
void plant_generic_think(edict_t *self)
{
	generic_agent_t	*aAgent = NULL;
	long			theTime = getTime();

	// called every frame regardless of state
	if(self && self->agent)
	{
		aAgent = castAgent(self->agent);

		// only generate when in sprout or established state
		if(	aAgent->plantAgent->state == PLANT_SPROUT_STATE || 
			aAgent->plantAgent->state == PLANT_ESTABLISHED_STATE)
		{
			if(theTime > (aAgent->plantAgent->lastRegen+ONE_SECOND))
			{
				self->health += aAgent->regenerationRate; // regeneration
				aAgent->plantAgent->lastRegen = theTime; // remember the time
			}

			// clip health to current bounds (state specific)
			if(self->health > self->max_health)
			{
				self->health = self->max_health; // clipped
			}
		}
	}
}


/**
 * determin if its time to change states
 *
 * @param edict_t : plant entity instance
 */
void plant_maturity_check(edict_t *self)
{
	generic_agent_t	*aAgent = castAgent(self->agent);
	long theTime = getTime();

	//writeMessage("Running maturity check...\n", DISPLAY_PLANT_FUNCTION_DEBUG);

	switch(aAgent->plantAgent->state)
	{
		case PLANT_SPROUT_STATE:
			// check if its time to become mature
			if(theTime > (aAgent->lastStateChangeTime+PLANT_SPROUT_MAX_LIFE_SECONDS) )
			{				
				plant_set_established_state(self); // time to become established
			}
			break;

		case PLANT_ESTABLISHED_STATE:
			// check for death
			if(theTime > (aAgent->lastStateChangeTime+PLANT_ESTABLISHED_MAX_LIFE_SECONDS) )
			{				
				plant_death_wrapper(self); // run death				
			}
			break;

		case PLANT_DEATH_STATE:
			// check if its time to disappear
			break;
		
		default:
			break;
	}
}

/**
 * Allows a plant to repoduce if it is allowed with in resourse
 * and time constraints
 *
 * @param edict_t : a plant instance
 * @param edict_t : another agent that has pollen attached to it
 */
void plant_reproduce(edict_t *self, 
					 edict_t *other)
{
	generic_agent_t	*selfAgent = castAgent(self->agent);
	generic_agent_t	*otherAgent = castAgent(other->agent);
	long			theTime = getTime();
	int				i;

	// check if the plant is allowed to reproduce
	if((self->health - PLANT_REPRODUCE_RESOURCE_HIT) < AGENT_MIN_HEALTH)
	{
		//writeMessage("[!] Not enough health.\n", DISPLAY_PLANT_FUNCTION_DEBUG);
		return; // not enough resources to reproduce
	}
	
	if(selfAgent->reproduceCount > selfAgent->plantAgent->dna.reproduceFrequency)
	{
		//writeMessage("[!] Reproduced too many times.\n", DISPLAY_PLANT_FUNCTION_DEBUG);
		return; // have reproduced too many times
	}

	if(theTime < (selfAgent->lastReproduceTime + PLANT_MIN_REPRODUCE_WAIT_SECONDS))
	{
		//writeMessage("[!] Min time between reproduce actions has not elapsed.\n", DISPLAY_PLANT_FUNCTION_DEBUG);
		return; // minimum time between reproduce actions
	}

	// cannot pollenate itself
	if(selfAgent->identity == otherAgent->pollenPlantID)
	{
		//writeMessage("[!] Cannot pollenate iteslf.\n", DISPLAY_PLANT_FUNCTION_DEBUG);
		return; // cannot pollenate itself
	}

	//
	// do a reproduce action
	//
	//writeMessage("-> Pollen has been received, doing a reproduce action.\n", DISPLAY_PLANT_FUNCTION_DEBUG);
	
	selfAgent->lastReproduceTime = theTime; // remember the time
	selfAgent->reproduceCount++; // count reproduce actions

	// create each of the seeds
	for(i=0; i<selfAgent->plantAgent->dna.numSeeds; i++)
	{
		// create a new seed
		spawn_plant_reproduce(self, &selfAgent->plantAgent->dna, &otherAgent->pollen, i);
	}
}


/**
 * Attempt to attach pollen to the other agent
 *
 */
void plant_attach_pollen( edict_t *self, 
						  edict_t *other)
{
	generic_agent_t	*selfAgent = castAgent(self->agent);
	generic_agent_t	*otherAgent = castAgent(other->agent);

	if(!other->takedamage || !other->health)
	{
		return; // must be able to take damage
	}

	if(otherAgent->plantAgent)
	{
		return; // plants cannot collect pollen from each other
	}

    // attach pollen
	cloneDNA(&selfAgent->plantAgent->dna, &otherAgent->pollen);
	otherAgent->hasPollen = true;
	otherAgent->pollenPlantID = selfAgent->identity;
	
	
	//writeMessage("Pollen has been attached to an entity.\n", DISPLAY_PLANT_FUNCTION_DEBUG);
}



/**
 * whether or not a plant is poisonous
 */
qboolean isPlantPoisonous(edict_t *plant)
{
	generic_agent_t	*selfAgent = castAgent(plant->agent);

	// check for 1 and 2
	if(isPoisonGeneActive(selfAgent->plantAgent->dna.poison1) && 
	   isPoisonGeneActive(selfAgent->plantAgent->dna.poison2) )
	{
		return true;
	}

	// check for 1 and 3
	if(isPoisonGeneActive(selfAgent->plantAgent->dna.poison1) && 
	   isPoisonGeneActive(selfAgent->plantAgent->dna.poison3) )
	{
		return true;
	} 

	// check for 2 and 3
	if(isPoisonGeneActive(selfAgent->plantAgent->dna.poison2) && 
	   isPoisonGeneActive(selfAgent->plantAgent->dna.poison3) )
	{
		return true;
	}

	return false; // not poisonous
}

/**
 * whether or not a poison gene is active
 */
qboolean isPoisonGeneActive(float gene)
{
	return (gene >= 0.50);
}

