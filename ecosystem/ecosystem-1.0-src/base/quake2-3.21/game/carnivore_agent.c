
// carnivore_agent.c


/**
 * Description: The carnivore agent
 *
 * Author: Jason Brownlee - 02-08-2002
 *
 * Change History
 * Who		When		Description
 * --------------------------------------------------------------------
 *
 *
 *
 */


#include "generic_agent.h"


//
// carnivore function prototypes
//
void spawn_carnivore(edict_t *self);
void spawn_carnivore_reproduce(edict_t *self, carnivore_agent_t *parent1, carnivore_agent_t *parent2);

void carnivore_hunt_state(edict_t *self);
void carnivore_reproduce_state(edict_t *self);
void carnivore_idle_state(edict_t *self);

void carnivore_find_target(edict_t *self);
void carnivore_maturity_check(edict_t *self);
void carnivore_reproduce(edict_t *self, carnivore_agent_t *mate);
void carnivore_state_evaluation(edict_t *self);

// fuzzy membership functions
float carnivore_hunt_membership(edict_t *self);
float carnivore_mate_membership(edict_t *self);
float carnivore_idle_membership(edict_t *self);
float carnivore_threat_membership(edict_t *self);
float carnivore_range_membership(edict_t *self);
float carnivore_age_membership(edict_t *self);

// monster framework functions
void carnivore_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void carnivore_pain(edict_t *self, edict_t *other, float kick, int damage);

void carnivore_stand(edict_t *self);
void carnivore_walk(edict_t *self);
void carnivore_run(edict_t *self);
void carnivore_attack(edict_t *self);






