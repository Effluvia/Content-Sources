#include "cbase.h"
#include "ai_basenpc_flyer.h"
#include "ai_route.h"
#include "hl2_shareddefs.h"
#include "npcevent.h"
#include "basegrenade_shared.h"
#include "props.h"
#include "weapon_physcannon.h"
#define ANTLION_SPIT
#ifdef ANTLION_SPIT
#include "particle_parse.h"
#include "particle_system.h"
#endif
#include "gib.h"
#include "ai_senses.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sk_spx_spit_velocity( "sk_spx_spit_velocity", "1750" );
ConVar sk_spx_spit_damage( "sk_spx_spit_damage", "15"); // 10,15,20

ConVar hoe_spxflyer_accel( "hoe_spxflyer_accel", "300.0" );
ConVar hoe_spxflyer_max_velocity( "hoe_spxflyer_max_velocity", "500.0" );

#define SPIT_MODEL "models/spx_mature/acidspit/acidspit.mdl"
#define SPIT_GRAVITY 400

#define ACID_LOOP

class CSpxSpit : public CBaseGrenade
{
public:
	DECLARE_CLASS( CSpxSpit, CBaseGrenade );
	DECLARE_DATADESC();

	CSpxSpit(void);

	void		Spawn( void );
	void		Precache( void );
	void		SpitThink( void );
	void 		SpitTouch( CBaseEntity *pOther );
	void		BurnThink( void );
	void		Event_Killed( const CTakeDamageInfo &info );
	void 		Detonate(void);

	void		Activate( void );
	void		UpdateOnRemove( void );

//	int			m_nSpitSprite;
	int			m_nSteamSprite;
	float		m_fSpitDeathTime;		// If non-zero won't detonate
	EHANDLE		m_hVictim;				// Guy receiving time-based damage
#ifdef ACID_LOOP
	bool		m_bSoundPlaying;
#else
	float		m_flSoundTime;			// When burn sound ends
#endif
#ifdef ANTLION_SPIT
	CHandle< CParticleSystem >	m_hSpitEffect;
#endif
	static void SpitRegister( CSpxSpit *pSpit );
	static void SpitUnregister( CSpxSpit *pSpit );
	static CSpxSpit *FindExistingSpitForVictim( CBaseEntity *pVictim );

	static CUtlVector<EHANDLE> s_BurningSpits; // Global list of spits that are burning
};

BEGIN_DATADESC( CSpxSpit )

//	DEFINE_FIELD( m_nSpitSprite, FIELD_INTEGER ),
	DEFINE_FIELD( m_fSpitDeathTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_hVictim, FIELD_EHANDLE ),
#ifdef ACID_LOOP
	DEFINE_FIELD( m_bSoundPlaying, FIELD_BOOLEAN ),
#else
	DEFINE_FIELD( m_flSoundTime, FIELD_TIME ),
#endif
	DEFINE_THINKFUNC( SpitThink ),
	DEFINE_THINKFUNC( BurnThink ),
	DEFINE_ENTITYFUNC( SpitTouch ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( spx_spit, CSpxSpit );

CUtlVector<EHANDLE> CSpxSpit::s_BurningSpits;

CSpxSpit::CSpxSpit(void)
{
}

void CSpxSpit::Spawn( void )
{
	Precache( );
	SetSolid( SOLID_BBOX );
	SetMoveType( MOVETYPE_FLYGRAVITY );

	SetModel( SPIT_MODEL );
	UTIL_SetSize(this, Vector(0, 0, 0), Vector(0, 0, 0));
#if 0
	m_nRenderMode		= kRenderTransAdd;
	SetRenderColor( 255, 255, 255, 255 );
	m_nRenderFX		= kRenderFxNone;
#endif
	SetThink( &CSpxSpit::SpitThink );
	SetUse( &CSpxSpit::DetonateUse );
	SetTouch( &CSpxSpit::SpitTouch );
	SetNextThink( gpGlobals->curtime + 0.1f );

	m_flDamage		= sk_spx_spit_damage.GetFloat();
	m_DmgRadius		= 10; // FIXME: remove radius damage
	m_takedamage	= DAMAGE_YES;
	m_iHealth		= 1;

	SetGravity( UTIL_ScaleForGravity( SPIT_GRAVITY ) );
	SetFriction( 0.8 );

	SetSequence( 0 );

	SetCollisionGroup( HL2COLLISION_GROUP_SPIT );

	AddEffects( EF_NOSHADOW|EF_NORECEIVESHADOW );

#ifdef ANTLION_SPIT
	m_hSpitEffect = (CParticleSystem *) CreateEntityByName( "info_particle_system" );
	if ( m_hSpitEffect != NULL )
	{
		// Setup our basic parameters
		m_hSpitEffect->KeyValue( "start_active", "1" );
		m_hSpitEffect->KeyValue( "effect_name", "antlion_spit_trail" );
		m_hSpitEffect->SetParent( this );
		m_hSpitEffect->SetLocalOrigin( vec3_origin );
		DispatchSpawn( m_hSpitEffect );
		if ( gpGlobals->curtime > 0.5f )
			m_hSpitEffect->Activate();
	}
#endif
}

void CSpxSpit::Event_Killed( const CTakeDamageInfo &info )
{
	Detonate( );
}

void CSpxSpit::SpitTouch( CBaseEntity *pOther )
{
	if ( m_fSpitDeathTime != 0 )
	{
		return;
	}
	if ( pOther->GetCollisionGroup() == HL2COLLISION_GROUP_SPIT)
	{
		return;
	}

#ifdef ANTLION_SPIT
	const trace_t *pTrace = &CBaseEntity::GetTouchTrace();

	const Vector tracePlaneNormal = pTrace->plane.normal;

	QAngle vecAngles;
	VectorAngles( tracePlaneNormal, vecAngles );

	// Do a lighter-weight effect
	DispatchParticleEffect( "antlion_spit_player", GetAbsOrigin(), vecAngles );
#endif

	if ( !pOther->m_takedamage )
	{
		// make a splat on the wall
		trace_t tr;
		UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + GetAbsVelocity() * 10, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );
		UTIL_DecalTrace(&tr, "BlackSpit" ); // see scripts/decals_subrect.txt

		// make some flecks
		// CPVSFilter filter( tr.endpos );
		//te->SpriteSpray( filter, 0.0,
		//	tr.endpos, tr.plane.normal, m_nSquidSpitSprite, 30, 0.8, 5 );

		// Smoke from acid burn
		CPVSFilter filter( tr.endpos );
		extern short g_sModelIndexSmoke;
		te->Smoke( filter, 0.0, 
			&tr.endpos, m_nSteamSprite,
			1.0,
			6 );

		Detonate();
	}
	else
	{
		EmitSound( "NPC_SpxMature.SpitHit" );

		// See if this victim was already hit by acid spit.  We only want one spit burning
		// and playing sound on each victim.
		CSpxSpit *pSpit = FindExistingSpitForVictim( pOther );
		if ( pSpit )
		{
			pSpit->m_flDamage += m_flDamage;
			UTIL_Remove( this );
		}
		else
		{
#ifdef ACID_LOOP
			EmitSound( "NPC_SpxMature.Acid" );
			EmitSound( "NPC_SpxMature.AcidLoop" );
			m_bSoundPlaying = true;
#else
			float flDuration;
			EmitSound( "NPC_SpxMature.Acid", 0.0, &flDuration );
			m_flSoundTime = gpGlobals->curtime + flDuration;
#endif
#ifdef ANTLION_SPIT
			if ( m_hSpitEffect )
			{
				UTIL_Remove( m_hSpitEffect );
				m_hSpitEffect = NULL;
			}
#endif

			AddEffects( EF_NODRAW );
			VPhysicsDestroyObject();
			FollowEntity( pOther );
			SetAbsVelocity( vec3_origin );

			m_takedamage = DAMAGE_NO;
			m_hVictim = pOther;

			SpitRegister( this );

			SetTouch( NULL ); // probably not needed since FollowEntity sets FSOLID_NOT_SOLID

			SetThink( &CSpxSpit::BurnThink );
			SetNextThink( gpGlobals->curtime );
		}

//		RadiusDamage( CTakeDamageInfo( this, GetThrower(), m_flDamage, DMG_ACID ), GetAbsOrigin(), m_DmgRadius, CLASS_NONE, NULL );
	}
}

