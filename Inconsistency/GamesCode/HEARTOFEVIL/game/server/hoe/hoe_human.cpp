#include "cbase.h"
#include "ai_senses.h"
#include "ai_tacticalservices.h"
#include "animation.h"
#include "basegrenade_shared.h"
#include "basehlcombatweapon_shared.h"
#include "grenade_frag.h"
#include "npcevent.h"
#include "hoe_human.h"
#include "hoe_corpse.h"
#include "RagdollBoogie.h"
#include "schedule_hacks.h"
#include "gib.h"
#include "EnvMessage.h"
#include "EventQueue.h"
#ifdef LOGIC_HUEY_DEPLOY
#include "logic_huey_deploy.h"
#endif
#include "physics_prop_ragdoll.h"
#include "hl2_player.h"
#include "bone_setup.h"
#include "effect_dispatch_data.h"
#include "te_effect_dispatch.h"
#include "hl2_shareddefs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern bool IsHumanTalking( void );
bool HOE_IsHuman( CBaseEntity *pEnt );

extern CBaseEntity *CreateCorpseSolver( CBaseEntity *pCorpse, CBaseEntity *pPhysicsObject, bool disableCollisions, float separationDuration );

#define INVALID_ATTACHMENT 0

#define HOECOLLISION_GROUP_CORPSE (HL2COLLISION_GROUP_COMBINE_BALL_NPC+1)

#define COMBINE_GRENADE_THROW_SPEED 650
#define COMBINE_GRENADE_TIMER		3.5
#define COMBINE_GRENADE_FLUSH_TIME	3.0		// Don't try to flush an enemy who has been out of sight for longer than this.
#define COMBINE_GRENADE_FLUSH_DIST	256.0	// Don't try to flush an enemy who has moved farther than this distance from the last place I saw him.
#define	COMBINE_MIN_GRENADE_CLEAR_DIST	250

//-----------------------------------------------------------------------------
// Spawnflags
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Animation events.
//-----------------------------------------------------------------------------
Animevent CHOEHuman::AE_NPC_KICK;
Animevent CHOEHuman::AE_HUMAN_GRENADE_TOSS;
Animevent CHOEHuman::AE_HUMAN_GRENADE_DROP;
Animevent CHOEHuman::AE_HUMAN_OPEN_VEHICLE;
Animevent CHOEHuman::AE_HUMAN_CLOSE_VEHICLE;

//-----------------------------------------------------------------------------
// Tasks
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Activities
//-----------------------------------------------------------------------------
Activity CHOEHuman::ACT_BARNACLE_RELEASED;
Activity CHOEHuman::ACT_BARNACLE_HIT_GROUND;
Activity CHOEHuman::ACT_EXPLOSION_HIT;
Activity CHOEHuman::ACT_EXPLOSION_FLY;
Activity CHOEHuman::ACT_WALK_CROUCH_HURT;
Activity CHOEHuman::ACT_RUN_CROUCH_HURT;
Activity CHOEHuman::ACT_TURN_LEFT_LOW;
Activity CHOEHuman::ACT_TURN_RIGHT_LOW;
Activity CHOEHuman::ACT_SIGNAL1_LOW;
Activity CHOEHuman::ACT_SIGNAL2_LOW;
Activity CHOEHuman::ACT_SIGNAL3_LOW;
Activity CHOEHuman::ACT_SMALL_FLINCH_LOW;
Activity CHOEHuman::ACT_IDLE_ANGRY_LOW;

//=========================================================
// Interactions
//=========================================================
int	g_interactionHumanMedicHeal = 0;
int	g_interactionHumanGrenade = 0;

extern int g_interactionBarnacleVictimBite;
extern int g_interactionBarnacleVictimGrab;
extern int g_interactionBarnacleVictimReleased;

//---------------------------------------------------------
// 
//---------------------------------------------------------
//IMPLEMENT_SERVERCLASS_ST(CHOEHuman, DT_NPC_HGrunt)
//END_SEND_TABLE()

BEGIN_DATADESC( CHOEHuman )
	DEFINE_KEYFIELD( m_bGameEndAlly, FIELD_BOOLEAN, "GameEndAlly" ),
	DEFINE_KEYFIELD( m_iszDeathMessage, FIELD_STRING, "DeathMessage" ),
#ifdef LOGIC_HUEY_DEPLOY
	DEFINE_KEYFIELD( m_iszDeployLogic, FIELD_STRING, "DeployLogic" ),
#endif

	DEFINE_FIELD( m_nHeadNum, FIELD_INTEGER),
#ifdef HOE_HUMAN_RR
	DEFINE_FIELD( m_flTimeInjuriesMentioned, FIELD_TIME ),
#else
	DEFINE_FIELD( m_flStopTalkTime, FIELD_TIME ),
	DEFINE_FIELD( m_flLastTalkTime, FIELD_TIME ),
	DEFINE_FIELD( m_flPainTime, FIELD_TIME ),
	DEFINE_FIELD( m_iSpokenConcepts, FIELD_INTEGER ),
#endif
	DEFINE_FIELD( m_flDelayedPainSoundTime, FIELD_TIME ),
	DEFINE_FIELD( m_flScriptedSentence, FIELD_TIME ),
	DEFINE_FIELD( m_fHandGrenades, FIELD_BOOLEAN ),
//	DEFINE_ENTITYFUNC( ExplFlyTouch ),
	DEFINE_FIELD( m_flAttackedByHueyTime, FIELD_TIME ),
	DEFINE_FIELD( m_flTimeLastRegen, FIELD_TIME ),
	DEFINE_FIELD( m_hTalkTarget, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hListenTarget, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hMedicThatHealedMe, FIELD_EHANDLE ),
	DEFINE_FIELD( m_flCrouchTime, FIELD_TIME ),
	DEFINE_FIELD( m_bStopCrouching, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bCrouchLocked, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flLastMedicSearch, FIELD_TIME ),
	DEFINE_FIELD( m_flLastMedicHealed, FIELD_TIME ),
	DEFINE_FIELD( m_flKilledEnemyTime, FIELD_TIME ),
	DEFINE_FIELD( m_flLastKick, FIELD_TIME ),
	DEFINE_FIELD( m_flLastFledZombieTime, FIELD_TIME ),
	DEFINE_FIELD( m_nBarnacleState, FIELD_INTEGER ),
	DEFINE_FIELD( m_bFollowingTarget, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flTimePlayerStartStare, FIELD_TIME ),
	DEFINE_FIELD( m_flIdleSpeechDelay, FIELD_TIME ),
	DEFINE_FIELD( m_flTimeScannedCorpses, FIELD_TIME ),
	DEFINE_FIELD( m_flTimeScannedSmells, FIELD_TIME ),
	DEFINE_FIELD( m_bShouldBoogie, FIELD_BOOLEAN ),

	DEFINE_FIELD( m_GrenadeInteraction.vecOrigin, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_GrenadeInteraction.flRadius, FIELD_FLOAT ),
	DEFINE_FIELD( m_GrenadeInteraction.flTime, FIELD_TIME ),
	DEFINE_FIELD( m_GrenadeInteraction.bRunning, FIELD_BOOLEAN ),

	DEFINE_FIELD( m_fHandGrenades, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flNextGrenadeCheck, FIELD_TIME ),
	DEFINE_FIELD( m_vecTossVelocity, FIELD_VECTOR ),

	DEFINE_FIELD( m_nLastSquadCommand, FIELD_INTEGER ),
	DEFINE_FIELD( m_fSquadCmdAcknowledged, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flLastSquadCmdTime, FIELD_TIME ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Behead", InputBehead ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Crouch", InputCrouch ),
	DEFINE_INPUTFUNC( FIELD_STRING, "EnterVehicle", InputEnterVehicle ),
	DEFINE_INPUTFUNC( FIELD_STRING, "EnterVehicleImmediate", InputEnterVehicleImmediate ),	DEFINE_INPUTFUNC( FIELD_VOID,	"ExitVehicle",				InputExitVehicle ),
	DEFINE_INPUTFUNC( FIELD_VOID, "ExitVehicle", InputExitVehicle ),
	DEFINE_INPUTFUNC( FIELD_VOID, "FinishedEnterVehicle", InputFinishedEnterVehicle ),
	DEFINE_INPUTFUNC( FIELD_VOID, "FinishedExitVehicle", InputFinishedExitVehicle ),
	DEFINE_INPUTFUNC( FIELD_STRING, "GiveWeapon", InputGiveWeapon ),
	DEFINE_INPUTFUNC( FIELD_VOID, "DropWeapon", InputDropWeapon ),
	DEFINE_INPUTFUNC( FIELD_BOOLEAN, "SetGameEndAlly", InputSetGameEndAlly ),
	DEFINE_INPUTFUNC( FIELD_STRING, "ThrowGrenadeAtTarget", InputThrowGrenadeAtTarget ),

	DEFINE_OUTPUT( m_OnFinishedEnterVehicle, "OnFinishedEnterVehicle" ),
	DEFINE_OUTPUT( m_OnFinishedExitVehicle, "OnFinishedExitVehicle" ),
END_DATADESC()

float g_flHumanSpeechTime = FLT_MIN;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHOEHuman::Spawn( void )
{
	BaseClass::Spawn();

	Precache();
	SetModel( STRING( GetModelName() ) );
	SelectModelGroups();

	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();
	SetSolid( SOLID_BBOX );

	SetBloodColor( BLOOD_COLOR_RED );
	m_flFieldOfView = 0.02;
	m_NPCState = NPC_STATE_NONE;

	m_HackedGunPos = Vector( 0, 0, 55 );

	CapabilitiesClear();
	CapabilitiesAdd( bits_CAP_SQUAD );
	CapabilitiesAdd( bits_CAP_ANIMATEDFACE | bits_CAP_TURN_HEAD ); // be careful with ANIMATEDFACE since we don't have eyes
	CapabilitiesAdd( bits_CAP_USE_WEAPONS | bits_CAP_AIM_GUN /* | bits_CAP_MOVE_SHOOT */ );
	CapabilitiesAdd( bits_CAP_DUCK | bits_CAP_DOORS_GROUP );
	CapabilitiesAdd( bits_CAP_USE_SHOT_REGULATOR );
	CapabilitiesAdd( /* bits_CAP_NO_HIT_PLAYER | */ bits_CAP_NO_HIT_SQUADMATES /* | bits_CAP_FRIENDLY_DMG_IMMUNE */ );
	CapabilitiesAdd( bits_CAP_MOVE_GROUND );

	if ( LookupActivity( "ACT_MELEE_ATTACK1" ) != ACT_INVALID )
		CapabilitiesAdd( bits_CAP_INNATE_MELEE_ATTACK1 );

	// Combine doesn't set this (because it is expensive to check?)
//	if ( HasSpawnFlags( SF_HUMAN_GRENADES ) )
//		CapabilitiesAdd( bits_CAP_INNATE_RANGE_ATTACK2 );

	SetMoveType( MOVETYPE_STEP );

#ifndef HOE_HUMAN_RR
	m_flLastTalkTime = 0.0f;
	m_flStopTalkTime = 0.0f;
	m_flPainTime = 0.0f;
	m_iSpokenConcepts = 0;
#endif
	m_flDelayedPainSoundTime = 0.0f;
	m_flScriptedSentence = 0.0f;
	m_flAttackedByHueyTime = 0.0f;
	m_flTimeLastRegen = 0.0f;
	m_fSquadCmdAcknowledged = false;
	m_nLastSquadCommand = SQUADCMD_NONE;
	m_flLastSquadCmdTime = FLT_MIN;
	m_hTalkTarget = NULL;
	m_flCrouchTime = 0;
	m_bStopCrouching = false;
	m_bCrouchLocked = false;
	m_flLastMedicSearch = 0.0f;
	m_flLastMedicHealed = 0.0f;
	m_flKilledEnemyTime = 0.0f;
	m_flLastFledZombieTime = 0;
	m_flIdleSpeechDelay = gpGlobals->curtime + 30.0f;

	m_GrenadeInteraction.flTime = -1.0f;

	m_fHandGrenades = HasSpawnFlags( SF_HUMAN_GRENADES );
	m_flNextGrenadeCheck = gpGlobals->curtime + 1;

	NPCInit();

	// Serverside ragdolls are used to:
	// 1) Have the death sound follow the ragdoll (in case of explosions). See
	//    also the npc_deathsound entity for client-side ragdoll death sound handling.
	// 2) Letting NPCs see dead friends.  This uses the hoe_corpse_marker entity.
	AddSpawnFlags( SF_HUMAN_SERVERSIDE_RAGDOLL );
}

//-----------------------------------------------------------------------------
void CHOEHuman::Precache( void )
{
	if ( GetModelName() == NULL_STRING )
	{
		SelectModel();
		Assert( GetModelName() );
	}
	PrecacheModel( STRING( GetModelName() ) );

	UTIL_PrecacheOther( "prop_physics"/*, GetHeadModelName()*/ );
	PrecacheModel( GetHeadModelName() );

#ifndef HOE_HUMAN_RR
	InitSentences();
#endif

	PrecacheScriptSound( "Weapon_Machete.HeadChop" );
	PrecacheScriptSound( "GiveWeapon" );
	PrecacheScriptSound( "NPC_Combine.WeaponBash" ); // kick impact

	UTIL_PrecacheOther( "npc_handgrenade" );

	BaseClass::Precache();
};

//-----------------------------------------------------------------------------
void CHOEHuman::OnRestore( void )
{
	BaseClass::OnRestore();

	// FIXME: This is only needed because the speech manager isn't saved across
	// level transitions.  Without this a follower will speak across every level
	// change.
	m_flIdleSpeechDelay = gpGlobals->curtime + 30.0f;
}

//-----------------------------------------------------------------------------
bool CHOEHuman::ShouldGib( const CTakeDamageInfo &info )
{
	if ( info.GetDamageType() & (DMG_NEVERGIB|DMG_DISSOLVE) )
		return false;

	if ( info.GetDamageType() & (DMG_ALWAYSGIB) ) // barnacle kill
		return true;

	return false;
}

//-----------------------------------------------------------------------------
bool CHOEHuman::AfraidOfHuey( void )
{
	// Don't run and hide from the huey unless it attacked us recently
	return TimeRecent( m_flAttackedByHueyTime, 10.0 );
}

//-----------------------------------------------------------------------------
Disposition_t CHOEHuman::IRelationType( CBaseEntity *pTarget )
{
	if ( pTarget && FClassnameIs( pTarget, "npc_huey" ) &&
		BaseClass::IRelationType( pTarget ) == D_HT )
	{
		if ( GetActiveWeapon() && FClassnameIs( GetActiveWeapon(), "weapon_rpg7" ) )
			return D_HT;
	
		if ( AfraidOfHuey() )
			return D_FR;

		return D_NU;
	}

	if ( pTarget && FClassnameIs( pTarget, "npc_barnacle" ) )
	{
		CBaseEntity *pBarnacleVictim = pTarget->GetEnemy();
		if ( pBarnacleVictim && ( IRelationType( pBarnacleVictim ) == D_LI ) )
		{
			return D_HT;
		}
		return D_NU;
	}

	if ( pTarget && FClassnameIs( pTarget, "npc_snark" ) )
	{
		CBaseEntity *pVictim = pTarget->GetEnemy();
		if ( pVictim && ( IRelationType( pVictim ) == D_LI ) )
		{
			return D_HT;
		}
		return D_NU;
	}

	return BaseClass::IRelationType( pTarget );
}

//-----------------------------------------------------------------------------
int CHOEHuman::IRelationPriority( CBaseEntity *pTarget )
{
	int priority = BaseClass::IRelationPriority( pTarget );

#ifdef HUMAN_UNSHOOTABLE
	// An enemy I was recently unable to find a weapon los node for
	// has lower priority than an enemy I can shoot.
	if ( pTarget && IsUnshootable( pTarget ) )
		priority -= 1; // FIXME: is negative priority allowed?
#endif

	return priority;
}

//-----------------------------------------------------------------------------
bool CHOEHuman::QueryHearSound( CSound *pSound )
{
	// When a M79 or handgrenade user wants to fire but a friend is in the way, SOUND_MOVE_AWAY is played
	// at the enemy position.

	if ( pSound->SoundContext() & SOUND_CONTEXT_COMBINE_ONLY )
		if ( !IsPlayerAlly() )
			return true;

	if ( pSound->SoundContext() & SOUND_CONTEXT_EXCLUDE_COMBINE )
		if ( !IsPlayerAlly() )
			return false;

	// Disregard footsteps from friends we've seen
	if ( pSound->IsSoundType( SOUND_COMBAT ) && pSound->SoundChannel() == SOUNDENT_CHANNEL_NPC_FOOTSTEP )
	{
		CBaseEntity *pSoundOwner = pSound->m_hOwner;
		if ( pSoundOwner )
		{
			for ( int i = 0; GetFriendClasses()[i] ; i++ )
			{
				if ( pSoundOwner->ClassMatches( GetFriendClasses()[i] ) /*&&
					 GetSenses()->DidSeeEntity( pSoundOwner )*/ )
				{
					return false;
				}
			}
		}
	}

	return BaseClass::QueryHearSound( pSound );
}

//-----------------------------------------------------------------------------
bool CHOEHuman::FindCoverPos( CSound *pSound, Vector *pResult )
{
#if 0
	if ( pSound->IsSoundType( SOUND_DANGER ) )
	{
		CUtlVector< CSound * > sounds;

		AISoundIter_t iter;
		CSound *pCurrent = GetSenses()->GetFirstHeardSound( &iter );

		while ( pCurrent )
		{
			if ( pCurrent->IsSoundType( SOUND_DANGER ) && !ShouldIgnoreSound( pCurrent ) )
			{
				sounds.AddToTail( pCurrent );
			}
			pCurrent = GetSenses()->GetNextHeardSound( &iter );
		}

		if ( GetTacticalServices()->FindCoverPosFromSounds( sounds, 
			0, CoverRadius(), pResult ) )
		{
			return true;
		}
	}
#endif
	return BaseClass::FindCoverPos( pSound, pResult );
}

//-----------------------------------------------------------------------------
float CHOEHuman::MaxYawSpeed( void )
{
	if ( IsCrouching() )
		return 20;
	return 30;
}

//-----------------------------------------------------------------------------
void CHOEHuman::SetTurnActivity( void )
{
	// Base class just sets ACT_IDLE when crouching
	if ( IsCrouching() )
	{
		float flYD;
		flYD = GetMotor()->DeltaIdealYaw();

		Activity activity = ACT_IDLE;
		if (flYD <= -45)
			activity = ACT_TURN_RIGHT_LOW;
		if (flYD >= 45)
			activity = ACT_TURN_LEFT_LOW;
		if ( SelectWeightedSequence ( activity ) == ACTIVITY_NOT_AVAILABLE )
			activity = ACT_IDLE;
		SetIdealActivity( activity );
		return;
	}

	BaseClass::SetTurnActivity();
}

//-----------------------------------------------------------------------------
Activity CHOEHuman::NPC_TranslateActivity( Activity eNewActivity )
{
	eNewActivity = BaseClass::NPC_TranslateActivity( eNewActivity );

	bool bHurt = HealthFraction() < 1.0/3.0;
	bool bCrouch = IsCrouching();

	bool bWasAimingRecently = false;
	if ( ( m_NPCState == NPC_STATE_COMBAT /*|| m_NPCState == NPC_STATE_ALERT*/ )
		 && ( gpGlobals->curtime - m_flLastAttackTime < 3 || gpGlobals->curtime - GetEnemyLastTimeSeen() < 8) /*&& !IsMoving()*/ )
	{
		bWasAimingRecently = true;
	}
//	if ( GetNavigator()->GetMovementActivity() == ACT_RUN_AIM )
//		bWasAimingRecently = true;

	if ( eNewActivity == ACT_IDLE )
	{
		// This will transition from shoot -> combatidle rather than shoot -> idle
		// which is important during the shot-regulator rest periods.
		// The check for IsMoving() is there so IDLE_ANGRY isn't used as the "interior"
		// sequence when finishing movement.
		if ( bWasAimingRecently )
		{
			return bCrouch ? ACT_IDLE_ANGRY_LOW : ACT_IDLE_ANGRY;
		}
		if ( bCrouch ) return ACT_CROUCHIDLE;
	}
	if ( eNewActivity == ACT_CROUCHIDLE )
	{
		if ( bWasAimingRecently )
		{
			return ACT_IDLE_ANGRY_LOW;
		}
		return eNewActivity;
	}
#if 1
	if ( eNewActivity == ACT_IDLE_ANGRY )
	{
		if ( bCrouch ) return ACT_IDLE_ANGRY_LOW;
	}
#endif
	if ( eNewActivity == ACT_SIGNAL1 )
	{
		if ( bCrouch ) return ACT_SIGNAL1_LOW;
	}
	if ( eNewActivity == ACT_SIGNAL2 )
	{
		if ( bCrouch ) return ACT_SIGNAL2_LOW;
	}
	if ( eNewActivity == ACT_SIGNAL3 )
	{
		if ( bCrouch ) return ACT_SIGNAL3_LOW;
	}

	if ( eNewActivity == ACT_WALK )
	{
		if ( bHurt )
		{
			return bCrouch ? ACT_WALK_CROUCH_HURT : ACT_WALK_HURT;
		}
		if ( bCrouch )
		{
			return ACT_WALK_CROUCH;
		}
	}
	if (eNewActivity == ACT_RUN)
	{
		if ( GetHealth() < GetMaxHealth() / 3 )
		{
			return bCrouch ? ACT_RUN_CROUCH_HURT : ACT_RUN_HURT;
		}
		if ( bCrouch )
		{
			return ACT_RUN_CROUCH;
		}
	}
	if ( eNewActivity == ACT_RANGE_ATTACK2 )
	{
		if ( bCrouch ) return ACT_RANGE_ATTACK2_LOW;
	}
	if ( eNewActivity == ACT_SMALL_FLINCH )
	{
		if ( bCrouch ) return ACT_SMALL_FLINCH_LOW;
	}

	return eNewActivity;
}

//-----------------------------------------------------------------------------
// Purpose: Turn off flinching under certain circumstances
//-----------------------------------------------------------------------------
bool CHOEHuman::CanFlinch( void )
{
//	if ( IsActiveDynamicInteraction() )
//		return false;

	if ( IsPlayingGesture( ACT_GESTURE_RANGE_ATTACK1 ) )
		return false;

	if ( m_hCine ) // added for choking mike in namd3 airfield
		return false;

	return BaseClass::CanFlinch();
}

//-----------------------------------------------------------------------------
bool CHOEHuman::AnyEnemyHasRangedAttack( void )
{
	AIEnemiesIter_t iter;
	for ( AI_EnemyInfo_t *pEMemory = GetEnemies()->GetFirst(&iter); pEMemory != NULL; pEMemory = GetEnemies()->GetNext(&iter) )
	{
		CBaseCombatCharacter *pEnemyBCC = pEMemory->hEnemy.Get()->MyCombatCharacterPointer();
		if ( !pEnemyBCC )
			continue;

		// Assume player has ranged weapons
		if ( pEnemyBCC->IsPlayer() )
			return true;

		// Ignore enemies that don't hate me
		if ( pEnemyBCC->IRelationType( this ) != D_HT )
			continue;

		CAI_BaseNPC *pEnemyNPC = pEnemyBCC->MyNPCPointer();
		if ( !pEnemyNPC )
			continue;

		// Look for enemies with ranged capabilities
		if ( pEnemyNPC->CapabilitiesGet() & ( bits_CAP_WEAPON_RANGE_ATTACK1 | bits_CAP_WEAPON_RANGE_ATTACK2 | bits_CAP_INNATE_RANGE_ATTACK1 | bits_CAP_INNATE_RANGE_ATTACK2 ) )
			return true;
	}

	return false;
}

#ifdef HUMAN_STRAFE
//-----------------------------------------------------------------------------
bool CHOEHuman::ShouldStrafe( void )
{
	if ( GetActiveWeapon() && FClassnameIs( GetActiveWeapon(), "weapon_rpg7" ) )
		return false;

	if ( !m_StrafeTimer.Expired() )
		return false;

	return AnyEnemyHasRangedAttack();
}
#endif

