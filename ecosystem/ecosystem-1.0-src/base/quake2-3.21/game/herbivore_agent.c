
// herbivore_agent.c


/**
 * Description: The herbivore agent, based on the parasite monster
 *
 * Author: Jason Brownlee - 
 *
 * Change History
 * Who			When		Description
 * --------------------------------------------------------------------
 * 
 *
 */

#include "generic_agent.h"
#include "m_parasite.h"

//
// Constants
//
#define HERBIVORE_MODEL				"models/monsters/parasite/tris.md2"
#define HERBIVORE_DEBUG				FALSE

// globals - file scope
static int	sound_pain1;
static int	sound_pain2;
static int	sound_die;
static int	sound_launch;
static int	sound_impact;
static int	sound_suck;
static int	sound_reelin;
static int	sound_sight;
static int	sound_tap;
static int	sound_scratch;
static int	sound_search;


// external function prototypes
void Killed(edict_t *targ, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
qboolean isPlantPoisonous(edict_t *plant);


//
// herbivore perceptron function prototypes
//
void herbivore_prepare_perceptrion(edict_t *self, edict_t *parent);
void remember_current_feedgoal(edict_t *self, qboolean feedResult);
edict_t *select_food_goal(edict_t *self);

//
// herbivore function prototypes
//
void spawn_herbivore(int delaySeconds, vec3_t spawn_origin, vec3_t spawn_angles);
void spawn_herbivore_reproduce(edict_t *parent, int childNumber);
void spawn_herbivore_start(edict_t *self);

// states
void herbivore_fly_check_timeout(edict_t *self);
void herbivore_death_wrapper(edict_t *self);
void herbivore_dead(edict_t *self);
void herbivore_set_child_state(edict_t *self);
void herbivore_set_adult_state(edict_t *self);
void herbivore_maturity_check(edict_t *self);
void herbivore_reproduce(edict_t *self);
void herbivore_server_frame(edict_t *self);
qboolean isHerbivoreHungry(edict_t *self, float threshold);
qboolean Herbivore_CheckAttack (edict_t *self);
void herbivore_perform_feed(edict_t *self);
void herbivore_flee(edict_t *self);

// goal selection
qboolean herbivore_find_target(edict_t *self);
void evaluate_current_goal(edict_t *self);
edict_t *getClosestAgent(edict_t *self);
edict_t *getRandomAgent(edict_t *self);
edict_t *getClosestPlant(edict_t *self);
edict_t *getRandomPlant();
edict_t *getRandomPlantFromClosest(edict_t *self);
int getNClosestPlants(edict_t *plantList[], int maxPlants, edict_t *self);
void herbivore_lost_goal(edict_t *self, qboolean wasFoodGoal);

// monster framework functions
void herbivore_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void herbivore_pain(edict_t *self, edict_t *other, float kick, int damage);
void herbivore_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);

void herbivore_stand(edict_t *self);
void herbivore_walk(edict_t *self);
void herbivore_run(edict_t *self);
void herbivore_attack(edict_t *self);




// herbivore death animation/AI cycle
mframe_t herbivore_death_frames [] = 
{ 
	ai_move, 0, NULL,	ai_move, 0, NULL,	ai_move, 0, NULL,	ai_move, 0, NULL,	ai_move, 0, NULL,
	ai_move, 0, NULL,	ai_move, 0, NULL,	 ai_move, 0, NULL,	ai_move, 0, NULL,	ai_move, 0, NULL
};
mmove_t herbivore_death_think_move = {0, 0, herbivore_death_frames, herbivore_dead};

// the herbivore standing around
mframe_t herbivore_frames_stand [] =
{
	ai_stand, 0, NULL,	// run 17 frames
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,

	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,

	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,

	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,

	ai_stand, 0, NULL
};
mmove_t	herbivore_move_stand = {FRAME_stand01, FRAME_stand17, herbivore_frames_stand, herbivore_stand};

// herbivore walking
mframe_t herbivore_frames_walk [] =
{
	ai_walk, 30, NULL,
	ai_walk, 30, NULL,
	ai_walk, 22, NULL,
	ai_walk, 19, NULL,
	ai_walk, 24, NULL,
	ai_walk, 28, NULL,
	ai_walk, 25, NULL
};
mmove_t herbivore_move_walk = {FRAME_run03, FRAME_run09, herbivore_frames_walk, herbivore_walk};

// pain movement - should end in a run
mframe_t herbivore_frames_pain1 [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 6,	NULL,
	ai_move, 16, NULL,
	ai_move, -6, NULL,
	ai_move, -7, NULL,
	ai_move, 0, herbivore_flee
};
mmove_t herbivore_move_pain1 = {FRAME_pain101, FRAME_pain111, herbivore_frames_pain1, herbivore_run};

// attack cycle - NO THINKING - should end in a run
mframe_t herbivore_frames_attack [] =
{
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 15,	herbivore_perform_feed,			
	ai_charge, 0,	herbivore_perform_feed,			
	ai_charge, 0,	herbivore_perform_feed,			
	ai_charge, 0,	herbivore_perform_feed,			
	ai_charge, 0,	herbivore_perform_feed,			
	ai_charge, -2,  herbivore_perform_feed,		
	ai_charge, -2,	herbivore_perform_feed,		
	ai_charge, -3,	herbivore_perform_feed,		
	ai_charge, -2,	herbivore_perform_feed,			
	ai_charge, 0,	herbivore_perform_feed,	
	ai_charge, -1,  herbivore_perform_feed,		
	ai_charge, 0,	NULL,							// let go
	ai_charge, -2,	NULL,
	ai_charge, -2,	NULL,
	ai_charge, -3,	NULL,
	ai_charge, 0,	evaluate_current_goal	// see if had enough to eat
};
mmove_t herbivore_move_attack = {FRAME_drain01, FRAME_drain18, herbivore_frames_attack, herbivore_run};

