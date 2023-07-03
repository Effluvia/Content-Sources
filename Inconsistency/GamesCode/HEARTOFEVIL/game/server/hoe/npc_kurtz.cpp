#include "cbase.h"
#include "npcevent.h"
#include "gib.h"
#include "hoe_human.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

enum {
	KURTZ_BODYGROUP_HEAD = 0,
	KURTZ_BODYGROUP_BODY
};

#define BARNEY_HEAD_MDL "models/barneyzombie/head/barneyzombiehead.mdl"

ConVar hoe_kurtz_head_velocity( "hoe_kurtz_head_velocity", "0 -150 100" );

class CNPC_Kurtz : public CHOEHuman
{
public:
	DECLARE_CLASS( CNPC_Kurtz, CHOEHuman );
	DEFINE_CUSTOM_AI;

	void Spawn( void );
	void Precache( void );
	void SelectModelGroups( void );

	Class_T Classify( void );
	bool ClassifyPlayerAlly( void ) { return false; }
	bool ClassifyPlayerAllyVital( void ) { return false; }

	void HandleAnimEvent( animevent_t *pEvent );
#if 0
	bool HandleInteraction( int interactionType, void *data, CBaseCombatCharacter* sourceEnt );
#endif
	void Event_Killed( const CTakeDamageInfo &info );

#if 1
	void SelectModel();

	int GetHeadGroupNum( void ) { return KURTZ_BODYGROUP_HEAD; }
	int GetNumHeads( void ) { return 2; }
	const char *GetHeadModelName( void ) { return "models/kurtzhead.mdl"; }

	virtual SpeechManagerID_t GetSpeechManagerID( void ) { return SPEECH_MANAGER_ALLY; }
	const char *GetSentenceGroup( void ) const { return "KURTZ_SENTENCE_GROUP"; }
	const char **GetFriendClasses( void ) const {
		static const char *szClasses[] = {
			NULL
		};
		return szClasses;
	};
	const char **GetWeaponClasses( void ) const {
		static const char *szClasses[] = {
			NULL
		};
		return szClasses;
	};

	const char *GetCCImageNameCStr( void )
	{
		return UTIL_VarArgs( "cc_kurtz%d", m_nHeadNum + 1 );
	}

	bool BecomeRagdollOnClient( const Vector &force )
	{
		bool bResult = BaseClass::BecomeRagdollOnClient( force );
		if ( m_hHead != NULL )
		{
			Vector velocity;
			UTIL_StringToVector( velocity.Base(), hoe_kurtz_head_velocity.GetString() );
			m_hHead->ApplyAbsVelocityImpulse( velocity );
		}
		return bResult;
	}

#else
	bool GetHeadPosition( Vector &origin, QAngle &angles );
	void Behead( void );
#endif
	static Animevent CNPC_Kurtz::AE_KURTZ_THROWHEAD;
};

LINK_ENTITY_TO_CLASS( npc_kurtz, CNPC_Kurtz );

//-----------------------------------------------------------------------------
// Spawnflags
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Animation events.
//-----------------------------------------------------------------------------
Animevent CNPC_Kurtz::AE_KURTZ_THROWHEAD;

//-----------------------------------------------------------------------------
// Tasks
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Conditions 
//-----------------------------------------------------------------------------

//BEGIN_DATADESC( CNPC_Kurtz )
//END_DATADESC()