void CSpxSpit::SpitThink( void )
{
	if (m_fSpitDeathTime != 0 &&
		m_fSpitDeathTime < gpGlobals->curtime)
	{
		UTIL_Remove( this );
	}
	SetNextThink( gpGlobals->curtime + 0.1f );
}

void CSpxSpit::BurnThink( void )
{
	// If victim got spit on once, do 1 pt dmg
	// If victim got spit on twice, do 2 pt dmg
	float flStartDmg = sk_spx_spit_damage.GetFloat();
	if ( flStartDmg <= 0 )
		flStartDmg = 1.0f;
	
	float flDmgRemaining = m_flDamage;
	if ( flDmgRemaining <= 0 )
		flDmgRemaining = 1.0f;

	float flDmgPerThink = ceil(flDmgRemaining / flStartDmg);
	if ( flDmgPerThink > flDmgRemaining )
		flDmgPerThink = flDmgRemaining;

//	DevMsg("CSpxSpit::BurnThink dmg=%.2f remaining=%.2f\n", flDmgPerThink, m_flDamage.Get());

	if ( m_hVictim != NULL )
	{
#ifndef ACID_LOOP
		// Replay the burning sound
		if ( gpGlobals->curtime >= m_flSoundTime )
		{
			float flDuration;
			EmitSound( "NPC_SpxMature.Acid", 0.0, &flDuration );
			m_flSoundTime = gpGlobals->curtime + flDuration;
		}
#endif
		if ( m_hVictim->IsPlayer() )
		{
			CBasePlayer *pPlayer = ToBasePlayer( m_hVictim );
			float punchAngle = pPlayer->m_Local.m_vecPunchAngle.GetX();
			m_hVictim->TakeDamage( CTakeDamageInfo( this, GetThrower(), flDmgPerThink, DMG_ACID ) );
			pPlayer->m_Local.m_vecPunchAngle.SetX( punchAngle );
		}
		else
		{
			m_hVictim->TakeDamage( CTakeDamageInfo( this, GetThrower(), flDmgPerThink, DMG_ACID ) );
		}
	}
	if ( m_hVictim == NULL || (m_flDamage -= flDmgPerThink) <= 0 )
	{
		UTIL_Remove( this );
		return;
	}
	SetNextThink( gpGlobals->curtime + 0.333f );
}

void CSpxSpit::Detonate(void)
{
	m_takedamage = DAMAGE_NO;	

	EmitSound( "NPC_SpxMature.SpitHit" );	
	EmitSound( "NPC_SpxMature.Acid" );	

	UTIL_Remove( this );
}

void CSpxSpit::Precache( void )
{
//	m_nSpitSprite = PrecacheModel("sprites/greenglow1.vmt"); // client side spittle.
	m_nSteamSprite = PrecacheModel("sprites/xssmke1.spr");

	PrecacheModel( SPIT_MODEL ); 

	PrecacheScriptSound( "NPC_SpxMature.Acid" );
#ifdef ACID_LOOP
	PrecacheScriptSound( "NPC_SpxMature.AcidLoop" );
	PrecacheScriptSound( "NPC_SpxMature.AcidLoopStop" );
#endif
	PrecacheScriptSound( "NPC_SpxMature.SpitHit" );

#ifdef ANTLION_SPIT
	PrecacheParticleSystem( "antlion_spit_player" );
	PrecacheParticleSystem( "antlion_spit" );
#endif
}

void CSpxSpit::SpitRegister( CSpxSpit *pSpit )
{
	if ( s_BurningSpits.Find( pSpit ) == -1 )
		s_BurningSpits.AddToTail( pSpit );
}

void CSpxSpit::SpitUnregister( CSpxSpit *pSpit )
{
	if ( s_BurningSpits.Find( pSpit ) != -1 )
		s_BurningSpits.FindAndRemove( pSpit );
}

CSpxSpit *CSpxSpit::FindExistingSpitForVictim( CBaseEntity *pVictim )
{
	for ( int i = 0; i < s_BurningSpits.Count(); i++ )
	{
		CSpxSpit *pSpit = dynamic_cast<CSpxSpit *>(s_BurningSpits[i].Get());
		if ( pSpit && pSpit->m_hVictim == pVictim )
			return pSpit;
	}
	return NULL;
}

void CSpxSpit::Activate( void )
{
	BaseClass::Activate();

	if ( m_hVictim != NULL )
		SpitRegister( this );
}

void CSpxSpit::UpdateOnRemove( void )
{
#ifdef ACID_LOOP
	if ( m_bSoundPlaying )
		EmitSound( "NPC_SpxMature.AcidLoopStop" );
#endif
#ifdef ANTLION_SPIT
	if ( m_hSpitEffect )
	{
		UTIL_Remove( m_hSpitEffect );
	}
#endif

	SpitUnregister( this );

	// Chain at end to mimic destructor unwind order
	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------

ConVar sk_spx_flyer_health( "sk_spx_flyer_health", "0" );

class CNPC_SpxFlyer : public CAI_BaseFlyingBot
{
public:
	DECLARE_CLASS( CNPC_SpxFlyer, CAI_BaseFlyingBot );
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	void Spawn( void );
	void Precache( void );
	void Activate( void );

	Class_T Classify( void );
	bool ClassifyPlayerAlly( void ) { return false; }
	bool ClassifyPlayerAllyVital( void ) { return false; }

	void HandleAnimEvent( animevent_t *pEvent );

	virtual float InnateRange1MinRange( void ) { return 256.0f; }
	virtual float InnateRange1MaxRange( void ) { return 2048.0f; }

	int RangeAttack1Conditions( float flDot, float flDist );

	int MeleeAttack1Conditions ( float flDot, float flDist );
	int MeleeAttack2Conditions ( float flDot, float flDist );
	CBaseEntity *ClawAttack( const Vector &vStart, const Vector &vEnd, int iDamage, QAngle &qaViewPunch, Vector &vecVelocityPunch, int BloodOrigin  );
	float GetClawAttackRange() const { return 55; }

	void IdleSound( void );
	void AlertSound( void );
	void AttackSound( void );
	void DeathSound( const CTakeDamageInfo &info );
	void PainSound( const CTakeDamageInfo &info );

	void AttackHitSound( void );
	void AttackMissSound( void );

	int GetSoundInterests( void );

	void PrescheduleThink( void );
	int SelectSchedule( void );
	int TranslateSchedule( int scheduleType );
	void BuildScheduleTestBits( void );
	void StartTask( const Task_t *pTask );
	void RunTask( const Task_t *pTask );

	void TranslateNavGoal( CBaseEntity *pTarget, Vector &chasePosition );
	bool OverrideMove( float flInterval );
	void MoveToTarget(float flInterval, const Vector &MoveTarget);
	void MoveExecute_Alive(float flInterval);

	Activity FlyActivity( void );
	Activity NPC_TranslateActivity( Activity eNewActivity );

	// Override CAI_BaseFlyingBot
	QAngle BodyAngles( void ) { return CBaseCombatCharacter::BodyAngles(); }

	void TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr );
	void Event_Killed( const CTakeDamageInfo &info );

	float m_flNextSpitTime;
	float m_flPainSoundTime;
	float m_flLastDivebombTime;
	bool m_bWingSoundPlaying;
#define SPXSTRAFE
#ifdef SPXSTRAFE
	float m_flNextStrafeTime;
	int m_iStrafeDir;
#endif

	//-----------------------------------------------------------------------------
	// Custom schedules.
	//-----------------------------------------------------------------------------
	enum
	{
		SCHED_SPXFLYER_MELEE_ATTACK1 = BaseClass::NEXT_SCHEDULE,
		SCHED_SPXFLYER_MELEE_ATTACK2,
		SCHED_SPXFLYER_DIVEBOMB,
		NEXT_SCHEDULE
	};

	//-----------------------------------------------------------------------------
	// Activities
	//-----------------------------------------------------------------------------
	static Activity ACT_SPXFLYER_FLY_FORWARD;
	static Activity ACT_SPXFLYER_FLY_BACKWARD;
	static Activity ACT_SPXFLYER_FLY_LEFT;
	static Activity ACT_SPXFLYER_FLY_RIGHT;
	static Activity ACT_SPXFLYER_FLY_UP;
	static Activity ACT_SPXFLYER_FLY_DOWN;
	static Activity ACT_SPXFLYER_HIT;
};