// run cycle
mframe_t herbivore_frames_run [] =
{
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 10, evaluate_current_goal // see if have been pursuing the goal too long
};
mmove_t herbivore_move_run = {FRAME_run03, FRAME_run09, herbivore_frames_run, herbivore_run};




/**
 * Spawn a single herbivore agent into the world
 *
 * @param int : the time delay in seconds, <=0 for no delay
 */
void spawn_herbivore(int delaySeconds, vec3_t spawn_origin, vec3_t spawn_angles)
{
	edict_t		*self = NULL;

	// never spawn in deathmatch game
	if(deathmatch->value)
	{
		return;
	}

	// check if can spawn
	if(!withinPopulationQuota(HERBIVORE_AGENT_TYPE))
	{
		writeMessage("HERBIVORE::Could not spawn - over population quota.\n", HERBIVORE_DEBUG);
		return;
	}

	self = G_Spawn();

	VectorCopy(spawn_origin, self->s.origin); // inform the entity of the spawn location
	self->s.origin[2] += 8; // ensure off the ground

	// allocate memory for the agent and herbivore data structures
	allocateAgentMemory(self, HERBIVORE_AGENT_TYPE);

	// prepare the new herbivores perceptron
	herbivore_prepare_perceptrion(self, NULL);
	
	// check if the spawn needs to be delayed
	if(delaySeconds > 0)
	{
		self->think = spawn_herbivore_start; // initialize to child state
		self->nextthink = level.time + delaySeconds;			
	}
	else // just call now and spawn into the world
	{
		spawn_herbivore_start(self); // initialize to child state
	}
}


/**
 * Create new herbivore agents as the product of a reproduction action.
 *
 * @param edict_t : the parent that created the new seed
 */
void spawn_herbivore_reproduce(edict_t *parent,
							   int childNumber)
{
	edict_t				*self = NULL;	

	// check if can spawn
	if(!withinPopulationQuota(HERBIVORE_AGENT_TYPE))
	{
		writeMessage("HERBIVORE::Could not spawn - over population quota.\n", HERBIVORE_DEBUG);
		return;
	}

	self = G_Spawn();

	// enter just above the parent plant 
	VectorCopy(parent->s.origin, self->s.origin);	// copy parents position
	self->s.origin[2] += 8; // ensure off the ground

	allocateAgentMemory(self, HERBIVORE_AGENT_TYPE); // allocate memory and initialise agent structure

	// populate the child with the parents knowledge
	herbivore_prepare_perceptrion(self, parent);

	// check if the spawn needs to be delayed
	if(childNumber > 0)
	{
		self->think = spawn_herbivore_start; // initialize to child state
		self->nextthink = level.time + childNumber;
	}
	else // just call now and spawn into the world
	{
		spawn_herbivore_start(self); // initialize to child state
	}
}



/**
 * Called after a herbivore has been spawned into the world,
 * used to initialise it to its child state
 *
 */
void spawn_herbivore_start(edict_t *self)
{
	generic_agent_t *aAgent = castAgent(self->agent);
	long			theTime = getTime();

	// set up sounds - needed?
	sound_pain1		= gi.soundindex ("parasite/parpain1.wav");	
	sound_pain2		= gi.soundindex ("parasite/parpain2.wav");	
	sound_die		= gi.soundindex ("parasite/pardeth1.wav");	
	sound_launch	= gi.soundindex("parasite/paratck1.wav");
	sound_impact	= gi.soundindex("parasite/paratck2.wav");
	sound_suck		= gi.soundindex("parasite/paratck3.wav");
	sound_reelin	= gi.soundindex("parasite/paratck4.wav");
	sound_sight		= gi.soundindex("parasite/parsght1.wav");
	sound_tap		= gi.soundindex("parasite/paridle1.wav");
	sound_scratch	= gi.soundindex("parasite/paridle2.wav");
	sound_search	= gi.soundindex("parasite/parsrch1.wav");

	// bounding box
	VectorSet(self->mins, -16, -16, -24);
	VectorSet(self->maxs, 16, 16, 24);
	
	//self->movetype			= MOVETYPE_TOSS;	// float down like a leaf
	//self->movetype			= MOVETYPE_STEP;	// walking monster
	self->movetype		= MOVETYPE_BOUNCE;

	self->solid				= SOLID_BBOX;
	self->gib_health		= -50;
	self->mass				= 50;			// reduced for flight
	self->gravity			= 0.50;			// reduced for flight
	
	//
	// herbivore specific stuff
	//
	aAgent->birthDate						= theTime; // set birth day/time
	aAgent->lastStateChangeTime				= theTime;
	aAgent->herbivoreAgent->lastModeChange	= theTime - HERBIVORE_NEW_GOAL_WAIT_SECONDS; // any time now
	aAgent->herbivoreAgent->lastDegen		= theTime;

	self->s.modelindex				= gi.modelindex(HERBIVORE_MODEL); // set up model
	self->health					= HERBIVORE_BIRTH_START_RESOURCE;
	self->max_health				= HERBIVORE_CHILD_MAX_RESOURCE;
	aAgent->herbivoreAgent->state	= HERBIVORE_BIRTH_STATE;	
	aAgent->agent_server_frame		= herbivore_server_frame; // run stuff every server frame regardless of state
	aAgent->herbivoreAgent->mode	= HERBIVORE_IDLE;
	aAgent->find_target				= herbivore_find_target; // find target
	self->monsterinfo.scale			= AGENT_MODEL_SCALE;

	// generic inputs
	self->pain						= herbivore_pain;
	self->die						= herbivore_die;
	self->touch						= herbivore_touch;		

	// monster specific inputs and actions
	self->monsterinfo.checkattack	= Herbivore_CheckAttack;
	self->monsterinfo.stand			= herbivore_stand;
	self->monsterinfo.walk			= herbivore_walk;	
	self->monsterinfo.melee			= herbivore_attack;		// melee only
	self->monsterinfo.run			= herbivore_run;	

	gi.linkentity(self);		// link to world		
	walkmonster_start(self);	// initialise as a monster

	// launch off at some angle, with some speed
	self->velocity[0] = 100.0 * crandom();
	self->velocity[1] = 100.0 * crandom();
	self->velocity[2] = 400;
}