// NOTE: This has to be high enough so Kurtz isn't killed by a machete chop to
// the head so CWeaponMachete::OnHitEntity can properly chop off his head.
#define KURTZ_HEALTH 60 // was gSkillData.hgruntHealth in HL1

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Kurtz::Spawn( void )
{
#if 1
	m_spawnEquipment = NULL_STRING;

	m_iHealth = KURTZ_HEALTH;

	BaseClass::Spawn();

	CapabilitiesRemove(
		bits_CAP_USE_WEAPONS |
		bits_CAP_AIM_GUN |
		bits_CAP_SQUAD | bits_CAP_NO_HIT_SQUADMATES |
		bits_CAP_USE_SHOT_REGULATOR );
#else
	BaseClass::Spawn();

	Precache();
	SetModel( STRING( GetModelName() ) );
	SelectModelGroups();

	SetHullSizeNormal();

	SetSolid( SOLID_BBOX );
	SetMoveType( MOVETYPE_STEP );

	CapabilitiesClear();
	CapabilitiesAdd( bits_CAP_MOVE_GROUND );

	SetBloodColor( BLOOD_COLOR_RED );
	m_NPCState = NPC_STATE_NONE;

	m_flFieldOfView = 0.5;
	SetViewOffset( VEC_VIEW );		// Position of the eyes relative to NPC's origin.

	m_iHealth = KURTZ_HEALTH;

	NPCInit();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Precaches all resources this monster needs.
//-----------------------------------------------------------------------------
void CNPC_Kurtz::Precache( void )
{
#if 1
	PrecacheScriptSound( "NPC_Barney.FootstepLeft" );
	PrecacheScriptSound( "NPC_Barney.FootstepRight" );

	// Because he throws the head
	PrecacheModel( BARNEY_HEAD_MDL );

	BaseClass::Precache();
#else
	SetModelName( AllocPooledString( "models/kurtz/kurtz.mdl" ) );
	PrecacheModel( STRING( GetModelName() ) );

	PrecacheModel( "models/kurtzhead.mdl" );

	PrecacheScriptSound( "NPC_Barney.FootstepLeft" );
	PrecacheScriptSound( "NPC_Barney.FootstepRight" );

	BaseClass::Precache();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Indicates this monster's place in the relationship table.
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_Kurtz::Classify( void )
{
	return CLASS_KURTZ; 
}

#if 1
//-----------------------------------------------------------------------------
void CNPC_Kurtz::SelectModel()
{
	SetModelName( AllocPooledString( "models/kurtz/kurtz.mdl" ) );
}
#endif

//-----------------------------------------------------------------------------
void CNPC_Kurtz::SelectModelGroups( void )
{
	switch ( m_nBody )
	{
	case 0:
		SetBodygroup( KURTZ_BODYGROUP_HEAD, 0 ); // Bald
		SetBodygroup( KURTZ_BODYGROUP_BODY, 1 ); // Pajamas
		m_nHeadNum = 0;
		break;
	case 1:
		SetBodygroup( KURTZ_BODYGROUP_HEAD, 1 ); // Beret
		SetBodygroup( KURTZ_BODYGROUP_BODY, 0 ); // Uniform
		m_nHeadNum = 1;
		break;
	}
}

//-----------------------------------------------------------------------------
ConVar sk_kurtz_barneyhead_velocity("sk_kurtz_barneyhead_velocity", "120 200");
void CNPC_Kurtz::HandleAnimEvent( animevent_t *pEvent )
{
	if ( pEvent->event == AE_KURTZ_THROWHEAD )
	{
		CGib *pGib = CREATE_ENTITY( CGib, "gib" );
		pGib->Spawn( BARNEY_HEAD_MDL );
		pGib->InitGib( this, 1.0, 1.0 );
		
		pGib->SetThink( &CBaseEntity::SUB_Remove );
		pGib->SetNextThink( gpGlobals->curtime + 4.0f );

		pGib->AddSolidFlags( FSOLID_NOT_SOLID );

		Vector origin;
		QAngle angles;
		GetAttachment( LookupAttachment( "righthand" ), origin, angles );
		pGib->SetAbsOrigin( origin );

		Vector forward, right, up;
		AngleVectors( GetAbsAngles(), &forward, &right, &up );
		float dy = 120, dz = 200;
		sscanf(sk_kurtz_barneyhead_velocity.GetString(), "%g %g", &dy, &dz);
		Vector velocity = forward * dy + up * dz;
		pGib->SetAbsAngles( QAngle( 90, GetAbsAngles().y, 0 ) );
		pGib->SetAbsVelocity( velocity );

		return;
	}

	switch( pEvent->event )
	{
	case AE_NPC_LEFTFOOT:
	case AE_NPC_RIGHTFOOT:
		{
			EmitSound(pEvent->options);
		}
		break;
	default:
		BaseClass::HandleAnimEvent( pEvent );
		break;
	}
}

#if 0
//---------------------------------------------------------
bool CNPC_Kurtz::HandleInteraction( int interactionType, void *data, CBaseCombatCharacter* sourceEnt )
{
	extern int g_interactionMacheteHeadChop;
	if ( interactionType == g_interactionMacheteHeadChop )
	{
		if ( GetBodygroup( KURTZ_BODYGROUP_HEAD ) < 2 )
		{
			Behead();
			TakeDamage( CTakeDamageInfo( sourceEnt, sourceEnt, GetHealth(), DMG_SLASH ) );
		}
		return true;
	}

	return false;
}
#endif

//-----------------------------------------------------------------------------
void CNPC_Kurtz::Event_Killed( const CTakeDamageInfo &info )
{
	BaseClass::Event_Killed( info );

	SentenceStop(); // Stops "the horror"; scripted_sentence doesn't have an StopSentence input?
}

#if 0
//-----------------------------------------------------------------------------
bool CNPC_Kurtz::GetHeadPosition( Vector &origin, QAngle &angles )
{
	int attachment = LookupAttachment( "head" );
	if ( attachment != -1 )
	{
		GetAttachment( attachment, origin, angles );
		origin.z += 12.0; // jump up to avoid spawning inside our body
		// Also note the origin of the head model is at its center, which
		// doesn't agree with the attachment point which is at the base of the neck
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void CNPC_Kurtz::Behead( void )
{
	CGib *pGib = CREATE_ENTITY( CGib, "gib" );
	pGib->Spawn( "models/kurtzhead.mdl" );
	pGib->m_nBody = 1 - m_nBody;
	pGib->InitGib( this, 100.0, 150.0 );
	pGib->m_lifeTime = 1.0f; // no effect with physics model?

	Vector origin;
	QAngle angles;
	if ( GetHeadPosition( origin, angles ) )
	{
		pGib->SetAbsOrigin( origin );
		pGib->SetAbsAngles( angles );

		Vector up;
		AngleVectors( angles, NULL, NULL, &up );
		UTIL_BloodSpray( origin, up, BloodColor(), 4, FX_BLOODSPRAY_ALL );
	}

	SetBodygroup( KURTZ_BODYGROUP_HEAD, 2 );

	SentenceStop(); // Don't talk if you have no head
}
#endif

AI_BEGIN_CUSTOM_NPC( npc_kurtz, CNPC_Kurtz )
	DECLARE_ANIMEVENT( AE_KURTZ_THROWHEAD )
AI_END_CUSTOM_NPC()
