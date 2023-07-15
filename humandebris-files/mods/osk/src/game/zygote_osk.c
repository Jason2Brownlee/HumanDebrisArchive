/****************************************************
 *
 * FILE:	zygote_osk.c
 * AUTHOR:	Jason "Zygote" Brownlee
 * DATE:	28/01/2001 - Q3A:1.27g
 * MAIL:	hop_cha@hotmail.com
 * WEB:		http://www.planetquake.com/humandebris
 *
 * This file holds all necessary functions for the 
 * "One Shot Kills" Quake3 Modification.
 ****************************************************
 */


#include "g_local.h"


#define	MISSILE_PRESTEP_TIME	50		// Zygote - required for some missile functions

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

/*
 ****************************************************
 * Function:	bounce_plasma()
 * Author:		Jason "Zygote" Brownlee
 *
 * This function is based on the fire_plasma()
 * function. It has been modifed so that the projectile
 * will bounce when it hits a wall
 ****************************************************
 */
gentity_t *bounce_plasma (gentity_t *self, vec3_t start, vec3_t dir)
{
	gentity_t	*bolt;

	VectorNormalize (dir);
	bolt = G_Spawn();
	bolt->classname = "plasma";
	bolt->nextthink = level.time + 1000;		// Zygote - bounce for 1 second
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_PLASMAGUN;	
	bolt->s.eFlags = EF_BOUNCE;					// Zygote - make it bounce
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 1000;						// Zygote - Hurts a lot more
	bolt->splashDamage = 0;						// Zygote - No splash
	bolt->splashRadius = 0;						// Zygote - No splash
	bolt->methodOfDeath = MOD_PLASMA;
	bolt->splashMethodOfDeath = MOD_PLASMA_SPLASH;
	bolt->clipmask = MASK_SHOT;
	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, 1500, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
	return bolt;
}


/*
 ****************************************************
 * Function:	fire_jump_and_gib()
 * Author:		Jason "Zygote" Brownlee
 *
 * This function is based on the fire_bfg()
 * function. It has been modifed so that the projectile
 * hurts more and moves faster
 ****************************************************
 */
gentity_t *fire_jump_and_gib (gentity_t *self, vec3_t start, vec3_t dir)
{
	gentity_t	*bolt;

	VectorNormalize (dir);
	bolt = G_Spawn();
	bolt->classname = "bfg";
	bolt->nextthink = level.time + 10000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_BFG;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;

	bolt->damage = 1000;						// Zygote - Hurts a lot more
	bolt->splashDamage = 0;						// Zygote - No splash
	bolt->splashRadius = 0;						// Zygote - No splash

	bolt->methodOfDeath = MOD_BFG;
	bolt->splashMethodOfDeath = MOD_BFG_SPLASH;
	bolt->clipmask = MASK_SHOT;
	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );

	// VectorScale( dir, 2000, bolt->s.pos.trDelta );
	VectorScale( dir, 50000, bolt->s.pos.trDelta );		// Zygote - Speed it up

	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
	return bolt;
}

/*
 ****************************************************
 * Function:	fire_delayed()
 * Author:		Jason "Zygote" Brownlee
 *
 * This function is based on the fire_bfg()
 * function. It has been modifed so that the projectile
 * hurts more and moves faster
 ****************************************************
 */
gentity_t *fire_delayed(gentity_t *self, vec3_t start, vec3_t dir)
{
	gentity_t	*bolt;

	VectorNormalize (dir);
	bolt = G_Spawn();
	bolt->classname = "delayed";						// Zygote - changed the name
	bolt->nextthink = level.time + 10000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_BFG;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;

	bolt->damage = 1000;						// Zygote - Hurts a lot more
	bolt->splashDamage = 0;						// Zygote - No splash
	bolt->splashRadius = 0;						// Zygote - No splash

	bolt->methodOfDeath = MOD_BFG;
	bolt->splashMethodOfDeath = MOD_BFG_SPLASH;
	bolt->clipmask = MASK_SHOT;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );

	//VectorScale( dir, 2000, bolt->s.pos.trDelta );
	VectorScale( dir, 50000, bolt->s.pos.trDelta ); // OSK Speed it up

	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
	return bolt;
}