/**
 * Wrapper for the death function
 */ 
void herbivore_death_wrapper(edict_t *self)
{
	Killed(self, self, self, -999, self->s.origin);
}

/**
 * Called when the herbivore is dead
 *
 */
void herbivore_dead(edict_t *self)
{
	writeMessage("herbivore_dead()::Herbivore is dead.\n", HERBIVORE_DEBUG);
}

/**
 * While flying (after being born), check if have been flying for too long
 *
 */
void herbivore_fly_check_timeout(edict_t *self)
{
	long			currentTime = getTime();
	generic_agent_t	*aAgent = castAgent(self->agent);

	// check if have hit the ground
	if(self->groundentity)
	{
		herbivore_set_child_state(self); // enter child state
	}
	// check if have been a seed for too long
	else if(currentTime > (aAgent->birthDate + HERBIVORE_BIRTH_TIMEOUT_SECONDS) )
	{
		// have been a seed for ages, remove self
		writeMessage("-->Herbivore been in birth state too long, time to die.\n", HERBIVORE_DEBUG);
		herbivore_death_wrapper(self);
	}
}

/**
 * Set the herbivore into its child state
 *
 */
void herbivore_set_child_state(edict_t *self)
{
	generic_agent_t	*aAgent = castAgent(self->agent);

	//writeMessage("Herbivore is now entering its child state.\n", HERBIVORE_DEBUG);

	self->s.origin[2] += 16; // ensure off the ground

	self->mass						= 200;
	self->gravity					= 1.00;
	self->movetype					= MOVETYPE_STEP;	// walking monster
	self->health					= HERBIVORE_BIRTH_START_RESOURCE;
	self->max_health				= HERBIVORE_CHILD_MAX_RESOURCE;

	aAgent->degenerationRate		= HERBIVORE_CHILD_DEGEN_RATE;
	aAgent->herbivoreAgent->mode	= HERBIVORE_IDLE;
	aAgent->herbivoreAgent->state	= HERBIVORE_CHILD_STATE;
	aAgent->lastStateChangeTime		= getTime(); // remember time of state change

	M_droptofloor(self); // get down on the ground (because of MOVETYPE_TOSS)
	self->monsterinfo.stand(self);	 // stand around
}


/**
 * Set the herbivore into its adult state
 *
 */
void herbivore_set_adult_state(edict_t *self)
{
	generic_agent_t	*aAgent = castAgent(self->agent);

	//writeMessage("Herbivore is now entering its adult state.\n", HERBIVORE_DEBUG);

	aAgent->lastReproduceTime		= (getTime() + getRandomInteger(0,30,FALSE));	// ensure can reproduce when ready (after a delay)
	self->max_health				= HERBIVORE_ADULT_MAX_RESOURCE;	// new max resource
	aAgent->degenerationRate		= HERBIVORE_ADULT_DEGEN_RATE;	// new degen rate	
	aAgent->herbivoreAgent->state	= HERBIVORE_ADULT_STATE;		// new state
	aAgent->lastStateChangeTime		= getTime();					// remember time of state change

	// ensure the goal is lost when growing up to an adult
	herbivore_lost_goal(self, false);
}


/**
 * Called when the herbivore entity tocuhes something
 *
 * @param edict_t : herbivore entity instance
 * @param edict_t : what was touched
 * @param cplane_t : the plane the thing was touched on (?)
 * @param csurface_t : the surface touched (?)
 *
 */
void herbivore_touch(edict_t *self, 
					edict_t *other, 
					cplane_t *plane, 
					csurface_t *surf)
{
	generic_agent_t	*aAgent = castAgent(self->agent);
	
	// a touch in different states will do different things
	switch(aAgent->herbivoreAgent->state)
	{
		case HERBIVORE_BIRTH_STATE:
			//writeMessage("Herbivore had a touch in birth state.\n", HERBIVORE_DEBUG);
			
			if(self->groundentity) // on the ground - now a child
			{
				herbivore_set_child_state(self);
			}
			break;

		case HERBIVORE_CHILD_STATE:
			//writeMessage("Herbivore had a touch in child state.\n", HERBIVORE_DEBUG);
			break;

		case HERBIVORE_ADULT_STATE:
			//writeMessage("Herbivore had a touch in adult state.\n", HERBIVORE_DEBUG);
			break;

		case HERBIVORE_DEATH_STATE:
			//writeMessage("Herbivore had a touch in death state.\n", HERBIVORE_DEBUG);
			break;

		default: // unknown state, error
			break;
	}
}


