#include "cbase.h"
#include "hoe_human_follower.h"
#include "npcevent.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define BARNEY_MODEL "models/barney/barney.mdl"

#define RECENT_DAMAGE_INTERVAL		3.0f
#define RECENT_DAMAGE_THRESHOLD		0.2f

ConVar sk_barney_health( "sk_barney_health", "0");
ConVar sk_barney_regen_time( "sk_barney_regen_time", "0.3003", FCVAR_NONE, "Time taken for Barney to regenerate a point of health." );

class CNPC_Barney : public CHOEHumanFollower
{
public:
	DECLARE_CLASS( CNPC_Barney, CHOEHumanFollower );
//	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	void Spawn( void );
	void Precache( void );
	void SelectModel();
	
	Class_T Classify( void );
	bool ClassifyPlayerAlly( void ) { return true; }
	bool ClassifyPlayerAllyVital( void ) { return true; }

	void Activate( void );

//	void SelectModelGroups( void );

	int GetHeadGroupNum( void ) { return 0; }
	int GetNumHeads( void ) { return 1; }
	const char *GetHeadModelName( void ) { return "models/barneyhead.mdl"; }

	virtual SpeechManagerID_t GetSpeechManagerID( void ) { return SPEECH_MANAGER_ALLY; }
	const char *GetSentenceGroup( void ) const { return "BA"; }
	const char **GetFriendClasses( void ) const {
		static const char *szClasses[] = {
			"npc_barney",
			"npc_mikeforce",
			"npc_mikeforce_medic",
//			"npc_peasant",
			"player",
			NULL
		};
		return szClasses;
	};
	const char **GetWeaponClasses( void ) const {
		static const char *szClasses[] = {
			"weapon_colt1911A1",
			NULL
		};
		return szClasses;
	};

	const char *GetCCImageNameCStr( void )
	{
		return "cc_barney";
	}

	int OnTakeDamage_Alive( const CTakeDamageInfo &info );

	void PrescheduleThink( void );
	int SelectSchedule( void );

#ifdef BARNEY_STARE // FIXME: remove
	void DoCustomSpeechAI( void );
	void GatherConditions();
#endif

	bool HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt);

	void HandleAnimEvent( animevent_t *pEvent );

	void CreateHolsteredColt( void );
	void Event_Killed( const CTakeDamageInfo &info );

	virtual float RegenerateTime( void ) { return sk_barney_regen_time.GetFloat(); }

#ifdef BARNEY_STARE // FIXME: remove
	CStopwatch m_SpeechWatch_PlayerLooking;
#endif

	// run away when taking damage (from npc_metropolice)
	int				m_nRecentDamage;
	float			m_flRecentDamageTime;

	EHANDLE			m_hHolsteredColt;
	bool			m_bSpawnHolstered;
};

LINK_ENTITY_TO_CLASS( npc_barney, CNPC_Barney );

//IMPLEMENT_SERVERCLASS_ST(CNPC_Barney, DT_NPC_Barney)
//END_SEND_TABLE()

BEGIN_DATADESC( CNPC_Barney )
//						m_FuncTankBehavior
#ifdef BARNEY_STARE // FIXME: remove
	DEFINE_EMBEDDED( m_SpeechWatch_PlayerLooking ),
#endif
	DEFINE_FIELD( m_hHolsteredColt, FIELD_EHANDLE ),
	DEFINE_KEYFIELD( m_bSpawnHolstered, FIELD_BOOLEAN, "SpawnHolstered" ),
END_DATADESC()

//-----------------------------------------------------------------------------
void CNPC_Barney::SelectModel()
{
	SetModelName( AllocPooledString( BARNEY_MODEL ) );
}

//-----------------------------------------------------------------------------
void CNPC_Barney::Spawn( void )
{
	m_spawnEquipment = AllocPooledString( "weapon_colt1911A1" );

	m_iHealth = sk_barney_health.GetInt();

	BaseClass::Spawn();

	CapabilitiesAdd( bits_CAP_MOVE_SHOOT );

#ifdef HOE_HUMAN_RR
	SetSpokeConcept( TLK_HELLO, NULL, false );
#else
	m_iSpokenConcepts |= HUMAN_SC_HELLO;
#endif

//	m_fHandGrenades = true;

	if ( m_bSpawnHolstered && GetActiveWeapon() )
	{
		GetActiveWeapon()->Holster();
		SetActiveWeapon( NULL );
		CreateHolsteredColt();
	}
}

