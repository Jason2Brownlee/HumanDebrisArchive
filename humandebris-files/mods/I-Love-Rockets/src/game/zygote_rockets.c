/****************************************************
 *
 * FILE:	zygote_rockets.c
 * AUTHOR:	Jason "Zygote" Brownlee
 * DATE:	28/01/2001 - Q3A:1.27g
 * MAIL:	hop_cha@hotmail.com
 * WEB:		http://www.planetquake.com/humandebris
 *
 * This file holds all necessary functions for the 
 * "I Love Rockets" Quake3 Modification.
 ****************************************************
 */


#include "g_local.h"

#define	MISSILE_PRESTEP_TIME	50	// Zygote - Needed for rocket functions

/*
 ****************************************************
 * Function:	RocketLauncher_Fire()	
 * Author:		Jason "Zygote" Brownlee
 *
 * This function is based on the Weapon_RocketLauncher_Fire()
 * function in g_weapons.c. The difference is that this function
 * takes a pointer to a function.
 ****************************************************
 */
/* Zygote - Moved into g_weapons.c
void RocketLauncher_Fire (gentity_t *ent, gentity_t *(*f)(gentity_t *self, vec3_t start, vec3_t dir))
{
	gentity_t	*m;

	// call the function passed in as an argument
	m = f(ent, muzzle, forward);

	m->damage *= s_quadFactor;
	m->splashDamage *= s_quadFactor;
}
*/

/*
 ****************************************************
 * Function:	findradius()	
 * Author:		Nobody - www.quakestyle.telefragged.com
 *
 * Function used with the homing rocket launcher.
 ****************************************************
 */
gentity_t *findradius (gentity_t *from, vec3_t org, float rad)
{
	vec3_t	eorg;
	int		j;

	if (!from)
		from = g_entities;
	else
		from++;

	for (; from < &g_entities[level.num_entities]; from++)
	{
		if (!from->inuse)
			continue;
		for (j=0; j<3; j++)
			eorg[j] = org[j] - (from->r.currentOrigin[j] + (from->r.mins[j] + from->r.maxs[j])*0.5);
		if (VectorLength(eorg) > rad)
			continue;
		return from;
	}

	return NULL;
}


/*
 ****************************************************
 * Function:	visible()	
 * Author:		Nobody - www.quakestyle.telefragged.com
 *
 * Function used with the homing rocket launcher.
 ****************************************************
 */
qboolean visible( gentity_t *ent1, gentity_t *ent2 ) {
        trace_t         trace;

        trap_Trace (&trace, ent1->s.pos.trBase, NULL, NULL, ent2->s.pos.trBase, ent1->s.number, MASK_SHOT );

        if ( trace.contents & CONTENTS_SOLID ) {
                return qfalse;
        }

        return qtrue;
}


/*
 ****************************************************
 * Function:	G_HomingMissile()	
 * Author:		www.quakestyle.telefragged.com
 *
 * Function used with the homing rocket launcher.
 ****************************************************
 */
void G_HomingMissile( gentity_t *ent )
{
	gentity_t       *target = NULL;
	gentity_t       *blip = NULL;
	vec3_t  dir, blipdir, temp_dir;

	while ((blip = findradius(blip, ent->r.currentOrigin, 2000)) != NULL)
	{
		if (blip->client==NULL)                 continue;
		if (blip==ent->parent)                  continue;
		if (blip->health<=0)                    continue;
		if (blip->client->sess.sessionTeam == TEAM_SPECTATOR)
			continue;

		if ((g_gametype.integer == GT_TEAM || g_gametype.integer == GT_CTF) &&
			blip->client->sess.sessionTeam == ent->parent->client->sess.sessionTeam)
			continue;

		//in old code,this ent->parent->cliend-> was blip->parent->client->,
		//so didn't work in CTF and team deathmatch.Now it will work.
		if (!visible (ent, blip))
				continue;

		VectorSubtract(blip->r.currentOrigin, ent->r.currentOrigin, blipdir);
		blipdir[2] += 16;
		if ((target == NULL) || (VectorLength(blipdir) < VectorLength(dir)))
		{
			//if new target is the nearest
			VectorCopy(blipdir,temp_dir);
			VectorNormalize(temp_dir);
			VectorAdd(temp_dir,ent->r.currentAngles,temp_dir);      
			//now the longer temp_dir length is the more straight path for the rocket.
			if(VectorLength(temp_dir)>1.6)
			{       
				//if this 1.6 were smaller,the rocket also get to target the enemy on his back.
				target = blip;
				VectorCopy(blipdir, dir);
			}
		}
	}

	if (target == NULL)
	{       
		ent->nextthink = level.time + 10000;
		// if once the rocket lose target,it will not search new enemy any more,and go away.
	} else {
		ent->s.pos.trTime=level.time;
		VectorCopy(ent->r.currentOrigin, ent->s.pos.trBase );
		//for exact trajectory calculation,set current point to base.
                        
		VectorNormalize(dir);
		VectorScale(dir, 0.3,dir);
		VectorAdd(dir,ent->r.currentAngles,dir);
		// this 0.3 is swing rate.this value is cheap,I think.try 0.8 or 1.5.
		// if you want fastest swing,comment out these 3 lines.

		VectorNormalize(dir);
		VectorCopy(dir,ent->r.currentAngles);
		//locate nozzle to target

		VectorScale (dir,VectorLength(ent->s.pos.trDelta)*1.1,ent->s.pos.trDelta);
		//trDelta is actual vector for movement.Because the rockets slow down
		// when swing large angle,so accelalate them.

		SnapVector (ent->s.pos.trDelta);                      // save net bandwidth
		ent->nextthink = level.time + 100;      //decrease this value also makes fast swing.
	}
}