/**
 * Function called when the herbivore dies, will remove the herbivore from the world
 * Based on g_misc.c ==> ThrowHead() function
 *
 * @param edict_t : plant entity instance
 * @param edict_t :
 * @param edict_t :
 * @param int : 
 * @param vec3_t : 
 * 
 */
void herbivore_die(	edict_t *self, 
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

	// throw gibs
	ThrowGib (self, "models/objects/gibs/bone/tris.md2", 200, GIB_ORGANIC);
	ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", 300, GIB_ORGANIC);
	ThrowHead (self, "models/objects/gibs/head2/tris.md2", 100, GIB_ORGANIC);

	aAgent = castAgent(self->agent);
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
	self->monsterinfo.currentmove = &herbivore_death_think_move;	

	aAgent->herbivoreAgent->state = HERBIVORE_DEATH_STATE; // plant dead state

	reclaimAgentMemory(self); // reclaim allocated system memory	

	self->takedamage	= DAMAGE_NO;
	self->touch			= NULL;		// no touching
	self->think			= G_FreeEdict;
	self->nextthink		= level.time; // ASAP

	gi.linkentity (self);
}



/**
 * Think function executed every server frame regardless of state
 */
void herbivore_server_frame(edict_t *self)
{
	generic_agent_t	*aAgent = NULL;
	long			theTime = getTime();

	// called every server frame regardless of state
	if(self && self->agent)
	{	
		aAgent = castAgent(self->agent);

		// only de-generate when in child or adult state
		if(	aAgent->herbivoreAgent->state == HERBIVORE_CHILD_STATE || 
			aAgent->herbivoreAgent->state == HERBIVORE_ADULT_STATE)
		{
			// resource degeneration occurs every second, not every sever frame
			if(theTime > (aAgent->herbivoreAgent->lastDegen+ONE_SECOND))
			{
				// time to run degen for this agent
				self->health -= aAgent->degenerationRate; //degeneration
				aAgent->herbivoreAgent->lastDegen = theTime; // remember the time
			}

			// clip health to current bounds (state specific)
			if(self->health > self->max_health)
			{
				self->health = self->max_health; // clipped
			}
			// check for death
			else if(self->health <= 0)
			{
				// out of health - time to die				
				writeMessage("herbivore_think():out of health - time to die.\n", HERBIVORE_DEBUG);
				herbivore_death_wrapper(self);
				return;
			}

			// draw a line to current goal	
			if(	HERBIVORE_SHOW_GOAL_SELECTION && self->enemy)
			{
				UtilDrawLine(self->s.origin, self->enemy->s.origin);
			}
		}

		// see if its time to change level in lifecycle
		herbivore_maturity_check(self);
	}
}



/**
 * Maturity check for the herbivore
 * Determines if it is time for the herbivore to grow up, reproduce,
 * or die of old age
 *
 * @param edict_t : a herbivore agent
 */
void herbivore_maturity_check(edict_t *self)
{
	generic_agent_t	*aAgent = castAgent(self->agent);
	long			theTime = getTime();

	switch(aAgent->herbivoreAgent->state)
	{
		case HERBIVORE_BIRTH_STATE:
			herbivore_fly_check_timeout(self); // check if have hit the ground yet
			break;

		case HERBIVORE_CHILD_STATE:						
			// see if it is time to grow up
			if(theTime > (aAgent->birthDate+HERBIVORE_ADULT_TIME_SECONDS_THRESHOLD))
			{
				//writeMessage("--> herbivore child->adult, time threshold exceeded.\n", HERBIVORE_DEBUG);
				herbivore_set_adult_state(self); // enter adult state				
			}
			break;

		case HERBIVORE_ADULT_STATE:		
			// see if its time to die from old age
			if(theTime > (aAgent->lastStateChangeTime+HERBIVORE_ADULT_MAX_LIFE_SECONDS))
			{
				writeMessage("--> herbivore adult->death, died of old age.\n", HERBIVORE_DEBUG);
				herbivore_death_wrapper(self); // die now herbivore
				return;
			}

			herbivore_reproduce(self); // try and reproduce
			break;

		case HERBIVORE_DEATH_STATE:
			// maybe linger around 
			break;

		default: // unknown state, error
			break;
	}
}


/**
 * The herbivore reproduce action
 * If the conditions are met, the herbivore will create new herbivores
 * as children, which will spawn into the world.
 *
 * @param edict_t : a parent herbivore 
 */
void herbivore_reproduce(edict_t *self)
{
	generic_agent_t	*aAgent = castAgent(self->agent);
	long			theTime = getTime();
	int				i = 0;

	// check if not exceeded the max number of reproduce actions
	if(aAgent->reproduceCount >= HERBIVORE_MAX_REPRODUCE)
	{
		return; // too many reproduce actions
	}

	// check if enough time has passed since the last reproduce action
	if(theTime < (aAgent->lastReproduceTime + HERBIVORE_REPRODUCE_MIN_WAIT_SECONDS))
	{
		return; // need to wait some more buddy
	}

	// check if enough health
	if((self->health - HERBIVORE_REPRODUCE_RESOURCE_HIT) < AGENT_MIN_HEALTH)
	{
		return; // not enough health
	}

	// remember the time
	aAgent->lastReproduceTime = theTime;

	// count the number of reproduction actions
	aAgent->reproduceCount++;

	// the herbivore takes a resource hit for this
	self->health -= HERBIVORE_REPRODUCE_RESOURCE_HIT;

	//writeMessage("--> perforing a reproduce action.\n", HERBIVORE_DEBUG);

	// reproduce time
	for(i=0; i<HERBIVORE_REPRODUCE_NUM_CHILDREN; i++)
	{
		spawn_herbivore_reproduce(self, i); // give birth
	}
}