//-----------------------------------------------------------------------------
void CNPC_Barney::Precache( void )
{
	PrecacheScriptSound( "NPC_Barney.FootstepLeft" );
	PrecacheScriptSound( "NPC_Barney.FootstepRight" );
	PrecacheScriptSound( "NPC_Barney.RunFootstepLeft" );
	PrecacheScriptSound( "NPC_Barney.RunFootstepRight" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
Class_T	CNPC_Barney::Classify( void )
{
	return CLASS_BARNEY;
}

//-----------------------------------------------------------------------------
void CNPC_Barney::Activate( void )
{
	// Alyx always kicks her health back up to full after loading a savegame.
	// Avoids problems with players saving the game in places where she dies immediately afterwards.
	m_iHealth = GetMaxHealth();

	BaseClass::Activate();
}

#ifdef BARNEY_STARE // FIXME: remove
//-----------------------------------------------------------------------------
void CNPC_Barney::DoCustomSpeechAI( void )
{
	CBasePlayer *pPlayer = UTIL_PlayerByIndex(1);
	float flDistSqr = ( GetAbsOrigin() - pPlayer->GetAbsOrigin() ).Length2DSqr();
	float flStareDist = 72; /*sk_citizen_player_stare_dist.GetFloat()*/
	if ( pPlayer && pPlayer->IsAlive() && (flDistSqr <= flStareDist * flStareDist) && pPlayer->FInViewCone( this ) && pPlayer->FVisible( this ) )
	{
		if ( m_SpeechWatch_PlayerLooking.Expired() )
		{
			if ( OkToSpeak() && !SpokeConcept( HUMAN_SC_STARE ) && !random->RandomInt(0, 3) )
				Speak( "STARE", HUMAN_SC_STARE, pPlayer );
			m_SpeechWatch_PlayerLooking.Stop();
		}
	}
	else
	{
		m_SpeechWatch_PlayerLooking.Start( 1.0f );
	}
}

//-----------------------------------------------------------------------------
void CNPC_Barney::GatherConditions()
{
	BaseClass::GatherConditions();

	// Handle speech AI. Don't speak AI speech if we're in scripts.
	if ( m_NPCState == NPC_STATE_IDLE || m_NPCState == NPC_STATE_ALERT || m_NPCState == NPC_STATE_COMBAT )
	{
		DoCustomSpeechAI();
	}
}

#endif

//-----------------------------------------------------------------------------
int CNPC_Barney::OnTakeDamage_Alive( const CTakeDamageInfo &inputInfo )
{
	CTakeDamageInfo info = inputInfo;

	if (info.GetAttacker() == GetEnemy())
	{
		// Keep track of recent damage by my attacker. If it seems like we're
		// being killed, consider running off and hiding.
		m_nRecentDamage += info.GetDamage();
		m_flRecentDamageTime = gpGlobals->curtime;
	}

	return BaseClass::OnTakeDamage_Alive( info );
}

//-----------------------------------------------------------------------------
void CNPC_Barney::PrescheduleThink( void )
{
	BaseClass::PrescheduleThink();

	// If Barney is in combat, and he doesn't have his gun out, fetch it
	if ( GetState() == NPC_STATE_COMBAT && IsWeaponHolstered() )
	{
		SetDesiredWeaponState( DESIREDWEAPONSTATE_UNHOLSTERED );
	}

	if (gpGlobals->curtime > m_flRecentDamageTime + RECENT_DAMAGE_INTERVAL)
	{
		m_nRecentDamage = 0;
		m_flRecentDamageTime = 0;
	}
}

//-----------------------------------------------------------------------------
int CNPC_Barney::SelectSchedule( void )
{
	// if Barney has an enemy and the enemy has a melee attack and the enemy is
	// within melee range then SCHED_BACK_AWAY_FROM_ENEMY. Perhaps only do this after
	// taking melee from the enemy for the first time.

	if ( GetState() == NPC_STATE_COMBAT )
	{
		// from npc_metropolice
		if (((float)m_nRecentDamage / (float)GetMaxHealth()) > RECENT_DAMAGE_THRESHOLD)
		{
			m_nRecentDamage = 0;
			m_flRecentDamageTime = 0;
	//		m_Sentences.Speak( "METROPOLICE_COVER_HEAVY_DAMAGE", SENTENCE_PRIORITY_MEDIUM, SENTENCE_CRITERIA_NORMAL );

			return SCHED_TAKE_COVER_FROM_ENEMY;
		}
	}
	
	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
// Purpose:  This is a generic function (to be implemented by sub-classes) to
//			 handle specific interactions between different types of characters
//			 (For example the barnacle grabbing an NPC)
// Input  :  Constant for the type of interaction
// Output :	 true  - if sub-class has a response for the interaction
//			 false - if sub-class has no response
//-----------------------------------------------------------------------------
bool CNPC_Barney::HandleInteraction( int interactionType, void *data, CBaseCombatCharacter* sourceEnt )
{
#if 0 // CNPC_PlayerCompanion::HandleInteraction
	if (interactionType == g_interactionHitByPlayerThrownPhysObj )
	{
		if ( IsOkToSpeakInResponseToPlayer() )
		{
			Speak( TLK_PLYR_PHYSATK );
		}
		return true;
	}
#endif
	return BaseClass::HandleInteraction( interactionType, data, sourceEnt );
}

//-----------------------------------------------------------------------------
void CNPC_Barney::HandleAnimEvent( animevent_t *pEvent )
{
	if ( (pEvent->type & AE_TYPE_NEWEVENTSYSTEM) && (pEvent->type & AE_TYPE_SERVER) )
	{
		if ( pEvent->event == AE_NPC_HOLSTER )
		{
			CreateHolsteredColt();
			if ( m_hHolsteredColt )
			{
				m_hHolsteredColt->RemoveEffects( EF_NODRAW );
			}

			// fall through to BaseClass
		}
		else if ( pEvent->event == AE_NPC_DRAW )
		{
			if ( m_hHolsteredColt )
			{
				m_hHolsteredColt->AddEffects( EF_NODRAW );
			}
		}
	}

	BaseClass::HandleAnimEvent( pEvent );
}

ConVar hoe_barney_colt_offset("hoe_barney_colt_offset", "2.3 -0.4 -1.5");
ConVar hoe_barney_colt_angles("hoe_barney_colt_angles", "4 90 5");

//-----------------------------------------------------------------------------
void CNPC_Barney::CreateHolsteredColt( void )
{
	if ( m_hHolsteredColt )
		return;

	m_hHolsteredColt = (CBaseAnimating*)CreateEntityByName( "prop_dynamic" );
	if ( m_hHolsteredColt )
	{
		m_hHolsteredColt->SetModel( "models/colt1911A1/w_colt1911A1/w_colt1911A1.mdl" );
		m_hHolsteredColt->SetName( AllocPooledString("Barney_HolsterdColt") );
		int iAttachment = LookupAttachment( "Colt_Holster" );
		m_hHolsteredColt->SetParent(this, iAttachment);
		m_hHolsteredColt->SetOwnerEntity(this);
		m_hHolsteredColt->SetSolid( SOLID_NONE );
		m_hHolsteredColt->SetLocalOrigin( Vector( 0, 0, 0 ) );
		m_hHolsteredColt->SetLocalAngles( QAngle( 0, 0, 0 ) );
#if 1
		float x, y, z;
		if ( sscanf(hoe_barney_colt_offset.GetString(), "%g %g %g", &x, &y, &z) == 3 )
		{
			m_hHolsteredColt->SetLocalOrigin( Vector( x, y, z ) );
		}
		if ( sscanf(hoe_barney_colt_angles.GetString(), "%g %g %g", &x, &y, &z) == 3 )
		{
			m_hHolsteredColt->SetLocalAngles( QAngle( x, y, z ) );
		}
#endif
		m_hHolsteredColt->AddSpawnFlags(SF_DYNAMICPROP_NO_VPHYSICS);
		m_hHolsteredColt->AddEffects( EF_PARENT_ANIMATES );
		m_hHolsteredColt->Spawn();
	}
}

//-----------------------------------------------------------------------------
void CNPC_Barney::Event_Killed( const CTakeDamageInfo &info )
{
	// Destroy our Colt since it won't follow us onto the ragdoll anyway
	if ( m_hHolsteredColt != NULL )
	{
		UTIL_Remove( m_hHolsteredColt );
	}

	BaseClass::Event_Killed( info );
}

//-----------------------------------------------------------------------------
AI_BEGIN_CUSTOM_NPC( npc_barney, CNPC_Barney )
AI_END_CUSTOM_NPC()