LINK_ENTITY_TO_CLASS( npc_spx_flyer, CNPC_SpxFlyer );

//-----------------------------------------------------------------------------
// Spawnflags
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Animation events.
//-----------------------------------------------------------------------------
static Animevent AE_SPXFLYER_SPIT;
static Animevent AE_SPXFLYER_CLAW_LEFT;
static Animevent AE_SPXFLYER_CLAW_RIGHT;
static Animevent AE_SPXFLYER_CLAW_LEFT_DOWN;
static Animevent AE_SPXFLYER_CLAW_RIGHT_DOWN;

//-----------------------------------------------------------------------------
// Activities
//-----------------------------------------------------------------------------
Activity CNPC_SpxFlyer::ACT_SPXFLYER_FLY_FORWARD;
Activity CNPC_SpxFlyer::ACT_SPXFLYER_FLY_BACKWARD;
Activity CNPC_SpxFlyer::ACT_SPXFLYER_FLY_LEFT;
Activity CNPC_SpxFlyer::ACT_SPXFLYER_FLY_RIGHT;
Activity CNPC_SpxFlyer::ACT_SPXFLYER_FLY_UP;
Activity CNPC_SpxFlyer::ACT_SPXFLYER_FLY_DOWN;
Activity CNPC_SpxFlyer::ACT_SPXFLYER_HIT;

//-----------------------------------------------------------------------------
// Tasks
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Conditions 
//-----------------------------------------------------------------------------

BEGIN_DATADESC( CNPC_SpxFlyer )
	DEFINE_FIELD( m_flNextSpitTime,		FIELD_TIME ),
	DEFINE_FIELD( m_flPainSoundTime,	FIELD_TIME ),
	DEFINE_FIELD( m_flLastDivebombTime,	FIELD_TIME ),
//	DEFINE_FIELD( m_bWingSoundPlaying,	FIELD_BOOLEAN ), NOT SAVED
END_DATADESC()

#define SPXFLYER_HEALTH sk_spx_flyer_health.GetFloat() // 60, 60, 100
#define SPXFLYER_DMG_SPIT 10 // 10, 15, 20
#define SPXFLYER_DMG_SLASH 15 // 20, 25, 30

#define SPX_SPIT_INTERVAL 1.5f

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_SpxFlyer::Spawn( void )
{
	BaseClass::Spawn();

	Precache();
	SetModel( STRING( GetModelName() ) );

	SetHullSizeNormal();

	SetSolid( SOLID_BBOX );
	SetMoveType( MOVETYPE_STEP );
	AddFlag( FL_FLY /*| FL_STEPMOVEMENT*/ );
	SetNavType( NAV_FLY );

	CapabilitiesClear();
	CapabilitiesAdd( bits_CAP_MOVE_FLY |
		bits_CAP_INNATE_RANGE_ATTACK1 |
		bits_CAP_INNATE_MELEE_ATTACK1 |
		bits_CAP_INNATE_MELEE_ATTACK2 );
	CapabilitiesAdd( bits_CAP_MOVE_SHOOT );

	SetBloodColor( BLOOD_COLOR_GREEN );
	m_NPCState = NPC_STATE_NONE;

	m_flFieldOfView = VIEW_FIELD_WIDE;
//	SetViewOffset( Vector( 0, 0, -2 ) ); // Position of the eyes relative to NPC's origin.
	SetViewOffset( Vector( 0, 0, 48 ) ); // Position of the eyes relative to NPC's origin.

	m_iHealth = SPXFLYER_HEALTH;

	m_flNextSpitTime = gpGlobals->curtime;

	// Just bob up and down.
	float flNoiseMod = random->RandomFloat( 1.7, 2.3 );
	SetNoiseMod( 0, 0, flNoiseMod );

	NPCInit();
}