/**
 * Herbivores pain function
 *
 */
void herbivore_pain(edict_t *self, 
					edict_t *other, 
					float kick, 
					int damage)
{
	generic_agent_t	*aAgent = castAgent(self->agent);

	if (self->health < (self->max_health / 2))
	{
		self->s.skinnum = 1; // pain skin
	}

	if (level.time < self->pain_debounce_time)
	{
		return;
	}

	self->pain_debounce_time = level.time + 3;

	// do some pain movement
	self->monsterinfo.currentmove = &herbivore_move_pain1;
}


/**
 * Run herbivore stand
 */
void herbivore_stand(edict_t *self)
{
	//writeMessage("Running --> herbivore_stand()\n", HERBIVORE_DEBUG);
	self->monsterinfo.currentmove	= &herbivore_move_stand;
}

/**
 * Run herbivore walk
 */
void herbivore_walk(edict_t *self)
{
	//writeMessage("Running --> herbivore_walk()\n", HERBIVORE_DEBUG);
	self->monsterinfo.currentmove	= &herbivore_move_walk;
}


/**
 * Run herbivore attack 
 */
void herbivore_attack(edict_t *self)
{
	//writeMessage("Running --> herbivore_attack()\n", HERBIVORE_DEBUG);
	self->monsterinfo.currentmove	= &herbivore_move_attack;
}

/**
 * Run the herbivore run
 */
void herbivore_run(edict_t *self)
{
	//writeMessage("Running --> herbivore_run()\n", HERBIVORE_DEBUG);
	self->monsterinfo.currentmove = &herbivore_move_run;
}



/**
 * Determine if the herbivore is hungry.
 * The herbivore is only hungry when its food resource drops below 
 * an established percentage of the total amount it can store
 *
 */
qboolean isHerbivoreHungry(edict_t *self, 
						   float threshold)
{
	float percentage = 0.0;

	percentage = (float) ((float)self->health / (float)self->max_health);

	if(percentage < threshold)
	{
		//Com_Printf("Herbivore is %3.2f%% hungry with %d/%d\n", percentage, self->health, self->max_health);
		return true; // hungry hungry herbivore
	}

	// sometime eat when not hungry
	if(RANDOM <= 0.20) // 20% change
	{
		//writeMessage("Herbivore decided to eat when not hungry\n", HERBIVORE_DEBUG);
		return true;
	}

	return false; // not hungry
}



/** 
 * Called when the herbivore is asked to check whether it can attack 
 * its goal. Based on M_CheckAttack().
 * Called by ai_checkattack() when the we need to know if the monster is
 * ready to attack, and what mode to attack in (melee or missile).
 * 
 */
qboolean Herbivore_CheckAttack (edict_t *self)
{
	vec3_t	spot1, spot2;
	trace_t	tr;
	float range = 0;

	//writeMessage("-> running Herbivore_CheckAttack().\n", HERBIVORE_DEBUG);

	// must be alive
	if(!self->enemy || self->enemy->health<=0)
	{
		return false; // its dead 
	}
	
	// see if any entities are in the way of the shot
	VectorCopy (self->s.origin, spot1);
	spot1[2] += self->viewheight / 2; //	remember jason - lower because plants are small
	VectorCopy (self->enemy->s.origin, spot2);
	spot2[2] += self->enemy->viewheight / 2; // remember jason - lower because plants are small

	tr = gi.trace (spot1, NULL, NULL, spot2, self, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_SLIME|CONTENTS_LAVA|CONTENTS_WINDOW);

	// do we have a clear shot?
	if(tr.ent != self->enemy || !tr.endpos)
	{
		return false;
	}
	
	// determine the distance the goal is from the agent
	range = UtilDistanceBetweenPoints(self->s.origin, tr.endpos);
	
	// must be close enough to bite
	if(range <= HERBIVORE_BITE_PROXIMITY)
	{
		// attack if close enough		
		self->monsterinfo.attack_state = AS_MELEE;
		return true;
	}

	return false;
}


/**
 * Perform a feed action on a plant - called each attach on current goal
 */
