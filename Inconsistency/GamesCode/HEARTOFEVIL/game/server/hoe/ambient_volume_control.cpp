// This entity isn't used. See name2.vmf.

#include "cbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar avc_debug( "avc_debug", "1", FCVAR_NONE, "Debug ambient_volume_control entities." );

#define AVC_UPDATE_RATE 0.1f

class CAmbientVolumeControl : public CPointEntity
{
public:
	DECLARE_CLASS( CAmbientVolumeControl, CPointEntity );
	DECLARE_DATADESC();

	CAmbientVolumeControl();
	~CAmbientVolumeControl();

	void Spawn( void );
	void Update( void );
	void UpdatePlayersInPVS( void );
	bool InRangeOfPlayer( CBasePlayer *pPlayer, float *flRange = 0 );
	void UpdateVolume( float flRange );
	void SetVolume( float flVolume );

	bool IsEnabled( void ) const;
	void Disable( void );
	void Enable( void );

	void InputEnable( inputdata_t &inputdata );
	void InputDisable( inputdata_t &inputdata );
	void InputToggleEnabled( inputdata_t &inputdata );

	float m_flMinVolume; // Volume when player is at "radius" distance
	float m_flMaxVolume; // Volume when player is at 0 distance
	float m_flRadius;
	float m_flFOV;
	bool m_bDisabled;
	float m_flLastUpdateFrac;
	float m_flLastVolume;

#define MAX_AMBIENT_GENERIC 5
	string_t m_iszAmbientGeneric[MAX_AMBIENT_GENERIC];
	EHANDLE m_hAmbientGeneric[MAX_AMBIENT_GENERIC];

	// Updated every couple seconds. Then we check against these players often.
	CUtlVector<CBasePlayerHandle> m_hPlayersInPVS;

	// Next time to update the m_hPlayersInPVS list.
	float m_flNextUpdatePlayersInPVS;
};

class CAVCSystem
{
public:
	DECLARE_CLASS_NOBASE( CAVCSystem );

	void AddAVC( CAmbientVolumeControl *pAVC );
	void RemoveAVC( CAmbientVolumeControl *pAVC );

	CUtlVector<CAmbientVolumeControl *>	m_avcEntities;
};

LINK_ENTITY_TO_CLASS( ambient_volume_control, CAmbientVolumeControl );

BEGIN_DATADESC( CAmbientVolumeControl )

	DEFINE_FIELD( m_flLastUpdateFrac, FIELD_FLOAT ),
	DEFINE_FIELD( m_flLastVolume, FIELD_FLOAT ),

	DEFINE_KEYFIELD( m_bDisabled, FIELD_BOOLEAN, "StartDisabled" ),
	DEFINE_KEYFIELD( m_flMinVolume, FIELD_FLOAT, "minvolume" ),
	DEFINE_KEYFIELD( m_flMaxVolume, FIELD_FLOAT, "maxvolume" ),
	DEFINE_KEYFIELD( m_flRadius, FIELD_FLOAT, "radius" ),
	DEFINE_KEYFIELD( m_flFOV, FIELD_FLOAT, "FOV" ),

	DEFINE_KEYFIELD( m_iszAmbientGeneric[0], FIELD_STRING, "ambient01" ),
	DEFINE_KEYFIELD( m_iszAmbientGeneric[1], FIELD_STRING, "ambient02" ),
	DEFINE_KEYFIELD( m_iszAmbientGeneric[2], FIELD_STRING, "ambient03" ),
	DEFINE_KEYFIELD( m_iszAmbientGeneric[3], FIELD_STRING, "ambient04" ),
	DEFINE_KEYFIELD( m_iszAmbientGeneric[4], FIELD_STRING, "ambient05" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "ToggleEnabled", InputToggleEnabled ),

	DEFINE_THINKFUNC( Update )

END_DATADESC()

CAmbientVolumeControl::CAmbientVolumeControl()
{
	m_bDisabled = false;
	m_flLastUpdateFrac = -1.0f;
	m_flLastVolume = -1.0f;
	m_flNextUpdatePlayersInPVS = 0;
	m_flFOV = -1;
//	g_AVCSystem.AddAVC( this );
}

CAmbientVolumeControl::~CAmbientVolumeControl()
{
//	g_AVCSystem.RemoveAVC( this );
}

void CAmbientVolumeControl::Spawn( void )
{
	if ( !m_bDisabled )
	{
		SetThink( &CAmbientVolumeControl::Update );
		SetNextThink( gpGlobals->curtime );
	}
}

void CAmbientVolumeControl::Update( void )
{
	Assert( IsEnabled() );
	if ( !IsEnabled() )
		return;

	UpdatePlayersInPVS();

	if ( m_hPlayersInPVS.Count() > 0 )
		SetNextThink( gpGlobals->curtime + AVC_UPDATE_RATE );
	else
		SetNextThink( m_flNextUpdatePlayersInPVS );

	for ( int i=0; i < m_hPlayersInPVS.Count(); i++ )
	{
		CBasePlayer *pPlayer = m_hPlayersInPVS[i];
		if ( !pPlayer )
		{
			if ( m_flLastUpdateFrac != -1.0f )
			{
				SetVolume( m_flLastUpdateFrac <= 0.5 ? m_flMaxVolume : m_flMinVolume );
				m_flLastUpdateFrac = -1.0f;
				m_flLastVolume = -1.0f;
			}
			continue;
		}

		float flRange;
		if ( !InRangeOfPlayer( pPlayer, &flRange ) )
		{
			if ( m_flLastUpdateFrac != -1.0f )
			{
				SetVolume( m_flLastUpdateFrac <= 0.5 ? m_flMaxVolume : m_flMinVolume );
				m_flLastUpdateFrac = -1.0f;
				m_flLastVolume = -1.0f;
			}
			continue;
		}

		UpdateVolume( flRange );

		if ( avc_debug.GetBool() )
		{
			NDebugOverlay::Line( GetAbsOrigin(), pPlayer->WorldSpaceCenter(), 0, 255, 0, true, 0.1 );
		}
	} 
}