//-----------------------------------------------------------------------------
// Purpose: Precaches all resources this monster needs.
//-----------------------------------------------------------------------------
void CNPC_SpxFlyer::Precache( void )
{
	SetModelName( AllocPooledString( "models/spx_mature/spx_mature.mdl" ) );
	PrecacheModel( STRING( GetModelName() ) );

	UTIL_PrecacheOther( "prop_physics" /*, "models/spx_mature/gibs/wingLF.mdl"*/ );
	PrecacheModel( "models/spx_mature/gibs/wingLF.mdl" );
	PrecacheModel( "models/spx_mature/gibs/wingRF.mdl" );
	PrecacheModel( "models/spx_mature/gibs/wingLR.mdl" );
	PrecacheModel( "models/spx_mature/gibs/wingRR.mdl" );

	PrecacheScriptSound( "NPC_SpxMature.Alert" );
	PrecacheScriptSound( "NPC_SpxMature.Attack" );
	PrecacheScriptSound( "NPC_SpxMature.Death" );
	PrecacheScriptSound( "NPC_SpxMature.Idle" );
	PrecacheScriptSound( "NPC_SpxMature.Pain" );
	PrecacheScriptSound( "NPC_SpxMature.Punch" );
	PrecacheScriptSound( "NPC_SpxMature.WingsLoop" );

	UTIL_PrecacheOther( "spx_spit" );
	PrecacheModel( SPIT_MODEL );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
void CNPC_SpxFlyer::Activate( void )
{
	BaseClass::Activate();
}

//-----------------------------------------------------------------------------
// Purpose: Indicates this monster's place in the relationship table.
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_SpxFlyer::Classify( void )
{
	return CLASS_SPX_FLYER; 
}

//-----------------------------------------------------------------------------
void CNPC_SpxFlyer::HandleAnimEvent( animevent_t *pEvent )
{
	if (pEvent->event == AE_SPXFLYER_SPIT)
	{
		AttackSound();

		Vector vSpitPos;
		GetAttachment( "mouth", vSpitPos );

		Vector vTarget = GetEnemy()->EyePosition();
		Vector vSpitDir = vTarget - vSpitPos;
		VectorNormalize( vSpitDir );

		Vector vToss = vSpitDir * sk_spx_spit_velocity.GetFloat();

		CSpxSpit *pGrenade = (CSpxSpit*)CreateNoSpawn( "spx_spit", vSpitPos, vec3_angle, this );
		pGrenade->Spawn( );
		pGrenade->SetThrower( this );
		pGrenade->SetAbsVelocity( vToss );

		QAngle angles;
		VectorAngles( vToss, angles );
		pGrenade->SetAbsAngles( angles );

		m_flNextSpitTime = gpGlobals->curtime + SPX_SPIT_INTERVAL;
		return;
	}

	if (pEvent->event == AE_SPXFLYER_CLAW_LEFT)
	{
		Vector right, forward;
		AngleVectors( GetLocalAngles(), &forward, &right, NULL );

		Vector vStart = WorldSpaceCenter();
		Vector vEnd = vStart + forward * GetClawAttackRange();

		right = right * -100;
		forward = forward * 200;

		ClawAttack( vStart, vEnd, SPXFLYER_DMG_SLASH, QAngle( -15, 20, -10 ), right + forward, -1 /* ZOMBIE_BLOOD_LEFT_HAND */);
		return;
	}

	if (pEvent->event == AE_SPXFLYER_CLAW_RIGHT)
	{
		Vector right, forward;
		AngleVectors( GetLocalAngles(), &forward, &right, NULL );
		
		Vector vStart = WorldSpaceCenter();
		Vector vEnd = vStart + forward * GetClawAttackRange();

		right = right * 100;
		forward = forward * 200;

		ClawAttack( vStart, vEnd, SPXFLYER_DMG_SLASH, QAngle( -15, -20, -10 ), right + forward, -1 /* ZOMBIE_BLOOD_RIGHT_HAND */);
		return;
	}

	if ( pEvent->event == AE_SPXFLYER_CLAW_LEFT_DOWN )
	{
		Vector forward, right, up;
		AngleVectors( GetLocalAngles(), &forward, &right, &up );

		Vector vStart = GetAbsOrigin();
		Vector vEnd = vStart - up * GetClawAttackRange();

		right = right * -100;

		ClawAttack( vStart, vEnd, SPXFLYER_DMG_SLASH, QAngle( -15, 20, -10 ), right, -1 /* ZOMBIE_BLOOD_LEFT_HAND */);
		return;
	}

	if ( pEvent->event == AE_SPXFLYER_CLAW_RIGHT_DOWN )
	{
		Vector forward, right, up;
		AngleVectors( GetLocalAngles(), &forward, &right, &up );

		Vector vStart = GetAbsOrigin();
		Vector vEnd = vStart - up * GetClawAttackRange();

		right = right * 100;

		ClawAttack( vStart, vEnd, SPXFLYER_DMG_SLASH, QAngle( -15, 20, -10 ), right, -1 /* ZOMBIE_BLOOD_LEFT_HAND */);
		return;
	}

	switch( pEvent->event )
	{
	case 1:
	default:
		BaseClass::HandleAnimEvent( pEvent );
		break;
	}
}

//-----------------------------------------------------------------------------
int CNPC_SpxFlyer::RangeAttack1Conditions( float flDot, float flDist )
{
#ifdef NO_MOVE_SHOOT
	if ( IsMoving() && flDist >= 512 )
	{
		// squid will far too far behind if he stops running to spit at this distance from the enemy.
		return ( COND_NONE );
	}
#endif

	if ( flDist > 256 && flDist <= 2048 && flDot >= 0.5 && gpGlobals->curtime >= m_flNextSpitTime )
	{
		if ( GetEnemy() != NULL )
		{
#define SPX_SPIT_MAX_DELTAZ 512.0f 
			if ( fabs( GetAbsOrigin().z - GetEnemy()->GetAbsOrigin().z ) > SPX_SPIT_MAX_DELTAZ )
			{
				// don't try to spit at someone up really high or down really low.
				return( COND_NONE );
			}
		}

#ifdef NO_MOVE_SHOOT
		if ( IsMoving() )
		{
			// don't spit again for a long time, resume chasing enemy.
			m_flNextSpitTime = gpGlobals->curtime + 5;
		}
		else
#endif
		{
#if 0 // set this when we actually spit
			// not moving, so spit again pretty soon.
#define SPX_SPIT_INTERVAL 1.5f
			m_flNextSpitTime = gpGlobals->curtime + SPX_SPIT_INTERVAL;
#endif
		}

		return( COND_CAN_RANGE_ATTACK1 );
	}

	return( COND_NONE );
}

//-----------------------------------------------------------------------------
int CNPC_SpxFlyer::MeleeAttack1Conditions( float flDot, float flDist )
{
	if (flDist > GetClawAttackRange() )
	{
		// Translate a hit vehicle into its passenger if found
		if ( GetEnemy() != NULL )
		{
//			CBaseCombatCharacter *pCCEnemy = GetEnemy()->MyCombatCharacterPointer();
//			if ( pCCEnemy != NULL && pCCEnemy->IsInAVehicle() )
//				return MeleeAttack1ConditionsVsEnemyInVehicle( pCCEnemy, flDot );

#if defined(HL2_DLL) && !defined(HL2MP)
			// If the player is holding an object, knock it down.
			if( GetEnemy()->IsPlayer() )
			{
				CBasePlayer *pPlayer = ToBasePlayer( GetEnemy() );

				Assert( pPlayer != NULL );

				// Is the player carrying something?
				CBaseEntity *pObject = GetPlayerHeldEntity(pPlayer);

				if( !pObject )
				{
					pObject = PhysCannonGetHeldEntity( pPlayer->GetActiveWeapon() );
				}

				if( pObject )
				{
					float flDist = pObject->WorldSpaceCenter().DistTo( WorldSpaceCenter() );

					if( flDist <= GetClawAttackRange() )
						return COND_CAN_MELEE_ATTACK1;
				}
			}
#endif
		}
		return COND_TOO_FAR_TO_ATTACK;
	}

	if (flDot < 0.7)
	{
		return COND_NOT_FACING_ATTACK;
	}

	// Build a cube-shaped hull, the same hull that ClawAttack() is going to use.
	Vector vecMins = GetHullMins();
	Vector vecMaxs = GetHullMaxs();
	vecMins.z = vecMins.x;
	vecMaxs.z = vecMaxs.x;

	Vector forward;
	GetVectors( &forward, NULL, NULL );

	trace_t	tr;
	CTraceFilterNav traceFilter( this, false, this, COLLISION_GROUP_NONE );
	AI_TraceHull( WorldSpaceCenter(), WorldSpaceCenter() + forward * GetClawAttackRange(), vecMins, vecMaxs, MASK_NPCSOLID, &traceFilter, &tr );

	if( tr.fraction == 1.0 || !tr.m_pEnt )
	{
		// This attack would miss completely. Trick the zombie into moving around some more.
		return COND_TOO_FAR_TO_ATTACK;
	}

	if( tr.m_pEnt == GetEnemy() || tr.m_pEnt->IsNPC() || (tr.m_pEnt->m_takedamage == DAMAGE_YES && (dynamic_cast<CBreakableProp*>(tr.m_pEnt))) )
	{
		// -Let the zombie swipe at his enemy if he's going to hit them.
		// -Also let him swipe at NPC's that happen to be between the zombie and the enemy. 
		//  This makes mobs of zombies seem more rowdy since it doesn't leave guys in the back row standing around.
		// -Also let him swipe at things that takedamage, under the assumptions that they can be broken.
		return COND_CAN_MELEE_ATTACK1;
	}
#if 0
	if( tr.m_pEnt->IsBSPModel() )
	{
		// The trace hit something solid, but it's not the enemy. If this item is closer to the zombie than
		// the enemy is, treat this as an obstruction.
		Vector vecToEnemy = GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter();
		Vector vecTrace = tr.endpos - tr.startpos;

		if( vecTrace.Length2DSqr() < vecToEnemy.Length2DSqr() )
		{
			return COND_ZOMBIE_LOCAL_MELEE_OBSTRUCTION;
		}
	}
#endif
#ifdef HL2_EPISODIC
	if ( !tr.m_pEnt->IsWorld() && GetEnemy() && GetEnemy()->GetGroundEntity() == tr.m_pEnt )
	{
		//Try to swat whatever the player is standing on instead of acting like a dill.
		return COND_CAN_MELEE_ATTACK1;
	}
#endif

	// Move around some more
	return COND_TOO_FAR_TO_ATTACK;
}

//-----------------------------------------------------------------------------
int CNPC_SpxFlyer::MeleeAttack2Conditions( float flDot, float flDist )
{
	if ( GetEnemy() == NULL )
		return COND_NONE;

	flDist = (GetAbsOrigin() - GetEnemy()->EyePosition()).Length();

	if (flDist > GetClawAttackRange() )
	{
		// Translate a hit vehicle into its passenger if found
		if ( GetEnemy() != NULL )
		{
//			CBaseCombatCharacter *pCCEnemy = GetEnemy()->MyCombatCharacterPointer();
//			if ( pCCEnemy != NULL && pCCEnemy->IsInAVehicle() )
//				return MeleeAttack1ConditionsVsEnemyInVehicle( pCCEnemy, flDot );

#if defined(HL2_DLL) && !defined(HL2MP)
			// If the player is holding an object, knock it down.
			if( GetEnemy()->IsPlayer() )
			{
				CBasePlayer *pPlayer = ToBasePlayer( GetEnemy() );

				Assert( pPlayer != NULL );

				// Is the player carrying something?
				CBaseEntity *pObject = GetPlayerHeldEntity(pPlayer);

				if( !pObject )
				{
					pObject = PhysCannonGetHeldEntity( pPlayer->GetActiveWeapon() );
				}

				if( pObject )
				{
					float flDist = pObject->WorldSpaceCenter().DistTo( WorldSpaceCenter() );

					if( flDist <= GetClawAttackRange() )
						return COND_CAN_MELEE_ATTACK1;
				}
			}
#endif
		}
		return COND_TOO_FAR_TO_ATTACK;
	}

	// Build a cube-shaped hull, the same hull that ClawAttack() is going to use.
	Vector vecMins = GetHullMins();
	Vector vecMaxs = GetHullMaxs();
	vecMins.z = vecMins.x;
	vecMaxs.z = vecMaxs.x;

	Vector up;
	GetVectors( NULL, NULL, &up );

	trace_t	tr;
	CTraceFilterNav traceFilter( this, false, this, COLLISION_GROUP_NONE );
	AI_TraceHull( GetAbsOrigin(), GetAbsOrigin() - up * GetClawAttackRange(),
		vecMins, vecMaxs, MASK_NPCSOLID, &traceFilter, &tr );

	if ( tr.fraction == 1.0 || !tr.m_pEnt )
	{
		// This attack would miss completely. Trick the zombie into moving around some more.
		return COND_TOO_FAR_TO_ATTACK;
	}

	if ( tr.m_pEnt == GetEnemy() || tr.m_pEnt->IsNPC() || (tr.m_pEnt->m_takedamage == DAMAGE_YES && (dynamic_cast<CBreakableProp*>(tr.m_pEnt))) )
	{
		// -Let the zombie swipe at his enemy if he's going to hit them.
		// -Also let him swipe at NPC's that happen to be between the zombie and the enemy. 
		//  This makes mobs of zombies seem more rowdy since it doesn't leave guys in the back row standing around.
		// -Also let him swipe at things that takedamage, under the assumptions that they can be broken.
		return COND_CAN_MELEE_ATTACK2;
	}
#if 0
	if( tr.m_pEnt->IsBSPModel() )
	{
		// The trace hit something solid, but it's not the enemy. If this item is closer to the zombie than
		// the enemy is, treat this as an obstruction.
		Vector vecToEnemy = GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter();
		Vector vecTrace = tr.endpos - tr.startpos;

		if( vecTrace.Length2DSqr() < vecToEnemy.Length2DSqr() )
		{
			return COND_ZOMBIE_LOCAL_MELEE_OBSTRUCTION;
		}
	}
#endif
#ifdef HL2_EPISODIC
	if ( !tr.m_pEnt->IsWorld() && GetEnemy() && GetEnemy()->GetGroundEntity() == tr.m_pEnt )
	{
		//Try to swat whatever the player is standing on instead of acting like a dill.
		return COND_CAN_MELEE_ATTACK2;
	}
#endif

	// Move around some more
	return COND_TOO_FAR_TO_ATTACK;
}

//-----------------------------------------------------------------------------
// Purpose: Look in front and see if the claw hit anything.
//
// Input  :	flDist				distance to trace		
//			iDamage				damage to do if attack hits
//			vecViewPunch		camera punch (if attack hits player)
//			vecVelocityPunch	velocity punch (if attack hits player)
//
// Output : The entity hit by claws. NULL if nothing.
//-----------------------------------------------------------------------------
CBaseEntity *CNPC_SpxFlyer::ClawAttack( const Vector &vStart, const Vector &vEnd, int iDamage, QAngle &qaViewPunch, Vector &vecVelocityPunch, int BloodOrigin  )
{
	// Added test because claw attack anim sometimes used when for cases other than melee
	if ( GetEnemy() )
	{
		trace_t	tr;
		AI_TraceHull( WorldSpaceCenter(), GetEnemy()->WorldSpaceCenter(), -Vector(8,8,8), Vector(8,8,8), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );

		if ( tr.fraction < 1.0f )
			return NULL;

		// Reduce damage done to NPCs to avoid mass slaughter.
		// FIXME: this may not be the entity hit by CheckTraceHullAttack().
		if ( GetEnemy()->IsNPC() )
			iDamage /= 2;
	}

	//
	// Trace out a cubic section of our hull and see what we hit.
	//
	Vector vecMins = GetHullMins();
	Vector vecMaxs = GetHullMaxs();
	vecMins.z = vecMins.x;
	vecMaxs.z = vecMaxs.x;

	CBaseEntity *pHurt = CheckTraceHullAttack( vStart, vEnd, vecMins, vecMaxs, iDamage, DMG_SLASH );
#if 0
	if ( !pHurt && m_hPhysicsEnt != NULL && IsCurSchedule(SCHED_ZOMBIE_ATTACKITEM) )
	{
		pHurt = m_hPhysicsEnt;

		Vector vForce = pHurt->WorldSpaceCenter() - WorldSpaceCenter(); 
		VectorNormalize( vForce );

		vForce *= 5 * 24;

		CTakeDamageInfo info( this, this, vForce, GetAbsOrigin(), iDamage, DMG_SLASH );
		pHurt->TakeDamage( info );

		pHurt = m_hPhysicsEnt;
	}
#endif
	if ( pHurt )
	{
		AttackHitSound();

		CBasePlayer *pPlayer = ToBasePlayer( pHurt );

		if ( pPlayer != NULL && !(pPlayer->GetFlags() & FL_GODMODE ) )
		{
			pPlayer->ViewPunch( qaViewPunch );
			
			pPlayer->VelocityPunch( vecVelocityPunch );
		}
#if 0
		else if( !pPlayer && UTIL_ShouldShowBlood(pHurt->BloodColor()) )
		{
			// Hit an NPC. Bleed them!
			Vector vecBloodPos;

			switch( BloodOrigin )
			{
			case ZOMBIE_BLOOD_LEFT_HAND:
				if( GetAttachment( "blood_left", vecBloodPos ) )
					SpawnBlood( vecBloodPos, g_vecAttackDir, pHurt->BloodColor(), min( iDamage, 30 ) );
				break;

			case ZOMBIE_BLOOD_RIGHT_HAND:
				if( GetAttachment( "blood_right", vecBloodPos ) )
					SpawnBlood( vecBloodPos, g_vecAttackDir, pHurt->BloodColor(), min( iDamage, 30 ) );
				break;

			case ZOMBIE_BLOOD_BOTH_HANDS:
				if( GetAttachment( "blood_left", vecBloodPos ) )
					SpawnBlood( vecBloodPos, g_vecAttackDir, pHurt->BloodColor(), min( iDamage, 30 ) );

				if( GetAttachment( "blood_right", vecBloodPos ) )
					SpawnBlood( vecBloodPos, g_vecAttackDir, pHurt->BloodColor(), min( iDamage, 30 ) );
				break;

			case ZOMBIE_BLOOD_BITE:
				// No blood for these.
				break;
			}
		}
#endif
	}
	else 
	{
		AttackMissSound();
	}
#if 0
	if ( pHurt == m_hPhysicsEnt && IsCurSchedule(SCHED_ZOMBIE_ATTACKITEM) )
	{
		m_hPhysicsEnt = NULL;
		m_flNextSwat = gpGlobals->curtime + random->RandomFloat( 2, 4 );
	}
#endif
	return pHurt;
}

//-----------------------------------------------------------------------------
void CNPC_SpxFlyer::AlertSound( void )
{
	EmitSound( "NPC_SpxMature.Alert" );
}

//-----------------------------------------------------------------------------
void CNPC_SpxFlyer::AttackSound( void )
{
	EmitSound( "NPC_SpxMature.Attack" );
}

//-----------------------------------------------------------------------------
void CNPC_SpxFlyer::IdleSound( void )
{
	EmitSound( "NPC_SpxMature.Idle" );
}

//-----------------------------------------------------------------------------
void CNPC_SpxFlyer::PainSound( const CTakeDamageInfo &info )
{
	if ( m_flPainSoundTime < gpGlobals->curtime )
	{
		float flDuration;
		EmitSound( "NPC_SpxMature.Pain", 0, &flDuration );
		m_flPainSoundTime = gpGlobals->curtime + flDuration +
			random->RandomFloat( 2.0, 4.0 );
	}
}

//-----------------------------------------------------------------------------
void CNPC_SpxFlyer::DeathSound( const CTakeDamageInfo &info )
{
	EmitSound( "NPC_SpxMature.Death" );
}

//-----------------------------------------------------------------------------
void CNPC_SpxFlyer::AttackHitSound( void )
{
	EmitSound( "NPC_SpxMature.Punch" );
}

//-----------------------------------------------------------------------------
void CNPC_SpxFlyer::AttackMissSound( void )
{
	EmitSound( "NPC_SpxFlyer.AttackMiss" );
}

//=========================================================
// GetSoundInterests - returns a bit mask indicating which types
// of sounds this monster regards. 
//=========================================================
int CNPC_SpxFlyer::GetSoundInterests( void ) 
{
	return BaseClass::GetSoundInterests();
}

//---------------------------------------------------------
void CNPC_SpxFlyer::PrescheduleThink( void )
{
	// This didn't work when I put it in Activate() for the first flyer spawned
	if ( IsAlive() && !m_bWingSoundPlaying )
	{
		CPASAttenuationFilter filter( this, "NPC_SpxMature.WingsLoop" );
		filter.MakeReliable();
		EmitSound( filter, entindex(), "NPC_SpxMature.WingsLoop" );

		m_bWingSoundPlaying = true;
	}

	BaseClass::PrescheduleThink();
}

//---------------------------------------------------------
int CNPC_SpxFlyer::SelectSchedule( void )
{
	return BaseClass::SelectSchedule();
}

//---------------------------------------------------------
int CNPC_SpxFlyer::TranslateSchedule( int scheduleType )
{
	switch( scheduleType )
	{
	case SCHED_MELEE_ATTACK1:
		return SCHED_SPXFLYER_MELEE_ATTACK1;
	case SCHED_MELEE_ATTACK2:
		return SCHED_SPXFLYER_MELEE_ATTACK2;
	case SCHED_CHASE_ENEMY:
	case SCHED_MOVE_TO_WEAPON_RANGE:
		if ( m_flLastDivebombTime < gpGlobals->curtime - 8 /*&&
			random->RandomInt(0,5) == 0*/ )
		{
			m_flLastDivebombTime = gpGlobals->curtime;
			return SCHED_SPXFLYER_DIVEBOMB;
		}
		break;
	}

	return BaseClass::TranslateSchedule( scheduleType );
}

//-----------------------------------------------------------------------------
// Purpose: Allows for modification of the interrupt mask for the current schedule.
//			In the most cases the base implementation should be called first.
//-----------------------------------------------------------------------------
void CNPC_SpxFlyer::BuildScheduleTestBits( void )
{
	// Ignore damage if we were recently damaged or we're attacking.
	if ( GetActivity() == ACT_MELEE_ATTACK1 )
	{
		ClearCustomInterruptCondition( COND_LIGHT_DAMAGE );
		ClearCustomInterruptCondition( COND_HEAVY_DAMAGE );
	}
#ifndef HL2_EPISODIC
	else if ( m_flNextFlinch >= gpGlobals->curtime )
	{
		ClearCustomInterruptCondition( COND_LIGHT_DAMAGE );
		ClearCustomInterruptCondition( COND_HEAVY_DAMAGE );
	}
#endif // !HL2_EPISODIC

	BaseClass::BuildScheduleTestBits();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_SpxFlyer::StartTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_ANNOUNCE_ATTACK:
		{
			EmitSound("NPC_SpxFlyer.AnnounceAttack");
			BaseClass::StartTask( pTask );
		}
		break;
	case TASK_GET_PATH_TO_ENEMY:
		{
			CBaseEntity *pEnemy = GetEnemy();
			if ( pEnemy == NULL )
			{
				TaskFail(FAIL_NO_ENEMY);
				return;
			}
						
			if ( IsUnreachable( pEnemy ) )
			{
				TaskFail(FAIL_NO_ROUTE);
				return;
			}

			if ( GetNavigator()->SetGoal( GOALTYPE_ENEMY ) )
			{
				TaskComplete();
			}
			else
			{
				// no way to get there =( 
				DevWarning( 2, "GetPathToEnemy failed!!\n" );
				RememberUnreachable(GetEnemy());
				TaskFail(FAIL_NO_ROUTE);
			}
			break;
		}
		break;
#if 0
	case TASK_SPXFLYER_DONT_EAT:
		break;
		
	case TASK_SPXFLYER_EAT:
		break;

	case TASK_SPXFLYER_SOUND_EAT:
		break;
#endif
	default:
		BaseClass::StartTask(pTask);
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_SpxFlyer::RunTask( const Task_t *pTask )
{
	switch (pTask->iTask)
	{
	// If my enemy has moved significantly, update my path
	case TASK_WAIT_FOR_MOVEMENT:
		{
			// BUG IN SDK: manhack calls "GetCurSchedule()->GetId() == SCHED_CHASE_ENEMY"
			// which is bogus because GetID() has the high bit set.  So this block of
			// code isn't used and doesn't appear to be needed anyway.
#if 0
			CBaseEntity *pEnemy = GetEnemy();
			if (pEnemy &&
				IsCurSchedule( SCHED_CHASE_ENEMY ) && 
				GetNavigator()->IsGoalActive() )
			{
				Vector vecEnemyPosition;
				vecEnemyPosition = pEnemy->EyePosition();
				if ( GetNavigator()->GetGoalPos().DistToSqr(vecEnemyPosition) > 40 * 40 )
				{
					GetNavigator()->UpdateGoalPos( vecEnemyPosition );
				}
			}
#endif
			// The fly activity should get updated constantly, but where to do it?
			if ( GetActivity() != FlyActivity() )
				SetIdealActivity( FlyActivity() );
			BaseClass::RunTask(pTask);
			break;
		}
	case TASK_WAIT:
		// The fly activity should get updated constantly, but where to do it?
		if ( GetActivity() != FlyActivity() )
			SetIdealActivity( FlyActivity() );
		break;
	case TASK_DIE:
#if 0
		{
			// BaseNPC::Event_Killed
			//    BaseNPC::CleanupOnDeath
			// BaseNPC::SelectDeadSchedule
			//    BaseNPC::CleanupOnDeath
			if (  IsActivityFinished() )
			{
				UTIL_Remove( this );

				TaskComplete();
			}
		}
		break;
#endif
	default:
		BaseClass::RunTask(pTask);
		break;
	}
}

//-----------------------------------------------------------------------------
void CNPC_SpxFlyer::TranslateNavGoal( CBaseEntity *pTarget, Vector &chasePosition )
{
	Assert( pTarget != NULL );

	if ( pTarget == NULL )
	{
		chasePosition = vec3_origin;
		return;
	}

	// Override CAI_BaseFlyingBot which chases the eyes
	chasePosition = pTarget->WorldSpaceCenter();
}

//-----------------------------------------------------------------------------
bool CNPC_SpxFlyer::OverrideMove( float flInterval )
{
	Vector vMoveTargetPos(0,0,0);
	CBaseEntity *pMoveTarget = NULL;
	
	// BUG IN SDK: CNPC_CScanner::OverrideMove says "GetCurWaypointFlags() | bits_WP_TO_PATHCORNER"
	if ( !GetNavigator()->IsGoalActive() || ( GetNavigator()->GetCurWaypointFlags() & bits_WP_TO_PATHCORNER ) )
	{
		if ( GetEnemy() != NULL )
		{
			pMoveTarget = GetEnemy();
			vMoveTargetPos = GetEnemy()->GetAbsOrigin();

			trace_t tr;
			AI_TraceHull( GetAbsOrigin(), vMoveTargetPos, GetHullMins(), GetHullMaxs(), MASK_NPCSOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );

			float fTargetDist = (1.0f-tr.fraction)*(GetAbsOrigin() - vMoveTargetPos).Length();
			
			if ( ( tr.m_pEnt == pMoveTarget ) || ( fTargetDist < 50 ) )
			{
				if ( 0 )
				{
					NDebugOverlay::Line(GetLocalOrigin(), vMoveTargetPos, 0,255,0, true, 0);
					NDebugOverlay::Cross3D(tr.endpos,Vector(-5,-5,-5),Vector(5,5,5),0,255,0,true,0.1);
				}

//				SetCondition( COND_SCANNER_FLY_CLEAR );
			}
			else		
			{
				if ( 0 )
				{
					NDebugOverlay::Line(GetLocalOrigin(), vMoveTargetPos, 255,0,0, true, 0);
					NDebugOverlay::Cross3D(tr.endpos,Vector(-5,-5,-5),Vector(5,5,5),255,0,0,true,0.1);
				}

//				SetCondition( COND_SCANNER_FLY_BLOCKED );
			}
		}
	}

#ifdef SPXSTRAFE
#if 1
	if ( /*HasCondition( COND_SEE_ENEMY )*/GetEnemy() != NULL && GetSenses()->DidSeeEntity( GetEnemy() ) )
	{
		Vector right, up;
		GetVectors( NULL, &right, &up );
		float noiseX = 2;
		float noiseZ = 1;
		float x = sin(noiseX * gpGlobals->curtime + noiseX);
		float z = cos(noiseZ * gpGlobals->curtime + noiseZ);
		SetCurrentVelocity( GetCurrentVelocity() + right * 50 * x + up * 10 * z );
		LimitSpeed( 200, hoe_spxflyer_max_velocity.GetFloat() );
	}
#else
	if ( GetEnemy() != NULL && m_flNextStrafeTime < gpGlobals->curtime )
	{
		Vector right, up;
		GetVectors( NULL, &right, &up );
		if ( !m_iStrafeDir ) m_iStrafeDir = 1;
		right *= m_iStrafeDir;
		up *= m_iStrafeDir;
		m_iStrafeDir *= -1;
		Vector vStrafe = GetAbsOrigin() + right * 100 + up * 20;
		trace_t tr;
		AI_TraceHull( GetAbsOrigin(), vStrafe, GetHullMins(), GetHullMaxs(), MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr );
		if ( tr.fraction > 0.75 )
		{
			SetCurrentVelocity( GetCurrentVelocity() + right * 200 + up * 100 );
		}
		if ( m_iStrafeDir )
			m_flNextStrafeTime = gpGlobals->curtime + random->RandomFloat( 0.75, 1.0 );
		else
			m_flNextStrafeTime = gpGlobals->curtime + random->RandomFloat( 1.5, 2.25 );
	}
#endif
#endif // SPXSTRAFE

	// -----------------------------------------------------------------
	// If I have a route, keep it updated and move toward target
	// -----------------------------------------------------------------
	if (GetNavigator()->IsGoalActive())
	{
		bool bReducible = GetNavigator()->GetPath()->GetCurWaypoint()->IsReducible();
		const float strictTolerance = 64.0;
		//NDebugOverlay::Line( GetAbsOrigin(), GetAbsOrigin() + Vector(0, 0, 10 ), 255, 0, 0, true, 0.1);
  		if ( ProgressFlyPath( flInterval, GetEnemy(), MASK_NPCSOLID, bReducible, strictTolerance ) == AINPP_COMPLETE )
			return true;
	}
	// -----------------------------------------------------------------
	// If I don't have anything better to do, just decelerate
	// -----------------------------------------------------------------
	else
	{
#if 1
		float decay = 1.0 - 0.3 * flInterval;
		decay = clamp( decay, 0.0f, 1.0f );
		m_vCurrentVelocity.x = (decay * m_vCurrentVelocity.x);
		m_vCurrentVelocity.y = (decay * m_vCurrentVelocity.y);
		m_vCurrentVelocity.z = (decay * m_vCurrentVelocity.z);
#else
		float	myDecay	 = 9;
		Decelerate( flInterval, myDecay );
#endif
		//		m_vTargetBanking = vec3_origin;

		// -------------------------------------
		// If I have an enemy turn to face him
		// -------------------------------------
		if (GetEnemy())
		{
//			TurnHeadToTarget(flInterval, GetEnemy()->EyePosition() );
			GetMotor()->SetIdealYawToTargetAndUpdate( GetEnemy()->EyePosition(), AI_KEEP_YAW_SPEED );
		}

		MoveExecute_Alive(flInterval);
	}

	return true;
}

//------------------------------------------------------------------------------
void CNPC_SpxFlyer::MoveToTarget(float flInterval, const Vector &vMoveTarget)
{
//	NDebugOverlay::Cross3D( vMoveTarget, -Vector(4,4,4), Vector(4,4,4), 255, 0, 0, true, .1 );

	Vector vecCurrentDir = GetCurrentVelocity();
	VectorNormalize( vecCurrentDir );

	Vector targetDir = vMoveTarget - GetAbsOrigin();
	float flDist = VectorNormalize( targetDir );
	float flDot = DotProduct( targetDir, vecCurrentDir );

	float	myAccel;
	float	myZAccel = 300.0f;
	float	myDecay	 = 0.3f;

	// Otherwise we should steer towards our goal
	if( flDot > 0.25 )
	{
		// If my target is in front of me, my flight model is a bit more accurate.
		myAccel = hoe_spxflyer_accel.GetFloat();
	}
	else
	{
		// Have a harder time correcting my course if I'm currently flying away from my target.
//		myAccel = 200;
		myAccel = hoe_spxflyer_accel.GetFloat();
	}

	// Clamp lateral acceleration
	if ( myAccel > ( flDist / flInterval ) )
	{
		myAccel = flDist / flInterval;
	}

	// Clamp vertical movement
	if ( myZAccel > flDist / flInterval )
	{
		myZAccel = flDist / flInterval;
	}

	MoveInDirection( flInterval, targetDir, myAccel, myZAccel, myDecay );

	MoveExecute_Alive( flInterval );

	if (GetEnemy())
		GetMotor()->SetIdealYawToTargetAndUpdate( GetEnemy()->EyePosition(), AI_KEEP_YAW_SPEED );
}

//-----------------------------------------------------------------------------
void CNPC_SpxFlyer::MoveExecute_Alive( float flInterval )
{
	Assert( flInterval > 0 );

	Vector velocityCur = GetCurrentVelocity();
	CHECK_VALID(velocityCur);
	
	Vector velocityAvoid = VelocityToAvoidObstacles( flInterval );
	CHECK_VALID(velocityAvoid);

	SetCurrentVelocity( velocityCur + velocityAvoid );

	LimitSpeed( 200, hoe_spxflyer_max_velocity.GetFloat() );

	// Amount of noise to add to flying
	float noiseScale = 7.0f;

	// Add time-coherent noise to the current velocity so that it never looks bolted in place.
	AddNoiseToVelocity( noiseScale );

	SetAbsVelocity( GetCurrentVelocity() );
//	NDebugOverlay::Line( GetLocalOrigin(), GetLocalOrigin() + GetCurrentVelocity(), 255, 255, 0, false, 0.1 );
}

//-----------------------------------------------------------------------------
Activity CNPC_SpxFlyer::FlyActivity( void )
{
	Vector forward, right, up;
	AngleVectors( GetLocalAngles(), &forward, &right, &up);

	float x = DotProduct( forward, GetCurrentVelocity() );
	float y = DotProduct( right, GetCurrentVelocity() );
	float z = DotProduct( up, GetCurrentVelocity() );

	if (fabs(x) > fabs(y) && fabs(x) > fabs(z))
	{
		if (x > 0)
			return ACT_SPXFLYER_FLY_FORWARD;
		else
			return ACT_SPXFLYER_FLY_BACKWARD;
	}
	else if (fabs(y) > fabs(z))
	{
		if (y > 0)
			return ACT_SPXFLYER_FLY_RIGHT;
		else
			return ACT_SPXFLYER_FLY_LEFT;
	}
	else
	{
		if ((z > 0) || (GetCurrentVelocity().z < 20) )
			return ACT_SPXFLYER_FLY_UP;
		else
		{
//			if ( pev->deadflag != DEAD_NO ) return LookupSequence("fall");

			return ACT_SPXFLYER_FLY_DOWN;
		}
	}
}

//-----------------------------------------------------------------------------
Activity CNPC_SpxFlyer::NPC_TranslateActivity( Activity eNewActivity )
{
	if ( eNewActivity == ACT_IDLE )
	{
		Activity act = FlyActivity();
		return act;
	}
	return eNewActivity;
}

//-----------------------------------------------------------------------------
void CNPC_SpxFlyer::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr )
{
	// Make these guys harder for NPCs to shoot, the player has a hard time with them
	// and they will survive longer.
	if ( info.GetDamageType() & DMG_BULLET &&
		info.GetAttacker() &&
		info.GetAttacker()->IsNPC() )
	{
		float flChanceToMiss = 0.5;
		if ( random->RandomFloat(0,1) <= flChanceToMiss )
			return;
	}

	BaseClass::TraceAttack( info, vecDir, ptr, 0 );

	// Since we hide the wings after death don't allow damage decals
	m_fNoDamageDecal = true;
}

//-----------------------------------------------------------------------------
void CNPC_SpxFlyer::Event_Killed( const CTakeDamageInfo &info )
{
	// Hide the wings
	SetBodygroup( 1, 1 );

	StopSound( "NPC_SpxMature.WingsLoop" ); // StopLoopingSounds

	// Break off the wings into some gibs
	static const char *gibs[] = {
		"models/spx_mature/gibs/wingLF.mdl",
		"models/spx_mature/gibs/wingRF.mdl",
		"models/spx_mature/gibs/wingLR.mdl",
		"models/spx_mature/gibs/wingRR.mdl"
	};
	for ( int i = 0; i < 4; i++ )
	{
#if 1
		CPhysicsProp *pGib = assert_cast<CPhysicsProp*>(CreateEntityByName( "prop_physics" ));
		pGib->SetModel( gibs[i] );

		Vector parentOrigin;
		QAngle dummy;
		pGib->SetAbsAngles( GetAbsAngles() );
		pGib->GetAttachment( pGib->LookupAttachment( "placementOrigin" ), parentOrigin, dummy );
		parentOrigin -= pGib->GetAbsOrigin(); // should have no effect, gib spawns at 0,0,0
		pGib->SetAbsOrigin( GetAbsOrigin() - parentOrigin );

		DispatchSpawn( pGib ); //pGib->Spawn();
//		pGib->SetMoveType( MOVETYPE_VPHYSICS );
		pGib->SetCollisionGroup( COLLISION_GROUP_DEBRIS );

		pGib->SetGravity( UTIL_ScaleForGravity( 250 ) ); // FIXME: doesn't seem to do anything

		// Start off with the same velocity as the NPC
		pGib->SetAbsVelocity( GetAbsVelocity() );

		// Add some angular velocity (see CGib::InitGib)
		QAngle vecNewAngularVelocity = GetLocalAngularVelocity();
		vecNewAngularVelocity.x = random->RandomFloat ( 100, 200 );
		vecNewAngularVelocity.y = random->RandomFloat ( 100, 300 );
		SetLocalAngularVelocity( vecNewAngularVelocity );

		// Add some more angular velocity (see CGib::InitGib)
		IPhysicsObject *pObj = pGib->VPhysicsGetObject();
		if ( pObj != NULL )
		{
			AngularImpulse angImpulse = RandomAngularImpulse( -500, 500 );
			pObj->AddVelocity( NULL, &angImpulse );
		}

#if 1
		Vector vecForce = info.GetDamageForce();
		pGib->ApplyAbsVelocityImpulse( vecForce * 0.01f );
#else
		Vector vecForce = info.GetDamageForce();
		VectorNormalize( vecForce );
		pGib->ApplyAbsVelocityImpulse( vecForce * 100.0f );
#endif
		pGib->SetThink( &CBaseEntity::SUB_FadeOut );
		pGib->SetNextThink( gpGlobals->curtime + 10.0f );
#else
		CGib *pGib = CREATE_ENTITY( CGib, "gib" );
		pGib->Spawn( gibs[i] );
		pGib->SetAbsAngles( GetAbsAngles() );
		pGib->InitGib( this, 10.0, 15.0 );
		pGib->SetBloodColor( DONT_BLEED );
		pGib->m_lifeTime = 10.0f;

		Vector parentOrigin;
		QAngle dummy;
		pGib->GetAttachmentLocal( pGib->LookupAttachment( "placementOrigin" ), parentOrigin, dummy );
//		pGib->SetAbsOrigin( GetAbsOrigin() - parentOrigin );
		Vector origin = GetAbsOrigin() - parentOrigin;
		pGib->Teleport( &origin, NULL, NULL );

		pGib->SetGravity( UTIL_ScaleForGravity( 250 ) );

		Vector vecForce = info.GetDamageForce();
		VectorNormalize( vecForce );
		pGib->ApplyAbsVelocityImpulse( vecForce * 100.0f );

		pGib->SetThink( &CGib::DieThink );
		pGib->SetNextThink( gpGlobals->curtime + pGib->m_lifeTime );
#endif
	}

	BaseClass::Event_Killed( info );
}

AI_BEGIN_CUSTOM_NPC( npc_spx_flyer, CNPC_SpxFlyer )

	DECLARE_ACTIVITY( ACT_SPXFLYER_FLY_FORWARD )
	DECLARE_ACTIVITY( ACT_SPXFLYER_FLY_BACKWARD )
	DECLARE_ACTIVITY( ACT_SPXFLYER_FLY_LEFT )
	DECLARE_ACTIVITY( ACT_SPXFLYER_FLY_RIGHT )
	DECLARE_ACTIVITY( ACT_SPXFLYER_FLY_UP )
	DECLARE_ACTIVITY( ACT_SPXFLYER_FLY_DOWN )
	DECLARE_ACTIVITY( ACT_SPXFLYER_HIT )

	DECLARE_ANIMEVENT( AE_SPXFLYER_SPIT )
	DECLARE_ANIMEVENT( AE_SPXFLYER_CLAW_LEFT )
	DECLARE_ANIMEVENT( AE_SPXFLYER_CLAW_RIGHT )
	DECLARE_ANIMEVENT( AE_SPXFLYER_CLAW_LEFT_DOWN )
	DECLARE_ANIMEVENT( AE_SPXFLYER_CLAW_RIGHT_DOWN )

	//=========================================================
	// BaseNPC behaviour will stop in the middle of a swing if the
	// zombie takes damage (among other things).
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_SPXFLYER_MELEE_ATTACK1,

		"	Tasks"
		"		TASK_STOP_MOVING		0"
		"		TASK_FACE_ENEMY			0"
		"		TASK_ANNOUNCE_ATTACK	1"	// 1 = primary attack
		"		TASK_MELEE_ATTACK1		0"
//		"		TASK_SET_SCHEDULE		SCHEDULE:SCHED_ZOMBIE_POST_MELEE_WAIT"
//		"		TASK_SET_SCHEDULE		SCHEDULE:SCHED_BACK_AWAY_FROM_ENEMY"
		"		TASK_SET_SCHEDULE		SCHEDULE:SCHED_RUN_RANDOM"
		""
		"	Interrupts"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
	)

	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_SPXFLYER_MELEE_ATTACK2,

		"	Tasks"
		"		TASK_STOP_MOVING		0"
		"		TASK_FACE_ENEMY			0"
		"		TASK_ANNOUNCE_ATTACK	2"	// 2 = secondary attack
		"		TASK_MELEE_ATTACK2		0"