void herbivore_perform_feed(edict_t *self)
{
	vec3_t			offset, start, f, r, end, dir;
	trace_t			tr;
	int				damage = 0;
	generic_agent_t	*aAgent = castAgent(self->agent);
	generic_agent_t	*plantAgent = NULL;
	qboolean poisonous;


	// check if a roal/flee goal has been reached
	if(aAgent->herbivoreAgent->mode != HERBIVORE_FEED)
	{
		//
		// add code here if you want the herbivore to pollenate roam/flee goals
		// not really a good idea - does not fit into to GA fitness function (fit == good food for herbi)
		//

		//writeMessage("[!] -> reached roam goal - but not attacking.\n", HERBIVORE_DEBUG);

		// ensure the agents waits a while before looking for a new goal
		aAgent->herbivoreAgent->lastModeChange	= getTime();
		herbivore_lost_goal(self, false); // lost goal - its been achieved
		return;
	}

	// check for too many bites from current food goal
	if(aAgent->herbivoreAgent->numPlantBites >= HERBIVORE_MAX_BITES)
	{
		aAgent->herbivoreAgent->lastModeChange	= getTime();		
		herbivore_lost_goal(self, false); // lost goal - its been achieved
		return;
	}

	AngleVectors (self->s.angles, f, r, NULL);
	VectorSet(offset, 0, 0, 0);
	G_ProjectSource (self->s.origin, offset, f, r, start);

	VectorCopy (self->enemy->s.origin, end);

	tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT);
	if (tr.ent != self->enemy)
	{
		return; // cannot hit
	}

	// determine how much to hurt the plant
	if(aAgent->herbivoreAgent->state == HERBIVORE_CHILD_STATE)
	{
		damage = HERBIVORE_CHILD_RESOURCE_CONSUME_RATE;
	}
	else if(aAgent->herbivoreAgent->state == HERBIVORE_ADULT_STATE)
	{
		damage = HERBIVORE_ADULT_RESOURCE_CONSUME_RATE;
	}
	else
	{
		Com_Printf("Herbivore attacking with an unknown state: %s", aAgent->herbivoreAgent->state);
		damage = 0;
	}

	if(self->s.frame != FRAME_drain03)	
	{
		//damage /= 2; // only full damage once per attack cycle
		damage = 0; // damage only once per attack cycle
	}

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_PARASITE_ATTACK);
	gi.WriteShort(self - g_edicts);
	gi.WritePosition(start);
	gi.WritePosition(end);
	gi.multicast(self->s.origin, MULTICAST_PVS);

	VectorSubtract (start, end, dir);

	// perform damage
	if(damage)
	{
		// count damage bites
		aAgent->herbivoreAgent->numPlantBites++;

		// damage the plant
		T_Damage(self->enemy, self, self, dir, self->enemy->s.origin, vec3_origin, damage, 0, DAMAGE_NO_KNOCKBACK, MOD_UNKNOWN);

		// determine result of feed action
		if(self->enemy->agent && (plantAgent = castAgent(self->enemy->agent))->plantAgent)
		{		
			poisonous = isPlantPoisonous(self->enemy);

			// plant is poisonous
			if(poisonous)
			{
				self->health -= (damage/2); // take resource to herbivore
			}
			// plant is not poisonous
			else
			{
				self->health += damage; // give resource to herbivore
			}

			// remember the current feed goal - and the result of talking a bite
			remember_current_feedgoal(self, !poisonous); // inverse of bite result
		}
		else
		{
			//Com_Printf("HERBIVORE:herbivore_perform_feed() : took a bite of a non-plant entity.\n");
		}
	}

	// regardless, the goal was achieved, ensure the goal timeout is pushed back
	aAgent->herbivoreAgent->lastModeChange	= getTime(); //  a new goal eval has occured
}



/**
 * Called when the herbivore is running. This means
 * the herbivore has a goal and is trying to get to it, or is in the process
 * of atacking it
 */
void evaluate_current_goal(edict_t *self)
{
	generic_agent_t	*aAgent = castAgent(self->agent);
	long theTime = getTime();

	// ensure the agent does have a goal to evaluate
	// this may happen when a herbivore looses goal in a non-normal way, whilst going for the goal.
	// an example is goal is lost when traversing from child --> to adult state
	// eg: attacking, become adult, loose goal, evaluate goal...
	if(!self->enemy)
	{		
		writeMessage("[!] -> evaluate_current_goal() no goal to evaluate.\n", HERBIVORE_DEBUG);
		herbivore_lost_goal(self, false);
		return;
	}

	// check if have been pursuing goal for too long
	if(theTime > (aAgent->herbivoreAgent->lastModeChange+HERBIVORE_GOAL_TIMEOUT_SECONDS))
	{
		//writeMessage("-> evaluate_current_goal() took too long trying to get to goal.\n", HERBIVORE_DEBUG);
		herbivore_lost_goal(self, (aAgent->herbivoreAgent->mode==HERBIVORE_FEED));
		return;
	}

	// make sure the goal still exists
	if(!self->enemy->inuse || 
		self->enemy->health<=0)
	{
		writeMessage("-> evaluate_current_goal() goal is dead.\n", HERBIVORE_DEBUG);
		herbivore_lost_goal(self, (aAgent->herbivoreAgent->mode==HERBIVORE_FEED));
		return;
	}

	// check if feeding, and had enough to eat
	if(aAgent->herbivoreAgent->mode == HERBIVORE_FEED)
	{
		if(!isHerbivoreHungry(self, HERBIVORE_HUNGRY_UPPER_THRESHOLD))
		{			
			//writeMessage("-> evaluate_current_goal() no longer hungry.\n", HERBIVORE_DEBUG);			
			herbivore_lost_goal(self, false);
			return;
		}
	}
}


/**
 * Called when the herbivore gets hurt, will ensure the herbivore
 * enters its flee mode, and get away from the danger
 *
 * Only enter flee mode if the herbivore has an enemy.
 * the enemy may or may not have been allocated by the 
 * M_ReactToDamage() function. Regardless, the herbivore is in
 * pain, and needs to get out of here.
 *
 */
void herbivore_flee(edict_t *self)
{
	generic_agent_t	*aAgent = castAgent(self->agent);

	// do not react when being born
	if(aAgent->herbivoreAgent->state == HERBIVORE_BIRTH_STATE)
	{
		self->enemy = NULL;
	}

	if(self->enemy)
	{
		// enter flee mode
		aAgent->herbivoreAgent->mode			= HERBIVORE_FLEE; // herbivore is in panic mode
		aAgent->herbivoreAgent->lastModeChange	= getTime(); //  a new goal eval has occured

		// get goal that is way out of here, may not get a goal if the herbivore is all by itself
		self->enemy = getRandomPlantFromClosest(self);

		// only if the herbivore did get a new goal
		if(self->enemy)
		{
			// need to ensure that when target is reached that it is NOT attacked - its a roam goal
			self->monsterinfo.aiflags &= ~AI_COMBAT_POINT; // not a combat target
			FoundTarget(self);
		}
		else
		{
			writeMessage("-> herbivore_flee() failed to locate a flee goal.\n", HERBIVORE_DEBUG);
		}
	}
}