//-----------------------------------------------------------------------------
void CHOEHuman::TrackFootSplashes( int iAttachment, bool &bPrevInWater, Vector &prevOrigin )
{
	if ( iAttachment == INVALID_ATTACHMENT )
		return;

	Vector footOrigin;
	GetAttachment( iAttachment, footOrigin );

	bool bInWater = GetWaterLevel() > 1;
	if ( GetWaterLevel() == 1 )
	{
#if 1
		// Compute the point to check for water state
		// see CBaseEntity::UpdateWaterState()
		Vector point;
		CollisionProp()->NormalizedToWorldSpace( Vector( 0.5f, 0.5f, 0.0f ), &point );

		float flWaterZ = UTIL_FindWaterSurface( point, point.z, point.z + 64.0 );
		bInWater = ( footOrigin.z < flWaterZ ) ? true : false;
#else
		bInWater = ( UTIL_PointContents( footOrigin ) & CONTENTS_WATER ) ? true : false;
#endif
	}
	if ( bInWater && !bPrevInWater )
	{
		trace_t waterTrace;
		UTIL_TraceLine( prevOrigin, footOrigin, CONTENTS_WATER, NULL, &waterTrace );

		CEffectData	data;
		data.m_vOrigin = waterTrace.endpos;
		data.m_vNormal = waterTrace.plane.normal;
		data.m_flScale = 3;
		if ( waterTrace.contents & CONTENTS_SLIME )
		{
			data.m_fFlags |= FX_WATER_IN_SLIME;
		}
		DispatchEffect( "footstepsplash", data ); // plays a sound too!
	}

	bPrevInWater = bInWater;
	prevOrigin = footOrigin;
}

static void PlayWaterFootstepSound( CBaseEntity *pEnt, const char *pSoundName, float fvol, const Vector &vecOrigin )
{
	CSoundParameters params;
	if ( !CBaseEntity::GetParametersForSound( pSoundName, params, NULL ) )
		return;

	EmitSound_t ep;
	ep.m_nChannel = CHAN_BODY;
	ep.m_pSoundName = params.soundname;
	ep.m_flVolume = fvol;
	ep.m_SoundLevel = params.soundlevel;
	ep.m_nFlags = 0;
	ep.m_nPitch = params.pitch;
	ep.m_pOrigin = &vecOrigin;

	CRecipientFilter filter;
	filter.AddRecipientsByPAS( vecOrigin );

	pEnt->EmitSound( filter, pEnt->entindex(), ep );
}

//-----------------------------------------------------------------------------
void CHOEHuman::HandleAnimEvent( animevent_t *pEvent )
{
	// From npc_combine.cpp
	// Fired by `$sequence frontkick` in model's .qc file
	if ( pEvent->event == AE_NPC_KICK )
	{
		m_flLastKick = gpGlobals->curtime;

		// Does no damage, because damage is applied based upon whether the target can handle the interaction
		CBaseEntity *pHurt = CheckTraceHullAttack( 70, -Vector(16,16,18), Vector(16,16,18), 0, DMG_CLUB );
		CBaseCombatCharacter *pBCC = ToBaseCombatCharacter( pHurt );
		if (pBCC)
		{
			Vector forward, up;
			AngleVectors( GetAbsAngles(), &forward, NULL, &up );

			if ( 1 /* !pBCC->DispatchInteraction( g_interactionCombineBash, NULL, this ) */)
			{
				if ( pBCC->IsPlayer() )
				{
					pBCC->ViewPunch( QAngle(-12,-7,0) );
					pHurt->ApplyAbsVelocityImpulse( forward * 100 + up * 50 );
				}

				CTakeDamageInfo info( this, this, GetKickDamage(), DMG_CLUB );
				CalculateMeleeDamageForce( &info, forward + up * 0.4, pBCC->GetAbsOrigin() );
				if ( FClassnameIs( pBCC, "npc_spx_baby" ) || FClassnameIs( pBCC, "npc_kophyaeger" ) )
					info.ScaleDamageForce( 2.0f );
				pBCC->TakeDamage( info );

				EmitSound( "NPC_Combine.WeaponBash" );
			}
		}			
	
//			m_Sentences.Speak( "COMBINE_KICK" );
//			handledEvent = true;
		return;
	}

	if ( pEvent->event == AE_HUMAN_GRENADE_TOSS )
	{
		Vector vecSpin;
		vecSpin.x = random->RandomFloat( -1000.0, 1000.0 );
		vecSpin.y = random->RandomFloat( -1000.0, 1000.0 );
		vecSpin.z = random->RandomFloat( -1000.0, 1000.0 );

		Vector vecStart;
		GetAttachment( "lefthand", vecStart );

		// Use the Velocity that AI gave us.
		CBaseEntity *pFrag = Fraggrenade_Create( vecStart, vec3_angle, m_vecTossVelocity, vecSpin, this, 3.5, false );
#if 1
		IPhysicsObject *pPhys = pFrag->VPhysicsGetObject();
		float damping = 0.0;
		if ( pPhys )
		{
//			pPhys->SetDamping( &damping, &damping );
			pPhys->SetDragCoefficient( &damping, &damping );
#if 0
				Vector unitVel = m_vecTossVelocity;
				VectorNormalize( unitVel );

				float flTest = 1000 / m_vecTossVelocity.Length();

				float flDrag = pPhys->CalculateLinearDrag( m_vecTossVelocity );
				m_vecTossVelocity = m_vecTossVelocity + ( unitVel * ( flDrag * flDrag ) ) / flTest;
			
				pPhys->SetVelocity( &m_vecTossVelocity, NULL );
#endif
		}
#endif
		return;
	}

	if ( pEvent->event == AE_HUMAN_GRENADE_DROP )
	{
		float flDelay = 3.5f;

		if ( pEvent->options != NULL && pEvent->options[0] != '\0')
		{
			flDelay = Q_atof( pEvent->options );
		}

		Vector vecStart;
		GetAttachment( "lefthand", vecStart );

		Fraggrenade_Create( vecStart, vec3_angle, vec3_origin, vec3_origin, this, flDelay, false );
		return;
	}

	if ( pEvent->event == AE_HUMAN_OPEN_VEHICLE )
	{
		CBaseEntity *pVehicleEnt = m_PassengerBehavior.GetTargetVehicle();
		if ( pVehicleEnt != NULL )
		{
			variant_t emptyVariant;
			pVehicleEnt->AcceptInput( "PassengerOpenVehicle", this, this, emptyVariant, 0 );
		}
		return;
	}

	if ( pEvent->event == AE_HUMAN_CLOSE_VEHICLE )
	{
		CBaseEntity *pVehicleEnt = m_PassengerBehavior.GetTargetVehicle();
		if ( pVehicleEnt != NULL )
		{
			variant_t emptyVariant;
			pVehicleEnt->AcceptInput( "PassengerCloseVehicle", this, this, emptyVariant, 0 );
		}
		return;
	}

	switch( pEvent->event )
	{
	case AE_NPC_LEFTFOOT: // Eh? different than NPC_EVENT_LEFTFOOT
		{/*
			It seems that to enable different footstep sounds for walking on different materials the model
			must emit CL_EVENT_MFOOTSTEP_*** events and also have attachments called "LeftFoot" and
			"RightFoot". This ends up calling MaterialFootstepSound() in c_baseanimating.cpp.

			Another option is the CL_EVENT_FOOTSTEP_*** (note lack of M) events with an option of the prefix
			which will be added to %s.FootstepLeft or %s.RunFootstepLeft. This won't play sounds based
			on the material however.
			*/

			// TODO: bloody footprints

			MakeAIFootstepSound( 180.0f ); // only antlions and zombies seem to use this

			bool bWalking = Q_stristr( pEvent->options, "Run" ) ? false : true;
			if ( GetWaterLevel() == 0 )
				EmitSound(pEvent->options);
			else if ( GetWaterLevel() == 1 )
				PlayWaterFootstepSound( this, "Water.StepLeft", bWalking ? 0.2 : 0.4, GetAbsOrigin() );
			else
				PlayWaterFootstepSound( this, "Wade.StepLeft", 0.65, GetAbsOrigin() );
#if 1
//			TrackFootSplashes( m_iLeftFootAttachment, m_bLeftFootInWater, m_vLeftFootOrigin );
#else
			if ( m_iLeftFootAttachment != INVALID_ATTACHMENT && GetWaterLevel() == 1 )
			{
				Vector footOrigin;
				GetAttachment( m_iLeftFootAttachment, footOrigin );
				bool bInWater = ( UTIL_PointContents( footOrigin ) & CONTENTS_WATER ) ? true : false;
				if ( bInWater && !m_bLeftFootInWater )
				{
					trace_t waterTrace;
					Vector vecTrace = Vector(0,0,32);
					UTIL_TraceLine( footOrigin, footOrigin + vecTrace, CONTENTS_WATER, NULL, &waterTrace );

					CEffectData	data;
 					data.m_vOrigin = footOrigin + vecTrace * waterTrace.fractionleftsolid;
					data.m_vNormal = waterTrace.plane.normal;
					data.m_flScale = 4;
					if ( waterTrace.contents & CONTENTS_SLIME )
					{
						data.m_fFlags |= FX_WATER_IN_SLIME;
					}
					DispatchEffect( "gunshotsplash", data );
				}

				m_bLeftFootInWater = bInWater;
			}

			if ( m_iRightFootAttachment != INVALID_ATTACHMENT && GetWaterLevel() == 1 )
			{
				Vector footOrigin;
				GetAttachment( m_iRightFootAttachment, footOrigin );
				m_bRightFootInWater = ( UTIL_PointContents( footOrigin ) & CONTENTS_WATER ) ? true : false;
			}
#endif
		}
		break;
	case AE_NPC_RIGHTFOOT: // Eh? different than NPC_EVENT_LEFTFOOT
		{
			// TODO: bloody footprints

			MakeAIFootstepSound( 180.0f ); // only antlions and zombies seem to use this

			bool bWalking = Q_stristr( pEvent->options, "Run" ) ? false : true;
			if ( GetWaterLevel() == 0 )
				EmitSound(pEvent->options);
			else if ( GetWaterLevel() == 1 )
				PlayWaterFootstepSound( this, "Water.StepRight", bWalking ? 0.2 : 0.4, GetAbsOrigin() );
			else
				PlayWaterFootstepSound( this, "Wade.StepRight", 0.65, GetAbsOrigin() );
#if 1
//			TrackFootSplashes( m_iRightFootAttachment, m_bRightFootInWater, m_vRightFootOrigin );
#else
			if ( m_iRightFootAttachment != INVALID_ATTACHMENT && GetWaterLevel() == 1 )
			{
				Vector footOrigin;
				GetAttachment( m_iRightFootAttachment, footOrigin );
				bool bInWater = ( UTIL_PointContents( footOrigin ) & CONTENTS_WATER ) ? true : false;
				if ( bInWater && !m_bRightFootInWater )
				{
					trace_t waterTrace;
					Vector vecTrace = Vector(0,0,32);
					UTIL_TraceLine( footOrigin, footOrigin + vecTrace, CONTENTS_WATER, NULL, &waterTrace );

					CEffectData	data;
					data.m_vOrigin = footOrigin + vecTrace * waterTrace.fractionleftsolid;
					data.m_vNormal = waterTrace.plane.normal;
					data.m_flScale = 4;
					if ( waterTrace.contents & CONTENTS_SLIME )
					{
						data.m_fFlags |= FX_WATER_IN_SLIME;
					}
					DispatchEffect( "gunshotsplash", data );
				}

				m_bRightFootInWater = bInWater;
			}

			if ( m_iLeftFootAttachment != INVALID_ATTACHMENT && GetWaterLevel() == 1 )
			{
				Vector footOrigin;
				GetAttachment( m_iLeftFootAttachment, footOrigin );
				m_bLeftFootInWater = ( UTIL_PointContents( footOrigin ) & CONTENTS_WATER ) ? true : false;
			}
#endif
		}
		break;
	default:
		BaseClass::HandleAnimEvent( pEvent );
		break;
	}
}

//-----------------------------------------------------------------------------
// Our humans can aim up/down using the aim_pitch pose parameter but not left/right. So
// clear out bits_CAP_AIM_GUN when seeing if our gun is pointed at the enemy.
bool CHOEHuman::FInAimCone( const Vector &vecSpot )
{
	Vector los = ( vecSpot - GetAbsOrigin() );

	// do this in 2D
	los.z = 0;
	VectorNormalize( los );

	Vector facingDir = BodyDirection2D( );

	float flDot = DotProduct( los, facingDir );
#if 1
	if (CapabilitiesGet() & bits_CAP_AIM_GUN && m_poseAim_Yaw != -1 )
	{
		// FIXME: query current animation for ranges
		return ( flDot > DOT_30DEGREE );
	}
#else
	if (CapabilitiesGet() & bits_CAP_AIM_GUN)
	{
		// FIXME: query current animation for ranges
		return ( flDot > DOT_30DEGREE );
	}
#endif
	if ( flDot > 0.994 )//!!!BUGBUG - magic number same as FacingIdeal(), what is this?
		return true;

	return false;
}

//-----------------------------------------------------------------------------
void CHOEHuman::MaintainLookTargets( float flInterval )
{
	BaseClass::MaintainLookTargets( flInterval );
}

//---------------------------------------------------------
bool CHOEHuman::HandleInteraction( int interactionType, void *data, CBaseCombatCharacter* sourceEnt )
{
	if ( interactionType == g_interactionBarnacleVictimGrab )
	{
		// Force choosing of a new schedule.
		ClearSchedule( "barnacle grab" );

		SentenceStop();

		// Gag the NPC so they won't talk anymore
		AddSpawnFlags( SF_NPC_GAG );

		// If I have a weapon already, drop it
		if ( GetActiveWeapon() && !ClassifyPlayerAllyVital() )
		{
			Weapon_Drop( GetActiveWeapon() );
		}

		if ( GetFlags() & FL_ONGROUND )
		{
			SetGroundEntity( NULL );
		}

		Vector vecOrigin = sourceEnt->GetAbsOrigin();
		vecOrigin.z = GetAbsOrigin().z;
		SetAbsOrigin( vecOrigin );

		m_nBarnacleState = BARNACLE_STATE_HIT;
		SetState( NPC_STATE_PRONE );
		SetMoveType( MOVETYPE_FLY );
		return true;
	}

	if ( interactionType == g_interactionBarnacleVictimBite )
	{
		// Force choosing of a new schedule
		ClearSchedule( "barnacle bite" );
		m_nBarnacleState = BARNACLE_STATE_CHOMP;
		return true;
	}
	
	if ( interactionType == g_interactionBarnacleVictimReleased )
	{
		// Force choosing of a new schedule
		ClearSchedule( "barnacle release" );
//		SetIdealState( NPC_STATE_IDLE );

		RemoveSpawnFlags( SF_NPC_GAG );

		m_nBarnacleState = BARNACLE_STATE_RELEASED;
		SetAbsVelocity( vec3_origin );
		SetMoveType( MOVETYPE_STEP );
		return true;
	}
	
	if ( interactionType == g_interactionHumanMedicHeal )
	{
		if ( OkForMedicToHealMe( (CAI_BaseNPC *) sourceEnt ) )
		{
			// Ditch my current schedule if I'm running a schedule that will
			// be interrupted if I'm hit.
			if ( ConditionInterruptsCurSchedule( COND_LIGHT_DAMAGE ) ||
				 ConditionInterruptsCurSchedule( COND_HEAVY_DAMAGE) )
			{
				SetTarget( sourceEnt ); // so we can face him
				SetSchedule( SCHED_HUMAN_WAIT_HEAL );
			}

//			SetCondition( COND_HUMAN_MEDIC_WANTS_TO_HEAL_ME );
		}
		return true;
	}

	// A friendly wishes to grenade an enemy but I am too close
	if ( interactionType == g_interactionHumanGrenade )
	{
		// Don't get bogged down by frequent notices
		if ( (m_GrenadeInteraction.flTime == -1.0f) || (m_GrenadeInteraction.flTime < gpGlobals->curtime - 8.0f) )
		{
			Assert( data );
			m_GrenadeInteraction = *(HumanGrenadeInteractionData *)data;
			m_GrenadeInteraction.flTime = gpGlobals->curtime;
			m_GrenadeInteraction.bRunning = false;
		}
		return true;
	}

	extern int g_interactionMacheteHeadChop;
	if ( interactionType == g_interactionMacheteHeadChop )
	{
		if ( HasAHead() )
		{
			sourceEnt->EmitSound( "Weapon_Machete.HeadChop" );
			Behead();
			TakeDamage( CTakeDamageInfo( sourceEnt, sourceEnt, GetHealth(), DMG_GENERIC ) );
		}
		return true;
	}

	return false;
}

//---------------------------------------------------------
bool CHOEHuman::WasHealedRecently( void )
{
	return TimeRecent( m_flLastMedicHealed, 30.0f );
}

//---------------------------------------------------------
bool CHOEHuman::OkForMedicToHealMe( CAI_BaseNPC *pMedic )
{
	if ( !IsAlive() || m_lifeState == LIFE_DYING )
		return false;

	if ( IsInAScript() )
		return false;

	if ( HasCondition( COND_SEE_ENEMY ) )
		return false;

	// Don't heal me repeatedly
	if ( WasHealedRecently() )
		return false;

	/* TODO: Did another medic want to heal me? */

	return true;
}

//---------------------------------------------------------
bool CHOEHuman::EnemyIsZombie( void )
{
	return GetEnemy() && FClassnameIs( GetEnemy(), "npc_spx" );
}

//---------------------------------------------------------
bool CHOEHuman::EnemyIsBullseye( void )
{
	return GetEnemy() && GetEnemy()->Classify() == CLASS_BULLSEYE;
}

