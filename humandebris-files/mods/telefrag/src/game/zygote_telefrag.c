/****************************************************
 *
 * FILE:	zygote_telefrag.c
 * AUTHOR:	Jason "Zygote" Brownlee
 * DATE:	26/01/2001 - Q3A:1.27g
 * MAIL:	hop_cha@hotmail.com
 * WEB:		http://www.planetquake.com/humandebris
 *
 * This file holds all necessary functions for the 
 * Telefrag Quake3 Modification.
 ****************************************************
 */


#include "g_local.h"


/*
 ****************************************************
 * Function:	Telefrag Player()
 * Author:		Jason "Zygote" Brownlee
 *
 * This function is based on the TeleportPlayer() function 
 * in g_misc.c.
 * The function has been modified so that it does not 
 * mofify player directions and so that it does not "spit"
 * the player out.
 ****************************************************
 */
void TelefragPlayer( gentity_t *player, vec3_t origin ) {
	gentity_t	*tent;

	// use temp events at source and destination to prevent the effect
	// from getting dropped by a second player event
	if ( player->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		tent = G_TempEntity( player->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
		tent->s.clientNum = player->s.clientNum;

		tent = G_TempEntity( origin, EV_PLAYER_TELEPORT_IN );
		tent->s.clientNum = player->s.clientNum;
	}

	// unlink to make sure it can't possibly interfere with G_KillBox
	trap_UnlinkEntity (player);

	VectorCopy ( origin, player->client->ps.origin );
	player->client->ps.origin[2] += 1;

	// toggle the teleport bit so the client knows to not lerp
	player->client->ps.eFlags ^= EF_TELEPORT_BIT;

	// kill anything at the destination
	if ( player->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		G_KillBox (player);
	}

	// save results of pmove
	BG_PlayerStateToEntityState( &player->client->ps, &player->s, qtrue );

	// use the precise origin for linking
	VectorCopy( player->client->ps.origin, player->r.currentOrigin );

	if ( player->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		trap_LinkEntity (player);
	}
}


/*
 ****************************************************
 * Function:	weapon_telefrag_fire()
 * Author:		Jason "Zygote" Brownlee
 *
 * This function is based on the weapon+reailgun_fire()
 * function. It has been modifed so that the projectile
 * will stop after hitting one person, and when it does
 * hit a player, it calls the above telefrag_player()
 * function.
 ****************************************************
 */
void weapon_telefrag_fire (gentity_t *ent,vec3_t muzzle,vec3_t forward,vec3_t right,vec3_t up)
{
	vec3_t		end;
	trace_t		trace;
	gentity_t	*tent;
	gentity_t	*traceEnt;
	int			damage = 1000;
	int			hits = 0;
	int			passent = ent->s.number;

	VectorMA (muzzle, 8192, forward, end);

	// Trace the projectile
	trap_Trace (&trace, muzzle, NULL, NULL, end, passent, MASK_SHOT );

	// The entity is a valid entity
	if ( trace.entityNum < ENTITYNUM_MAX_NORMAL ) {

		// Who exactly is this entity (a reference to the entity structure)
		traceEnt = &g_entities[ trace.entityNum ];

		// Can this entity be damaged?
		if ( traceEnt->takedamage )
		{
			if(LogAccuracyHit(traceEnt,ent))
			{
				hits++;
			}

			// It is important that all this checking is done. If you try and 
			// telefrag a door or a spectator, the game *will* crash.

			// is the entity a client, alive and not a spectator
			if((traceEnt->client) && (traceEnt->client->ps.pm_type != PM_DEAD) && (traceEnt->client->sess.sessionTeam != TEAM_SPECTATOR))
			{
				// if the attacker was on the same team and TF_NO_FRIENDLY_FIRE is set
				// do not telefrag, just do a normal damage
				if (OnSameTeam(traceEnt,ent) && (!g_friendlyFire.integer))
				{
					G_Damage (traceEnt, ent, ent, forward, trace.endpos, damage, 0, MOD_RAILGUN);

				// Not on same team, or on same team and can friendly fire
				// Damage, then telefrag
				}else{
					G_Damage (traceEnt, ent, ent, forward, trace.endpos, damage, 0, MOD_RAILGUN);
					TelefragPlayer(ent, traceEnt->r.currentOrigin);
				}
			}else{
				// Damage 
				G_Damage (traceEnt, ent, ent, forward, trace.endpos, damage, 0, MOD_RAILGUN);
			}
		}
	}

	// snap the endpos to integers to save net bandwidth, but nudged towards the line
	SnapVectorTowards( trace.endpos, muzzle );

	// send railgun beam effect
	tent = G_TempEntity( trace.endpos, EV_RAILTRAIL );

	// set player number for custom colors on the railtrail
	tent->s.clientNum = ent->s.clientNum;

	VectorCopy( muzzle, tent->s.origin2 );
	// move origin a bit to come closer to the drawn gun muzzle
	VectorMA( tent->s.origin2, 4, right, tent->s.origin2 );
	VectorMA( tent->s.origin2, -1, up, tent->s.origin2 );

	// no explosion at end if SURF_NOIMPACT, but still make the trail
	if ( trace.surfaceFlags & SURF_NOIMPACT )
	{
		tent->s.eventParm = 255;	// don't make the explosion at the end
	} else {
		tent->s.eventParm = DirToByte( trace.plane.normal );
	}
	tent->s.clientNum = ent->s.clientNum;

	// give the shooter a reward sound if they have made two railgun hits in a row
	if ( hits == 0 )
	{
		// complete miss
		ent->client->accurateCount = 0;
	} else {
		// check for "impressive" reward sound
		ent->client->accurateCount += hits;
		if ( ent->client->accurateCount >= 2 )
		{
			ent->client->accurateCount -= 2;
			ent->client->ps.persistant[PERS_IMPRESSIVE_COUNT]++;
			// add the sprite over the player's head
			ent->client->ps.eFlags &= ~(EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP );
			ent->client->ps.eFlags |= EF_AWARD_IMPRESSIVE;
			ent->client->rewardTime = level.time + REWARD_SPRITE_TIME;
		}
		ent->client->accuracy_hits++;
	}
}