void CAmbientVolumeControl::UpdatePlayersInPVS( void )
{
	// Only update players in PVS every 2 seconds.
	if ( gpGlobals->curtime < m_flNextUpdatePlayersInPVS )
		return;

	m_flNextUpdatePlayersInPVS = gpGlobals->curtime + 2.0f;

	// Find the players in our PVS.
	unsigned char myPVS[16 * 1024];
	int pvsLen = engine->GetPVSForCluster( engine->GetClusterForOrigin( GetAbsOrigin() ), sizeof( myPVS ), myPVS );

	m_hPlayersInPVS.Purge();
	for ( int i=1; i <= gpGlobals->maxClients; i++ )
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );

		if ( !pPlayer )
			continue;

		Vector vecWorldMins, vecWorldMaxs;
		pPlayer->CollisionProp()->WorldSpaceAABB( &vecWorldMins, &vecWorldMaxs );
		if ( engine->CheckBoxInPVS( vecWorldMins, vecWorldMaxs, myPVS, pvsLen ) )
		{
			m_hPlayersInPVS.AddToTail( pPlayer );
		}
	}
}

bool CAmbientVolumeControl::InRangeOfPlayer( CBasePlayer *pTarget, float *flRange ) 
{
	Vector vecSpot1 = EarPosition();
	Vector vecSpot2 = pTarget->EarPosition();

	// Check if facing player
	if ( m_flFOV != -1 )
	{
		Vector los = vecSpot2 - vecSpot1;
		los.z = 0;
		VectorNormalize( los );
		Vector facingDir;
		AngleVectors( GetAbsAngles(), &facingDir );
		float flDot = DotProduct( los, facingDir );
		if ( flDot < m_flFOV )
			return false;
	}

	// calc range to player
	Vector vecRange = vecSpot2 - vecSpot1;
	float range = vecRange.Length();
	if ( m_flRadius > range || m_flRadius == -1 )
	{
		trace_t tr;

		UTIL_TraceLine( vecSpot1, vecSpot2, MASK_SOLID_BRUSHONLY|MASK_WATER, pTarget, COLLISION_GROUP_NONE, &tr );

		if ( tr.fraction == 1 && !tr.startsolid )
		{
			if ( flRange != NULL )
				*flRange = vecRange.Length2D();
			return true;
		}
	}

	return false;
}

void CAmbientVolumeControl::UpdateVolume( float flRange )
{
	// Calculate player's distance from this entity as range from 0.0 (closest) to 1.0 (farthest)
	float flDistFrac = flRange / m_flRadius;
	flDistFrac = clamp(flDistFrac, 0.0f, 1.0f);

	// Calculate volume at the calculated distance as fraction from 0-1
	float flVolume = flDistFrac * m_flMinVolume + (1.0f - flDistFrac) * m_flMaxVolume;
	
	SetVolume( flVolume );

	m_flLastUpdateFrac = flDistFrac;
}

void CAmbientVolumeControl::SetVolume( float flVolume )
{
	if ( flVolume == m_flLastVolume )
		return;

	if ( avc_debug.GetBool() ) DevMsg("avc volume %.2f\n", flVolume);

	for ( int i = 0; i < MAX_AMBIENT_GENERIC; i++ )
	{
		if ( m_iszAmbientGeneric[i] == NULL_STRING )
			continue;

		CBaseEntity *pAG = (CBaseEntity *) gEntList.FindEntityByName( NULL, m_iszAmbientGeneric[i] );
		if ( pAG == NULL )
			continue;

		variant_t value;
		value.SetFloat( flVolume * 10 );
		pAG->AcceptInput( "VolumeIfPlaying", this, this, value, USE_TOGGLE );
	}

	m_flLastVolume = flVolume;
}

void CAmbientVolumeControl::InputEnable( inputdata_t &inputdata )
{
	if ( !IsEnabled() )
	{
		Enable();
	}
}

void CAmbientVolumeControl::InputDisable( inputdata_t &inputdata )
{
	if ( IsEnabled() )
	{
		Disable();
	}
}

void CAmbientVolumeControl::InputToggleEnabled( inputdata_t &inputdata )
{
	if ( IsEnabled() )
	{
		Disable();
	}
	else
	{
		Enable();
	}
}

//-----------------------------------------------------------------------------
bool CAmbientVolumeControl::IsEnabled( void ) const
{
	return !m_bDisabled;
}

//-----------------------------------------------------------------------------
void CAmbientVolumeControl::Disable( void )
{
	m_bDisabled = true;

	SetThink( NULL );
}


//-----------------------------------------------------------------------------
void CAmbientVolumeControl::Enable( void )
{
	m_bDisabled = false;

	SetThink( &CAmbientVolumeControl::Update );
	SetNextThink( gpGlobals->curtime );
}