static bool InViewCone( CBaseCombatCharacter *pViewer, const Vector &vecSpot, float flFieldOfView )
{
	Vector los = ( vecSpot - pViewer->EyePosition() );

	// do this in 2D
	los.z = 0;
	VectorNormalize( los );

	Vector facingDir = pViewer->EyeDirection2D( );

	float flDot = DotProduct( los, facingDir );

	if ( flDot > flFieldOfView )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
void CHOEHuman::GatherConditions( void )
{
	BaseClass::GatherConditions();

	if ( HasCondition( COND_SEE_PLAYER ) )
	{
		bool bPlayerIsLooking = false;
		CHL2_Player *pPlayer = dynamic_cast<CHL2_Player*>(AI_GetSinglePlayer());
		if ( pPlayer && ( pPlayer->GetAbsOrigin() - GetAbsOrigin() ).Length2DSqr() < Square(72.0) &&
			!IsInAScript() && !IsHumanTalking() /*!GetTalkTarget() && !GetListenTarget()*/ )
		{
			if ( InViewCone( pPlayer, EyePosition(), 0.99f ) && ( pPlayer->m_hUseableEntity == this ) )
			{
				if ( pPlayer->GetSmoothedVelocity().LengthSqr() < Square( 100 ) )
					bPlayerIsLooking = true;
			}
		}
		
		if ( bPlayerIsLooking )
		{
//			SetCondition( COND_TALKER_PLAYER_STARING );
			if ( m_flTimePlayerStartStare == 0 )
				m_flTimePlayerStartStare = gpGlobals->curtime;

			if ( IsMedic() && TimePassed( m_flTimePlayerStartStare - 2.0f ) &&
				HealthFraction( pPlayer ) <= 2.0f/3.0f )
			{
				DevMsg( "Provoking medic to heal player\n" );
				SetCondition( COND_PROVOKED );
			}
		}
		else
		{
			m_flTimePlayerStartStare = 0;
//			ClearCondition( COND_TALKER_PLAYER_STARING );
		}
	}
}

//---------------------------------------------------------
void CHOEHuman::GatherEnemyConditions( CBaseEntity *pEnemy )
{
	BaseClass::GatherEnemyConditions( pEnemy );

	float distSqEnemy = ( pEnemy->GetAbsOrigin() - GetAbsOrigin() ).LengthSqr();

	// If in close quarters with a zombie then try to stay away from it.
	ClearCondition( COND_ZOMBIE_TOO_CLOSE );
	if ( EnemyIsZombie() )
	{
		if ( distSqEnemy <= GetZombieSafeDistanceSqr() )
			SetCondition( COND_ZOMBIE_TOO_CLOSE );
	}

	// Definitely want to stay away from bullsquid/gorilla melee attacks.
	ClearCondition( COND_HUMAN_ENEMY_CLOSE );
	if ( distSqEnemy < GetZombieSafeDistanceSqr() &&
		( FClassnameIs( pEnemy, "npc_bullsquid" ) ||
		FClassnameIs( pEnemy, "npc_gorilla" ) ) )
	{
		SetCondition( COND_HUMAN_ENEMY_CLOSE );
	}
}

//-----------------------------------------------------------------------------
void CHOEHuman::OnStateChange( NPC_STATE OldState, NPC_STATE NewState )
{
	BaseClass::OnStateChange( OldState, NewState );

	if ( OldState == NPC_STATE_COMBAT )
	{
		// Clear conversation bits.
		// But don't say hello again.
#ifdef HOE_HUMAN_RR // FIXME: this is dumb
		GetExpresser()->ClearSpokeConcept( TLK_HEAR );
		GetExpresser()->ClearSpokeConcept( TLK_HURTC );
		GetExpresser()->ClearSpokeConcept( TLK_HURTB );
		GetExpresser()->ClearSpokeConcept( TLK_HURTA );
		GetExpresser()->ClearSpokeConcept( TLK_WOUND );
		GetExpresser()->ClearSpokeConcept( TLK_MORTAL );
		GetExpresser()->ClearSpokeConcept( TLK_QUESTION );
		GetExpresser()->ClearSpokeConcept( TLK_IDLE );
		GetExpresser()->ClearSpokeConcept( TLK_HEALED );
#else
		m_iSpokenConcepts &= HUMAN_SC_HELLO;
#endif

		// Don't chatter if we just survived death
		if ( GetSpeechManager() )
		{
			GetSpeechManager()->ExtendSpeechCategoryTimer( SPEECH_CATEGORY_IDLE, 30 );
			GetSpeechManager()->ExtendSpeechCategoryTimer( SPEECH_CATEGORY_INJURY, 5 );
		}

		// Stand up soon after combat
		if ( IsCrouching() )
			m_flCrouchTime = gpGlobals->curtime + 1.5f;
	}
}

//-----------------------------------------------------------------------------
void CHOEHuman::PrescheduleThink( void )
{
	BaseClass::PrescheduleThink();

	if ( !m_bCrouchLocked && IsCrouching() && ( m_flCrouchTime <= gpGlobals->curtime || GetWaterLevel() == 3 ) )
	{
		ClearForceCrouch();
		Stand();
	}

	// If it's been a long time since my last squad command was given, forget about it
	if ( m_flLastSquadCmdTime < gpGlobals->curtime - SQUAD_COMMAND_MEMORY_TIME )
	{
		m_nLastSquadCommand = SQUADCMD_NONE;
		m_flLastSquadCmdTime = FLT_MIN;
	}

	// Next bit is from ai_playerally.cpp

	// Vital allies regenerate
	if( GetHealth() >= GetMaxHealth() )
	{
		// Avoid huge deltas on first regeneration of health after long period of time at full health.
		m_flTimeLastRegen = gpGlobals->curtime;
	}
	else if ( RegenerateTime() > 0.0f )
	{
		float flDelta = gpGlobals->curtime - m_flTimeLastRegen;
		float flHealthPerSecond = 1.0f / RegenerateTime();

		float flHealthRegen = flHealthPerSecond * flDelta;

		if ( g_pGameRules->IsSkillLevel(SKILL_HARD) )
			flHealthRegen *= 0.5f;
		else if ( g_pGameRules->IsSkillLevel(SKILL_EASY) )
			flHealthRegen *= 1.5f;

		if ( flHealthRegen >= 1.0 )
		{
			m_flTimeLastRegen = gpGlobals->curtime;
			TakeHealth( flHealthRegen, DMG_GENERIC );
		}
	}

#if 1
	// Try to figure out if the player is looking at an enemy we don't know about.
	CBasePlayer *pPlayer = AI_GetSinglePlayer();
	if ( pPlayer != NULL &&
		IsPlayerAlly() &&
		( GetState() == NPC_STATE_IDLE || GetState() == NPC_STATE_ALERT ) &&
		FInViewCone( pPlayer ) && FVisible( pPlayer ) )
	{
		Vector forward;
		pPlayer->EyeVectors( &forward );
		trace_t tr;
		UTIL_TraceLine( pPlayer->EyePosition(), pPlayer->EyePosition() + forward * 1024.0f, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr );
		if ( tr.m_pEnt != NULL && IRelationType( tr.m_pEnt ) == D_HT )
		{
			UpdateEnemyMemory( tr.m_pEnt, tr.m_pEnt->GetAbsOrigin(), pPlayer );
		}
	}
#endif

	if ( m_flDelayedPainSoundTime != 0.0f )
	{
		if ( TimeRecent( m_flDelayedPainSoundTime, 0.3 ) )
			DelayedPainSound();
		m_flDelayedPainSoundTime = 0.0f;
	}

	if ( (GetState() == NPC_STATE_IDLE || GetState() == NPC_STATE_ALERT) 
		 && !HasCondition(COND_RECEIVED_ORDERS) && !IsInAScript() )
	{
		IdleSpeech();
	}

	if ( GetTalkTarget() && !IsSpeaking() )
	{
		SetTalkTarget( NULL );
	}
	if ( GetListenTarget() &&
		HumanPointer( GetListenTarget() ) &&
		!HumanPointer( GetListenTarget() )->IsSpeaking() )
	{
		SetListenTarget( NULL );
	}

	TrackFootSplashes( m_iLeftFootAttachment, m_bLeftFootInWater, m_vLeftFootOrigin );
	TrackFootSplashes( m_iRightFootAttachment, m_bRightFootInWater, m_vRightFootOrigin );
#if 0
	if ( GetWaterLevel() > 0 && m_flWaterSoundTime < gpGlobals->curtime )
	{
		float flDuration;
		EmitSound( "Wade.StepLeft", 0.0f, &flDuration );
		m_flWaterSoundTime = gpGlobals->curtime + flDuration;
	}
#endif
}

//-----------------------------------------------------------------------------
void CHOEHuman::BuildScheduleTestBits( void )
{
	BaseClass::BuildScheduleTestBits();

	// Always interrupt to get into a vehicle
	SetCustomInterruptCondition( COND_HUMAN_BECOMING_PASSENGER );

	if ( GetCurSchedule()->HasInterrupt( COND_IDLE_INTERRUPT ) )
	{
		SetCustomInterruptCondition( COND_BETTER_WEAPON_AVAILABLE );
	}

	// We only care about stink to talk about it, don't let it interrupt schedules.
	// Otherwise the NPC constantly slams to NPC_STATE_ALERT and SCHED_ALERT_STAND
	// is constantly interrupted.
	ClearCustomInterruptCondition( COND_SMELL );

	// Add check for dead enemy same as SCHED_HUMAN_HIDE_AND_RELOAD
	if ( IsCurSchedule( SCHED_HIDE_AND_RELOAD ) )
	{
		SetCustomInterruptCondition( COND_ENEMY_DEAD );
	}
}

bool g_bSelectScheduleCheck = false;

//-----------------------------------------------------------------------------
int CHOEHuman::SelectSchedule( void )
{
	// Prevent recursion due to follow behavior calling our methods via
	// FollowCallBaseSelectSchedule.  Those methods should NOT call
	// BaseClass::SelectSchedule.
	if ( g_bSelectScheduleCheck )
	{
		Assert( 0 );
		DevWarning( "CHOEHuman::SelectSchedule RECURSION (%s)\n", GetDebugName() );
		return SCHED_NONE;
	}

	// Always defer to passenger if it's running
	if ( m_PassengerBehavior.CanSelectSchedule() /*ShouldDeferToPassengerBehavior()*/ )
	{
		DeferSchedulingToBehavior( &m_PassengerBehavior );
		return BaseClass::SelectSchedule();
	}

	if ( m_RappelBehavior.CanSelectSchedule() )
	{
		DeferSchedulingToBehavior( &m_RappelBehavior );
		return BaseClass::SelectSchedule();
	}

	if ( m_HueyRappelBehavior.CanSelectSchedule() )
	{
		DeferSchedulingToBehavior( &m_HueyRappelBehavior );
		return BaseClass::SelectSchedule();
	}
#if 0
	if ( m_hForcedInteractionPartner )
		return SelectInteractionSchedule();
#endif

	if ( GetState() == NPC_STATE_PRONE )
	{
		switch ( m_nBarnacleState )
		{
		case BARNACLE_STATE_HIT: // initial grab
			m_nBarnacleState = BARNACLE_STATE_PULL;
			return SCHED_HUMAN_BARNACLE_HIT;
		case BARNACLE_STATE_PULL: // being pulled up
			return SCHED_HUMAN_BARNACLE_PULL;
		case BARNACLE_STATE_CHOMP: // big bite
			m_nBarnacleState = BARNACLE_STATE_CHEW; // revert to chewing when big bite is finished
			return SCHED_HUMAN_BARNACLE_CHOMP;
		case BARNACLE_STATE_CHEW: // idle chewing
			return SCHED_HUMAN_BARNACLE_CHEW;
		case BARNACLE_STATE_RELEASED:
			{
				trace_t tr;
				UTIL_TraceHull( GetAbsOrigin(), GetAbsOrigin() - Vector(0,0,200), GetHullMins(), GetHullMaxs(),
					MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr );
				if ( tr.fraction == 1.0 )
					return SCHED_HUMAN_BARNACLE_RELEASED_HIGH;
				return SCHED_HUMAN_BARNACLE_RELEASED_LOW;
			}
		default:
			DevWarning( "unknown m_nBarnacleState %d for %s\n", m_nBarnacleState, GetDebugName() );
			Assert(0);
			break;
		}
	}
#if 0
	if ( IsWaitingToRappel() && BehaviorSelectSchedule() )
	{
		return BaseClass::SelectSchedule();
	}
#endif
#if 0
	// If have been crouching for long time get up
	if ( !m_bCrouchLocked && IsCrouching() && ( m_flCrouchTime <= gpGlobals->curtime || GetWaterLevel() == 3 ) )
	{
		return SCHED_HUMAN_UNCROUCH; // FIXME: probably don't need a schedule for this
	}
#endif

	int sched;

	sched = SelectFlinchSchedule();
	if ( sched != SCHED_NONE )
		return sched;

	sched = SelectScheduleDanger();
	if ( sched != SCHED_NONE )
		return sched;

	sched = SelectSchedulePriority();
	if ( sched != SCHED_NONE )
		return sched;

	if ( BehaviorSelectSchedule() )
	{
#if 1
		return BaseClass::SelectSchedule();
#else
		sched = BaseClass::SelectSchedule();

		// See CHOEFollowBehavior::SelectSchedule()
		if ( sched != SCHED_NONE )
			return sched;

		// Clear out follow behavior if it didn't select a schedule
		DeferSchedulingToBehavior( NULL );
#endif
	}

	switch ( GetState() )
	{
	case NPC_STATE_IDLE:
		sched = SelectScheduleIdle();
		break;
	case NPC_STATE_ALERT:
		sched = SelectScheduleAlert();
		break;
	case NPC_STATE_COMBAT:
		sched = SelectScheduleCombat();
		break;
	}
	if ( sched != SCHED_NONE )
		return sched;

	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
int CHOEHuman::SelectScheduleDanger( void )
{
	if ( HasCondition( COND_HEAR_DANGER ) )
	{
		CSound *pSound = GetBestSound( SOUND_DANGER );
		ASSERT( pSound != NULL );

		if ( pSound && (pSound->m_iType & SOUND_DANGER) )
		{
			CBaseEntity *pSoundOwner = pSound->m_hOwner;
			if ( pSoundOwner )
			{
				CBaseGrenade *pGrenade = dynamic_cast<CBaseGrenade *>(pSoundOwner);
				if ( pGrenade && pGrenade->GetThrower() )
				{
					if ( IRelationType( pGrenade->GetThrower() ) != D_LI )
					{
						// special case call out for enemy grenades
						GrenadeSound(); 
					}
				}
			}

			return SCHED_TAKE_COVER_FROM_BEST_SOUND;
		}
	}
	return SCHED_NONE;
}

//-----------------------------------------------------------------------------
int CHOEHuman::SelectSchedulePriority( void )
{
	if ( HasCondition(COND_BETTER_WEAPON_AVAILABLE) )
	{
		CBaseHLCombatWeapon *pWeapon = dynamic_cast<CBaseHLCombatWeapon *>(Weapon_FindUsable( WEAPON_SEARCH_DELTA ));
		if ( pWeapon )
		{
			m_flNextWeaponSearchTime = gpGlobals->curtime + 10.0;
			// Now lock the weapon for several seconds while we go to pick it up.
			pWeapon->Lock( 10.0, this );
			SetTarget( pWeapon );
			return SCHED_NEW_WEAPON;
		}
	}

	// We've been told to move away from a target to make room for a grenade to be thrown at it
	m_GrenadeInteraction.bRunning = false;
	if ( (m_GrenadeInteraction.flTime != -1.0f) && (m_GrenadeInteraction.flTime >= gpGlobals->curtime - 3.0f) )
	{
		m_GrenadeInteraction.bRunning = true;
		return SCHED_RUN_RANDOM; // FIXME: want to flee this position
	}

	if ( HasCondition( COND_HEAR_MOVE_AWAY ) )
	{
		return SCHED_MOVE_AWAY;
	}

	if ( HasCondition( COND_HUMAN_MEDIC_WANTS_TO_HEAL_ME ) )
	{
		ClearCondition( COND_HUMAN_MEDIC_WANTS_TO_HEAL_ME );
		return SCHED_HUMAN_WAIT_HEAL;
	}

	// If in close quarters with a zombie then try to stay away from it.
	if ( HasCondition( COND_ZOMBIE_TOO_CLOSE ) && TimeNotRecent( m_flLastFledZombieTime, 3.0f ) )
	{
//		DevMsg("COND_ZOMBIE_TOO_CLOSE\n");

		// FIXME: Need to detect when fleeing failed and not try it again too soon.
		m_flLastFledZombieTime = gpGlobals->curtime;

		if ( GetActiveWeapon() == NULL )
			return SCHED_TAKE_COVER_FROM_ENEMY;

		return SCHED_HUMAN_MOVE_TO_WEAPON_RANGE;
	}

	return SCHED_NONE;
}

//-----------------------------------------------------------------------------
int CHOEHuman::SelectScheduleIdle( void )
{
	// no ammo
	if ( HasCondition ( COND_NO_PRIMARY_AMMO ) || HasCondition( COND_LOW_PRIMARY_AMMO ) )
	{
		return SCHED_RELOAD;
	}

	// Squad Commands that only need a response when not fighting
	// ( This applies to all squad commands )

	int sched = SelectScheduleFromSquadCommand();
	if ( sched != SCHED_NONE ) return sched;

#if 0
	// Follow leader
	if ( IsFollowing() )
	{
		if ( GetEnemy() == NULL )
		{
			if ( !GetFollowTarget()->IsAlive() )
			{
				// UNDONE: Comment about the recently dead player here?
				StopFollowing( );
				return SCHED_NONE;
			}
			return SCHED_TARGET_FACE;
		}
	}
#endif

	// If you hear a sound that you can't see the source of (and isn't a grenade) 
	// then crouch and say "Shhh... I hear something".   
	CSound *pSound = GetSenses()->GetClosestSound( false );
	if ( pSound && (pSound->m_iVolume > 0) )
	{
		if ( !FVisible( pSound->GetSoundReactOrigin() ) )
		{
			return SCHED_HUMAN_HEAR_SOUND;
		}
		else
		{
			return SCHED_ALERT_FACE;
		}
	}

	return SCHED_NONE;
}

//-----------------------------------------------------------------------------
int CHOEHuman::SelectScheduleAlert( void )
{
	// Taking damage code is in the BaseMonster Schedule and has high priority
	if ( HasCondition( COND_LIGHT_DAMAGE ) ||  HasCondition( COND_HEAVY_DAMAGE ) )
	{
		return SCHED_NONE;
	}

	// If we have just killed the enemy and haven't got anyone else to shoot
	if ( HasCondition( COND_ENEMY_DEAD ) )
	{
		Forget( bits_MEMORY_HUMAN_NO_COVER ); // If my enemy is killed the cover situation may have changed

		if ( IsSquadLeader() )
		{
			// If I am the squad leader, now would be the time to issue some kind of squad order

			if ( m_nLastSquadCommand < SQUADCMD_CHECK_IN )	
			{
				// Try and get a new enemy from one of my squad members
				return SCHED_HUMAN_SIGNAL_CHECK_IN;
			}
			else if ( SquadIsScattered() && m_nLastSquadCommand < SQUADCMD_COME_HERE )
			{
				// Order my squad to report back in and regroup
				return SCHED_HUMAN_SIGNAL_COME_TO_ME;
			} 
			else if ( m_nLastSquadCommand < SQUADCMD_SEARCH_AND_DESTROY )
			{
				// If we have no enemies and are close together it's time for some action
				return SCHED_HUMAN_SIGNAL_SEARCH_AND_DESTROY;
			}
		}

		if ( !IsMedic() && HealthFraction() <= 2.0/3.0 && TimeNotRecent( m_flLastMedicSearch, HUMAN_MEDIC_SEARCH_TIME ) )
		{
			m_flLastMedicSearch = gpGlobals->curtime;
			return SCHED_HUMAN_FIND_MEDIC;
		}
	}

	// Acknowledge 'check in' etc
	int sched = SelectScheduleFromSquadCommand();
	if ( sched != SCHED_NONE )
		return sched;

	// If I killed an enemy then brag about it
	if ( TimeRecent( m_flKilledEnemyTime, 6.0 ) )
	{
		m_flKilledEnemyTime = 0.0f;
		return SCHED_VICTORY_DANCE;
	}

	// no ammo
	if ( HasCondition( COND_NO_PRIMARY_AMMO ) || HasCondition( COND_LOW_PRIMARY_AMMO ) )
	{
		return SCHED_RELOAD;
	}

	return SCHED_NONE;
}

//-----------------------------------------------------------------------------
int CHOEHuman::SelectScheduleCombat( void )
{
	// Squad Commands that are urgent enough to respond to even in the thick of combat
	if ( m_nLastSquadCommand >= SQUADCMD_RETREAT )
	{
		int sched = SelectScheduleFromSquadCommand();
		if (sched != SCHED_NONE)
			return sched;
	}

	// dead enemy
	if ( HasCondition( COND_ENEMY_DEAD ) )
	{
		// call base class, all code to handle dead enemies is centralized there.
		return SCHED_NONE;
	}

	// Heli
	if ( IRelationType( GetEnemy() ) == D_FR )
	{
		return SCHED_NONE;
	}

	// Check if need to reload
	if ( HasCondition( COND_NO_PRIMARY_AMMO ) )
	{
		return SCHED_HIDE_AND_RELOAD;
	}

	// Don't stop and reload if I have some ammo and my enemy is in my face
	if ( HasCondition( COND_LOW_PRIMARY_AMMO ) && !HasCondition( COND_HUMAN_ENEMY_CLOSE ) )
	{
		return SCHED_HIDE_AND_RELOAD;
	}

	// new enemy
	if ( HasCondition( COND_NEW_ENEMY ) && gpGlobals->curtime - GetEnemies()->FirstTimeSeen(GetEnemy()) < 2.0 )
	{
		if ( GetActiveWeapon() == NULL && !( CapabilitiesGet() & bits_CAP_INNATE_MELEE_ATTACK1 ) )
		{
			return SCHED_TAKE_COVER_FROM_ENEMY;
		}
		else if ( IsInSquad() )
		{
//					CSquadMonster *pSquadLeader = MySquadLeader();
//					if ( pSquadLeader ) pSquadLeader->m_fEnemyEluded = FALSE;
			
			if ( !IsSquadLeader() )
			{
				if ( SquadAnyIdle() )	// If anyone in my squad isn't doing anything, inform them
				{
					return SCHED_HUMAN_FOUND_ENEMY;
				}
			}
			else if ( HasCondition( COND_CAN_RANGE_ATTACK1 ) )
			{
				if ( SquadIsHealthy() )
				{
					// Attack this target and signal to my squad to do likewise
					if ( m_nLastSquadCommand < SQUADCMD_ATTACK && OccupyStrategySlotRange( SQUAD_SLOT_HUMAN_ENGAGE1, SQUAD_SLOT_HUMAN_ENGAGE2 ) )
					{
						return SCHED_HUMAN_SIGNAL_ATTACK;
					}
				}
				else if ( m_nLastSquadCommand < SQUADCMD_RETREAT )
				{
					// Decide to fire a few shots and signal an orderly retreat
					return SCHED_HUMAN_SIGNAL_RETREAT;
				}
			}
		}

		return SCHED_WAKE_ANGRY;
	}

	// damaged
	if ( HasCondition( COND_LIGHT_DAMAGE ) || HasCondition( COND_HEAVY_DAMAGE ) )
	{
		if ( HealthFraction() < 1.0/3.0 && TimeNotRecent( m_flLastMedicSearch, HUMAN_MEDIC_SEARCH_TIME ) )
		{
			// Find a medic, this schedule will also call my squad to defend me (or at least the weakest member)
			m_flLastMedicSearch = gpGlobals->curtime;
			return SCHED_HUMAN_FIND_MEDIC_COMBAT;
		}
		else if ( HealthFraction() < 2.0/3.0 && !HasMemory( bits_MEMORY_HUMAN_NO_COVER ) )
		{
			return SCHED_TAKE_COVER_FROM_ENEMY;
		}
#if 0
		else
		{
			return SCHED_SMALL_FLINCH;
		}
#endif
	}

	// can kick
	if ( HasCondition ( COND_CAN_MELEE_ATTACK1 ) )
	{
		return SCHED_MELEE_ATTACK1;
	}

	if ( HasCondition( COND_TOO_CLOSE_TO_ATTACK ) )
	{
		if ( FClassnameIs( GetActiveWeapon(), "weapon_m79" ) ||
			FClassnameIs( GetActiveWeapon(), "weapon_rpg7" ) )
			return SCHED_HUMAN_MOVE_TO_WEAPON_RANGE;

		// This often fails when an NPC gets backed into a corner.
		return SCHED_BACK_AWAY_FROM_ENEMY;
	}

	if ( HasCondition( COND_HUMAN_ENEMY_CLOSE ) && TimeNotRecent( m_flFailedBackAwayTime, 3.0f ) )
	{
		return SCHED_BACK_AWAY_FROM_ENEMY;
	}

	// can shoot
	if ( HasCondition ( COND_CAN_RANGE_ATTACK1 ) )
	{
		if ( IsInSquad() )
		{
	//			CSquadMonster *pSquadLeader = MySquadLeader();
	//			if ( pSquadLeader ) pSquadLeader->m_fEnemyEluded = FALSE;

			// if the enemy has eluded the squad and a squad member has just located the enemy
			// and the enemy does not see the squad member, issue a call to the squad to waste a 
			// little time and give the player a chance to turn.
			if ( !EnemyIsBullseye() && !HasCondition( COND_ENEMY_FACING_ME ) && SquadAnyIdle() )
			{
				return SCHED_HUMAN_FOUND_ENEMY;
			}

			if ( !EnemyIsBullseye() && IsSquadLeader() )
			{
				if ( SquadIsHealthy() && m_nLastSquadCommand < SQUADCMD_ATTACK )
				{
					// We are fit so pro-actively attack the bastards
					return SCHED_HUMAN_SIGNAL_ATTACK;
				}
#if 0 // FIXME - needs some work
				else if ( m_nLastSquadCommand < SQUADCMD_SURPRESSING_FIRE )
				{
					// Things are not looking so good, we are less confident, so lay down surpressing fire
					return SCHED_HUMAN_SIGNAL_SURPRESS;
				}
#endif
			}
		}

		if ( HasCondition( COND_WEAPON_PLAYER_IN_SPREAD ) || 
				HasCondition( COND_WEAPON_BLOCKED_BY_FRIEND ) || 
				HasCondition( COND_WEAPON_SIGHT_OCCLUDED ) )
		{
			extern bool M79FriendInBlastZone( CBaseCombatWeapon *pWeapon );
			if ( M79FriendInBlastZone( GetActiveWeapon() ) )
			{
#ifdef HUMAN_UNSHOOTABLE
				RememberUnshootable( GetEnemy() );
#endif
				// Hack -- A friend is too close to the blast zone of the M79.
				// Don't try to establish line-of-fire, just wait for clearance.
				return SCHED_COMBAT_FACE;
			}

			if ( FClassnameIs( GetActiveWeapon(), "weapon_m79" ) )
				DevMsg( "M79 SCHED_ESTABLISH_LINE_OF_FIRE #1\n" );

			return SCHED_HUMAN_ESTABLISH_LINE_OF_FIRE;
		}

		if ( GetShotRegulator()->IsInRestInterval() )
		{
//			if ( HasCondition(COND_CAN_RANGE_ATTACK1) )
				return SCHED_COMBAT_FACE;
		}

		if ( OccupyStrategySlotRange( SQUAD_SLOT_HUMAN_ENGAGE1,  SQUAD_SLOT_HUMAN_ENGAGE2 ) )
		{
			// try to take an available ENGAGE slot
			return SCHED_RANGE_ATTACK1;
		}
		else if ( CanGrenadeEnemy() && OccupyStrategySlotRange( SQUAD_SLOT_HUMAN_GRENADE1, SQUAD_SLOT_HUMAN_GRENADE2 ) )
		{
			// throw a grenade if can and no engage slots are available
			return SCHED_RANGE_ATTACK2;
		}
		else
		{
			// hide!
			DesireCrouch();
			return SCHED_TAKE_COVER_FROM_ENEMY;
		}
	}

	// can't see enemy (even if I turn round)
	if ( HasCondition( COND_ENEMY_OCCLUDED ) )
	{
		// Squad Commands that I don't care about if I'm in the thick of battle but may pay attention to
		// if I can't see my enemy even if he's not dead
		int sched = SelectScheduleFromSquadCommand();
		if ( sched != SCHED_NONE )
			return sched;

		if ( GetEnemy() )
		{
			// We don't see our enemy. If it hasn't been long since I last saw him,
			// and he's pretty close to the last place I saw him, throw a grenade in 
			// to flush him out. A wee bit of cheating here...

			float flTime;
			float flDist;

			flTime = gpGlobals->curtime - GetEnemies()->LastTimeSeen( GetEnemy() );
			flDist = ( GetEnemy()->GetAbsOrigin() - GetEnemies()->LastSeenPosition( GetEnemy() ) ).Length();

			//Msg("Time: %f   Dist: %f\n", flTime, flDist );
			if ( flTime <= COMBINE_GRENADE_FLUSH_TIME && flDist <= COMBINE_GRENADE_FLUSH_DIST &&
				CanGrenadeEnemy( false ) &&
				OccupyStrategySlotRange( SQUAD_SLOT_HUMAN_GRENADE1, SQUAD_SLOT_HUMAN_GRENADE2 ) )
			{
				return SCHED_RANGE_ATTACK2;
			}
		}

		if ( HealthFraction() < 1.0/3.0 && TimeNotRecent( m_flLastMedicSearch, HUMAN_MEDIC_SEARCH_TIME ) )
		{
			// Find a medic
			m_flLastMedicSearch = gpGlobals->curtime;
			return SCHED_HUMAN_FIND_MEDIC_COMBAT;
		}
		else if ( GetActiveWeapon() )
		{
			// Hack -- A friend is too close to the blast zone of the M79
//			if ( FClassnameIs( GetActiveWeapon(), "weapon_m79" ) )
//				return SCHED_COMBAT_FACE;

			return SCHED_HUMAN_ESTABLISH_LINE_OF_FIRE;
		}
		else
		{
			return SCHED_STANDOFF;
		}
	}

	// If we can see the enemy but can't attack him then we need to establish a line of fire whatever our
	// slot or squad command is
	if ( HasCondition( COND_SEE_ENEMY ) && !HasCondition( COND_CAN_RANGE_ATTACK1 ) )
	{
		if ( GetActiveWeapon() )
		{
			extern bool M79FriendInBlastZone( CBaseCombatWeapon *pWeapon );
			if ( M79FriendInBlastZone( GetActiveWeapon() ) )
			{
#ifdef HUMAN_UNSHOOTABLE
				RememberUnshootable( GetEnemy() );
#endif
				// Hack -- A friend is too close to the blast zone of the M79.
				// Don't try to establish line-of-fire, just wait for clearance.
				return SCHED_COMBAT_FACE;
			}

			if ( FClassnameIs( GetActiveWeapon(), "weapon_m79" ) )
				DevMsg( "M79 SCHED_ESTABLISH_LINE_OF_FIRE #2\n" );

			return SCHED_HUMAN_ESTABLISH_LINE_OF_FIRE;
		}
		else
		{
			return SCHED_TAKE_COVER_FROM_ENEMY;
		}
	}

	return SCHED_NONE;
}

//-----------------------------------------------------------------------------
int CHOEHuman::SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode )
{
	if ( failedSchedule == SCHED_BACK_AWAY_FROM_ENEMY )
	{
		// Most likely backed into a corner.
		m_flFailedBackAwayTime = gpGlobals->curtime;
		return SelectScheduleCombat();
	}

#ifdef HUMAN_UNSHOOTABLE
	// 1) SCHED_ESTABLISH_LINE_OF_FIRE fails due to no shoot position
	// 2) RememberUnshootable is called
	// 3) RunAI calls GatherConditions which gets a new enemy
	// 4) MaintainSchedule calls SelectFailSchedule for the failed SCHED_ESTABLISH_LINE_OF_FIRE which does not apply to the new enemy
	if ( failedSchedule == SCHED_ESTABLISH_LINE_OF_FIRE ||
		 failedSchedule == SCHED_HUMAN_ESTABLISH_LINE_OF_FIRE )
	{
		if ( HasCondition( COND_NEW_ENEMY ) ) // FIXME: could this apply to the same unshootable enemy?
		{
			return SelectSchedule(); // Get a schedule that applies to the new enemy
		}

		// SCHED_ESTABLISH_LINE_OF_FIRE_FALLBACK chases the enemy. Don't bother if we know
		// the enemy is unreachable.
		if ( IsUnreachable( GetEnemy() ) )
		{
			return SCHED_TAKE_COVER_FROM_ENEMY;
		}

		return SCHED_ESTABLISH_LINE_OF_FIRE_FALLBACK;
	}
#endif

	return BaseClass::SelectFailSchedule( failedSchedule, failedTask, taskFailCode );
}

//-----------------------------------------------------------------------------
int CHOEHuman::TranslateSchedule( int scheduleType )
{
	switch( scheduleType )
	{
	case SCHED_ALERT_REACT_TO_COMBAT_SOUND:
		return SCHED_HUMAN_ALERT_REACT_TO_COMBAT_SOUND;
		break;
	case SCHED_ESTABLISH_LINE_OF_FIRE_FALLBACK:
		// Don't chase the enemy if we don't have a shooting position.
		// It may be that friendlies are in the area.
		if ( GetActiveWeapon() && FClassnameIs( GetActiveWeapon(), "weapon_m79" ) )
		{
			return SCHED_COMBAT_FACE;
		}
		break;
	case SCHED_HIDE_AND_RELOAD:
		{
			// from alyx_episodic
			// If I don't have a ranged attacker as an enemy, don't try to hide
			if ( AnyEnemyHasRangedAttack() )
				return SCHED_HIDE_AND_RELOAD;
#if 0
			AIEnemiesIter_t iter;
			for ( AI_EnemyInfo_t *pEMemory = GetEnemies()->GetFirst(&iter); pEMemory != NULL; pEMemory = GetEnemies()->GetNext(&iter) )
			{
				CBaseCombatCharacter *pEnemyBCC = pEMemory->hEnemy.Get()->MyCombatCharacterPointer();
				if ( !pEnemyBCC )
					continue;

				// Ignore enemies that don't hate me
				if ( pEnemyBCC->IRelationType( this ) != D_HT )
					continue;

				// Assume player has ranged weapons
				if ( pEnemyBCC->IsPlayer() )
					return SCHED_HIDE_AND_RELOAD;

				CAI_BaseNPC *pEnemyNPC = pEnemyBCC->MyNPCPointer();
				if ( !pEnemyNPC )
					continue;

				// Look for enemies with ranged capabilities
				if ( pEnemyNPC->CapabilitiesGet() & ( bits_CAP_WEAPON_RANGE_ATTACK1 | bits_CAP_WEAPON_RANGE_ATTACK2 | bits_CAP_INNATE_RANGE_ATTACK1 | bits_CAP_INNATE_RANGE_ATTACK2 ) )
					return SCHED_HIDE_AND_RELOAD;
			}
#endif
			return SCHED_RELOAD; // reload without crouching
		}
		break;
	case SCHED_RELOAD:
		if ( GetState() == NPC_STATE_COMBAT )
		{
			if ( AnyEnemyHasRangedAttack() )
				return SCHED_HUMAN_RELOAD;
#if 0
			AIEnemiesIter_t iter;
			for ( AI_EnemyInfo_t *pEMemory = GetEnemies()->GetFirst(&iter); pEMemory != NULL; pEMemory = GetEnemies()->GetNext(&iter) )
			{
				CBaseCombatCharacter *pEnemyBCC = pEMemory->hEnemy.Get()->MyCombatCharacterPointer();
				if ( !pEnemyBCC )
					continue;

				// Ignore enemies that don't hate me
				if ( pEnemyBCC->IRelationType( this ) != D_HT )
					continue;

				// Assume player has ranged weapons
				if ( pEnemyBCC->IsPlayer() )
					return SCHED_HIDE_AND_RELOAD;

				CAI_BaseNPC *pEnemyNPC = pEnemyBCC->MyNPCPointer();
				if ( !pEnemyNPC )
					continue;

				// Look for enemies with ranged capabilities
				if ( pEnemyNPC->CapabilitiesGet() & ( bits_CAP_WEAPON_RANGE_ATTACK1 | bits_CAP_WEAPON_RANGE_ATTACK2 | bits_CAP_INNATE_RANGE_ATTACK1 | bits_CAP_INNATE_RANGE_ATTACK2 ) )
					return SCHED_HUMAN_RELOAD; // crouch before reloading
			}
#endif
		}
		return SCHED_RELOAD; // reload without crouching
		break;
	case SCHED_FAIL_TAKE_COVER:
		if ( HasCondition( COND_NO_PRIMARY_AMMO ) )
			return SCHED_RELOAD;
		return SCHED_RUN_RANDOM; // run 500 units away from where we are
		break;
	case SCHED_TAKE_COVER_FROM_BEST_SOUND:
		return SCHED_HUMAN_TAKE_COVER_FROM_BEST_SOUND;
		break;
	case SCHED_COWER:
		return SCHED_HUMAN_COWER;
		break;
	case SCHED_INVESTIGATE_SOUND:
		return SCHED_HUMAN_INVESTIGATE_SOUND;
		break;
	case SCHED_VICTORY_DANCE:
		return SCHED_HUMAN_VICTORY_DANCE;
		break;
	case SCHED_RANGE_ATTACK1:
		if ( m_bStopCrouching )
			return SCHED_HUMAN_POPUP_ATTACK;
#ifdef HUMAN_STRAFE
		if ( ShouldStrafe() )
		{
			m_StrafeTimer.Set( 1.0, 2.0 );
			return SCHED_HUMAN_STRAFE;
		}
#endif
		return SCHED_HUMAN_RANGE_ATTACK1;
		break;
	case SCHED_RANGE_ATTACK2:
		return SCHED_HUMAN_RANGE_ATTACK2;
		break;
#if 0
	case SCHED_HUMAN_ESTABLISH_LINE_OF_FIRE:
		if ( GetActiveWeapon() &&
			( FClassnameIs( GetActiveWeapon(), "weapon_rpg7" ) ||
			  FClassnameIs( GetActiveWeapon(), "weapon_m79" ) ) )
			return SCHED_HUMAN_EXPLOSIVE_ELOF;
		return SCHED_HUMAN_ESTABLISH_LINE_OF_FIRE;
		break;
#endif
	case SCHED_FAIL_ESTABLISH_LINE_OF_FIRE:
		return SCHED_HUMAN_STANDOFF;
		break;
	case SCHED_STANDOFF:
		return SCHED_HUMAN_STANDOFF;
		break;
	case SCHED_TARGET_CHASE:
		return SCHED_HUMAN_FOLLOW;
		break;
	case SCHED_TAKE_COVER_FROM_ENEMY:
		if ( m_fHandGrenades )
		{
			if ( random->RandomInt(0, 1) && CanGrenadeEnemy() &&
				!( HasCondition( COND_LIGHT_DAMAGE ) || HasCondition( COND_HEAVY_DAMAGE ) ) &&
				OccupyStrategySlotRange( SQUAD_SLOT_HUMAN_GRENADE1, SQUAD_SLOT_HUMAN_GRENADE2 ) )
			{
				return SCHED_HUMAN_TOSS_GRENADE_COVER;
			}
#if 0
			Vector vecOrigin = GetAbsOrigin();
			if ( random->RandomInt(0, 1) &&
				GetEnemy() != NULL &&
				( GetEnemy()->GetMoveType() == MOVETYPE_WALK ||
				 GetEnemy()->GetMoveType() == MOVETYPE_STEP ) &&
				 (GetEnemyLKP() - vecOrigin).LengthSqr() > Square(256))
			{
				CBaseEntity *pTarget = NULL;
				while ( ( pTarget = gEntList.FindEntityInSphere( pTarget, vecOrigin, COMBINE_MIN_GRENADE_CLEAR_DIST ) ) != NULL )
				{
					if ( pTarget == this || pTarget->MyCombatCharacterPointer() == NULL )
						continue;

					if ( IRelationType( pTarget ) != D_LI )
						continue;

					break;
				}
				if ( pTarget == NULL )
					return SCHED_HUMAN_GRENADE_COVER;
			}
#elif 0
			// FIXME: check for any friendlies, not just squad members. Try CAI_BaseNPC::PlayerInRange() as well
			if ( random->RandomInt(0, 1) && !( IsInSquad() && GetSquad()->SquadMemberInRange( GetLocalOrigin(), 256 ) != NULL ) )
			{
				return SCHED_HUMAN_GRENADE_COVER;
			}
#endif
		}
		return SCHED_HUMAN_TAKE_COVER;
		break;
	case SCHED_HUMAN_TAKECOVER_FAILED:
		Remember( bits_MEMORY_HUMAN_NO_COVER );
//		return SCHED_FAIL;
		return SCHED_RUN_RANDOM;
		break;
	case SCHED_HUMAN_SURPRESS:
		if ( m_bStopCrouching )
			return SCHED_HUMAN_POPUP_ATTACK;
		return SCHED_HUMAN_SURPRESS;
		break;
	case SCHED_IDLE_STAND:
		return SCHED_HUMAN_IDLE_STAND;
		break;
	case SCHED_TARGET_FACE:
//		return SCHED_HUMAN_FACE_TARGET; remove this schedule
		break;
	case SCHED_MELEE_ATTACK1:
		return SCHED_HUMAN_PRIMARY_MELEE_ATTACK1;
		break;
	case SCHED_FAIL:
		return SCHED_HUMAN_FAIL;
		break;
	case SCHED_AISCRIPT:
#if 1
		if ( IsCrouching() )
		{
			ClearForceCrouch();
			Stand();
		}
#else
		if ( IsCrouching() )
			return SCHED_HUMAN_UNCROUCH_SCRIPT;
#endif
		break;
	}

	return BaseClass::TranslateSchedule( scheduleType );
}

static bool TaskRandomCheck( const Task_t *pTask )
{
	return pTask->flTaskData == 0 || random->RandomFloat(0, 1) <= pTask->flTaskData;
}

//-----------------------------------------------------------------------------
int CHOEHuman::MeleeAttack1Conditions( float flDot, float flDist )
{
	// Don't repeat kicks too often, unless we have an explosive weapon (i.e. might not have a choice)
	if ( HasCondition( COND_CAN_RANGE_ATTACK1 ) && TimeRecent( m_flLastKick, HUMAN_KICK_INTERVAL ) )
		return COND_NONE;

	if ( flDist > 64 )
		return COND_NONE; // COND_TOO_FAR_TO_ATTACK;

	if ( flDot < 0.7 )
		return COND_NONE; // COND_NOT_FACING_ATTACK;

	if ( GetEnemy() == NULL )
		return COND_NONE;

	if ( FClassnameIs( GetEnemy(), "npc_snark" ) )
		return COND_NONE;

	// Trace hull forward from hips to just below the feet
	float flWidth = GetHullWidth();
	float flHeight = GetHullHeight();

	Vector center = GetAbsOrigin();
	center.z += flHeight / 4;

	Vector vecMins = Vector( -flWidth/2, -flWidth/2, -flHeight/4 - 6 );
	vecMins.z = -flHeight/4 + 1; // can't be below ground
	Vector vecMaxs = Vector( flWidth/2, flWidth/2, flHeight/4 );

	Vector forward;
	GetVectors( &forward, NULL, NULL );

	trace_t	tr;
	CTraceFilterNav traceFilter( this, false, this, COLLISION_GROUP_NONE );
	AI_TraceHull( center, center + forward * HUMAN_KICK_RANGE, vecMins, vecMaxs, MASK_NPCSOLID, &traceFilter, &tr );

//	NDebugOverlay::Box(center, vecMins, vecMaxs, 0, 255, 255, 0, 0);
//	NDebugOverlay::Box(center + forward * HUMAN_KICK_RANGE, vecMins, vecMaxs, 0, 0, 255, 0, 0);

	if ( tr.fraction == 1.0 || !tr.m_pEnt || tr.m_pEnt != GetEnemy() )
	{
		return COND_NONE;
	}

	return COND_CAN_MELEE_ATTACK1;
}

//-----------------------------------------------------------------------------
bool CHOEHuman::WeaponLOSCondition( const Vector &ownerPos, const Vector &targetPos, bool bSetConditions )
{
	/*
	CAI_BaseNPC::WeaponLOSCondition() calls CBaseCombatWeapon::WeaponLOSCondition() which does a trace line
	from the shooter's Weapon_ShootPosition() to the given target position; targetPos is either the enemy's
	EyePosition() or (if the eye pos can't be hit) a *random* BodyTarget(). If a friendly is in the way
	then CBaseCombatWeapon::WeaponLOSCondition() sets COND_WEAPON_BLOCKED_BY_FRIEND.

	CAI_BaseNPC::WeaponLOSCondition() then calls PlayerInSpread() and IsSquadmateInSpread() to avoid
	hitting a friendly; these checks take into account the weapon's shot spread BUT start from the
	shooter's GetAbsOrigin() NOT WeaponShootPosition().

	When an NPC actually shoots the SMG1 the bullets start at Weapon_ShootPosition() at an angle of
	GetActualShootTrajectory(). GetActualShootTrajectory() calls GetActualShootPosition() which
	calls GetEnemy()->GetBodyTarget() which produces a random shot location; this random shot location is
	almost certainly different than the targetPos passed to CAI_BaseNPC::WeaponLOSCondition().

	The 100% perfect shot location is then affected by CShotManipulator::ApplySpread() using the weapon's
	spread and bias. That vector is then affected by ai_spread_cone_focus_time (0.6) and
	ai_spread_defocused_cone_multiplier (3.0) in GetAttackSpread().
	*/
	return BaseClass::WeaponLOSCondition( ownerPos, targetPos, bSetConditions );
}

//-----------------------------------------------------------------------------
WeaponProficiency_t CHOEHuman::CalcWeaponProficiency( CBaseCombatWeapon *pWeapon )
{
	return WEAPON_PROFICIENCY_PERFECT;
#if 0
	if( FClassnameIs( pWeapon, "weapon_ar2" ) )
	{
		if( hl2_episodic.GetBool() )
		{
			return WEAPON_PROFICIENCY_VERY_GOOD;
		}
		else
		{
			return WEAPON_PROFICIENCY_GOOD;
		}
	}
	else if( FClassnameIs( pWeapon, "weapon_shotgun" )	)
	{
		return WEAPON_PROFICIENCY_PERFECT;
	}
	else if( FClassnameIs( pWeapon, "weapon_smg1" ) )
	{
		return WEAPON_PROFICIENCY_GOOD;
	}

	return BaseClass::CalcWeaponProficiency( pWeapon );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Return true if the combine has grenades, hasn't checked lately, and
//			can throw a grenade at the target point.
// Input  : &vecTarget - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHOEHuman::CanThrowGrenade( const Vector &vecTarget )
{
	if ( !m_fHandGrenades )
	{
		return false;
	}
#if 0 
	if( m_iNumGrenades < 1 )
	{
		// Out of grenades!
		return false;
	}
#endif
	if (gpGlobals->curtime < m_flNextGrenadeCheck )
	{
		// Not allowed to throw another grenade right now.
		return false;
	}

	float flDist;
	flDist = ( vecTarget - GetAbsOrigin() ).Length();

	if( flDist > 1024 || flDist < 128 )
	{
		// Too close or too far!
		m_flNextGrenadeCheck = gpGlobals->curtime + 1; // one full second.
		return false;
	}

	// -----------------------
	// If moving, don't check.
	// -----------------------
	if ( m_flGroundSpeed != 0 )
		return false;

#if 0
	Vector vecEnemyLKP = GetEnemyLKP();
	if ( !( GetEnemy()->GetFlags() & FL_ONGROUND ) && GetEnemy()->GetWaterLevel() == 0 && vecEnemyLKP.z > (GetAbsOrigin().z + WorldAlignMaxs().z)  )
	{
		//!!!BUGBUG - we should make this check movetype and make sure it isn't FLY? Players who jump a lot are unlikely to 
		// be grenaded.
		// don't throw grenades at anything that isn't on the ground!
		return COND_NONE;
	}
#endif
	
	// ---------------------------------------------------------------------
	// Are any of my squad members near the intended grenade impact area?
	// ---------------------------------------------------------------------
	if ( IsInSquad() )
	{
		if ( GetSquad()->SquadMemberInRange( vecTarget, COMBINE_MIN_GRENADE_CLEAR_DIST ))
		{
			// crap, I might blow my own guy up. Don't throw a grenade and don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->curtime + 1; // one full second.

			// Tell my squad members to clear out so I can get a grenade in
			int type = IsPlayerAlly() ? SOUND_CONTEXT_ALLIES_ONLY : SOUND_CONTEXT_COMBINE_ONLY;
			CSoundEnt::InsertSound( SOUND_MOVE_AWAY | type, vecTarget, COMBINE_MIN_GRENADE_CLEAR_DIST, 0.1 );
			return false;
		}
	}
	
	return CheckCanThrowGrenade( vecTarget );
}

//-----------------------------------------------------------------------------
ConVar hoe_human_grenade_debug( "hoe_human_grenade_debug", "0" );
extern ConVar sv_gravity;

bool CHOEHuman::CheckThrowFromPosition( const Vector &shootPos, const Vector &targetPos, Vector *pVecOut )
{
	// CGrenadeFrag size
	Vector vecMins = Vector(-4);
	Vector vecMaxs = Vector(4);

	// ----- First calculate the velocity like VecCheckThrow() -----
	float flSpeed = COMBINE_GRENADE_THROW_SPEED;
	float flGravity = sv_gravity.GetFloat();

	Vector vecGrenadeVel = (targetPos - shootPos);

	// throw at a constant time
	float time = vecGrenadeVel.Length() / flSpeed;
	vecGrenadeVel = vecGrenadeVel * (1.0 / time);

	// adjust upward toss to compensate for gravity loss
	vecGrenadeVel.z += flGravity * time * 0.5;

	////////////////// Using the supplied velocity 
	vecGrenadeVel = *pVecOut;
	//////////////////

	// ----- Now do expensive check like CAI_BaseNPC::ThrowLimit
	Vector vecStart		= shootPos;
	Vector vecEnd		= targetPos;
	Vector vecFrom		= vecStart;

	Vector rawJumpVel = vecGrenadeVel;

	// Calculate the total time of the jump minus a tiny fraction
	float jumpTime		= (vecStart - vecEnd).Length2D()/rawJumpVel.Length2D();
	float timeStep		= jumpTime / 10.0;

	Vector gravity = Vector(0,0,flGravity);

	// this loop takes single steps to the goal.
	for (float flTime = 0; flTime < jumpTime-0.1; flTime += timeStep )
	{
		// Calculate my position after the time step (average velocity over this time step)
		Vector nextPos = vecFrom + (rawJumpVel - 0.5 * gravity * timeStep) * timeStep;

		// If last time step make next position the target position
		if ((flTime + timeStep) > jumpTime)
		{
			nextPos = vecEnd;
		}

		trace_t tr;
		AI_TraceHull( vecFrom, nextPos, vecMins, vecMaxs, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );

		if (tr.startsolid || tr.fraction < 1.0)
		{
			if ( tr.m_pEnt == GetEnemy() || flTime >= jumpTime * 0.9 )
			{
				if ( hoe_human_grenade_debug.GetBool() )
					NDebugOverlay::Line( vecFrom, nextPos, 255, 255, 255, true, 1.0 );

				if ( pVecOut )
					*pVecOut = vecGrenadeVel;
				return true;
			}

			if ( hoe_human_grenade_debug.GetBool() )
				NDebugOverlay::Line( vecFrom, nextPos, 255, 0, 0, true, 1.0 );

			return false;
		}
		else if ( hoe_human_grenade_debug.GetBool() )
		{
			NDebugOverlay::Line( vecFrom, nextPos, 255, 255, 255, true, 1.0 );
		}

		rawJumpVel  = rawJumpVel - gravity * timeStep;
		vecFrom		= nextPos;
	}

	if ( pVecOut )
		*pVecOut = vecGrenadeVel;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if the combine can throw a grenade at the specified target point
// Input  : &vecTarget - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHOEHuman::CheckCanThrowGrenade( const Vector &vecTarget )
{
	//NDebugOverlay::Line( EyePosition(), vecTarget, 0, 255, 0, false, 5 );

	// ---------------------------------------------------------------------
	// Check that throw is legal and clear
	// ---------------------------------------------------------------------
	// FIXME: this is only valid for hand grenades, not RPG's
	Vector vecToss;
	Vector vecMins = -Vector(4,4,4);
	Vector vecMaxs = Vector(4,4,4);
	if( FInViewCone( vecTarget ) && CBaseEntity::FVisible( vecTarget ) )
	{
		vecToss = VecCheckThrow( this, EyePosition(), vecTarget, COMBINE_GRENADE_THROW_SPEED, 1.0, &vecMins, &vecMaxs );
	}
	else
	{
		// Have to try a high toss. Do I have enough room?
		trace_t tr;
		AI_TraceLine( EyePosition(), EyePosition() + Vector( 0, 0, 64 ), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );
		if( tr.fraction != 1.0 )
		{
			return false;
		}

//		vecToss = VecCheckThrow( this, EyePosition(), vecTarget, COMBINE_GRENADE_THROW_SPEED, 1.0, &vecMins, &vecMaxs );
		vecToss = VecCheckToss( this, EyePosition(), vecTarget, -1, 1.0, true, &vecMins, &vecMaxs );
	}

	if ( vecToss != vec3_origin && CheckThrowFromPosition( EyePosition(), vecTarget, &vecToss ) )
	{
		m_vecTossVelocity = vecToss;

		// don't check again for a while.
		m_flNextGrenadeCheck = gpGlobals->curtime + 1; // 1/3 second.
		return true;
	}
	else
	{
		// don't check again for a while.
		m_flNextGrenadeCheck = gpGlobals->curtime + 1; // one full second.
		return false;
	}
}

//-----------------------------------------------------------------------------
bool CHOEHuman::CanGrenadeEnemy( bool bUseFreeKnowledge )
{
	CBaseEntity *pEnemy = GetEnemy();

	Assert( pEnemy != NULL );

	if ( pEnemy )
	{
		// Don't throw grenades at a bullseye attached to a ragdoll (see Event_KilledOther)
		if ( pEnemy->Classify() == CLASS_BULLSEYE )
			return false;

		if ( bUseFreeKnowledge )
		{
			// throw to where we think they are.
			return CanThrowGrenade( GetEnemies()->LastKnownPosition( pEnemy ) );
		}
		else
		{
			// hafta throw to where we last saw them.
			Vector vecTarget = GetEnemies()->LastSeenPosition( pEnemy );
			if ( vecTarget == vec3_invalid || vecTarget == vec3_origin )
			{
				DevWarning( "%s trying to grenade enemy never seen\n", GetDebugName() );
				vecTarget = GetEnemies()->LastKnownPosition( pEnemy );
			}
			return CanThrowGrenade( vecTarget );
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
void CHOEHuman::InputThrowGrenadeAtTarget( inputdata_t &inputdata )
{
	const char *entName = inputdata.value.String();
	CBaseEntity *pEnt = gEntList.FindEntityByName( NULL, entName );
	if ( pEnt == NULL )
	{
		DevWarning( "CHOEHuman::InputThrowGrenadeAtTarget: No such entity '%s'\n", entName );
		return;
	}
	Vector vecPos = pEnt->GetAbsOrigin();

	if ( CheckCanThrowGrenade( pEnt->GetAbsOrigin() ) )
	{
		CBaseEntity *pFrag = Fraggrenade_Create( EyePosition(), vec3_angle, m_vecTossVelocity, vec3_origin, this, 3.5, false );
		IPhysicsObject *pPhys = pFrag->VPhysicsGetObject();
		float damping = 0.0;
		if ( pPhys )
		{
//			pPhys->SetDamping( &damping, &damping );
			pPhys->SetDragCoefficient( &damping, &damping );
#if 0
				Vector unitVel = m_vecTossVelocity;
				VectorNormalize( unitVel );

				float flTest = 2200 / m_vecTossVelocity.Length();

				float flDrag = pPhys->CalculateLinearDrag( m_vecTossVelocity );
				m_vecTossVelocity = m_vecTossVelocity + ( unitVel * ( flDrag * flDrag ) ) / flTest;
			
				pPhys->SetVelocity( &m_vecTossVelocity, NULL );
#endif
		}
	}
}

//-----------------------------------------------------------------------------
bool CHOEHuman::ShouldIgnoreSound( CSound *pSound )
{
	if ( !BaseClass::ShouldIgnoreSound( pSound ) )
	{
		if ( pSound->IsSoundType( SOUND_DANGER ) && !SoundIsVisible(pSound) )
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
bool CHOEHuman::GetHeadPose( CBaseAnimating *pHead, Vector &_origin, QAngle &_angles )
{
#if 1
	// This code will line up the "head" attachment on the helmet model with
	// the "head" attachment on the NPC model.

	int iAttachment = pHead->LookupAttachment( "head" );
	if ( iAttachment != INVALID_ATTACHMENT )
	{
		Vector origin;
		QAngle angles;
		matrix3x4_t localToWorld;
		GetAttachmentLocalSpace( pHead->GetModelPtr(), iAttachment-1, localToWorld );

		MatrixGetColumn( localToWorld, 3, origin );
		MatrixAngles( localToWorld, angles.Base() );

		angles.y -= 90; // studiomdl screws this up?

		_angles = -angles;

		VectorRotate( -origin, -angles, _origin );

		return true;
	}
	return false;
#else
	int attachment = LookupAttachment( "head" );
	if ( attachment != INVALID_ATTACHMENT )
	{
		GetAttachment( attachment, origin, angles );
		angles = GetAbsAngles();
		origin.z += 12.0; // jump up to avoid spawning inside our body
		// Also note the origin of the head model is at its center, which
		// doesn't agree with the attachment point which is at the base of the neck
		return true;
	}
	return false;
#endif
}

//-----------------------------------------------------------------------------
void CHOEHuman::Behead( void )
{
	Vector origin;
	QAngle angles;
//	(void) GetHeadPose( origin, angles );

#if 1
	// These head models have world_interaction bloodsplat which was hacked into
	// CPhysicsProp::VPhysicsCollision
	CPhysicsProp *pGib = assert_cast<CPhysicsProp*>(CreateEntityByName( "prop_physics" ));
	pGib->SetModel( GetHeadModelName() );
	pGib->SetBodygroup( 0, m_nHeadNum );

	GetHeadPose( pGib, origin, angles );
	pGib->SetParent( this, LookupAttachment( "head" ) );
	pGib->SetLocalOrigin( origin );
	pGib->SetLocalAngles( angles );
	pGib->SetParent( NULL );

	pGib->Spawn();
//	pGib->SetMoveType( MOVETYPE_VPHYSICS );
	pGib->SetCollisionGroup( HOECOLLISION_GROUP_CORPSE /*COLLISION_GROUP_DEBRIS*/ );
#else
	CGib *pGib = CREATE_ENTITY( CGib, "gib" );
	pGib->SetAbsOrigin( origin );
	pGib->SetAbsAngles( angles );
	pGib->Spawn( GetHeadModelName() );
	pGib->m_nBody = m_nHeadNum;
	pGib->InitGib( this, 100.0, 150.0 );
	pGib->SetBloodColor( BLOOD_COLOR_RED ); // FIXME: want blood declas on impact
	pGib->m_lifeTime = 1.0f; // no effect with physics model?
#endif

//		NDebugOverlay::Cross3D(origin,Vector(-5,-5,-5),Vector(5,5,5),255,0,0,true,2.0);

	Vector up;
	AngleVectors( angles, NULL, NULL, &up );
	GetAttachment( "neck", origin );
	UTIL_BloodSpray( origin, up, BloodColor(), 4, FX_BLOODSPRAY_ALL );

	if ( DropHelmet() )
		SpawnHelmet( up );

	m_nHeadNum = GetNumHeads();
	SetBodygroup( GetHeadGroupNum(), m_nHeadNum );

	SentenceStop(); // Don't talk if you have no head

	// Put a non-stinking corpse marker on the head so MikeForce can speak the TLK_HEAD concept
	CHOECorpseMarker *pMarker = assert_cast<CHOECorpseMarker *>( CBaseEntity::Create( "hoe_corpse_marker", GetAbsOrigin(), GetAbsAngles(), pGib ) );
	pMarker->InitMarker( pGib, AllocPooledString( "head" ), false );

	m_hHead = pGib;

	if ( m_hHelmet )
	{
		CreateCorpseSolver( pGib, m_hHelmet, true, 4.0f );
	}
}

//-----------------------------------------------------------------------------
void CHOEHuman::InputBehead( inputdata_t &inputdata )
{
	g_vecAttackDir = vec3_origin;
	Behead();
}

//-----------------------------------------------------------------------------
bool CHOEHuman::GetHelmetPose( CBaseAnimating *pHelmet, Vector &_origin, QAngle &_angles )
{
	// This code will line up the "head" attachment on the helmet model with
	// the "head" attachment on the NPC model.

	int iAttachment = pHelmet->LookupAttachment( "head" );
	Assert( iAttachment != INVALID_ATTACHMENT );

	Vector origin;
	QAngle angles;
	matrix3x4_t localToWorld;
	GetAttachmentLocalSpace( pHelmet->GetModelPtr(), iAttachment-1, localToWorld );

	MatrixGetColumn( localToWorld, 3, origin );
	MatrixAngles( localToWorld, angles.Base() );

	angles.y -= 90; // studiomdl screws this up?

	_angles = -angles;

	VectorRotate( -origin, -angles, _origin );

	return true;
}

#if 1 // helmet removal
class CHelmetRemover : public CLogicalEntity
{
public:
	DECLARE_CLASS( CHelmetRemover, CLogicalEntity );
	DECLARE_DATADESC();

	static void CreateHelmetRemover( CBaseEntity *pHelmet, CBaseEntity *pCorpse )
	{
		CBaseEntity *pEnt = CreateEntityByName( "hoe_helmet_remover" );
		Assert( pEnt != NULL );
		if ( pEnt )
		{
			CHelmetRemover *pRemover = assert_cast<CHelmetRemover*>( pEnt );
			pRemover->Init( pHelmet, pCorpse );
			DispatchSpawn( pEnt );
			pEnt->Activate();
		}
	}

	void Init( CBaseEntity *pHelmet, CBaseEntity *pCorpse )
	{
		Assert( pHelmet != NULL );
		Assert( pCorpse != NULL );

		m_hHelmetEnt = pHelmet;
		m_hCorpseEnt = pCorpse;

		FollowEntity( pHelmet );

		g_pNotify->AddEntity( this, pCorpse );
	}

	void NotifySystemEvent( CBaseEntity *pNotify, notify_system_event_t eventType, const notify_system_event_params_t &params )
	{
		BaseClass::NotifySystemEvent( pNotify, eventType, params );
		Assert( pNotify == m_hCorpseEnt );
		if ( eventType == NOTIFY_EVENT_DESTROY )
		{
			CBaseEntity *pEnt = m_hHelmetEnt;
			if ( pEnt )
			{
				pEnt->SetThink( &CBaseEntity::SUB_Vanish );
				pEnt->SetNextThink( gpGlobals->curtime );
			}

			UTIL_Remove( this );
		}
	}

	EHANDLE m_hCorpseEnt;
	EHANDLE m_hHelmetEnt;
};

BEGIN_DATADESC( CHelmetRemover )
	DEFINE_FIELD( m_hCorpseEnt, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hHelmetEnt, FIELD_EHANDLE ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( hoe_helmet_remover, CHelmetRemover );

#endif // helmet removal

//-----------------------------------------------------------------------------
void CHOEHuman::SpawnHelmet( const Vector& impactDir )
{
	const char *szModel = GetHelmetModelName();
	if ( szModel == NULL || szModel[0] == '0' )
		return;

	Vector origin;
	QAngle angles;

	CPhysicsProp *pGib = assert_cast<CPhysicsProp*>(CreateEntityByName( "prop_physics" ));
	pGib->SetModel( szModel );

	GetHelmetPose( pGib, origin, angles );
	pGib->SetParent( this, LookupAttachment( "head" ) );
	pGib->SetLocalOrigin( origin );
	pGib->SetLocalAngles( angles );
	pGib->SetParent( NULL );

	pGib->Spawn();
	pGib->SetCollisionGroup( HOECOLLISION_GROUP_CORPSE/*COLLISION_GROUP_INTERACTIVE_DEBRIS*/ );

	// Gotta pass this to CreateRagdollCorpse
	m_hHelmet = pGib;

return;
	pGib->ApplyAbsVelocityImpulse( impactDir * 200.0f );
	AngularImpulse ang;
	ang.Random( -800.0f, 800.0f );
	pGib->ApplyLocalAngularVelocityImpulse( ang );
}

//------------------------------------------------------------------------------
void CHOEHuman::InputGiveWeapon( inputdata_t &inputdata )
{
	// Give the NPC the specified weapon
	string_t iszWeaponName = inputdata.value.StringID();
	if ( iszWeaponName != NULL_STRING )
	{
		if ( ClassifyPlayerAllyVital() )
		{
			m_iszPendingWeapon = iszWeaponName;
		}
		else
		{
			GiveWeapon( iszWeaponName );
		}

		// Hack - play a soumd.  This is for NPCs picking up weapons from a crate.
		// FIXME: shouldn't hard-code this, perhaps a second argument to this input
		// should specify a sound to play (or a boolean to play this sound or not).
		EmitSound( "GiveWeapon" );
	}
}

//------------------------------------------------------------------------------
void CHOEHuman::InputDropWeapon( inputdata_t &inputdata )
{
	if ( GetActiveWeapon() )
	{
		Weapon_Drop( GetActiveWeapon() );
	}
}

//-----------------------------------------------------------------------------
bool CHOEHuman::CanEnterVehicle( void )
{
	return true; // FIXME: check various conditions
}

//-----------------------------------------------------------------------------
bool CHOEHuman::CanExitVehicle( void )
{
	// See if we can exit our vehicle
	INPCPassengerCarrier *pCarrier = dynamic_cast<INPCPassengerCarrier *>( m_PassengerBehavior.GetTargetVehicle() );
	if ( pCarrier != NULL && pCarrier->NPC_CanExitVehicle( this, true ) == false )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
void CHOEHuman::EnterVehicle( CBaseEntity *pEntityVehicle, string_t sRole, bool bImmediateEnter )
{
	// Must be allowed to do this
	if ( CanEnterVehicle() == false )
		return;

	// Find the target vehicle
	INPCPassengerCarrier *pCarrier = dynamic_cast<INPCPassengerCarrier *>( pEntityVehicle );

	// Get in the vehicle if it's valid
	if ( pCarrier != NULL && pCarrier->NPC_CanEnterVehicle( this, true ) )
	{
		// Set her into a "passenger" behavior
		m_PassengerBehavior.m_sRoleName = sRole;
		m_PassengerBehavior.Enable( pEntityVehicle, bImmediateEnter );
		m_PassengerBehavior.EnterVehicle();

		// Only do this if we're outside the vehicle
		if ( m_PassengerBehavior.GetPassengerState() == PASSENGER_STATE_OUTSIDE )
		{
			SetCondition( COND_HUMAN_BECOMING_PASSENGER );
		}
	}
}

//-----------------------------------------------------------------------------
void CHOEHuman::EnterVehicleInputHelper( inputdata_t &inputdata, bool bImmediateEnter )
{
	char sBuf[MAX_PATH];
	Q_strncpy( sBuf, inputdata.value.String(), sizeof(sBuf) );
	char *sVehicleName = sBuf;
	char *sRole = strchr( sBuf, ':' );
	if ( sVehicleName[0] && sRole && sRole[1])
	{
		*sRole = '\0';
		sRole += 1;
		CBaseEntity *pEntity = FindNamedEntity( sVehicleName );
		if ( pEntity == NULL )
		{
			Warning("CHOEHuman::InputEnterVehicle: can't find entity named \"%s\".\n",
				sVehicleName);
			return;
		}
		IServerVehicle *pServerVehicle = pEntity->GetServerVehicle();
		if ( pServerVehicle == NULL )
		{
			Warning("CHOEHuman::InputEnterVehicle: entity named \"%s\" isn't a vehicle.\n",
				sVehicleName);
			return;
		}
		INPCPassengerCarrier *pCarrier = dynamic_cast<INPCPassengerCarrier *>( pEntity );
		if ( pCarrier == NULL )
		{
			Warning("CHOEHuman::InputEnterVehicle: vehicle named \"%s\" doesn't accept passengers.\n",
				sVehicleName);
			return;
		}
		string_t strRoleName = AllocPooledString( sRole );
		if ( pServerVehicle->NPC_HasAvailableSeat( strRoleName ) == false )
		{
			Warning("CHOEHuman::InputEnterVehicle: vehicle named \"%s\" has no available seat for role \"%s\".\n",
				sVehicleName, sRole );
			return;
		}
		EnterVehicle( pEntity, strRoleName, bImmediateEnter );
	}
	else
	{
		Warning("CHOEHuman::InputEnterVehicle: got \"%s\" expected \"vehicle:role\".\n",
			sBuf);
	}
}

//-----------------------------------------------------------------------------
void CHOEHuman::InputEnterVehicle( inputdata_t &inputdata )
{
	EnterVehicleInputHelper( inputdata, false );
}

//-----------------------------------------------------------------------------
void CHOEHuman::InputEnterVehicleImmediate( inputdata_t &inputdata )
{
	EnterVehicleInputHelper( inputdata, true );
}

//-----------------------------------------------------------------------------
void CHOEHuman::InputExitVehicle( inputdata_t &inputdata )
{
	// See if we're allowed to exit the vehicle
	if ( CanExitVehicle() == false )
		return;

	m_PassengerBehavior.ExitVehicle();
}

//-----------------------------------------------------------------------------
void CHOEHuman::InputFinishedEnterVehicle( inputdata_t &inputdata )
{
	m_OnFinishedEnterVehicle.FireOutput( this, this );
}

//-----------------------------------------------------------------------------
void CHOEHuman::InputFinishedExitVehicle( inputdata_t &inputdata )
{
	m_OnFinishedExitVehicle.FireOutput( this, this );
}

//------------------------------------------------------------------------------
void CHOEHuman::InputCrouch( inputdata_t &inputdata )
{
	m_bCrouchLocked = true;
	ForceCrouch();
}

//-----------------------------------------------------------------------------
bool CHOEHuman::IsCrouchedActivity( Activity activity )
{
	Activity realActivity = TranslateActivity(activity);

	if ( realActivity == ACT_IDLE_ANGRY_LOW )
		return true;

	return BaseClass::IsCrouchedActivity(activity);
}

//-----------------------------------------------------------------------------
bool CHOEHuman::ShouldLookForBetterWeapon( void )
{
	// Don't search for a weapon if we already have one. We only pick up
	// a weapon if we dropped ours after we were grabbed by a barnacle.
	if ( GetActiveWeapon() != NULL )
		return false;
	
	// FIXME: if prisoner return false

	return BaseClass::ShouldLookForBetterWeapon();
}

//-----------------------------------------------------------------------------
// For picking up a better weapon
bool CHOEHuman::Weapon_CanUse( CBaseCombatWeapon *pWeapon )
{
	for ( int i = 0; GetWeaponClasses()[i]; i++ )
	{
		if ( FClassnameIs( pWeapon, GetWeaponClasses()[i] ) )
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
void CHOEHuman::AnalyzeGunfireSound( CSound *pSound )
{
	Assert( pSound != NULL );

	if ( GetState() != NPC_STATE_ALERT && GetState() != NPC_STATE_IDLE )
	{
		// Only have code for IDLE and ALERT now. 
		return;
	}

	// Have to verify a bunch of stuff about the sound. It must have a valid BaseCombatCharacter as the owner,
	// must have a valid target, and we need a valid pointer to the player.
	if ( pSound->m_hOwner.Get() == NULL )
		return;

	if ( pSound->m_hTarget.Get() == NULL )
		return;

	CBaseCombatCharacter *pSoundOriginBCC = pSound->m_hOwner->MyCombatCharacterPointer();
	if( pSoundOriginBCC == NULL )
		return;

	CBaseEntity *pSoundTarget = pSound->m_hTarget.Get();

	if ( pSoundTarget == this )
	{
		// The shooter is firing at me. Assume if Alyx can hear the gunfire, she can deduce its origin.
		UpdateEnemyMemory( pSoundOriginBCC, pSoundOriginBCC->GetAbsOrigin(), this );

#if 1
		// If a enemy huey is spraying bullets around the NPC then react as in TraceAttack.
		if ( FClassnameIs( pSoundOriginBCC, "npc_huey" ) && 
			BaseClass::IRelationType( pSoundOriginBCC ) == D_HT )
		{
			m_flAttackedByHueyTime = gpGlobals->curtime;
		}
#endif
		return;
	}

	if ( !IsPlayerAlly() ) // FIXME: and not provoked by player
		return;

	CBasePlayer *pPlayer = AI_GetSinglePlayer();
	Assert( pPlayer != NULL );

	if ( pSoundTarget == pPlayer )
	{
		// The shooter is firing at the player. Assume Alyx can deduce the origin if the player COULD see the origin, and Alyx COULD see the player.
		if ( pPlayer->FVisible( pSoundOriginBCC ) && FVisible( pPlayer ) )
		{
			UpdateEnemyMemory( pSoundOriginBCC, pSoundOriginBCC->GetAbsOrigin(), this );
		}
		return;
	}
}

//-----------------------------------------------------------------------------
void CHOEHuman::StartTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_ANNOUNCE_ATTACK: // FIXME: unused?
		BaseClass::StartTask( pTask );
		break;
#if 0
	case TASK_HUMAN_EXPLOSION_FLY:
		SetIdealActivity( ACT_EXPLOSION_FLY );
		SetTouch( &CHOEHuman::ExplFlyTouch );
		break;
#endif
	case TASK_FACE_ENEMY:
		{
			BaseClass::StartTask( pTask );

			// HOE soliders have a twisted combat stance which reveals a lack of a look-target.
			// See CAI_BaseActor::StartTaskRangeAttack1.
			// This is for the case of long shot-regulator rest intervals (i.e 870)
			if ( GetEnemy() && FacingIdeal() )
				AddLookTarget( GetEnemy(), 1.0, 0.5, 0.2 );
		}
		break;
#if 1
	case TASK_GET_PATH_TO_ENEMY_LOS:
	case TASK_GET_PATH_TO_ENEMY_LKP_LOS:
		{
			if ( GetEnemy() == NULL )
			{
				TaskFail(FAIL_NO_ENEMY);
				return;
			}
		
			AI_PROFILE_SCOPE(CAI_BaseNPC_FindLosToEnemy);
			float flMaxRange = 2000;
			float flMinRange = 0;
			
			if ( GetActiveWeapon() )
			{
				flMaxRange = max( GetActiveWeapon()->m_fMaxRange1, GetActiveWeapon()->m_fMaxRange2 );
				flMinRange = min( GetActiveWeapon()->m_fMinRange1, GetActiveWeapon()->m_fMinRange2 );

				// Keep running until we can stop, turn and shoot
				if ( IsCurSchedule( SCHED_HUMAN_MOVE_TO_WEAPON_RANGE ) ||
					IsCurSchedule( SCHED_ESTABLISH_LINE_OF_FIRE ) )
				{
					if ( FClassnameIs( GetActiveWeapon(), "weapon_m79" ) )
						flMinRange *= 2;

					if ( FClassnameIs( GetActiveWeapon(), "weapon_rpg7" ) )
						flMinRange *= 1.5;

					// For fleeing zombies get a min distance away
					if ( flMinRange < 384 )
						flMinRange = 384;
				}
			}

			//Check against NPC's max range
			if ( flMaxRange > m_flDistTooFar )
			{
				flMaxRange = m_flDistTooFar;
			}

			Vector vecEnemy 	= ( pTask->iTask == TASK_GET_PATH_TO_ENEMY_LOS ) ? GetEnemy()->GetAbsOrigin() : GetEnemyLKP();
			Vector vecEnemyEye	= vecEnemy + GetEnemy()->GetViewOffset();

			Vector posLos;
			bool found = false;

			if ( GetTacticalServices()->FindLateralLos( vecEnemyEye, &posLos ) )
			{
				float dist = ( posLos - vecEnemyEye ).Length();
				if ( dist < flMaxRange && dist > flMinRange )
					found = true;
			}

			if ( !found && GetTacticalServices()->FindLos( vecEnemy, vecEnemyEye, flMinRange, flMaxRange, 1.0, &posLos ) )
			{
				found = true;
			}

			if ( !found )
			{
				TaskFail( FAIL_NO_SHOOT );
#ifdef HUMAN_UNSHOOTABLE
				if ( GetEnemy() )
				{
					// We couldn't go anywhere to shoot our best enemy. Remember the enemy is
					// unshootable for a few seconds during which time his relationship priority
					// is lowered.  If another enemy is shootable we want to attack him.
					RememberUnshootable( GetEnemy() );
				}
#endif
			}
			else
			{
				// else drop into run task to offer an interrupt
				m_vInterruptSavePosition = posLos;
			}
		}
		break;
#endif
#ifdef HUMAN_STRAFE
	case TASK_GET_STRAFE_PATH:
		{
			if ( GetEnemy() == NULL )
			{
				TaskFail(FAIL_NO_ENEMY);
				return;
			}
		
			float flMaxRange = 2000;
			float flMinRange = 0;

			if ( GetActiveWeapon() )
			{
				flMaxRange = max( GetActiveWeapon()->m_fMaxRange1, GetActiveWeapon()->m_fMaxRange2 );
				flMinRange = min( GetActiveWeapon()->m_fMinRange1, GetActiveWeapon()->m_fMinRange2 );
			}

			//Check against NPC's max range
			if ( flMaxRange > m_flDistTooFar )
			{
				flMaxRange = m_flDistTooFar;
			}

			Vector vecEnemy 	= GetEnemy()->GetAbsOrigin();
			Vector vecEnemyEye	= vecEnemy + GetEnemy()->GetViewOffset();

			Vector posLos;
			bool found = false;

			if ( GetTacticalServices()->FindStrafeLos( vecEnemyEye, 32, 96, &posLos ) )
			{
				float dist = ( posLos - vecEnemyEye ).Length();
				if ( dist < flMaxRange && dist > flMinRange )
					found = true;
			}

			if ( !found )
			{
				TaskFail( FAIL_NO_SHOOT );
			}
			else
			{
				// else drop into run task to offer an interrupt
				m_vInterruptSavePosition = posLos;
			}
		}
		break;
#endif // HUMAN_STRAFE
	// from alyx_episodic
	case TASK_REACT_TO_COMBAT_SOUND:
		{
			CSound *pSound = GetBestSound();

			if ( pSound && pSound->IsSoundType( SOUND_COMBAT ) && pSound->IsSoundType( SOUND_CONTEXT_GUNFIRE ) )
			{
				AnalyzeGunfireSound( pSound );
			}

			TaskComplete();
		}
		break;
	case TASK_WAIT_FOR_SPEAK_FINISH:
		{
			if ( !IsSpeaking() )
				return BaseClass::StartTask( pTask );
		}
		break;
	case TASK_HUMAN_GET_PATH_TO_RANDOM_NODE:
		{
			ChainStartTask( TASK_GET_PATH_TO_RANDOM_NODE, m_GrenadeInteraction.flRadius );
		}
		break;
	case TASK_HUMAN_CHECK_FIRE:
		if ( NoFriendlyFire() )
			TaskComplete();
		else
			TaskFail( FAIL_NO_SHOOT );
		break;
	case TASK_HUMAN_CROUCH:
		m_flCrouchTime = gpGlobals->curtime + HUMAN_CROUCH_TIME;
		if ( IsCrouching() || HasSpawnFlags( SF_HUMAN_PREDISASTER ) || GetWaterLevel() == 3 )
			TaskComplete();
		else
#if 1 // let the transition graph determine the crouch animation to play
		{
			ForceCrouch();
			TaskComplete();
		}
#else
			SetIdealActivity( ACT_CROUCH );
#endif
		break;
	case TASK_HUMAN_EXPLOSION_FLY:
		Msg("TASK_HUMAN_EXPLOSION_FLY\n");
		TaskFail( NO_TASK_FAILURE );
		break;
	case TASK_HUMAN_EXPLOSION_LAND:
	case TASK_HUMAN_EYECONTACT:
	case TASK_HUMAN_FACE_TOSS_DIR:
		break;
	case TASK_HUMAN_FIND_MEDIC:
		// First try looking for a medic in my squad
		if ( IsInSquad() )
		{
			AISquadIter_t iter;
			CAI_BaseNPC *pSquadmate;
			for ( pSquadmate = m_pSquad->GetFirstMember( &iter );
				pSquadmate != NULL;
				pSquadmate = m_pSquad->GetNextMember( &iter ))
			{
				if ( pSquadmate == this || !pSquadmate->IsAlive() || !pSquadmate->IsMedic() )
					continue;

				// If I'm in the player's squad I shouldn't violate the rules of the
				// follow behavior when chasing a medic.
				if ( (pSquadmate->GetAbsOrigin() - GetAbsOrigin()).LengthSqr() > Square( 768.0f ) )
					continue;

				CHOEHuman *pHuman = HumanPointer( pSquadmate );
				if ( !pHuman )
					continue;

				StartFollowing( pHuman );
				if ( !pHuman->IsFollowing() )
				{
					pHuman->SetCondition( COND_PROVOKED );
					pHuman->StartFollowing( this );
				}
				TaskComplete();
				return;
			}
		}

		// If that doesn't work, just look around and see if I can SEE a medic
		{
			CHOEHuman *pMedic = NULL;
			float flDistToClosest = FLT_MAX;

			for ( int i = 0; GetFriendClasses()[i] ; i++ )
			{
				const char *pszFriend = GetFriendClasses()[i];
		
				if ( !Q_strcmp( pszFriend, "player" ) )
					continue;

				CBaseEntity *pEntity = NULL;
				while ( ( pEntity = gEntList.FindEntityByClassnameWithin( pEntity, pszFriend, GetAbsOrigin(), 1024.0f ) ) != 0 )
				{
					CHOEHuman *pHuman = HumanPointer( pEntity );
					if ( pHuman == NULL || pHuman == this || !pHuman->IsAlive() || !pHuman->IsMedic() )
						continue;

					float flDist2 = (pEntity->GetAbsOrigin() - GetAbsOrigin()).LengthSqr();
					if ( flDistToClosest < flDist2 )
						continue;
					
					trace_t tr;
					UTIL_TraceLine( EyePosition(), pHuman->EyePosition(), MASK_OPAQUE, this, COLLISION_GROUP_NONE, &tr );
					if ( tr.fraction < 1.0f )
						continue;

					pMedic = pHuman;
					flDistToClosest = flDist2;
				}
			}

			if ( pMedic != NULL )
			{
				StartFollowing( pMedic );
				if ( !pMedic->IsFollowing() )
				{
					pMedic->SetCondition( COND_PROVOKED );
					pMedic->StartFollowing( this );
				}
				TaskComplete();
				return;
			}
		}

		// If still can't find one, suffer in silence
		// And don't try to look for one again for a while
		TaskFail( FAIL_NO_TARGET );
		break;
	case TASK_HUMAN_GET_EXPLOSIVE_PATH_TO_ENEMY: /* see TASK_GET_PATH_TO_ENEMY_LKP_LOS */
		{
			if ( GetEnemy() == NULL )
			{
				TaskFail(FAIL_NO_ENEMY);
				return;
			}
		
			float flMaxRange = HUMAN_EXPLOSIVE_MAX_RANGE;
			float flMinRange = HUMAN_EXPLOSIVE_MIN_RANGE;
			
			Vector vecEnemy 	= GetEnemyLKP();
			Vector vecEnemyEye	= vecEnemy + GetEnemy()->GetViewOffset();

			Vector posLos;
			bool found = false;

			if ( GetTacticalServices()->FindLateralLos( vecEnemyEye, &posLos ) )
			{
				float dist = ( posLos - vecEnemyEye ).Length();
				if ( dist < flMaxRange && dist > flMinRange )
					found = true;
			}

			if ( !found && GetTacticalServices()->FindLos( vecEnemy, vecEnemyEye, flMinRange, flMaxRange, 1.0, &posLos ) )
			{
				found = true;
			}

			if ( !found )
			{
				TaskFail( FAIL_NO_SHOOT );
			}
			else
			{
				TaskComplete();
			}
		}
		break;
	case TASK_HUMAN_GET_MELEE_PATH_TO_ENEMY: /* TASK_GET_PATH_TO_TARGET */
		{
			if ( GetEnemy() == NULL )
			{
				TaskFail(FAIL_NO_ENEMY);
				break;
			}
			if ( (GetEnemy()->GetAbsOrigin() - GetLocalOrigin()).Length() < 1 )
			{
				TaskComplete();
				break;
			}
			AI_NavGoal_t goal( static_cast<const Vector&>(GetEnemy()->EyePosition()) );
			goal.pTarget = GetEnemy();
			GetNavigator()->SetGoal( goal );
			GetNavigator()->SetArrivalDistance( HUMAN_KICK_RANGE /* pTask->flTaskData */ );
		}
		break;
	case TASK_HUMAN_IDEALYAW:
		if ( GetTalkTarget() != NULL )
			GetMotor()->SetIdealYawToTarget( GetTalkTarget()->WorldSpaceCenter() );
		else if ( GetListenTarget() != NULL )
			GetMotor()->SetIdealYawToTarget( GetListenTarget()->WorldSpaceCenter() );
		TaskComplete();
		break;
	case TASK_HUMAN_PLAY_SEQUENCE_WAIT:
		SetIdealActivity( (Activity)(int)pTask->flTaskData );
		DelayMoveStart( 10.0f );
		break;
	/* TASK_HUMAN_RETREAT_FACE unused */
	case TASK_HUMAN_SOUND_ATTACK:
		if ( /*TaskRandomCheck( pTask ) &&*/ OkToShout() /* && SpokeRecently( 10.0 ) <-- quiet! */ )
		{
#ifdef HOE_HUMAN_RR
			SpeechSelection_t selection;
			if ( SelectSpeechResponse( TLK_SQUAD_ATTACK, NULL, selection ) )
			{
				if ( GetSpeechManager() )
				{
					GetSpeechManager()->ExtendSpeechConceptTimer( TLK_ALERT, 5 );
					GetSpeechManager()->ExtendSpeechConceptTimer( TLK_CHARGE, 5 );
				}
				DispatchSpeechSelection( selection );
				SetIdealActivity( ACT_SIGNAL1 );
			}
#else
			Speak( "ATTACK" );
#endif
		}
		else
			TaskComplete();
		break;
	case TASK_HUMAN_SOUND_CHARGE:
		if ( !EnemyIsBullseye() && /*TaskRandomCheck( pTask ) && */
			OkToShout() && GetEnemy() != NULL )
		{
#ifdef HOE_HUMAN_RR
			SpeechSelection_t selection;
			if ( SelectSpeechResponse( TLK_CHARGE, NULL, selection ) )
			{
				SetTalkTarget( GetEnemy() );
				DispatchSpeechSelection( selection );
			}
#else
			SetTalkTarget( GetEnemy() );
			Speak( EnemyIsZombie() ? "ZOMBIE" : "CHARGE" );
#endif
		}
		TaskComplete();
		break;
	case TASK_HUMAN_SOUND_CHECK_IN:
		if ( /*TaskRandomCheck( pTask ) &&*/ OkToShout() )
		{
			m_flWaitFinished = gpGlobals->curtime + 1;
#ifdef HOE_HUMAN_RR
			Speak( TLK_SQUAD_CHECK );
#else
			Speak( "CHECK" );
#endif
		}
		else
			TaskComplete();
		break;
	case TASK_HUMAN_SOUND_CLEAR:
		{
			CAI_Squad *pSquad = GetSquad();
			if ( pSquad && !IsSquadLeader() )
			{
				int index = pSquad->GetSquadIndex( this );
				m_flWaitFinished = gpGlobals->curtime + index;
			}
			else
				TaskFail( NO_TASK_FAILURE );
		}
		break;
	case TASK_HUMAN_SOUND_COME_TO_ME:
		if ( TaskRandomCheck( pTask ) && OkToShout() )
		{
			SetIdealActivity( ACT_SIGNAL3 );
#ifdef HOE_HUMAN_RR
			Speak( TLK_SQUAD_COME );
#else
			Speak( "COME" );
#endif
		}
		else
			TaskComplete();
		break;
	case TASK_HUMAN_SOUND_COVER:
		if ( TaskRandomCheck( pTask ) && OkToShout() &&
			HasCondition( COND_SEE_ENEMY ) )
		{
#ifdef HOE_HUMAN_RR
			SpeechSelection_t selection;
			if ( SelectSpeechResponse( TLK_COVER, NULL, selection ) )
			{
				DispatchSpeechSelection( selection );
			}
#else
			SetTalkTarget( GetSquadLeader() );
			Speak( "COVER" );
#endif
		}
		TaskComplete();
		break;
	case TASK_HUMAN_SOUND_EXPL:
		if ( TaskRandomCheck( pTask ) && HasAHead() )
		{
#ifdef HOE_HUMAN_RR
			Speak( TLK_EXPL );
#else
			Speak( "EXPL" );
#endif
			SquadIssueCommand( SQUADCMD_DISTRESS );
		}
		TaskComplete();
		break;
	case TASK_HUMAN_SOUND_FOUND_ENEMY:
		if ( TaskRandomCheck( pTask ) && OkToShout() )
		{
#ifdef HOE_HUMAN_RR
			FoundEnemySound();
#else
			Speak( "FOUND" );
#endif
			SetIdealActivity( ACT_SIGNAL1 );
		}
		else
			TaskComplete();
		break;
#if 0 // FIXME: unused
	case TASK_HUMAN_SOUND_GRENADE:
		if ( TaskRandomCheck( pTask ) && OkToShout() )
		{
			GrenadeSound();
		}
		TaskComplete();
		break;
	case TASK_HUMAN_SOUND_HEALED:
		if ( TaskRandomCheck( pTask ) && OkToSpeak() )
		{
			SetTalkTarget( GetTarget() );
			Speak( "HEALED" );
		}
		TaskComplete();
		break;
#endif
	case TASK_HUMAN_SOUND_HEAR:
		if ( TaskRandomCheck( pTask ) && OkToSpeak() )
		{
#ifdef HOE_HUMAN_RR
			// Humans seem to hear something when they spawn so use the idle-speech
			// delay.
			if ( TimePassed( m_flIdleSpeechDelay ) && !SpokeConcept( TLK_HEAR ) )
			{
				SpeechSelection_t selection;
				if ( SelectSpeechResponse( TLK_HEAR, NULL, selection ) )
				{
					DispatchSpeechSelection( selection );
				}
			}
#else
			if ( !SpokeConcept( HUMAN_SC_HEAR ) )
				Speak( "HEAR", HUMAN_SC_HEAR );
#endif
		}
		TaskComplete();
		break;
	case TASK_HUMAN_SOUND_HELP:
		if ( TaskRandomCheck( pTask ) && OkToShout() )
		{
#ifdef HOE_HUMAN_RR
			Speak( TLK_HELP );
#else
			Speak( "HELP" );
#endif
			SquadIssueCommand( SQUADCMD_DEFENSE );
		}
		TaskComplete();
		break;
	case TASK_HUMAN_SOUND_MEDIC:
		if ( /*TaskRandomCheck( pTask ) &&*/ OkToShout() )
		{
#ifdef HOE_HUMAN_RR
			SpeechSelection_t selection;
			if ( TimeNotRecent( m_flTimeInjuriesMentioned, 30.0f ) &&
				TimeRecent( GetLastDamageTime(), 30.0f ) &&
				SelectSpeechResponse( TLK_MEDIC, NULL, selection ) )
			{
				DispatchSpeechSelection( selection );

				// Don't talk/be-talked-to about my injuries for a while
				m_flTimeInjuriesMentioned = gpGlobals->curtime;

				// Don't let others talk about any injuries for a while
				if ( GetSpeechManager() )
					GetSpeechManager()->ExtendSpeechCategoryTimer( SPEECH_CATEGORY_INJURY, 20 );
			}
#else
			Speak( "MEDIC" );
#endif
		}
		TaskComplete();
		break;
	case TASK_HUMAN_SOUND_RESPOND:
		if ( TaskRandomCheck( pTask ) && OkToSpeak() )
		{
			Speak( "ANSWER" );
		}
		TaskComplete();
		break;
	case TASK_HUMAN_SOUND_RETREAT:
		if ( TaskRandomCheck( pTask ) && OkToShout() )
		{
			SetIdealActivity( ACT_SIGNAL3 );
#ifdef HOE_HUMAN_RR
			Speak( TLK_SQUAD_RETREAT );
#else
			Speak( "RETREAT" );
#endif
		}
		else
			TaskComplete();
		break;
	case TASK_HUMAN_SOUND_RETREATING:
		if ( TaskRandomCheck( pTask ) && OkToShout() && !m_fSquadCmdAcknowledged )
		{
#ifdef HOE_HUMAN_RR
			Speak( TLK_SQUAD_ACK_RETREAT );
#else
			SetTalkTarget( GetSquadLeader() );
			Speak( "RETREATING" );
#endif
			m_fSquadCmdAcknowledged = true;
		}
		TaskComplete();
		break;
	case TASK_HUMAN_SOUND_SEARCHING:
		if ( TaskRandomCheck( pTask ) && OkToShout() && !m_fSquadCmdAcknowledged )
		{
#ifdef HOE_HUMAN_RR
			Speak( TLK_SQUAD_ACK_SEARCH );
#else
			SetTalkTarget( GetSquadLeader() );
			Speak( "SEARCHING" );
#endif
			m_fSquadCmdAcknowledged = true;
		}
		TaskComplete();
		break;
	case TASK_HUMAN_SOUND_SEARCH_AND_DESTROY:
		if ( TaskRandomCheck( pTask ) && OkToShout() )
		{
			SetIdealActivity( ACT_SIGNAL2 );
#ifdef HOE_HUMAN_RR
			Speak( TLK_SQUAD_SEARCH );
#else
			Speak( "SEARCH" );
#endif
		}
		else
			TaskComplete();
		break;
	case TASK_HUMAN_SOUND_SURPRESS:
		if ( TaskRandomCheck( pTask ) && OkToShout() )
		{
#ifdef HOE_HUMAN_RR
			Speak( TLK_SQUAD_SUPPRESS );
#else
			Speak( "SURPRESS" );
#endif
			SetIdealActivity( ACT_SIGNAL1 );
		}
		else
			TaskComplete();
		break;
	case TASK_HUMAN_SOUND_SURPRESSING:
		if ( TaskRandomCheck( pTask ) && OkToShout() && !m_fSquadCmdAcknowledged )
		{
#ifdef HOE_HUMAN_RR
			Speak( TLK_SQUAD_ACK_SUPPRESS );
#else
			SetTalkTarget( GetSquadLeader() );
			Speak( "SURPRESSING" );
#endif
			m_fSquadCmdAcknowledged = true;
		}
		TaskComplete();
		break;
	case TASK_HUMAN_SOUND_TAUNT:
		// Medics shouldn't shout "Come out and fight you coward!" since they want to hide
		if ( !IsMedic() /*&& TaskRandomCheck( pTask )*/ && OkToShout() )
		{
#ifdef HOE_HUMAN_RR
			SpeechSelection_t selection;
			if ( SelectSpeechResponse( TLK_TAUNT, NULL, selection ) )
			{
				SetTalkTarget( GetEnemy() );
				DispatchSpeechSelection( selection );
			}
#else
			SetTalkTarget( GetEnemy() );
			Speak( "TAUNT" );
#endif
		}
		TaskComplete();
		break;
	case TASK_HUMAN_SOUND_THROW:
		if ( TaskRandomCheck( pTask ) && OkToShout() )
		{
#ifdef HOE_HUMAN_RR
			Speak( TLK_THROW );
#else
			Speak( "THROW" );
#endif
		}
		TaskComplete();
		break;
	case TASK_HUMAN_SOUND_VICTORY:
		if ( /*TaskRandomCheck( pTask ) &&*/ OkToShout() )
		{
#ifdef HOE_HUMAN_RR
			SpeechSelection_t selection;
			if ( SelectSpeechResponse( TLK_KILL, NULL, selection ) )
				DispatchSpeechSelection( selection );
#else
			Speak( "KILL" );
#endif
		}
		TaskComplete();
		break;
	case TASK_HUMAN_UNCROUCH:
#if 1
		if ( IsCrouching() && !m_bCrouchLocked )
		{
			ClearForceCrouch();
			Stand();
		}
		TaskComplete();
#else
		if ( !IsCrouching() )
			TaskComplete();
		else
			SetIdealActivity( ACT_STAND );
#endif
		break;
	case TASK_HUMAN_WAIT_GOAL_VISIBLE:
		/* nothing */
		break;

	case TASK_HUMAN_DEFER_SQUAD_GRENADES:
	{
		if ( IsInSquad() )
		{
			// iterate my squad and stop everyone from throwing grenades for a little while.
			AISquadIter_t iter;
			CAI_BaseNPC *pSquadmate = GetSquad()->GetFirstMember( &iter );
			while ( pSquadmate )
			{
				CHOEHuman *pHuman = HumanPointer( pSquadmate );

				if( pHuman )
				{
					pHuman->m_flNextGrenadeCheck = gpGlobals->curtime + 5;
				}

				pSquadmate = m_pSquad->GetNextMember( &iter );
			}
		}

		TaskComplete();
		break;
	}

	case TASK_FIND_FAR_NODE_COVER_FROM_ORIGIN:
		{
			Vector coverPos;

			if ( GetTacticalServices()->FindCoverPos( GetLocalOrigin(), EyePosition(), pTask->flTaskData, CoverRadius(), &coverPos ) ||
				 GetTacticalServices()->FindCoverPos( GetLocalOrigin(), EyePosition(), 0, CoverRadius(), &coverPos ) ) 
			{
				AI_NavGoal_t goal(coverPos, ACT_RUN, AIN_HULL_TOLERANCE);
				GetNavigator()->SetGoal( goal );

				TaskComplete();
			}
			else
			{
				// no coverwhatsoever.
				TaskFail(FAIL_NO_COVER);
			}
		}

		break;

	default:
		BaseClass::StartTask( pTask );
		break;
	}
}

//-----------------------------------------------------------------------------
void CHOEHuman::RunTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
#ifdef HUMAN_STRAFE
		// Copied from BaseNPC::RunTask for TASK_GET_PATH_TO_ENEMY_LOS
	case TASK_GET_STRAFE_PATH:
		{
			if ( GetEnemy() == NULL )
			{
				TaskFail(FAIL_NO_ENEMY);
				return;
			}
			if ( GetTaskInterrupt() > 0 )
			{
				ClearTaskInterrupt();

				Vector vecEnemy = GetEnemy()->GetAbsOrigin();
				AI_NavGoal_t goal( m_vInterruptSavePosition, ACT_RUN, AIN_HULL_TOLERANCE );

				GetNavigator()->SetGoal( goal, AIN_CLEAR_TARGET );
				GetNavigator()->SetArrivalDirection( vecEnemy - goal.dest );

#if 1
				float flDuration = GetNavigator()->GetPathTimeToGoal();
				AddLookTarget( GetEnemy(), 1.0, max( flDuration, 1.5 ), 0 );
#endif
			}
			else
				TaskInterrupt();
		}
		break;
#endif // HUMAN_STRAFE
	case TASK_FACE_ENEMY:
		{
			BaseClass::RunTask( pTask );

			// HOE soliders have a twisted combat stance which reveals a lack of a look-target.
			// See CAI_BaseActor::StartTaskRangeAttack1.
			// This is for the case of long shot-regulator rest intervals (i.e 870)
			if ( GetEnemy() && FacingIdeal() )
				AddLookTarget( GetEnemy(), 1.0, 0.5, 0.2 );
		}
		break;
#ifdef HUMAN_UNSHOOTABLE
	case TASK_GET_PATH_TO_ENEMY_LOS:
		BaseClass::RunTask( pTask );
		if ( IsCurSchedule( SCHED_ESTABLISH_LINE_OF_FIRE) && GetEnemy() && HasCondition( COND_TASK_FAILED ) )
		{
			// We couldn't go anywhere to shoot our best enemy. Remember the enemy is
			// unshootable for a few seconds during which time his relationship priority
			// is lowered.  If another enemy is shootable we want to attack him.
			RememberUnshootable( GetEnemy() );
		}
		break;
#endif
	case TASK_RANGE_ATTACK1:
		{
			BaseClass::RunTask( pTask );

			// HOE soliders have a twisted combat stance which reveals a lack of a look-target.
			// See CAI_BaseActor::StartTaskRangeAttack1.
			// This is for the case of long bursts (i.e. M16).
			if ( GetEnemy() )
				AddLookTarget( GetEnemy(), 1.0, 0.5, 0.2 );
		}
		break;
	case TASK_HUMAN_GET_PATH_TO_RANDOM_NODE:
		{
			ChainRunTask( TASK_GET_PATH_TO_RANDOM_NODE, m_GrenadeInteraction.flRadius );
		}
		break;
	case TASK_MOVE_TO_TARGET_RANGE:
	{
		// If we're moving to a medic for healing, and the medic dies, stop
		if ( ( IsCurSchedule( SCHED_HUMAN_FIND_MEDIC_COMBAT ) || IsCurSchedule( SCHED_HUMAN_FIND_MEDIC ) )
			&& ( !GetFollowTarget() || !GetFollowTarget()->IsAlive() ) )
		{
			TaskFail( FAIL_NO_TARGET );
			return;
		}

		BaseClass::RunTask( pTask );
		break;
	}
	case TASK_RUN_PATH:
		if ( IsCurSchedule( SCHED_HUMAN_MOVE_TO_WEAPON_RANGE ) )
		{
			// Keep running until we can stop, turn and shoot
			if ( GetEnemy() && GetActiveWeapon() &&
				(GetEnemy()->GetAbsOrigin() - GetAbsOrigin()).Length() > GetActiveWeapon()->m_fMinRange1 * 2 )
			{
				TaskComplete();
				return;
			}
		}
		BaseClass::RunTask( pTask );
		break;
	case TASK_WAIT_FOR_SPEAK_FINISH:
		{
			if ( !IsSpeaking() )
				return BaseClass::RunTask( pTask );
		}
		break;
#if 0
	case TASK_HUMAN_EXPLOSION_FLY:
		if ( m_lifeState == LIFE_DEAD )
			TaskComplete();
		break;
#endif
/*	case TASK_HUMAN_CHECK_FIRE:*/
	case TASK_HUMAN_CROUCH:
		if ( IsActivityFinished() )
		{
			ForceCrouch();
			TaskComplete();
		}
		break;
	case TASK_HUMAN_EXPLOSION_FLY:
	case TASK_HUMAN_EXPLOSION_LAND:
		TaskComplete(); // FIXME
		break;
	case TASK_HUMAN_EYECONTACT:
/*		if ( !IsMoving() && IsTalking() && GetTalkTarget() != NULL )
			IdleHeadTurn( GetTalkTarget()->GetAbsOrigin() );
		else*/
			TaskComplete();
		break;
	case TASK_HUMAN_FACE_TOSS_DIR:
		{
			// project a point along the toss vector and turn to face that point.
			GetMotor()->SetIdealYawToTargetAndUpdate( GetLocalOrigin() + m_vecTossVelocity * 64, AI_KEEP_YAW_SPEED );

			if ( FacingIdeal() )
			{
				TaskComplete( true );
			}
		}
		break;
/*	case TASK_HUMAN_FIND_MEDIC:
	case TASK_HUMAN_GET_EXPLOSIVE_PATH_TO_ENEMY:
	case TASK_HUMAN_GET_MELEE_PATH_TO_ENEMY:
	case TASK_HUMAN_IDEALYAW:*/
	case TASK_HUMAN_PLAY_SEQUENCE_WAIT:
		{
			AutoMovement( );
			if ( IsActivityFinished() )
			{
				DelayMoveStart( 0 );
				TaskComplete();
			}
			break;
		}
	case TASK_HUMAN_SOUND_ATTACK:
		if ( IsActivityFinished() )
		{
			SquadIssueCommand( SQUADCMD_ATTACK );
			TaskComplete();
		}
		break;
/*	case TASK_HUMAN_SOUND_CHARGE:*/
	case TASK_HUMAN_SOUND_CHECK_IN:
		if ( gpGlobals->curtime >= m_flWaitFinished )
		{
			SquadIssueCommand( SQUADCMD_CHECK_IN );
			TaskComplete();
		}
		break;
	case TASK_HUMAN_SOUND_CLEAR:
		if ( gpGlobals->curtime >= m_flWaitFinished )
		{
			if ( OkToShout() )
#ifdef HOE_HUMAN_RR
				Speak( TLK_SQUAD_ACK_CHECK );
#else
				Speak( "CLEAR" );
#endif
			TaskComplete();
		}
		break;
	case TASK_HUMAN_SOUND_COME_TO_ME:
		if ( IsActivityFinished() )
		{
			SquadIssueCommand( SQUADCMD_COME_HERE );
			TaskComplete();
		}
		break;
/*	case TASK_HUMAN_SOUND_COVER:
	case TASK_HUMAN_SOUND_EXPL:*/
	case TASK_HUMAN_SOUND_FOUND_ENEMY:
		if ( IsActivityFinished() )
		{
			SquadIssueCommand( SQUADCMD_FOUND_ENEMY );
			TaskComplete();
		}
		break;
/*	case TASK_HUMAN_SOUND_GRENADE:
	case TASK_HUMAN_SOUND_HEALED:
	case TASK_HUMAN_SOUND_HEAR:
	case TASK_HUMAN_SOUND_HELP:
	case TASK_HUMAN_SOUND_MEDIC:
	case TASK_HUMAN_SOUND_RESPOND:*/
	case TASK_HUMAN_SOUND_RETREAT:
		if ( IsActivityFinished() )
		{
			SquadIssueCommand( SQUADCMD_RETREAT );
			TaskComplete();
		}
		break;
/*	case TASK_HUMAN_SOUND_RETREATING:
	case TASK_HUMAN_SOUND_SEARCHING:*/
	case TASK_HUMAN_SOUND_SEARCH_AND_DESTROY:
		if ( IsActivityFinished() )
		{
			SquadIssueCommand( SQUADCMD_SEARCH_AND_DESTROY );
			TaskComplete();
		}
		break;
	case TASK_HUMAN_SOUND_SURPRESS:
		if ( IsActivityFinished() )
		{
			SquadIssueCommand( SQUADCMD_SURPRESSING_FIRE );
			TaskComplete();
		}
		break;
/*	case TASK_HUMAN_SOUND_SURPRESSING:
	case TASK_HUMAN_SOUND_TAUNT:
	case TASK_HUMAN_SOUND_THROW:
	case TASK_HUMAN_SOUND_VICTORY:*/
	case TASK_HUMAN_UNCROUCH:
		if ( m_bCrouchLocked )
		{
			TaskComplete();
			break;
		}
		if ( IsActivityFinished() )
		{
			ClearForceCrouch();
			Stand();
			TaskComplete();
		}
		break;
	case TASK_HUMAN_WAIT_GOAL_VISIBLE:
		TaskComplete(); /* FIXME */
		break;

	default:
		BaseClass::RunTask( pTask );
		break;
	}
}

//-----------------------------------------------------------------------------
// Behaviors! Lovely behaviors
//-----------------------------------------------------------------------------
bool CHOEHuman::CreateBehaviors( void )
{
	AddBehavior( &m_AssaultBehavior );
	AddBehavior( &m_StandoffBehavior );
	AddBehavior( &m_RappelBehavior );
	AddBehavior( &m_HueyRappelBehavior );
	AddBehavior( &m_PassengerBehavior );
	
	return BaseClass::CreateBehaviors();
}

//-----------------------------------------------------------------------------
bool CHOEHuman::IsInAScript( void )
{
	if ( m_NPCState == NPC_STATE_SCRIPT )
		return true;

	if ( BaseClass::IsInAScript() )
		return true;

	if ( m_flScriptedSentence >= gpGlobals->curtime )
		return true;

	return false;
}

// How long to shoot the ragdoll of a dead enemy in seconds
ConVar hoe_shoot_dead_enemy_time( "hoe_shoot_dead_enemy_time", "2.0", 0 );

//-----------------------------------------------------------------------------
void HOEHumanNPCKilled( CBaseEntity *pVictim )
{
	CAI_BaseNPC *pBullseye = NULL;
	CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();
	for ( int i = 0; i < g_AI_Manager.NumAIs(); i++ )
	{
		if ( !ppAIs[i]->IsAlive() )
			continue;

		if ( ppAIs[i]->GetEnemy() != pVictim )
			continue;

		if ( !HOE_IsHuman( ppAIs[i] ) && !FClassnameIs( ppAIs[i], "npc_superzombie" ) )
			continue;

		// FIXME: count visible living enemies
		if ( ppAIs[i]->GetEnemies()->NumEnemies() > 1 &&
			 ppAIs[i]->GetAbsOrigin().DistTo( pVictim->GetAbsOrigin()) < 96.0f )
		{
			// Don't shoot at an enemy corpse that dies very near to me. This will prevent Alyx attacking
			// Other nearby enemies.
			continue;
		}

		if ( !ppAIs[i]->GetActiveWeapon() ||
//			FClassnameIs( ppAIs[i]->GetActiveWeapon(), "weapon_870" ) ||
			FClassnameIs( ppAIs[i]->GetActiveWeapon(), "weapon_rpg7" ) ||
			FClassnameIs( ppAIs[i]->GetActiveWeapon(), "weapon_m79" ) ||
			FClassnameIs( ppAIs[i]->GetActiveWeapon(), "weapon_m21" ) )
		{
			continue;
		}

		// FIXME: the victim might gib or not become a ragdoll
		if ( pBullseye == NULL )
		{
			pBullseye = ppAIs[i]->CreateCustomTarget( pVictim->GetAbsOrigin(),
				hoe_shoot_dead_enemy_time.GetFloat() );

			extern void RegisterBullseyeOnRagdoll( int entindex, CBaseEntity *pBullseye );
			RegisterBullseyeOnRagdoll( pVictim->entindex(), pBullseye );
		}

		ppAIs[i]->AddEntityRelationship( pBullseye, ppAIs[i]->IRelationType(pVictim),
			ppAIs[i]->IRelationPriority(pVictim) );

		// Update or Create a memory entry for this target and make Alyx think she's seen this target recently.
		// This prevents the baseclass from not recognizing this target and forcing Alyx into 
		// SCHED_WAKE_ANGRY, which wastes time and causes her to change animation sequences rapidly.
		ppAIs[i]->GetEnemies()->UpdateMemory( ppAIs[i]->GetNavigator()->GetNetwork(),
			pBullseye, pBullseye->GetAbsOrigin(), 0.0f, true );
		AI_EnemyInfo_t *pMemory = ppAIs[i]->GetEnemies()->Find( pBullseye );

		if ( pMemory )
		{
			// Pretend we've known about this target longer than we really have.
			pMemory->timeFirstSeen = gpGlobals->curtime - 10.0f;
		}
	}
}

//-----------------------------------------------------------------------------
void CHOEHuman::Event_KilledOther( CBaseEntity *pVictim, const CTakeDamageInfo &info )
{
	if ( pVictim )
	{
		// Create a bullseye to attach to the victim's ragdoll unless the
		// victim is about to become chunks.
		if ( !pVictim->MyCombatCharacterPointer() ||
			!pVictim->MyCombatCharacterPointer()->ShouldGib( info ) )
		{
			HOEHumanNPCKilled( pVictim );
		}

		// Don't brag about killing friendlies
		if ( IRelationType( pVictim ) != D_HT )
		{
			return;
		}
#if 0
		// Don't brag about killing enemies the player shot recently
		if ( pVictim->IsNPC() && 
			 pVictim->MyNPCPointer()->GetLastPlayerDamageTime() != 0 &&
			 ( gpGlobals->curtime - pVictim->MyNPCPointer()->GetLastPlayerDamageTime() < 2 ) )
		{
			return;
		}
#endif
		DevMsg( "KilledOther: %s killed %s\n", GetDebugName(), pVictim->GetDebugName() );
		m_flKilledEnemyTime = gpGlobals->curtime;
	}
}

//-----------------------------------------------------------------------------
void CHOEHuman::OnKilledNPC( CBaseCombatCharacter *pKilled )
{
	// This seems useless.  It is called from CBaseCombatCharacter::InputKilledNPC
	// But by the time the event is processed the victim entity no longer exists.
	// g_EventQueue.AddEvent( info.GetAttacker(), "KilledNPC", 0.3, this, this );
}

//-----------------------------------------------------------------------------
// My buddies got killed!
//-----------------------------------------------------------------------------
void CHOEHuman::NotifyDeadFriend( CBaseEntity* pFriend )
{
	// Called when a squad member is killed
//	SetCondition( COND_SQUADMEMBER_KILLED );
}

//-----------------------------------------------------------------------------
void CHOEHuman::OnFriendDamaged( CBaseCombatCharacter *pSquadmate, CBaseEntity *pAttackerEnt )
{
	BaseClass::OnFriendDamaged( pSquadmate, pAttackerEnt );

	// see CNPC_PlayerCompanion::OnFriendDamaged

	CAI_BaseNPC *pAttacker = pAttackerEnt->MyNPCPointer();
	if ( pAttacker )
	{
		bool bDirect = ( pSquadmate->FInViewCone(pAttacker) &&
						 ( ( pSquadmate->IsPlayer() && HasCondition(COND_SEE_PLAYER) ) || 
						 ( /*pSquadmate->MyNPCPointer() && pSquadmate->MyNPCPointer()->IsPlayerAlly() && */

						 // Note that by default friendlies are NOT seen by GetSenses(). See QuerySeeEntity().
						 GetSenses()->DidSeeEntity( pSquadmate ) ) ) );
		if ( bDirect )
		{
			if ( UpdateEnemyMemory( pAttacker, pAttacker->GetAbsOrigin(), pSquadmate ) )
				DevMsg("CHOEHuman::OnFriendDamaged got new enemy from friend\n");
		}
		else
		{
			if ( FVisible( pSquadmate ) )
			{
				AI_EnemyInfo_t *pInfo = GetEnemies()->Find( pAttacker );
				if ( !pInfo || ( gpGlobals->curtime - pInfo->timeLastSeen ) > 15.0 )
				{
					if ( UpdateEnemyMemory( pAttacker, pSquadmate->GetAbsOrigin(), pSquadmate ) )
						DevMsg("CHOEHuman::OnFriendDamaged got new enemy from friend\n");
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
bool CHOEHuman::QuerySeeEntity( CBaseEntity *pEntity, bool bOnlyHateOrFearIfNPC )
{
	// See all humans so medics can heal them. (Medics don't heal people they haven't seen before).
	// See OnFriendDamaged() as well.
	// Note this returns true for enemies but that is okay.
	if ( pEntity && HumanPointer( pEntity ) )
	{
		return true;
	}

	return BaseClass::QuerySeeEntity( pEntity, bOnlyHateOrFearIfNPC );
}

//------------------------------------------------------------------------------
void CHOEHuman::DecalTrace( trace_t *pTrace, char const *decalName )
{
	// FIXME: not sure this does anything. See m_fNoDamageDecal in TraceAttack()

	// Don't bloody up Barney. See CNPC_Alyx::TraceAttack.
	if ( ClassifyPlayerAllyVital() )
	{
		if ( m_fNoDamageDecal )
			m_fNoDamageDecal = false;
		return;
	}

	// Do not decal a player companion's head or face, no matter what.
	// No damage decal on *anyones* head in case it gets chopped off.
	if ( /*ClassifyPlayerAlly() && */pTrace->hitgroup == HITGROUP_HEAD )
	{
		if ( m_fNoDamageDecal )
			m_fNoDamageDecal = false;
		return;
	}

	BaseClass::DecalTrace( pTrace, decalName );
}

#if 0
// Unfortunately CTakeDamageInfo.GetInflictor() is the player not the weapon so we can't ask "which weapon is doing
// damage to us.". According to takedamageinfo.h GetInflictor()==weapon and GetAttacker()==player/NPC.
bool g_bMacheteOrChainsawAttack = false;
#endif

//-----------------------------------------------------------------------------
void CHOEHuman::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr )
{
	// Disable all friendly fire (but not blast damage)
	if ( info.GetDamageType() & DMG_BULLET )
	{
		CAI_BaseNPC *pNPC = info.GetAttacker() ? info.GetAttacker()->MyNPCPointer() : NULL;
		if ( pNPC &&
			pNPC->IRelationType( this ) != D_HT &&
			pNPC->IRelationType( this ) != D_FR )
		{
			m_fNoDamageDecal = true;
			DevWarning("CHOEHuman::TraceAttack %s shot by non-enemy %s IGNORED\n", GetDebugName(), pNPC->GetDebugName());
			return;
		}
	}
#if 0 // head chopping is handled by HandleInteraction
	int hitgroup = ptr->hitgroup;

	// hitgroup will be zero if the trace wasn't performed with CONTENTS_HITBOX, which is the
	// case for the machete and chainsaw.
	if ( hitgroup == 0 )
	{
		trace_t tr2;
		UTIL_TraceLine( ptr->startpos, ptr->endpos, MASK_SHOT, NULL, COLLISION_GROUP_NONE, &tr2 );
		hitgroup = tr2.hitgroup;
	}

	// If hit by blow from machete or chainsaw to head, chop off head.
	if( ( hitgroup == HITGROUP_HEAD ) &&
		g_bMacheteOrChainsawAttack &&
		HasAHead() &&
		!IsInAScript() &&
		( GetState() != NPC_STATE_PRONE ) &&
		( GetState() != NPC_STATE_NONE ) )
	{
		Behead();

		CTakeDamageInfo newInfo = info;
		newInfo.SetDamage( GetHealth() );
		BaseClass::TraceAttack( newInfo, vecDir, ptr );
		m_fNoDamageDecal = true; // no blood decal on now-removed head
		return;
	}
#endif

	// Destroy the head on headshots from colt/sniper rifle
	if ( HasAHead() &&
		( ptr->hitgroup == HITGROUP_HEAD ) &&
		( info.GetDamageType() & DMG_BULLET ) &&
		( info.GetDamage() >= 20 ) )
	{
		Vector origin;
		QAngle angles;

		if ( DropHelmet() )
			SpawnHelmet( vecDir );

		m_nHeadNum = GetNumHeads();
		SetBodygroup( GetHeadGroupNum(), m_nHeadNum );

		GetAttachment( "neck", origin, angles );
		SpawnBlood( origin, vec3_origin, BloodColor(), 20.0f );

		CTakeDamageInfo newInfo = info;
		newInfo.SetDamage( GetHealth() );
		BaseClass::TraceAttack( newInfo, vecDir, ptr, 0 );

		m_fNoDamageDecal = true; // no blood decal on now-removed head

		return;
	}

	BaseClass::TraceAttack( info, vecDir, ptr, 0 );

	// Don't bloody up Barney. See CNPC_Alyx::TraceAttack.
	// FIXME: hack until some way of removing decals after healing
	if ( ClassifyPlayerAllyVital() )
		m_fNoDamageDecal = true;

	// Do not decal a player companion's head or face, no matter what.
	// No damage decal on *anyones* head in case it gets chopped off.
	if ( /*ClassifyPlayerAlly() &&*/ ptr->hitgroup == HITGROUP_HEAD )
		m_fNoDamageDecal = true;
}

//-----------------------------------------------------------------------------
int CHOEHuman::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
#if 1
	CTakeDamageInfo newInfo = info;
	if( IsInSquad() && (info.GetDamageType() & DMG_BLAST) && info.GetInflictor() )
	{
//		if ( npc_citizen_explosive_resist.GetBool() )
		{
			// Blast damage. If this kills a squad member, give the 
			// remaining citizens a resistance bonus to this inflictor
			// to try to avoid having the entire squad wiped out by a
			// single explosion.
			if( m_pSquad->IsSquadInflictor( info.GetInflictor() ) )
			{
				newInfo.ScaleDamage( 0.5 );
				DevMsg( "%s took 1/2 damage from blast\n", GetDebugName() );
			}
			else
			{
				// If this blast is going to kill me, designate the inflictor
				// so that the rest of the squad can enjoy a damage resist.
				if( info.GetDamage() >= GetHealth() )
				{
					m_pSquad->SetSquadInflictor( info.GetInflictor() );
				}
			}
		}
	}

	// Disallow damage from server-side ragdolls sitting on the ground.
	if ( info.GetAttacker() &&
		FClassnameIs( info.GetAttacker(), "prop_ragdoll" ) )
	{
		return 0;
	}

	int ret = BaseClass::OnTakeDamage_Alive( newInfo );

	if ( info.GetAttacker() &&
		info.GetAttacker() != this &&
		IRelationType( info.GetAttacker() ) != D_HT &&
		IRelationType( info.GetAttacker() ) != D_FR )
	{
		DevWarning("CHOEHuman::OnTakeDamage_Alive %s hurt by non-enemy %s\n", GetDebugName(), info.GetAttacker()->GetDebugName());
		CAI_BaseNPC *pNPC = info.GetAttacker()->MyNPCPointer();
		if ( pNPC && pNPC->HasCondition( COND_WEAPON_BLOCKED_BY_FRIEND ) )
			DevWarning("CHOEHuman::OnTakeDamage_Alive COND_WEAPON_BLOCKED_BY_FRIEND\n");
	}

	if ( info.GetAttacker() &&
		 FClassnameIs( info.GetAttacker(), "npc_huey" ) &&
		 BaseClass::IRelationType( info.GetAttacker() ) == D_HT )
	{
		m_flAttackedByHueyTime = gpGlobals->curtime;
	}

	if ( info.GetAttacker() && FClassnameIs( info.GetAttacker(), "npc_vortigaunt" ) )
	{
		if ( (info.GetDamageType() & DMG_SHOCK) && (GetHealth() <= 0) )
		{
#if 1
			m_bShouldBoogie = true;
#else
			// FIXME: death sound doesn't come from this ragdoll
			BecomeRagdollBoogie( this, info.GetDamageForce(), 5.0f, SF_RAGDOLL_BOOGIE_ELECTRICAL );
#endif
		}
	}

	return ret;
#else
	int ret = BaseClass::OnTakeDamage_Alive( info );

	if ( IsInAScript() || GetState() == NPC_STATE_PRONE )
		return ret;

	if ( info.GetInflictor() &&
		( GetHealth() <= 0 ) &&
		( info.GetDamageType() & DMG_BLAST ) )
	{
//		m_lifeState = LIFE_DYING;
		m_takedamage = DAMAGE_NO;
		m_iHealth = 1;

		m_explosionDmgInfo = info;
		IPhysicsObject *pPhysObj = VPhysicsGetObject();
		if ( pPhysObj )
		{
			pPhysObj->ApplyForceCenter( info.GetDamageForce() );
		}
		else
		{
			// FIXME
			Vector vec = info.GetDamageForce();
			VectorNormalize( vec );
			ApplyAbsVelocityImpulse( vec );
		}

		ClearSchedule();
		SetSchedule( SCHED_HUMAN_EXPLOSION_DIE );
	}

	return ret;
#endif
}

//-----------------------------------------------------------------------------
void CHOEHuman::Event_Killed( const CTakeDamageInfo &info )
{
	if ( HasAHead() && DropHelmet() )
	{
		Vector impactDir = info.GetDamageForce();
		impactDir.NormalizeInPlace();
		SpawnHelmet( impactDir );
	}

	BaseClass::Event_Killed( info );

	if ( GetSpeechSemaphore( this )->GetOwner() == this )
		GetSpeechSemaphore( this )->Release();

	// If Barney is killed, display message and reload the game.
	DisplayDeathMessage();
}

//-----------------------------------------------------------------------------
bool CHOEHuman::ShouldPickADeathPose( void )
{
	// There are no death pose animations currently.
	// This death-pose stuff doesn't affect us when using server-side ragdolls.
	return false;
}

//-----------------------------------------------------------------------------
bool CHOEHuman::CanBecomeRagdoll( void )
{
	return BaseClass::CanBecomeRagdoll();

	// If you want the death animation followed by ragdoll, use this code.
	// Add AE_RAGDOLL into each ACT_DIE_XXX animation.
	// Lots of cases where this looks bad (getting blown up, hit by bullsquid tail, etc).
	// Override GetDeathActivity() to handle crouching etc.
#if 0
	return /*( m_nKillingDamageType & DMG_CRUSH ) ||*/
		IsCurSchedule( SCHED_DIE, false ) ||								// Finished playing death anim, time to ragdoll
		/*IsCurSchedule( SCHED_HUNTER_CHARGE_ENEMY, false ) ||*/				// While moving, it looks better to ragdoll instantly
		IsCurSchedule( SCHED_SCRIPTED_RUN, false ) ||
		( GetActivity() == ACT_WALK ) || ( GetActivity() == ACT_RUN ) ||
		GetCurSchedule() == NULL;											// Failsafe
#endif
}

//-----------------------------------------------------------------------------
bool CHOEHuman::BecomeRagdoll( const CTakeDamageInfo &info, const Vector &forceVector )
{
	return BaseClass::BecomeRagdoll( info, forceVector );
}

//-----------------------------------------------------------------------------
bool CHOEHuman::BecomeRagdollOnClient( const Vector &force )
{
	if ( !CanBecomeRagdoll() ) 
		return false;

	// Become server-side ragdoll if we're flagged to do it
	if ( HasSpawnFlags( SF_HUMAN_SERVERSIDE_RAGDOLL ) || m_bShouldBoogie )
	{
		// For namd3 AE_NPC_RAGDOLL
		if ( !m_hHelmet && HasAHead() && DropHelmet() )
		{
			Vector impactDir = force;
			impactDir.NormalizeInPlace();
			SpawnHelmet( impactDir );
		}

		CreateRagdollCorpse( force, true, true );

		// Get rid of our old body
		// RemoveDeffered lets the client copy the damage decals to the ragdoll
		// from this NPC before the NPC is deleted.
		RemoveDeferred();/*UTIL_Remove(this);*/ 

		return true;
	}

	return BaseClass::BecomeRagdollOnClient( force );
}

//-----------------------------------------------------------------------------
CBaseEntity *CHOEHuman::CreateRagdollCorpse( const Vector &force, bool bUseLRURetirement, bool bSmell )
{
	CTakeDamageInfo	info;

	// Fake the info
	info.SetDamageType( DMG_GENERIC );
	info.SetDamageForce( force );
	info.SetDamagePosition( WorldSpaceCenter() );

	// This copies damage decals like clientside ragdoll does.
	// See m_hCopyDecalsFromThisEnt in CreateServerRagdoll.
	CBaseEntity *pRagdoll = CreateServerRagdoll( this, 0, info, HOECOLLISION_GROUP_CORPSE/*COLLISION_GROUP_INTERACTIVE_DEBRIS*/, bUseLRURetirement );

	// Transfer our name to the new ragdoll
	pRagdoll->SetName( GetEntityName() );
//		pRagdoll->SetCollisionGroup( COLLISION_GROUP_INTERACTIVE_DEBRIS );

	// Hack to allow ragdolls to show blood splatter when shot
	((CRagdollProp *)pRagdoll)->SetBloodColor( BloodColor() );

	// Allow closed caption icon display (hanging THEM)
	((CRagdollProp *)pRagdoll)->SetCCImageName( GetCCImageName() );

	// Got zero-force assert
	pRagdoll->AddEFlags( EFL_NO_ROTORWASH_PUSH );

	// FIXME: becoming ragdoll doesn't mean DeathSound() should play
	if ( HasAHead() )
	{
		char szSentence[128];
		Q_snprintf( szSentence, sizeof(szSentence), "%s_%s", GetSentenceGroup(), "DIE" );
		float volume = VOL_NORM;
		soundlevel_t soundlevel = SNDLVL_90dB/*SNDLVL_TALKING*/;
		SENTENCEG_PlayRndSz( pRagdoll->edict(), szSentence, volume, soundlevel, 0, PITCH_NORM );
	}
	else
	{
		int bone = GetAttachmentBone( LookupAttachment( "head" ) );
		if ( bone >= 0 )
			((CRagdollProp *)pRagdoll)->RemoveBone( GetModelPtr()->pBone( bone )->pszName() );

		if ( m_hHead )
		{
			IPhysicsObject *list[VPHYSICS_MAX_OBJECT_LIST_COUNT];
			int listCount = pRagdoll->VPhysicsGetObjectList( list, ARRAYSIZE(list) );
			int iPhysicsBone = ((CBaseAnimating *)pRagdoll)->GetPhysicsBone( GetAttachmentBone( LookupAttachment( "head" ) ) );
			if ( iPhysicsBone < listCount && list[iPhysicsBone] != NULL )
			{
				Vector velocity;
				list[iPhysicsBone]->GetVelocityAtPoint( m_hHead->GetAbsOrigin(), &velocity );
				m_hHead->ApplyAbsVelocityImpulse( velocity );
			}

			CreateCorpseSolver( pRagdoll, m_hHead, true, 4.0f );
		}
	}
#if 0
	void NPCBecameRagdollWrapper( int entindex, void *ragdoll );
	NPCBecameRagdollWrapper( entindex(), pRagdoll );
#else
	extern CBaseEntity *GetRagdollBullseyeForVictim( int entindex );
	CBaseEntity *pBullseye = GetRagdollBullseyeForVictim( entindex() );
	if ( pBullseye )
	{
		pBullseye->SetParent( pRagdoll );
	}
#endif
	// Create a marker to emit the carcass smell and so NPCs can
	// recognize dead friends.
	CHOECorpseMarker *pMarker = assert_cast<CHOECorpseMarker *>( CBaseEntity::Create( "hoe_corpse_marker", GetAbsOrigin(), GetAbsAngles(), pRagdoll ) );
	pMarker->InitMarker( pRagdoll, m_iClassname, bSmell );

	// The ragdoll origin is often well below ground so attach to a known point
	// on the model.
	if ( pRagdoll->GetBaseAnimating()->LookupAttachment( "CorpseMarker" ) != INVALID_ATTACHMENT )
	{
		pMarker->SetParentAttachment( "SetParentAttachment", "CorpseMarker", false );
#if 1
		if ( pBullseye )
			pBullseye->SetParentAttachment( "SetParentAttachment", "CorpseMarker", false );
#endif
	}

	if ( m_bShouldBoogie )
		CRagdollBoogie::Create( pRagdoll, 200, gpGlobals->curtime, 5.0f, SF_RAGDOLL_BOOGIE_ELECTRICAL );

	// FUGLY: If NPC dropped a helmet when killed then remove it along with the ragdoll corpse
	if ( m_hHelmet )
	{
		CHelmetRemover::CreateHelmetRemover( m_hHelmet, pRagdoll );

#if 1
		IPhysicsObject *list[VPHYSICS_MAX_OBJECT_LIST_COUNT];
		int listCount = pRagdoll->VPhysicsGetObjectList( list, ARRAYSIZE(list) );
		int iPhysicsBone = ((CBaseAnimating *)pRagdoll)->GetPhysicsBone( GetAttachmentBone( LookupAttachment( "head" ) ) );
		if ( iPhysicsBone < listCount && list[iPhysicsBone] != NULL )
		{
			Vector velocity;
			list[iPhysicsBone]->GetVelocityAtPoint( m_hHelmet->GetAbsOrigin(), &velocity );
			m_hHelmet->ApplyAbsVelocityImpulse( velocity );
		}
#else // can't parent a physics object
		if ( HasAHead() )
		{
			Vector origin;
			QAngle angles;
			GetHelmetPose( m_hHelmet->GetBaseAnimating(), origin, angles );
			m_hHelmet->SetParent( pRagdoll, LookupAttachment( "head" ) );
			m_hHelmet->SetLocalOrigin( origin );
			m_hHelmet->SetLocalAngles( angles );
			g_EventQueue.AddEvent( m_hHelmet, "ClearParent", 2.0f, pRagdoll, pRagdoll );
		}
#endif

		CreateCorpseSolver( pRagdoll, m_hHelmet, true, 4.0f );

	}

	return pRagdoll;
}

//-----------------------------------------------------------------------------
void CHOEHuman::Activate( void )
{
	BaseClass::Activate();

#ifdef LOGIC_HUEY_DEPLOY
	if ( m_iszDeployLogic != NULL_STRING )
	{
		CLogicHueyDeploy::RegisterNPC( m_iszDeployLogic, this );
	}
#endif

	m_iLeftFootAttachment = LookupAttachment( "left_foot" );
	m_iRightFootAttachment = LookupAttachment( "right_foot" );

	UpdateWaterState();

	if ( m_iLeftFootAttachment != INVALID_ATTACHMENT && GetWaterLevel() > 0 )
	{
		GetAttachment( m_iLeftFootAttachment, m_vLeftFootOrigin );
		m_bLeftFootInWater = ( UTIL_PointContents( m_vLeftFootOrigin ) & MASK_WATER ) ? true : false;
	}
	if ( m_iRightFootAttachment != INVALID_ATTACHMENT && GetWaterLevel() > 0 )
	{
		GetAttachment( m_iRightFootAttachment, m_vRightFootOrigin );
		m_bRightFootInWater = ( UTIL_PointContents( m_vRightFootOrigin ) & MASK_WATER ) ? true : false;
	}

	const char *szImage = GetCCImageNameCStr();
	if ( szImage && szImage[0] )
		SetCCImageName( AllocPooledString( szImage ) );
}

//-----------------------------------------------------------------------------
void CHOEHuman::UpdateOnRemove( void )
{
#ifdef LOGIC_HUEY_DEPLOY
	if ( m_iszDeployLogic != NULL_STRING )
	{
		CLogicHueyDeploy::UnregisterNPC( m_iszDeployLogic, this );
	}
#endif

	// Chain at end to mimic destructor unwind order
	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
void CHOEHuman::InputSetGameEndAlly( inputdata_t &inputdata )
{
	m_bGameEndAlly = inputdata.value.Bool();
}

CBaseEntity *CreatePlayerLoadSave( Vector vOrigin, float flDuration, float flHoldTime, float flLoadTime );

//-----------------------------------------------------------------------------
void CHOEHuman::DisplayDeathMessage( void )
{
	if ( m_bGameEndAlly == false )
		return;

	CBaseEntity *pReload = CreatePlayerLoadSave( GetAbsOrigin(), 2.0f, 6.0f, 6.0f );

	if ( pReload == NULL )
		return;

	pReload->SetRenderColor( 0, 0, 0, 255 );
	g_EventQueue.AddEvent( pReload, "Reload", 0.0f, pReload, pReload );

	CBaseEntity *pPlayer = AI_GetSinglePlayer();

	if ( pPlayer )	
	{
		if ( GetDeathMessage() != NULL_STRING )
		{
			CMessage *pEnvMessage = (CMessage *) CreateEntityByName( "env_message" );
			if ( pEnvMessage )
			{
				pEnvMessage->SetMessage( GetDeathMessage() );
				g_EventQueue.AddEvent( pEnvMessage, "ShowMessage", 2.0f, pEnvMessage, pEnvMessage );
			}
		}

		ToBasePlayer(pPlayer)->NotifySinglePlayerGameEnding();
	}
}

#if 0

//-----------------------------------------------------------------------------
void CHOEHuman::ExplFlyTouch( CBaseEntity *pOther )
{
	DevMsg("CHOEHuman::ExplFlyTouch\n");
	CTakeDamageInfo info = m_explosionDmgInfo;
	info.SetDamageForce( Vector( 1, 1, 1 ) );
	m_takedamage = DAMAGE_YES;
	TakeDamage( info );
}

#endif

//-----------------------------------------------------------------------------
int CHOEHuman::DrawDebugTextOverlays(void)
{
	int text_offset = BaseClass::DrawDebugTextOverlays();

	if (m_debugOverlays & OVERLAY_TEXT_BIT)
	{
		char tempstr[512];
		if ( m_flTimePlayerStartStare )
		{
			Q_snprintf(tempstr,sizeof(tempstr),"Staring: %.1f", GetTimePlayerStaring() );
			EntityText(text_offset++, tempstr, 0);
		}
		EntityText(text_offset++, IsCrouching() ? "Crouching" : "Not Crouching", 0);
	}

	return text_offset;
}

#ifdef ENEMY_TELEPORTED_STUFF
//-----------------------------------------------------------------------------
void CHOEHuman::EnemyTeleported( CBaseEntity *pEnemy )
{
	if ( pEnemy == NULL || pEnemy != GetEnemy() )
		return;

	m_hEnemyThatTeleported = GetEnemy();
	m_vEnemyThatTeleportedPosition = GetEnemyLKP();
	m_bEnemyThatTeleportedSeen = HasCondition( COND_SEE_ENEMY );

	bool fVisible = FInViewCone( pEnemy ) && FVisible( pEnemy );

	if ( m_bEnemyThatTeleportedSeen && !fVisible )
	{
		SetEnemy( NULL );

		if ( IsCurSchedule( SCHED_HUMAN_ESTABLISH_LINE_OF_FIRE ) )
			ClearSchedule( "Vortigaunt teleported" );

		GetEnemies()->SetTimeValidEnemy( pEnemy, gpGlobals->curtime + 0.5 );

		GetEnemies()->MarkAsEluded( pEnemy );
	}

	// See the hack in CAI_Enemies::RefreshMemories eliminating the free knowledge duration for an
	// enemy that eluded me.

#if 0
	// Was I chasing the enemy?
	if ( ( GetNavigator()->GetGoalType() == GOALTYPE_ENEMY ) &&
		( GetNavigator()->GetGoalTarget() == this ) )
	{
		// If I saw the enemy before it teleported, stop chasing it
		if ( HasCondition( COND_SEE_ENEMY ) )
		{
		}

		// The enemy didn't see me teleport away.  Have them continue running
		// towards my last location.
		else
		{
			GetNavigator()->SetGoal( GOALTYPE_LOCATION
		}
	}
#endif
}
#endif

#ifdef HUMAN_UNSHOOTABLE
//------------------------------------------------------------------------------
void CHOEHuman::RememberUnshootable(CBaseEntity *pEntity, float duration )
{
	if ( pEntity == GetEnemy() )
	{
		ForceChooseNewEnemy();
	}

	const float NPC_UNREACHABLE_TIMEOUT = ( duration > 0.0 ) ? duration : 3;
	// Only add to list if not already on it
	for (int i=m_UnshootableEnts.Size()-1;i>=0;i--)
	{
		// If record already exists just update mark time
		if (pEntity == m_UnshootableEnts[i].hUnshootableEnt)
		{
			m_UnshootableEnts[i].fExpireTime	 = gpGlobals->curtime + NPC_UNREACHABLE_TIMEOUT;
			m_UnshootableEnts[i].vLocationWhenUnshootable = pEntity->GetAbsOrigin();
			return;
		}
	}

	// Add new unshootable entity to list
	int nNewIndex = m_UnshootableEnts.AddToTail();
	m_UnshootableEnts[nNewIndex].hUnshootableEnt = pEntity;
	m_UnshootableEnts[nNewIndex].fExpireTime	 = gpGlobals->curtime + NPC_UNREACHABLE_TIMEOUT;
	m_UnshootableEnts[nNewIndex].vLocationWhenUnshootable = pEntity->GetAbsOrigin();
}

//------------------------------------------------------------------------------
bool CHOEHuman::IsUnshootable(CBaseEntity *pEntity)
{
	float UNREACHABLE_DIST_TOLERANCE_SQ = (120*120);

	// Note that it's ok to remove elements while I'm iterating
	// as long as I iterate backwards and remove them using FastRemove
	for (int i=m_UnshootableEnts.Size()-1;i>=0;i--)
	{
		// Remove any dead elements
		if (m_UnshootableEnts[i].hUnshootableEnt == NULL)
		{
			m_UnshootableEnts.FastRemove(i);
		}
		else if (pEntity == m_UnshootableEnts[i].hUnshootableEnt)
		{
			// Test for shootablility on any elements that have timed out
			if ( gpGlobals->curtime > m_UnshootableEnts[i].fExpireTime ||
				  pEntity->GetAbsOrigin().DistToSqr(m_UnshootableEnts[i].vLocationWhenUnshootable) > UNREACHABLE_DIST_TOLERANCE_SQ)
			{
				m_UnshootableEnts.FastRemove(i);
				return false;
			}
			return true;
		}
	}
	return false;
}
#endif // HUMAN_UNSHOOTABLE

// Hack -- only humans are grabbed by Barnacle
bool HOE_IsHuman( CBaseEntity *pEnt )
{
	return dynamic_cast<CHOEHuman *>( pEnt ) != NULL;
}

bool IsHumanTalking( void )
{
#ifdef HOE_HUMAN_RR
	g_flHumanSpeechTime = g_AIFriendliesTalkSemaphore.GetReleaseTime();
	return g_flHumanSpeechTime >= gpGlobals->curtime;
#else
	return CHOEHuman::IsAnyoneTalking();
#endif
}

HOE_BEGIN_CUSTOM_NPC( hoe_human, CHOEHuman )
	DECLARE_CONDITION( COND_ZOMBIE_TOO_CLOSE )
	DECLARE_CONDITION( COND_HUMAN_ENEMY_CLOSE )
	DECLARE_CONDITION( COND_HUMAN_MEDIC_WANTS_TO_HEAL_ME )
	DECLARE_CONDITION( COND_HUMAN_BECOMING_PASSENGER )

	DECLARE_INTERACTION( g_interactionHumanMedicHeal )
	DECLARE_INTERACTION( g_interactionHumanGrenade )

	DECLARE_TASK( TASK_HUMAN_CHECK_FIRE )
	DECLARE_TASK( TASK_HUMAN_CROUCH )
	DECLARE_TASK( TASK_HUMAN_EXPLOSION_FLY )
	DECLARE_TASK( TASK_HUMAN_EXPLOSION_LAND )
	DECLARE_TASK( TASK_HUMAN_EYECONTACT )
	DECLARE_TASK( TASK_HUMAN_FACE_TOSS_DIR )
	DECLARE_TASK( TASK_HUMAN_FIND_MEDIC )
	DECLARE_TASK( TASK_HUMAN_GET_EXPLOSIVE_PATH_TO_ENEMY )
	DECLARE_TASK( TASK_HUMAN_GET_MELEE_PATH_TO_ENEMY )
	DECLARE_TASK( TASK_HUMAN_GET_PATH_TO_RANDOM_NODE )
	DECLARE_TASK( TASK_HUMAN_IDEALYAW )
	DECLARE_TASK( TASK_HUMAN_PLAY_SEQUENCE_WAIT )
	DECLARE_TASK( TASK_HUMAN_SOUND_ATTACK )
	DECLARE_TASK( TASK_HUMAN_SOUND_CHARGE )
	DECLARE_TASK( TASK_HUMAN_SOUND_CHECK_IN )
	DECLARE_TASK( TASK_HUMAN_SOUND_CLEAR )
	DECLARE_TASK( TASK_HUMAN_SOUND_COME_TO_ME )
	DECLARE_TASK( TASK_HUMAN_SOUND_COVER )
	DECLARE_TASK( TASK_HUMAN_SOUND_EXPL )
	DECLARE_TASK( TASK_HUMAN_SOUND_FOUND_ENEMY )
	DECLARE_TASK( TASK_HUMAN_SOUND_GRENADE )
	DECLARE_TASK( TASK_HUMAN_SOUND_HEALED )
	DECLARE_TASK( TASK_HUMAN_SOUND_HEAR )
	DECLARE_TASK( TASK_HUMAN_SOUND_HELP )
	DECLARE_TASK( TASK_HUMAN_SOUND_MEDIC )
	DECLARE_TASK( TASK_HUMAN_SOUND_RESPOND )
	DECLARE_TASK( TASK_HUMAN_SOUND_RETREAT )
	DECLARE_TASK( TASK_HUMAN_SOUND_RETREATING )
	DECLARE_TASK( TASK_HUMAN_SOUND_SEARCHING )
	DECLARE_TASK( TASK_HUMAN_SOUND_SEARCH_AND_DESTROY )
	DECLARE_TASK( TASK_HUMAN_SOUND_SURPRESS )
	DECLARE_TASK( TASK_HUMAN_SOUND_SURPRESSING )
	DECLARE_TASK( TASK_HUMAN_SOUND_TAUNT )
	DECLARE_TASK( TASK_HUMAN_SOUND_THROW )
	DECLARE_TASK( TASK_HUMAN_SOUND_VICTORY )
	DECLARE_TASK( TASK_HUMAN_UNCROUCH )
	DECLARE_TASK( TASK_HUMAN_WAIT_GOAL_VISIBLE )
	DECLARE_TASK( TASK_FIND_FAR_NODE_COVER_FROM_ORIGIN )
	DECLARE_TASK( TASK_HUMAN_DEFER_SQUAD_GRENADES )
#ifdef HUMAN_STRAFE
	DECLARE_TASK( TASK_GET_STRAFE_PATH )
#endif

	DECLARE_SCHEDULE( SCHED_HUMAN_ALERT_REACT_TO_COMBAT_SOUND )
	DECLARE_SCHEDULE( SCHED_HUMAN_BARNACLE_HIT )
	DECLARE_SCHEDULE( SCHED_HUMAN_BARNACLE_PULL )
	DECLARE_SCHEDULE( SCHED_HUMAN_BARNACLE_CHOMP )
	DECLARE_SCHEDULE( SCHED_HUMAN_BARNACLE_CHEW )
	DECLARE_SCHEDULE( SCHED_HUMAN_BARNACLE_RELEASED_HIGH )
	DECLARE_SCHEDULE( SCHED_HUMAN_BARNACLE_RELEASED_LOW )
	DECLARE_SCHEDULE( SCHED_HUMAN_CHECK_IN )
	DECLARE_SCHEDULE( SCHED_HUMAN_COWER )
	DECLARE_SCHEDULE( SCHED_HUMAN_ESTABLISH_LINE_OF_FIRE )
	DECLARE_SCHEDULE( SCHED_HUMAN_EXPLOSION_DIE )
	DECLARE_SCHEDULE( SCHED_HUMAN_EXPLOSIVE_ELOF )
	DECLARE_SCHEDULE( SCHED_HUMAN_FACE_TARGET )
	DECLARE_SCHEDULE( SCHED_HUMAN_FAIL )
	DECLARE_SCHEDULE( SCHED_HUMAN_FIND_MEDIC )
	DECLARE_SCHEDULE( SCHED_HUMAN_FIND_MEDIC_COMBAT )
	DECLARE_SCHEDULE( SCHED_HUMAN_FOLLOW )
	DECLARE_SCHEDULE( SCHED_HUMAN_FOLLOW_CLOSE )
	DECLARE_SCHEDULE( SCHED_HUMAN_FOUND_ENEMY )
	DECLARE_SCHEDULE( SCHED_HUMAN_GRENADE_COVER )
	DECLARE_SCHEDULE( SCHED_HUMAN_GRENADE_INTERACTION )
	DECLARE_SCHEDULE( SCHED_HUMAN_HEAR_SOUND )
	DECLARE_SCHEDULE( SCHED_HUMAN_HIDE_RELOAD )
	DECLARE_SCHEDULE( SCHED_HUMAN_IDLE_RESPONSE )
	DECLARE_SCHEDULE( SCHED_HUMAN_IDLE_STAND )
	DECLARE_SCHEDULE( SCHED_HUMAN_INVESTIGATE_SOUND )
	DECLARE_SCHEDULE( SCHED_HUMAN_MELEE_ELOF )
	DECLARE_SCHEDULE( SCHED_HUMAN_MOVE_TO_ENEMY_LKP )
	DECLARE_SCHEDULE( SCHED_HUMAN_MOVE_TO_WEAPON_RANGE )
	DECLARE_SCHEDULE( SCHED_HUMAN_POPUP_ATTACK )
	DECLARE_SCHEDULE( SCHED_HUMAN_PRIMARY_MELEE_ATTACK1 )
	DECLARE_SCHEDULE( SCHED_HUMAN_RANGE_ATTACK1 )
	DECLARE_SCHEDULE( SCHED_HUMAN_RANGE_ATTACK2 )
	DECLARE_SCHEDULE( SCHED_HUMAN_RELOAD )
	DECLARE_SCHEDULE( SCHED_HUMAN_REPEL )
	DECLARE_SCHEDULE( SCHED_HUMAN_REPEL_ATTACK )
	DECLARE_SCHEDULE( SCHED_HUMAN_REPEL_LAND )
	DECLARE_SCHEDULE( SCHED_HUMAN_REPEL_LAND_SEARCH )
	DECLARE_SCHEDULE( SCHED_HUMAN_RETREAT )
	DECLARE_SCHEDULE( SCHED_HUMAN_SEARCH_AND_DESTROY )
	DECLARE_SCHEDULE( SCHED_HUMAN_SIGNAL_ATTACK )
	DECLARE_SCHEDULE( SCHED_HUMAN_SIGNAL_CHECK_IN )
	DECLARE_SCHEDULE( SCHED_HUMAN_SIGNAL_COME_TO_ME )
	DECLARE_SCHEDULE( SCHED_HUMAN_SIGNAL_RETREAT )
	DECLARE_SCHEDULE( SCHED_HUMAN_SIGNAL_SEARCH_AND_DESTROY )
	DECLARE_SCHEDULE( SCHED_HUMAN_SIGNAL_SURPRESS )
	DECLARE_SCHEDULE( SCHED_HUMAN_STANDOFF )
#ifdef HUMAN_STRAFE
	DECLARE_SCHEDULE( SCHED_HUMAN_STRAFE )
#endif
	DECLARE_SCHEDULE( SCHED_HUMAN_SURPRESS )
	DECLARE_SCHEDULE( SCHED_HUMAN_TAKE_COVER )
	DECLARE_SCHEDULE( SCHED_HUMAN_TAKE_COVER_FROM_BEST_SOUND )
	DECLARE_SCHEDULE( SCHED_HUMAN_TAKECOVER_FAILED )
	DECLARE_SCHEDULE( SCHED_HUMAN_TOSS_GRENADE_COVER )
	DECLARE_SCHEDULE( SCHED_HUMAN_TURN_ROUND )
	DECLARE_SCHEDULE( SCHED_HUMAN_UNCROUCH )
	DECLARE_SCHEDULE( SCHED_HUMAN_UNCROUCH_SCRIPT )
	DECLARE_SCHEDULE( SCHED_HUMAN_VICTORY_DANCE )
	DECLARE_SCHEDULE( SCHED_HUMAN_WAIT_HEAL )
	DECLARE_SCHEDULE( SCHED_HUMAN_WAIT_IN_COVER )

	LOAD_SCHEDULES_FILE( npc_human )

	DECLARE_ACTIVITY( ACT_BARNACLE_RELEASED )
	DECLARE_ACTIVITY( ACT_BARNACLE_HIT_GROUND )
	DECLARE_ACTIVITY( ACT_EXPLOSION_HIT )
	DECLARE_ACTIVITY( ACT_EXPLOSION_FLY )
	DECLARE_ACTIVITY( ACT_WALK_CROUCH_HURT )
	DECLARE_ACTIVITY( ACT_RUN_CROUCH_HURT )
	DECLARE_ACTIVITY( ACT_TURN_LEFT_LOW )
	DECLARE_ACTIVITY( ACT_TURN_RIGHT_LOW )
	DECLARE_ACTIVITY( ACT_SIGNAL1_LOW )
	DECLARE_ACTIVITY( ACT_SIGNAL2_LOW )
	DECLARE_ACTIVITY( ACT_SIGNAL3_LOW )
	DECLARE_ACTIVITY( ACT_SMALL_FLINCH_LOW )
	DECLARE_ACTIVITY( ACT_IDLE_ANGRY_LOW )

	DECLARE_ANIMEVENT( AE_NPC_KICK )
	DECLARE_ANIMEVENT( AE_HUMAN_GRENADE_TOSS )
	DECLARE_ANIMEVENT( AE_HUMAN_GRENADE_DROP )
	DECLARE_ANIMEVENT( AE_HUMAN_OPEN_VEHICLE )
	DECLARE_ANIMEVENT( AE_HUMAN_CLOSE_VEHICLE )

HOE_END_CUSTOM_NPC()
