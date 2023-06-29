#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "items.h"
#include "ammodef.h"
#include "eventlist.h"
#include "npcevent.h"
#include "animation.h"
#include "collisionutils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CItem_MedCrate : public CBaseAnimating
{
public:
	DECLARE_CLASS( CItem_MedCrate, CBaseAnimating );

	void	Spawn( void );
	void	Precache( void );
	bool	CreateVPhysics( void );

	virtual void HandleAnimEvent( animevent_t *pEvent );

	void	SetupCrate( void );
	void	OnRestore( void );

	void	DisplayContents( void );

	int		ObjectCaps( void );
	void	EmitPlayerUseSound( void ) { /* nothing */ }
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

//	void	InputKill( inputdata_t &data );
	void	CrateThink( void );
	
	virtual int OnTakeDamage( const CTakeDamageInfo &info );

protected:

	bool	HitboxWorldBounds( const char *szHitboxGroup, int nHitbox, Vector &vecOrigin, Vector &vecMin, Vector &vecMax );
	void	HitboxWorldBounds( mstudiobbox_t *pBox, Vector &vecOrigin, Vector &vecMin, Vector &vecMax );

//	int						m_iHealthRemaining; // 0-100, health points to dispense
	float					m_flCloseTime;
	COutputEvent			m_OnUsed;
	CHandle< CBasePlayer >	m_hActivator;

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS( item_med_crate, CItem_MedCrate );

BEGIN_DATADESC( CItem_MedCrate )

//	DEFINE_KEYFIELD( m_iHealthRemaining, FIELD_INTEGER, "health" ),

	DEFINE_FIELD( m_flCloseTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_hActivator, FIELD_EHANDLE ),

	DEFINE_OUTPUT( m_OnUsed, "OnUsed" ),

//	DEFINE_INPUTFUNC( FIELD_VOID, "Kill", InputKill ),

	DEFINE_THINKFUNC( CrateThink ),

END_DATADESC()

#define	MEDCRATE_CLOSE_DELAY 1.5f
#define	MEDCRATE_MAX_HEALTH 100

enum {
	MEDCRATE_BODYGROUP_CRATE = 0,
	MEDCRATE_BODYGROUP_CONTENTS
};

enum {
	MEDCRATE_CONTENTS_FULL = 0,
	MEDCRATE_CONTENTS_90,
	MEDCRATE_CONTENTS_80,
	MEDCRATE_CONTENTS_70,
	MEDCRATE_CONTENTS_60,
	MEDCRATE_CONTENTS_50,
	MEDCRATE_CONTENTS_40,
	MEDCRATE_CONTENTS_30,
	MEDCRATE_CONTENTS_20,
	MEDCRATE_CONTENTS_10,
	MEDCRATE_CONTENTS_1,
	MEDCRATE_CONTENTS_EMPTY
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItem_MedCrate::Spawn( void )
{
	Precache();

	BaseClass::Spawn();

	SetModel( STRING( GetModelName() ) );
	SetMoveType( MOVETYPE_NONE );
	SetSolid( SOLID_VPHYSICS );
	CreateVPhysics();

	m_iHealth = clamp( m_iHealth, 0, MEDCRATE_MAX_HEALTH );
	SetupCrate();

	ResetSequence( LookupSequence( "Idle" ) );

	// Hide contents when closed for rendering performance
	SetBodygroup( MEDCRATE_BODYGROUP_CONTENTS, MEDCRATE_CONTENTS_EMPTY );

	m_flCloseTime = gpGlobals->curtime;
	m_flAnimTime = gpGlobals->curtime;
	m_flPlaybackRate = 0.0;
	SetCycle( 0 );

	m_takedamage = DAMAGE_EVENTS_ONLY;

}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
bool CItem_MedCrate::CreateVPhysics( void )
{
	return ( VPhysicsInitStatic() != NULL );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItem_MedCrate::Precache( void )
{
	if ( GetModelName() == NULL_STRING )
		SetModelName( AllocPooledString( "models/medkit/w_medcrate.mdl" ) );
	PrecacheModel( STRING( GetModelName() ) );
#if 0
	PrecacheScriptSound( "AmmoCrate.Open" );
	PrecacheScriptSound( "AmmoCrate.Close" );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItem_MedCrate::SetupCrate( void )
{
//	SetModelName( AllocPooledString( m_lpzModelNames[m_nAmmoType] ) );
	
//	m_nAmmoIndex = GetAmmoDef()->Index( m_lpzAmmoNames[m_nAmmoType] );
}

//-----------------------------------------------------------------------------
void CItem_MedCrate::DisplayContents( void )
{
	int iContents;

	float flHealthPercent = GetHealth() / (float)MEDCRATE_MAX_HEALTH;

	/* There is some sort of bug with the release build such that x/x is not >= 1.0.
	   If you DevMsg("%f", flHealthPercent) then it works.  The debug build is fine.
	if ( flHealthPercent >= 1.0 ) */
	if ( GetHealth() >= MEDCRATE_MAX_HEALTH )
		iContents = MEDCRATE_CONTENTS_FULL;
	else if ( flHealthPercent >= 0.90 )
		iContents = MEDCRATE_CONTENTS_90;
	else if ( flHealthPercent >= 0.80 )
		iContents = MEDCRATE_CONTENTS_80;
	else if ( flHealthPercent >= 0.70 )
		iContents = MEDCRATE_CONTENTS_70;
	else if ( flHealthPercent >= 0.60 )
		iContents = MEDCRATE_CONTENTS_60;
	else if ( flHealthPercent >= 0.50 )
		iContents = MEDCRATE_CONTENTS_50;
	else if ( flHealthPercent >= 0.40 )
		iContents = MEDCRATE_CONTENTS_40;
	else if ( flHealthPercent >= 0.30 )
		iContents = MEDCRATE_CONTENTS_30;
	else if ( flHealthPercent >= 0.20 )
		iContents = MEDCRATE_CONTENTS_20;
	else if ( flHealthPercent >= 0.10 )
		iContents = MEDCRATE_CONTENTS_10;
	else if ( flHealthPercent > 0.0 )
		iContents = MEDCRATE_CONTENTS_1;
	else
		iContents = MEDCRATE_CONTENTS_EMPTY;

	SetBodygroup( MEDCRATE_BODYGROUP_CONTENTS, iContents );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItem_MedCrate::OnRestore( void )
{
	BaseClass::OnRestore();

	// Restore our internal state
	SetupCrate();
}

//-----------------------------------------------------------------------------
int CItem_MedCrate::ObjectCaps( void )
{
	int caps = BaseClass::ObjectCaps();

	//FIXME: May not want to have this used in a radius
	if ( GetHealth() > 0 )
		caps |= (FCAP_IMPULSE_USE|FCAP_USE_IN_RADIUS);

	return caps;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pActivator - 
//			*pCaller - 
//			useType - 
//			value - 
//-----------------------------------------------------------------------------
void CItem_MedCrate::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBasePlayer *pPlayer = ToBasePlayer( pActivator );

	if ( pPlayer == NULL )
		return;

	if ( GetHealth() <= 0 )
		return; // m_OnUsedEmpty.FireOutput( pActivator, this );

	m_OnUsed.FireOutput( pActivator, this );

	int iSequence = LookupSequence( "Open" );

	// See if we're not opening already
	if ( GetSequence() != iSequence )
	{
#if 1
#if 1
		Vector vecPosition, min, max ;
		if ( !HitboxWorldBounds( "trace", 0, vecPosition, min, max ) )
			return;
#else
		CStudioHdr *pStudioHdr = GetModelPtr();
		if (!pStudioHdr)
			return;
		int iHitboxSet = FindHitboxSetByName( pStudioHdr, "trace" );
		mstudiohitboxset_t *set = pStudioHdr->pHitboxSet( iHitboxSet );
		if ( !set || !set->numhitboxes )
			return;

		mstudiobbox_t *pbox = set->pHitbox( 0 );

		matrix3x4_t bonetoworld;
		GetBoneTransform( pbox->bone, bonetoworld );

		Vector vecPosition;
		QAngle vecAngles;
		MatrixAngles( bonetoworld, vecAngles, vecPosition );

		Vector min, max ;
		TransformAABB( bonetoworld, pbox->bbmin, pbox->bbmax, min, max );

		min -= vecPosition;
		max -= vecPosition;
#endif
//		NDebugOverlay::Box( vecPosition, min, max, 0, 255, 0, true, 2.0f );

		trace_t tr;
		CTraceFilterSkipTwoEntities traceFilter( pPlayer, this, COLLISION_GROUP_NONE );
		UTIL_TraceHull( vecPosition, vecPosition, min, max, MASK_SOLID, &traceFilter, &tr );

#else
		Vector mins, maxs;
		trace_t tr;

		CollisionProp()->WorldSpaceAABB( &mins, &maxs );

		Vector vOrigin = GetAbsOrigin();
		vOrigin.z += ( maxs.z - mins.z );
		mins = (mins - GetAbsOrigin()) * 0.2f;
		maxs = (maxs - GetAbsOrigin()) * 0.2f;
		mins.z = ( GetAbsOrigin().z - vOrigin.z );  

		UTIL_TraceHull( vOrigin, vOrigin, mins, maxs, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );
#endif		

		if ( tr.startsolid || tr.allsolid )
			 return;
			
		m_hActivator = pPlayer;

		DisplayContents();

		// Animate!
		ResetSequence( iSequence );
#if 0
		// Make sound
		CPASAttenuationFilter sndFilter( this, "AmmoCrate.Open" );
		EmitSound( sndFilter, entindex(), "AmmoCrate.Open" );
#endif
		// Start thinking to make it return
		SetThink( &CItem_MedCrate::CrateThink );
		SetNextThink( gpGlobals->curtime + 0.1f );
	}

	// Don't close again for N seconds
	m_flCloseTime = gpGlobals->curtime + MEDCRATE_CLOSE_DELAY;
}

//-----------------------------------------------------------------------------
// Purpose: allows the crate to open up when hit by a crowbar
//-----------------------------------------------------------------------------
int CItem_MedCrate::OnTakeDamage( const CTakeDamageInfo &info )
{
#if 0
	// if it's the player hitting us with a crowbar, open up
	CBasePlayer *player = ToBasePlayer(info.GetAttacker());
	if (player)
	{
		CBaseCombatWeapon *weapon = player->GetActiveWeapon();

		if (weapon && !stricmp(weapon->GetName(), "weapon_crowbar"))
		{
			// play the normal use sound
			player->EmitSound( "HL2Player.Use" );
			// open the crate
			Use(info.GetAttacker(), info.GetAttacker(), USE_TOGGLE, 0.0f);
		}
	}
#endif
	// don't actually take any damage
	return 0;
}


//-----------------------------------------------------------------------------
// Purpose: Catches the monster-specific messages that occur when tagged
//			animation frames are played.
// Input  : *pEvent - 
//-----------------------------------------------------------------------------
void CItem_MedCrate::HandleAnimEvent( animevent_t *pEvent )
{
	if ( pEvent->event == AE_AMMOCRATE_PICKUP_AMMO )
	{
		if ( m_hActivator && ( GetHealth() > 0 ) )
		{
			CBasePlayer *pPlayer = m_hActivator;

			// See if the player is still within use distance
//			if ( GetAbsOrigin().DistTo( pPlayer->EyePosition() ) > PLAYER_USE_RADIUS * 1. )
			if ( pPlayer->FindUseEntity() != this )
				return;

			int iPlayerHealth = pPlayer->GetHealth();
			if ( pPlayer->TakeHealth( GetHealth(), DMG_GENERIC ) )
			{
				CPASAttenuationFilter filter( pPlayer, "HealthKit.Touch" );
				EmitSound( filter, pPlayer->entindex(), "HealthKit.Touch" );

				int iHealthTaken = pPlayer->GetHealth() - iPlayerHealth;
				m_iHealth -= iHealthTaken;
				m_iHealth = clamp( GetHealth(), 0, MEDCRATE_MAX_HEALTH );

				DisplayContents();
				return;
			}

			m_hActivator = NULL;
		}
		return;
	}
	BaseClass::HandleAnimEvent( pEvent );
}

	
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItem_MedCrate::CrateThink( void )
{
	StudioFrameAdvance();
	DispatchAnimEvents( this );

	SetNextThink( gpGlobals->curtime + 0.1f );

	// Start closing if we're not already
	if ( GetSequence() != LookupSequence( "Close" ) )
	{
		// Not ready to close?
		if ( m_flCloseTime <= gpGlobals->curtime )
		{
			m_hActivator = NULL;

			ResetSequence( LookupSequence( "Close" ) );
		}
	}
	else
	{
		// See if we're fully closed
		if ( IsSequenceFinished() )
		{
			// Stop thinking
			SetThink( NULL );
#if 0
			CPASAttenuationFilter sndFilter( this, "AmmoCrate.Close" );
			EmitSound( sndFilter, entindex(), "AmmoCrate.Close" );
#endif
			// FIXME: We're resetting the sequence here
			// but setting Think to NULL will cause this to never have
			// StudioFrameAdvance called. What are the consequences of that?
			if ( GetHealth() <= 0 )
				ResetSequence( LookupSequence( "Empty" ) );
			else
				ResetSequence( LookupSequence( "Idle" ) );
			
			// Hide contents when closed for rendering performance
			SetBodygroup( MEDCRATE_BODYGROUP_CONTENTS, MEDCRATE_CONTENTS_EMPTY );
		}
	}
}

#if 0 // not sure why item_ammo_crate had this, since CBaseEntity has it
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
void CItem_MedCrate::InputKill( inputdata_t &data )
{
	UTIL_Remove( this );
}
#endif

//-----------------------------------------------------------------------------
bool CItem_MedCrate::HitboxWorldBounds( const char *szHitboxGroup, int nHitbox, Vector &vecOrigin, Vector &vecMin, Vector &vecMax )
{
	CStudioHdr *pStudioHdr = GetModelPtr();
	if (!pStudioHdr)
		return false;

	int iHitboxSet = FindHitboxSetByName( pStudioHdr, "trace" );
	mstudiohitboxset_t *set = pStudioHdr->pHitboxSet( iHitboxSet );
	if ( !set || ( nHitbox < 0 ) || ( nHitbox >= set->numhitboxes ) )
		return false;

	mstudiobbox_t *pBox = set->pHitbox( nHitbox );
	HitboxWorldBounds( pBox, vecOrigin, vecMin, vecMax );
	return true;
}

//-----------------------------------------------------------------------------
void CItem_MedCrate::HitboxWorldBounds( mstudiobbox_t *pBox, Vector &vecOrigin, Vector &vecMin, Vector &vecMax )
{
	matrix3x4_t boneToWorld;
	GetBoneTransform( pBox->bone, boneToWorld );

	QAngle vecAngles;
	MatrixAngles( boneToWorld, vecAngles, vecOrigin );

	Vector min, max ;
	TransformAABB( boneToWorld, pBox->bbmin, pBox->bbmax, vecMin, vecMax );

	vecMin -= vecOrigin;
	vecMax -= vecOrigin;
}