/*
 ****************************************************
 * Function:	G_MissileImpact_New()
 * Author:		Jason "Zygote" Brownlee
 *
 * This function is based on the G_MissileImpact()
 * function. It has been modifed so that it can handle
 * all the different bfg-instagib types.
 ****************************************************
 */
void G_MissileImpact_New( gentity_t *ent, trace_t *trace )
{
	gentity_t		*other;
	qboolean		hitClient = qfalse;

	// Zygote - added
	static	vec3_t	forward, right, up;			

	other = &g_entities[trace->entityNum];

	// check for bounce
	if ( !other->takedamage &&
		( ent->s.eFlags & ( EF_BOUNCE | EF_BOUNCE_HALF ) ) ) {
		G_BounceMissile( ent, trace );
		G_AddEvent( ent, EV_GRENADE_BOUNCE, 0 );
		return;
	}

	
// Zygote Start
	// Jump and GIB
	if (!strcmp(ent->classname, "bfg")) {
		if ((other->takedamage) && (other->client) && (other->client->ps.pm_type != PM_DEAD) && (other->client->sess.sessionTeam != TEAM_SPECTATOR)) { 
			if (other->client->delayswitch == qfalse) { // false hit em
				if ((OnSameTeam (other, ent->parent)) && (!g_friendlyFire.integer)) { // if "other" is on the same team and frendly fire is off
				G_Damage (other, ent, &g_entities[ent->r.ownerNum], NULL, ent->s.origin, ent->damage, 0, ent->methodOfDeath); // do a normal hurt
				} else { // can really hurt them (dealyed)
					// Jump Test
					// set aiming directions
					AngleVectors (ent->parent->client->ps.viewangles, forward, right, up);
					VectorNormalize(up);
					VectorScale( up, 1000, other->client->ps.velocity ); // go up fast!
					other->client->ps.pm_time = 50;		// hold time
					other->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
					// Get on with it...
					other->client->delaytime = level.time + 1000; // 1000 == 1 second (SET TIME)
					other->client->delayeffect = qtrue; // effect before death
					other->client->delayattacker = &g_entities[ent->r.ownerNum]; //owner of missile
					// other->takedamage = qfalse; //  not needed
					other->client->delayswitch = qtrue; // this should go last
					if( LogAccuracyHit( other, &g_entities[ent->r.ownerNum] ) ) {
						g_entities[ent->r.ownerNum].client->accuracy_hits++;
						hitClient = qtrue;
					}
				}
			} 
		}
	} 


	// Delayed + CLASSIC
	if (!strcmp(ent->classname, "delayed")) {
		if ((other->takedamage) && (other->client) && (other->client->ps.pm_type != PM_DEAD) && (other->client->sess.sessionTeam != TEAM_SPECTATOR)) { 
			if (other->client->delayswitch == qfalse) { // false hit em
				if ((OnSameTeam (other, ent->parent)) && (!g_friendlyFire.integer)) { // if "other" is on the same team and frendly fire is off
					G_Damage (other, ent, &g_entities[ent->r.ownerNum], NULL, ent->s.origin, ent->damage, 0, ent->methodOfDeath); // do a normal hurt
				} else { // can really hurt them (dealyed)
					other->client->delaytime = level.time + 5000; // 1000 == 1 second (SET TIME)
					other->client->delayblasteffect = qtrue; // effect before death
					other->client->delayattacker = &g_entities[ent->r.ownerNum]; //owner of missile
					other->client->delayswitch = qtrue; // this should go last
					if( LogAccuracyHit( other, &g_entities[ent->r.ownerNum] ) ) {
						g_entities[ent->r.ownerNum].client->accuracy_hits++;
						hitClient = qtrue;
					}
				}
			} else { // they have already been hit
				if ((OnSameTeam (other, ent->parent)) && (!g_friendlyFire.integer)) { // if "other" is on the same team and frendly fire is off
					G_Damage (other, ent, &g_entities[ent->r.ownerNum], NULL, ent->s.origin, ent->damage, 0, ent->methodOfDeath); // do a normal hurt
				} else { // can really hurt them again (dealyed (CLASSIC))
					switch (g_oskmode.integer) { // pick you style!!!
					case 6: // CLASSIC
						if (other->client->ps.powerups[PW_QUAD] > level.time && other->client->delayhitagain == qfalse) { // make sure still alive and not been hit twice
							other->client->delayattacker = &g_entities[ent->r.ownerNum]; //owner of missile
							other->client->delayhitagain = qtrue;
							if( LogAccuracyHit( other, &g_entities[ent->r.ownerNum] ) ) {
								g_entities[ent->r.ownerNum].client->accuracy_hits++;
								hitClient = qtrue;
							}
						}
						break;
					case 8: // INVISIBLE
						if (other->client->ps.powerups[PW_INVIS] > level.time && other->client->delayhitagain == qfalse) { // make sure still alive and not been hit twice
							other->client->delayattacker = &g_entities[ent->r.ownerNum]; //owner of missile
							other->client->delayhitagain = qtrue;
							if( LogAccuracyHit( other, &g_entities[ent->r.ownerNum] ) ) {
								g_entities[ent->r.ownerNum].client->accuracy_hits++;
								hitClient = qtrue;
							}
						}						
						break;
					case 9: // HASTE
						if (other->client->ps.powerups[PW_HASTE] > level.time && other->client->delayhitagain == qfalse) { // make sure still alive and not been hit twice
							other->client->delayattacker = &g_entities[ent->r.ownerNum]; //owner of missile
							other->client->delayhitagain = qtrue;
							if( LogAccuracyHit( other, &g_entities[ent->r.ownerNum] ) ) {
								g_entities[ent->r.ownerNum].client->accuracy_hits++;
								hitClient = qtrue;
							}
						}						
						break;
					case 10: // BLAST!
						if (other->client->ps.powerups[PW_BATTLESUIT] > level.time && other->client->delayhitagain == qfalse) { // make sure still alive and not been hit twice
							other->client->delayattacker = &g_entities[ent->r.ownerNum]; //owner of missile
							other->client->delayhitagain = qtrue;
							if( LogAccuracyHit( other, &g_entities[ent->r.ownerNum] ) ) {
								g_entities[ent->r.ownerNum].client->accuracy_hits++;
								hitClient = qtrue;
							}
						}						
						break;
					default:
						break;
					}				
					
				}
			}
		}
	} 
// Zygote Emd

	// impact damage
	// Zygote Modified
	if ((other->takedamage) && !(!strcmp(ent->classname, "bfg")) && !(!strcmp(ent->classname, "delayed"))) {
		// FIXME: wrong damage direction?
		if ( ent->damage ) {
			vec3_t	velocity;

			if( LogAccuracyHit( other, &g_entities[ent->r.ownerNum] ) ) {
				g_entities[ent->r.ownerNum].client->accuracy_hits++;
				hitClient = qtrue;
			}
			BG_EvaluateTrajectoryDelta( &ent->s.pos, level.time, velocity );
			if ( VectorLength( velocity ) == 0 ) {
				velocity[2] = 1;	// stepped on a grenade
			}
			G_Damage (other, ent, &g_entities[ent->r.ownerNum], velocity,
				ent->s.origin, ent->damage, 
				0, ent->methodOfDeath);
		}
	}

	// is it cheaper in bandwidth to just remove this ent and create a new
	// one, rather than changing the missile into the explosion?

	if ( other->takedamage && other->client ) {
		G_AddEvent( ent, EV_MISSILE_HIT, DirToByte( trace->plane.normal ) );
		ent->s.otherEntityNum = other->s.number;
	} else {
		G_AddEvent( ent, EV_MISSILE_MISS, DirToByte( trace->plane.normal ) );
	}

	ent->freeAfterEvent = qtrue;

	// change over to a normal entity right at the point of impact
	ent->s.eType = ET_GENERAL;

	SnapVectorTowards( trace->endpos, ent->s.pos.trBase );	// save net bandwidth

	G_SetOrigin( ent, trace->endpos );

	// splash damage (doesn't apply to person directly hit)
	if ( ent->splashDamage ) {
		if( G_RadiusDamage( trace->endpos, ent->parent, ent->splashDamage, ent->splashRadius, 
			other, ent->splashMethodOfDeath ) ) {
			if( !hitClient ) {
				g_entities[ent->r.ownerNum].client->accuracy_hits++;
			}
		}
	}

	trap_LinkEntity( ent );
}