//		"		TASK_SET_SCHEDULE		SCHEDULE:SCHED_BACK_AWAY_FROM_ENEMY"
		"		TASK_SET_SCHEDULE		SCHEDULE:SCHED_RUN_RANDOM"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
//		"		COND_LIGHT_DAMAGE"
//		"		COND_HEAVY_DAMAGE"
		"		COND_ENEMY_OCCLUDED"
	)

	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_SPXFLYER_DIVEBOMB,

		"	Tasks"
		"		TASK_STOP_MOVING				0"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_CHASE_ENEMY_FAILED"
		"		TASK_GET_CHASE_PATH_TO_ENEMY	300"
//		"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_SPXFLYER_HIT"
		"		TASK_RUN_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		"		TASK_FACE_ENEMY					0"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_ENEMY_UNREACHABLE"
//		"		COND_CAN_RANGE_ATTACK1"
		"		COND_CAN_MELEE_ATTACK1"
//		"		COND_CAN_RANGE_ATTACK2"
		"		COND_CAN_MELEE_ATTACK2"
		"		COND_TOO_CLOSE_TO_ATTACK"
		"		COND_TASK_FAILED"
		"		COND_LOST_ENEMY"
//		"		COND_BETTER_WEAPON_AVAILABLE"
		"		COND_HEAR_DANGER"
	)

AI_END_CUSTOM_NPC()