// Below here is goal selection behavior
// ==============================================================================================================
//
//







/**
 * Called every time the herbivore is just standing around killing time.
 * 
 * @param edict_t : a herbivore agent
 * @return qboolean : true if the herbivore locates a target, otherwise false.
 */
qboolean herbivore_find_target(edict_t *self)
{
	generic_agent_t	*aAgent = castAgent(self->agent);	
	edict_t			*selectedGoal = NULL;
	long			theTime = getTime();

	// cannot look for targets whilst in birth state
	if(aAgent->herbivoreAgent->state == HERBIVORE_BIRTH_STATE)
	{
		return false; // wait 'till you grow up kid!
	}

	// this function cannot be called too often by the herbivore instance
	// a new goal can onlt be looked for every "HERBIVORE_NEW_GOAL_WAIT_SECONDS" seconds
	if(theTime < (aAgent->herbivoreAgent->lastModeChange + HERBIVORE_NEW_GOAL_WAIT_SECONDS))
	{
		return false; // wait some more
	}

	// remember the time
	aAgent->herbivoreAgent->lastModeChange = theTime;

	// determine if the herbivore is hungry
	if(isHerbivoreHungry(self, HERBIVORE_HUNGRY_LOWER_THRESHOLD))
	{		
		// use AI to select a good food target (food goal)
		selectedGoal = select_food_goal(self);
		
		if(selectedGoal != NULL) // check if a goal was selected
		{
			//writeMessage("-> herbivore_find_target() hungry and have a target.\n", HERBIVORE_DEBUG);				
			aAgent->herbivoreAgent->mode = HERBIVORE_FEED;	// mode change			
			aAgent->herbivoreAgent->numPlantBites = 0;// reset number of bites				
			self->enemy = selectedGoal; // remember food target and persue it				
			self->monsterinfo.aiflags &= ~AI_COMBAT_POINT; // not a combat target
			FoundTarget(self); // a target has been found
			return true;
		}
		writeMessage("-> herbivore_find_target() hungry but failed to select a GOOD target.\n", HERBIVORE_DEBUG);				

		// failed to locate a food goal, and the herbivore is hungry		
		//writeMessage("-> herbivore_find_target() hungry but without a target.\n", HERBIVORE_DEBUG);		
		aAgent->herbivoreAgent->mode = HERBIVORE_IDLE; // enter idle state		
		self->enemy = getRandomAgent(self); // get a roam goal

		if(self->enemy)
		{
			//writeMessage("-> herbivore_find_target() not hungry, got a roam goal.\n", HERBIVORE_DEBUG);
			self->monsterinfo.aiflags &= ~AI_COMBAT_POINT; // not a combat target
			FoundTarget(self); 
			return true;
		}
		else
		{
			writeMessage("-> herbivore_find_target() failed to find a roam goal - I AM HUNGRY.\n", HERBIVORE_DEBUG);
			return false;
		}
	}

	// the herbivore is not hungry or fleeing, become idle
	// if the herbivore was fleeing this function would not have been called
	aAgent->herbivoreAgent->mode = HERBIVORE_IDLE;

	// herbivore laziness
	if(RANDOM <= HERBIVORE_LAZINESS)	
	{		 
		self->enemy = getRandomPlantFromClosest(self); // get a food goal to walk to

		if(self->enemy)
		{
			//writeMessage("-> herbivore_find_target() not hungry, got a roam goal.\n", HERBIVORE_DEBUG);			
			self->monsterinfo.aiflags &= ~AI_COMBAT_POINT; // not a combat target
			FoundTarget(self); 
			return true;
		}
		else
		{
			writeMessage("-> herbivore_find_target() failed to find a roam goal - not hungry.\n", HERBIVORE_DEBUG);
		}
	}

	return false;
}






/**
 * select the closest agent to self in the level
 */
edict_t *getClosestAgent(edict_t *self)
{
	edict_t *selectedGoal = NULL; //  the selected goal agent
	float currentDistance = 0.0;
	float closestDistance = 9999999.9; 
	int totalAgents = 0; // total agents in collection	
	edict_t	*allAgents[MAX_AGENT_POPULATION]; // storage for all agents in world
	int i;

	// get a listing of all plants
	totalAgents = getAllAgentsOfType(allAgents, ALL_AGENT_TYPE, MAX_AGENT_POPULATION);

	// check that current agent is not only agent in the world
	if(totalAgents <= 1)
	{
		writeMessage("getClosestAgent()::no entities returned from getAllAgentsOfType()\n", HERBIVORE_DEBUG);
		return NULL;
	}	

	// go through all agents looking for the closest
	for(i=0; i<totalAgents; i++)
	{
		if(allAgents[i] == self)
		{
			continue; // never eval self as a target
		}

		currentDistance = UtilDistanceBetweenEntities(self, allAgents[i]);

		// check for something closer
		if(currentDistance < closestDistance)
		{
			selectedGoal = allAgents[i]; // new closest
			closestDistance = currentDistance;
		}
	}

	return selectedGoal;
}


/**
 * select a random agent that is NOT self, from all agents in the level
 */
