#include "cbase.h"
#include "hoe_base_zombie.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sk_superzombie_health( "sk_superzombie_health", "0" );

//=====================
// Spawnflags
//=====================
#define SF_SZOMBIE_FRIENDLY (1 << 16)

#define SZOMBIE_HEALTH sk_superzombie_health.GetFloat() // 400, 600, 800
#define SZOMBIE_MODEL "models/superzombie/superzombie.mdl"

//=====================
// BodyGroups
//=====================

enum 
{
	SZOMBIE_BODYGROUP_HEAD = 0,
	SZOMBIE_BODYGROUP_TORSO,
	SZOMBIE_BODYGROUP_AMMO
};

#define SZOMBIE_NUM_HEADS 2 // The last head is no head at all. The horror, the horror.

class CNPC_SuperZombie : public CHOEBaseZombie
{
public:
	DECLARE_CLASS( CNPC_SuperZombie, CHOEBaseZombie );
//	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	void Spawn( void );
	void Precache( void );
	void SelectModel();
	void SelectModelGroups( void );

	Class_T Classify( void );
	bool ClassifyPlayerAlly( void ) { return HasSpawnFlags( SF_SZOMBIE_FRIENDLY ); }
	bool ClassifyPlayerAllyVital( void ) { return false; }

	Activity NPC_TranslateActivity( Activity eNewActivity );

	const char *GetSentenceGroup( void ) const { return "SZ"; }

	int GetHeadGroupNum( void ) { return SZOMBIE_BODYGROUP_HEAD; }
	int GetNumHeads( void ) { return SZOMBIE_NUM_HEADS; }
	const char *GetHeadModelName( void ) { return "models/superzombiehead.mdl"; }

	bool CheckFollowPlayer( void ) { return HasSpawnFlags( SF_SZOMBIE_FRIENDLY ); }

	float HealthFraction( CBaseEntity *pEnt )
	{
		if (pEnt->GetMaxHealth() == 0)
			return 1.0f;

		float flFraction = (float)pEnt->GetHealth() / (float)pEnt->GetMaxHealth();
		flFraction = clamp( flFraction, 0.0f, 1.0f );
		return flFraction;
	}
	float HealthFraction( void ) { return HealthFraction( this ); }

	WeaponProficiency_t CalcWeaponProficiency( CBaseCombatWeapon *pWeapon );

	void Event_KilledOther( CBaseEntity *pVictim, const CTakeDamageInfo &info );
};

LINK_ENTITY_TO_CLASS( npc_superzombie, CNPC_SuperZombie );

//IMPLEMENT_SERVERCLASS_ST(CNPC_SuperZombie, DT_SuperZombie)
//END_SEND_TABLE()

BEGIN_DATADESC( CNPC_SuperZombie )
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_SuperZombie::Spawn( void )
{
	if ( m_spawnEquipment == NULL_STRING || !Q_strcmp(STRING(m_spawnEquipment), "random") )
	{
		switch ( random->RandomInt( 0, 5 ) )
		{
		case 0:
		case 1:
			m_spawnEquipment = AllocPooledString( "weapon_m16" );
			break;
		case 2:
		case 3:
			m_spawnEquipment = AllocPooledString( "weapon_870" );
			break;
		case 4:
			m_spawnEquipment = AllocPooledString( "weapon_m60" );
			break;
		case 5:
			m_spawnEquipment = AllocPooledString( "weapon_rpg7" );
			break;
		}
	}
	else if ( !Q_strcmp(STRING(m_spawnEquipment), "none") )
	{
		m_spawnEquipment = NULL_STRING;
	}

	m_iHealth = SZOMBIE_HEALTH;

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_SuperZombie::SelectModel()
{
	SetModelName( AllocPooledString( SZOMBIE_MODEL ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_SuperZombie::SelectModelGroups( void )
{
	if ( m_nBody == -1 )
	{
		m_nHeadNum = random->RandomInt( 0, SZOMBIE_NUM_HEADS - 1 ); 
	}
	else 
	{
		m_nHeadNum = m_nBody;
	}

	m_nBody = 0;
	SetBodygroup( SZOMBIE_BODYGROUP_HEAD, m_nHeadNum );
	SetBodygroup( SZOMBIE_BODYGROUP_TORSO, m_nHeadNum );

	if (m_spawnEquipment != NULL_STRING && !Q_strcmp(STRING(m_spawnEquipment), "weapon_m60"))
		SetBodygroup( SZOMBIE_BODYGROUP_AMMO, 1 );
	else
		SetBodygroup( SZOMBIE_BODYGROUP_AMMO, 0 );
}

//-----------------------------------------------------------------------------
// Purpose: Precaches all resources this monster needs.
//-----------------------------------------------------------------------------
void CNPC_SuperZombie::Precache( void )
{
	PrecacheScriptSound( "NPC_Spx.FootstepLeft" );
	PrecacheScriptSound( "NPC_Spx.FootstepRight" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Indicates this monster's place in the relationship table.
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_SuperZombie::Classify( void )
{
	if ( HasSpawnFlags( SF_SZOMBIE_FRIENDLY ) )
		return CLASS_SUPERZOMBIE_FRIEND;

	return CLASS_SUPERZOMBIE;
}

//-----------------------------------------------------------------------------
Activity CNPC_SuperZombie::NPC_TranslateActivity( Activity eNewActivity )
{
	eNewActivity = BaseClass::NPC_TranslateActivity( eNewActivity );

	bool bHurt = HealthFraction() < 1.0/3.0;

	if ( eNewActivity == ACT_IDLE )
	{
		// This will transition from shoot -> combatidle rather than shoot -> idle
		// which is important during the shot-regulator rest periods.
		// The check for IsMoving() is there so IDLE_ANGRY isn't used as the "interior"
		// sequence when finishing movement.
		if ( ( m_NPCState == NPC_STATE_COMBAT /*|| m_NPCState == NPC_STATE_ALERT*/ ) &&
			 ( gpGlobals->curtime - m_flLastAttackTime < 3 ) && !IsMoving() )
		{
			return ACT_IDLE_ANGRY;
		}
	}
	if ( eNewActivity == ACT_WALK || eNewActivity == ACT_RUN )
	{
		if ( bHurt )
		{
			return ACT_WALK_HURT;
		}
	}

	return eNewActivity;
}

//-----------------------------------------------------------------------------
WeaponProficiency_t CNPC_SuperZombie::CalcWeaponProficiency( CBaseCombatWeapon *pWeapon )
{
	if( FClassnameIs( pWeapon, "weapon_870" )	)
	{
		return WEAPON_PROFICIENCY_PERFECT;
	}

	return BaseClass::CalcWeaponProficiency( pWeapon );
}

//-----------------------------------------------------------------------------
void CNPC_SuperZombie::Event_KilledOther( CBaseEntity *pVictim, const CTakeDamageInfo &info )
{
	if ( pVictim )
	{
		// Create a bullseye to attach to the victim's ragdoll unless the
		// victim is about to become chunks.
		if ( !pVictim->MyCombatCharacterPointer() ||
			!pVictim->MyCombatCharacterPointer()->ShouldGib( info ) )
		{
			extern void HOEHumanNPCKilled( CBaseEntity *pVictim );
			HOEHumanNPCKilled( pVictim );
		}
	}
}

AI_BEGIN_CUSTOM_NPC( npc_superzombie, CNPC_SuperZombie )
AI_END_CUSTOM_NPC()
