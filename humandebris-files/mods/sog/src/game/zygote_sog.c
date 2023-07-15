/****************************************************
 *
 * FILE:	zygote_sog.c
 * AUTHOR:	Jason "Zygote" Brownlee
 * DATE:	26/01/2001 - Q3A:1.27g
 * MAIL:	hop_cha@hotmail.com
 * WEB:		http://www.planetquake.com/humandebris
 *
 * This file holds all necessary functions for the 
 * "Sog's Rifle" Quake3 Modification.
 ****************************************************
 */


#include "g_local.h"


/*
 ****************************************************
 * Function: Rifle_Fire()	
 * Author:		Jason "Zygote" Brownlee
 *
 * This function is based on the Bullet_Fire() function for 
 * the machinegun. It has been modified so that there is no
 * pread and 1000 points of damage per round.
 ****************************************************
 */
void Rifle_Fire(gentity_t *ent, vec3_t forward, vec3_t muzzle)
{
	trace_t		tr;
	vec3_t		end;
	gentity_t	*tent;
	gentity_t	*traceEnt;
	int			passent = ent->s.number;
	int			damage = 1000;

	VectorMA (muzzle, 8192, forward, end);
	trap_Trace (&tr, muzzle, NULL, NULL, end, passent, MASK_SHOT);

	if ( tr.surfaceFlags & SURF_NOIMPACT )
	{
		return;
	}

	traceEnt = &g_entities[ tr.entityNum ];

	// snap the endpos to integers, but nudged towards the line
	SnapVectorTowards( tr.endpos, muzzle );

	// send bullet impact
	if(traceEnt->takedamage && traceEnt->client)
	{
		tent = G_TempEntity( tr.endpos, EV_BULLET_HIT_FLESH );
		tent->s.eventParm = traceEnt->s.number;
		if( LogAccuracyHit( traceEnt, ent ) )
		{
			ent->client->accuracy_hits++;
		}
	}else{
		tent = G_TempEntity( tr.endpos, EV_BULLET_HIT_WALL );
		tent->s.eventParm = DirToByte( tr.plane.normal );
	}

	tent->s.otherEntityNum = ent->s.number;

	if(traceEnt->takedamage)
	{
		G_Damage( traceEnt, ent, ent, forward, tr.endpos,damage, 0, MOD_MACHINEGUN);
	}
}