edict_t *getRandomAgent(edict_t *self)
{
	int totalAgents = 0; // total agents in collection	
	edict_t	*allAgents[MAX_AGENT_POPULATION]; // storage for all agents in world
	edict_t *selectedGoal = NULL; //  the selected goal agent

	// get a listing of all plants
	totalAgents = getAllAgentsOfType(allAgents, ALL_AGENT_TYPE, MAX_AGENT_POPULATION);

	// check that current agent is not only agent in the world
	if(totalAgents <= 1)
	{
		writeMessage("getRandomAgent()::no entities returned from getAllAgentsOfType()\n", HERBIVORE_DEBUG);
		return NULL;
	}

	do
	{
		// select an agent from the list randomly
		selectedGoal = allAgents[getRandomInteger(0, totalAgents, FALSE)];
	}
	while(selectedGoal != self);

	return selectedGoal;
}



/**
 * select the closest plant to self in the level
 */
edict_t *getClosestPlant(edict_t *self)
{
	edict_t *selectedGoal = NULL; //  the selected goal agent
	float currentDistance = 0.0;
	float closestDistance = 9999999.9; 
	int totalAgents = 0; // total agents in collection	
	edict_t	*allAgents[MAX_PLANT_POPULATION]; // storage for all plants in world
	int i;

	// get a listing of all plants
	totalAgents = getAllAgentsOfType(allAgents, PLANT_AGENT_TYPE, MAX_PLANT_POPULATION);

	// check that there are plants
	if(totalAgents <= 0)
	{
		writeMessage("getClosestPlant()::no entities returned from getAllAgentsOfType()\n", HERBIVORE_DEBUG);
		return NULL;
	}

	// go through all agents looking for the closest
	for(i=0; i<totalAgents; i++)
	{
		currentDistance = UtilDistanceBetweenEntities(self, allAgents[i]);

		// check for something closer
		if(currentDistance < closestDistance)
		{
			selectedGoal = allAgents[i]; // new closest
			closestDistance = currentDistance;
		}
	}

	return selectedGoal;
}



/**
 * Select random plant from all plants
 */
edict_t *getRandomPlant()
{
	edict_t *selectedGoal = NULL; //  the selected goal agent
	int totalAgents = 0; // total agents in collection	
	edict_t	*allAgents[MAX_PLANT_POPULATION]; // storage for all plants in world

	// get a listing of all plants
	totalAgents = getAllAgentsOfType(allAgents, PLANT_AGENT_TYPE, MAX_PLANT_POPULATION);

	// check that there are plants
	if(totalAgents <= 0)
	{
		writeMessage("getRandomPlant()::no entities returned from getAllAgentsOfType()\n", HERBIVORE_DEBUG);
		return NULL;
	}	

	// select a plant from the list randomly
	selectedGoal = allAgents[getRandomInteger(0, totalAgents, FALSE)];
	return selectedGoal;
}


/**
 * Select random plant from the n closest plants
 */
edict_t *getRandomPlantFromClosest(edict_t *self)
{
	edict_t	*possibleFoodGoals[HERBIVORE_PERCEPTRON_MAX_GOALS];
	edict_t *selectedGoal = NULL; //  the selected goal agent
	int totalAgents = 0; // total agents in collection

	// get a collection of possible food targets that are close
	totalAgents = getNClosestPlants(possibleFoodGoals, HERBIVORE_PERCEPTRON_MAX_GOALS, self);

	// check that there are plants
	if(totalAgents <= 0)
	{
		writeMessage("getRandomPlantFromClosest()::no entities returned from getAllAgentsOfType()\n", HERBIVORE_DEBUG);
		return NULL;
	}	

	// select a plant from the list randomly
	selectedGoal = possibleFoodGoals[getRandomInteger(0, totalAgents, FALSE)];
	return selectedGoal;
}


/**
 * populate the provided array with pointers to the closest plants
 * to self. List will not exceed maxPlants.
 *
 * Not efficient but easy to follow
 *
 * returns number of elements populated into list (incase there is < maxPlants in world)
 */
int getNClosestPlants(edict_t *plantList[], int maxPlants, edict_t *self)
{
	//float iDistance, jDistance;
	edict_t *tmp;
	int i, j, returnCount;
	int totalAgents = 0; // total agents in collection	
	edict_t	*allAgents[MAX_PLANT_POPULATION]; // storage for all plants in world

	// get a listing of all plants
	totalAgents = getAllAgentsOfType(allAgents, PLANT_AGENT_TYPE, MAX_PLANT_POPULATION);

	// check that there are plants
	if(totalAgents <= 0)
	{
		writeMessage("getNClosestPlants()::no entities returned from getAllAgentsOfType()\n", HERBIVORE_DEBUG);
		return 0;
	}

	// bubble sort the listing by distance - bubble big distance to end of array	
	for(i=0; i<totalAgents-1; i++) 
	{		
		for(j=0; j<totalAgents-1-i; j++)
		{
			if(UtilDistanceBetweenEntities(self, allAgents[j+1]) < UtilDistanceBetweenEntities(self, allAgents[j])) 
			{  
				tmp = allAgents[j];         
				allAgents[j] = allAgents[j+1];
				allAgents[j+1] = tmp;
			}
		}
	}

	// populate the return structure
	returnCount = (totalAgents<maxPlants) ? totalAgents : maxPlants;

	for(i=0; i<returnCount; i++)
	{
		plantList[i] = allAgents[i];
	}

	return returnCount;
}


/**
 * Used when a herbivore has lost its goal, via a mechanism in this
 * file
 */
void herbivore_lost_goal(edict_t *self, 
						 qboolean wasFoodGoal)
{
	generic_agent_t	*aAgent = castAgent(self->agent); 	

	aAgent->herbivoreAgent->mode = HERBIVORE_IDLE; // prob not needed

	self->enemy = NULL;
	self->monsterinfo.pausetime = level.time + 100000000;
	self->monsterinfo.stand(self);
}