/*
 ****************************************************
 * Function:	G_GuideMissile()	
 * Author:		Jason "Zygote" Brownlee
 *
 * This function is called mant times a second, and
 * is used to guide a missile. (laser guided missile).
 * This function is called by laser guided rockets.
 ****************************************************
 */
void G_GuideMissile( gentity_t *ent )
{
	vec3_t          dir, forward, right, up;

	if (ent->parent->client->ps.pm_type == PM_DEAD)
	{
		// normal rocket again
		ent->nextthink = level.time + 2000;
		ent->think = G_ExplodeMissile;
		return;
	} else {

		ent->s.pos.trTime=level.time;
		VectorCopy(ent->r.currentOrigin, ent->s.pos.trBase );
		
		// Where is my owner looking?
		AngleVectors (ent->parent->client->ps.viewangles, forward, right, up);
		// Copy foward into dir
		VectorCopy(forward, dir);

		VectorScale (dir,VectorLength(ent->s.pos.trDelta)*1.1,ent->s.pos.trDelta);
		// trDelta is actual vector for movement.Because the rockets slow down
		// when swing large angle,so accelalate them.

		SnapVector (ent->s.pos.trDelta); // save net bandwidth

		if (level.time > ent->wait) // kill it after 5 seconds
		{
			ent->nextthink = level.time + 10;
			ent->think = G_ExplodeMissile;
			return;
		}
		ent->nextthink = 50; // do it again
	}
}


/*
 ****************************************************
 * Function:	fire_rocket_gravity()	
 * Author:		Jason "Zygote" Brownlee
 *
 * This function is based on the fire_rocket() function 
 * in g_missile.c. The difference is that this rocket
 * is effected by gravity.
 ****************************************************
 */
gentity_t *fire_rocket_gravity (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*bolt;

	VectorNormalize (dir);
	bolt = G_Spawn();
	bolt->classname = "gravity";			// Zygote - changed from rockets
	bolt->nextthink = level.time + 10000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_ROCKET_LAUNCHER;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 100;
	bolt->splashDamage = 100;
	bolt->splashRadius = 120;
	bolt->methodOfDeath = MOD_ROCKET;
	bolt->splashMethodOfDeath = MOD_ROCKET_SPLASH;
	bolt->clipmask = MASK_SHOT;

	bolt->s.pos.trType = TR_GRAVITY;		// Zygote - Effected By Gravity

	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, 900, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
	return bolt;
}


/*
 ****************************************************
 * Function:	G_HomingMissile()	
 * Author:		www.quakestyle.telefragged.com
 *
 * Function used with the homing rocket launcher.
 ****************************************************
 */
gentity_t *fire_rocket_homing (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*bolt;

	VectorNormalize (dir);
	bolt = G_Spawn();

	bolt->classname = "homing";			// Zygote - Changed from rockets
	bolt->nextthink = level.time + 60;	// Zygote - Changed from 10000
	bolt->think = G_HomingMissile;		// Zygote - Changed from G_ExplodeMissile()    

	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_ROCKET_LAUNCHER;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 100;
	bolt->splashDamage = 100;
	bolt->splashRadius = 120;
	bolt->methodOfDeath = MOD_ROCKET;
	bolt->splashMethodOfDeath = MOD_ROCKET_SPLASH;
	bolt->clipmask = MASK_SHOT;
	bolt->s.pos.trType = TR_LINEAR; 
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, 900, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
	VectorCopy (dir, bolt->r.currentAngles); // Rockets
	return bolt;
}


/*
 ****************************************************
 * Function:	fire_rocket_rapid()	
 * Author:		Jason "Zygote" Brownlee
 *
 * This function is based on the fire_rocket() function 
 * in g_missile.c. The difference is that these rockets
 * fire a lot faster (both fire speed and travel speed)
 * and they hurt less.
 ****************************************************
 */
gentity_t *fire_rocket_rapid (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "rocket";
	bolt->nextthink = level.time + 10000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_ROCKET_LAUNCHER;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;

	bolt->damage = 50;							// Zygote - Changed from 100 
	bolt->splashDamage = 50;					// Zygote - Changed from 100
	bolt->splashRadius = 70;					// Zygote - Changed from 120

	bolt->methodOfDeath = MOD_ROCKET;
	bolt->splashMethodOfDeath = MOD_ROCKET_SPLASH;
	bolt->clipmask = MASK_SHOT;
	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, 1500, bolt->s.pos.trDelta );	//Zygote - Changed from 1000
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}


/*
 ****************************************************
 * Function:	fire_rocket_laser()	
 * Author:		Jason "Zygote" Brownlee
 *
 * This function is based on the fire_rocket() function 
 * in g_missile.c. These rockets are guided by the player 
 * Half-Life style.
 ****************************************************
 */
gentity_t *fire_rocket_laser (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();

	bolt->classname = "laser";			// Zygote - Changed from rocket
	bolt->nextthink = level.time + 10;	// Zygote - Changed from 1000
	bolt->think = G_GuideMissile;		// Zygote - Changed from G_ExplodeMissile
	bolt->wait = level.time + 5000;		// Zygote - Added, rockets explode after 5 seconds.

	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_ROCKET_LAUNCHER;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 100; 
	bolt->splashDamage = 100;
	bolt->splashRadius = 120;

	bolt->methodOfDeath = MOD_ROCKET;
	bolt->splashMethodOfDeath = MOD_ROCKET_SPLASH;
	bolt->clipmask = MASK_SHOT;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, 1000, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}


