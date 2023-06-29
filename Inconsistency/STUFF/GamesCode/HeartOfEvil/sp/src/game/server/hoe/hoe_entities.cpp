#include "cbase.h"
#include "ai_utils.h"
#include "ammodef.h"
#include "entityinput.h"
#include "entityoutput.h"
#include "eventqueue.h"
#include "mathlib/mathlib.h"
#include "globalstate.h"
#include "ndebugoverlay.h"
#include "saverestore_utlvector.h"
#include "vstdlib/random.h"
#include "gameinterface.h"
#include "fmtstr.h"
#include "baseanimating.h"
#include "hl2_player.h"

#include "physics_saverestore.h"
#include "vphysics/friction.h"
#include "movevars_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


//-----------------------------------------------------------------------------
// replacement for multisource
//-----------------------------------------------------------------------------

class CLogicAnd : public CLogicalEntity
{
public:
	DECLARE_CLASS( CLogicAnd, CLogicalEntity );

	void Spawn( );
	bool KeyValue( const char *szKeyName, const char *szValue );
	void ToggleValue( inputdata_t &inputdata, int index );
	int Test( ::CBaseEntity *pActivator );

	int	ObjectCaps( void ) { return(BaseClass::ObjectCaps() | FCAP_MASTER); }
	bool IsTriggered( ::CBaseEntity *pActivator );

	DECLARE_DATADESC();

#define MAX_LOGIC_AND_VALUES 8
	bool m_Values[MAX_LOGIC_AND_VALUES];

	// Input handlers
	void InputToggleValue01( inputdata_t &inputdata );
	void InputToggleValue02( inputdata_t &inputdata );
	void InputToggleValue03( inputdata_t &inputdata );
	void InputToggleValue04( inputdata_t &inputdata );
	void InputToggleValue05( inputdata_t &inputdata );
	void InputToggleValue06( inputdata_t &inputdata );
	void InputToggleValue07( inputdata_t &inputdata );
	void InputToggleValue08( inputdata_t &inputdata );

	// Outputs
	COutputEvent m_OnAllTrue;
	COutputEvent m_OnAllFalse;
	COutputEvent m_OnMixed;

	string_t	m_globalstate;
};

BEGIN_DATADESC( CLogicAnd )

	DEFINE_KEYFIELD( m_Values[0], FIELD_BOOLEAN, "Value01" ),
	DEFINE_KEYFIELD( m_Values[1], FIELD_BOOLEAN, "Value02" ),
	DEFINE_KEYFIELD( m_Values[2], FIELD_BOOLEAN, "Value03" ),
	DEFINE_KEYFIELD( m_Values[3], FIELD_BOOLEAN, "Value04" ),
	DEFINE_KEYFIELD( m_Values[4], FIELD_BOOLEAN, "Value05" ),
	DEFINE_KEYFIELD( m_Values[5], FIELD_BOOLEAN, "Value06" ),
	DEFINE_KEYFIELD( m_Values[6], FIELD_BOOLEAN, "Value07" ),
	DEFINE_KEYFIELD( m_Values[7], FIELD_BOOLEAN, "Value08" ),

	DEFINE_KEYFIELD( m_globalstate, FIELD_STRING, "globalstate" ),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_INPUT, "ToggleValue01", InputToggleValue01 ),
	DEFINE_INPUTFUNC( FIELD_INPUT, "ToggleValue02", InputToggleValue02 ),
	DEFINE_INPUTFUNC( FIELD_INPUT, "ToggleValue03", InputToggleValue03 ),
	DEFINE_INPUTFUNC( FIELD_INPUT, "ToggleValue04", InputToggleValue04 ),
	DEFINE_INPUTFUNC( FIELD_INPUT, "ToggleValue05", InputToggleValue05 ),
	DEFINE_INPUTFUNC( FIELD_INPUT, "ToggleValue06", InputToggleValue06 ),
	DEFINE_INPUTFUNC( FIELD_INPUT, "ToggleValue07", InputToggleValue07 ),
	DEFINE_INPUTFUNC( FIELD_INPUT, "ToggleValue08", InputToggleValue08 ),

	// Outputs
	DEFINE_OUTPUT(m_OnAllTrue, "OnAllTrue"),
	DEFINE_OUTPUT(m_OnAllFalse, "OnAllFalse"),
	DEFINE_OUTPUT(m_OnMixed, "OnMixed"),

END_DATADESC()


LINK_ENTITY_TO_CLASS( logic_and, CLogicAnd );


//-----------------------------------------------------------------------------
// Purpose: Cache user entity field values until spawn is called.
// Input  : szKeyName - Key to handle.
//			szValue - Value for key.
// Output : Returns true if the key was handled, false if not.
//-----------------------------------------------------------------------------
bool CLogicAnd::KeyValue( const char *szKeyName, const char *szValue )
{
	if (	FStrEq(szKeyName, "style") ||
				FStrEq(szKeyName, "height") ||
				FStrEq(szKeyName, "killtarget") ||
				FStrEq(szKeyName, "value1") ||
				FStrEq(szKeyName, "value2") ||
				FStrEq(szKeyName, "value3"))
	{
	}
	else
	{
		return BaseClass::KeyValue( szKeyName, szValue );
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLogicAnd::Spawn()
{ 
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLogicAnd::ToggleValue( inputdata_t &inputdata, int index )
{
	if (index < 1 || index > MAX_LOGIC_AND_VALUES)
		return;

	m_Values[index - 1] ^= 1;

	switch ( Test( inputdata.pActivator ) )
	{
	case 0: m_OnAllFalse.FireOutput(inputdata.pActivator, this); break;
	case 1: m_OnAllTrue.FireOutput(inputdata.pActivator, this); break;
	case 2: m_OnMixed.FireOutput(inputdata.pActivator, this); break;
	default: break; // global state disabled
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLogicAnd::InputToggleValue01( inputdata_t &inputdata )
{
	ToggleValue( inputdata, 1);
}
void CLogicAnd::InputToggleValue02( inputdata_t &inputdata )
{
	ToggleValue( inputdata, 2);
}
void CLogicAnd::InputToggleValue03( inputdata_t &inputdata )
{
	ToggleValue( inputdata, 3);
}
void CLogicAnd::InputToggleValue04( inputdata_t &inputdata )
{
	ToggleValue( inputdata, 4);
}
void CLogicAnd::InputToggleValue05( inputdata_t &inputdata )
{
	ToggleValue( inputdata, 5);
}
void CLogicAnd::InputToggleValue06( inputdata_t &inputdata )
{
	ToggleValue( inputdata, 6);
}
void CLogicAnd::InputToggleValue07( inputdata_t &inputdata )
{
	ToggleValue( inputdata, 7);
}
void CLogicAnd::InputToggleValue08( inputdata_t &inputdata )
{
	ToggleValue( inputdata, 8);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CLogicAnd::Test( CBaseEntity * )
{
	if (m_globalstate != NULL_STRING)
	{
		if (!GlobalEntity_IsInTable( m_globalstate ))
			return -1;
		if (GlobalEntity_GetState( m_globalstate ) != GLOBAL_ON)
			return -1;
	}

	bool oneTrue = false, oneFalse = false;
	for (int i = 0; i < MAX_LOGIC_AND_VALUES; i++)
	{
		if (m_Values[i]) oneTrue = true;
		else oneFalse = true;
	}

	if (!oneTrue && oneFalse) return 0; // all false
	if (oneTrue && !oneFalse) return 1; // all true
	return 2; // mixed
}

bool CLogicAnd::IsTriggered( ::CBaseEntity *pActivator )
{
	return ( Test( pActivator ) == 1 );
}

//-----------------------------------------------------------------------------

#if 0

class CEnvRender : public CBaseEntity
{
public:
	DECLARE_CLASS( CEnvRender, CBaseEntity );
//	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CEnvRender()
	{
		for ( int i = 0; i < 8; i++ )
			m_targets[i] = NULL_STRING;
	}
					
	void InputActivate(inputdata_t &data);

	string_t m_targets[8];
};

LINK_ENTITY_TO_CLASS( env_render, CEnvRender );

//IMPLEMENT_SERVERCLASS_ST_NOBASE( CEnvRender, DT_EnvRender )
//END_SEND_TABLE()

BEGIN_DATADESC( CEnvRender )
	DEFINE_KEYFIELD( m_targets[0], FIELD_STRING, "Target01" ),
	DEFINE_KEYFIELD( m_targets[1], FIELD_STRING, "Target02" ),
	DEFINE_KEYFIELD( m_targets[2], FIELD_STRING, "Target03" ),
	DEFINE_KEYFIELD( m_targets[3], FIELD_STRING, "Target04" ),
	DEFINE_KEYFIELD( m_targets[4], FIELD_STRING, "Target05" ),
	DEFINE_KEYFIELD( m_targets[5], FIELD_STRING, "Target06" ),
	DEFINE_KEYFIELD( m_targets[6], FIELD_STRING, "Target07" ),
	DEFINE_KEYFIELD( m_targets[7], FIELD_STRING, "Target08" ),
END_DATADESC()

void CEnvRender::InputActivate(inputdata_t &inputdata)
{
	CBaseEntity *pTargetEnt;

	for ( int i = 0; i < 8; i++ )
	{
		if ( m_targets[i] == NULL_STRING )
			continue;

		for ( pTargetEnt = gEntList.FindEntityByName( NULL, m_targets[i] );
			pTargetEnt != NULL;
			pTargetEnt = gEntList.FindEntityByName( pTargetEnt, m_targets[i] ) )
		{
			if ( !( m_spawnflags & 1 ) )
				pTargetEnt->m_nRenderFX = m_nRenderFX;
			if ( !( m_spawnflags & 2 ) )
				pTargetEnt->SetRenderColorA( GetRenderColor().a );
			if ( !( m_spawnflags & 4 ) )
				pTargetEnt->SetRenderMode( GetRenderMode() );
			if ( !( m_spawnflags & 8 ) )
				pTargetEnt->SetRenderColor( GetRenderColor().r, GetRenderColor().g, GetRenderColor().b );
		}
	}
}

#endif

//-----------------------------------------------------------------------------

// FIXME: this should just be a breakable prop
class CPalmProp : public CBaseAnimating
{
public:
	DECLARE_CLASS( CPalmProp, CBaseAnimating );

	CPalmProp();

	void Spawn( void );
	void Precache( void );
	void TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr );
	void Event_Killed( const CTakeDamageInfo &info );
};

LINK_ENTITY_TO_CLASS( prop_palm, CPalmProp );

CPalmProp::CPalmProp()
{
	m_nBody = -1;
}

void CPalmProp::Spawn( void )
{
	BaseClass::Spawn();

	Precache();
	SetModel( STRING( GetModelName() ) );

	SetMoveType( MOVETYPE_NONE );
	SetSolid( SOLID_BBOX );
//	UTIL_SetSize( this, Vector(-6, -6, 0), Vector(6, 6, 155) ); // FIXME: get hitbox 0
	CollisionProp()->SetSurroundingBoundsType( USE_HITBOXES );

	m_iHealth = 100;
	m_takedamage = DAMAGE_NO; // TODO: gibs
}

void CPalmProp::Precache( void )
{
	if ( m_nBody == -1 )
	{
		m_nBody = random->RandomInt(0, 2);
	}

	CFmtStr fmt;
	fmt.sprintf("models/palm/palm%d/palm%d.mdl", m_nBody + 1, m_nBody + 1);
	SetModelName( AllocPooledString( fmt ) );

	PrecacheModel( STRING( GetModelName() ) );
}

void CPalmProp::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr )
{
	DevMsg("CPalmProp::TraceAttack health %d\n", m_iHealth);

	// No damage caused by bullets or most other things
	if ( ( info.GetDamageType() & (DMG_BURN | DMG_BLAST) ) == 0 )
		return;

	BaseClass::TraceAttack( info, vecDir, ptr );
}

void CPalmProp::Event_Killed( const CTakeDamageInfo &info )
{
	// HL1 version would spawn propgibs

//	UTIL_Remove( this );

	BaseClass::Event_Killed( info );
}

//-----------------------------------------------------------------------------

class CLogicNewGameItem : public CLogicalEntity
{
public:
	DECLARE_CLASS( CLogicNewGameItem, CLogicalEntity );
	DECLARE_DATADESC();

	void Activate(void);
	void Think(void);
	void Equip(void);

	bool item_suit;
};

BEGIN_DATADESC( CLogicNewGameItem )

	DEFINE_KEYFIELD( item_suit, FIELD_BOOLEAN, "item_suit" ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( logic_newgame_item, CLogicNewGameItem );

//------------------------------------------------------------------------------
void CLogicNewGameItem::Activate(void)
{
	BaseClass::Activate();
	SetNextThink( gpGlobals->curtime + 0.2 );
}

//-----------------------------------------------------------------------------
void CLogicNewGameItem::Think(void)
{
	if ( gpGlobals->eLoadType == MapLoad_NewGame )
	{
		Equip();
	}
	UTIL_Remove(this);
}

//-----------------------------------------------------------------------------
void CLogicNewGameItem::Equip(void)
{
	CBasePlayer *pPlayer = AI_GetSinglePlayer();
	if ( pPlayer == NULL )
		return;

	if ( item_suit ) pPlayer->EquipSuit( false );
}

//-----------------------------------------------------------------------------

class CLogicNewGameWeapon : public CLogicalEntity
{
public:
	DECLARE_CLASS( CLogicNewGameWeapon, CLogicalEntity );
	DECLARE_DATADESC();

	void Activate(void);
	void Think(void);
	void Equip(void);

	bool weapon_ak47;
	bool weapon_chainsaw;
	bool weapon_colt1911A1;
	bool weapon_handgrenade;
	bool weapon_machete;
	bool weapon_m16;
	bool weapon_m21;
	bool weapon_m60;
	bool weapon_m79;
	bool weapon_870;
	bool weapon_rpg7;
	bool weapon_snark;
	bool weapon_tripmine;
};

BEGIN_DATADESC( CLogicNewGameWeapon )

	DEFINE_KEYFIELD( weapon_ak47, FIELD_BOOLEAN, "weapon_ak47" ),
	DEFINE_KEYFIELD( weapon_chainsaw, FIELD_BOOLEAN, "weapon_chainsaw" ),
	DEFINE_KEYFIELD( weapon_colt1911A1, FIELD_BOOLEAN, "weapon_colt1911A1" ),
	DEFINE_KEYFIELD( weapon_handgrenade, FIELD_BOOLEAN, "weapon_handgrenade" ),
	DEFINE_KEYFIELD( weapon_machete, FIELD_BOOLEAN, "weapon_machete" ),
	DEFINE_KEYFIELD( weapon_m16, FIELD_BOOLEAN, "weapon_m16" ),
	DEFINE_KEYFIELD( weapon_m21, FIELD_BOOLEAN, "weapon_m21" ),
	DEFINE_KEYFIELD( weapon_m60, FIELD_BOOLEAN, "weapon_m60" ),
	DEFINE_KEYFIELD( weapon_m79, FIELD_BOOLEAN, "weapon_m79" ),
	DEFINE_KEYFIELD( weapon_870, FIELD_BOOLEAN, "weapon_870" ),
	DEFINE_KEYFIELD( weapon_rpg7, FIELD_BOOLEAN, "weapon_rpg7" ),
	DEFINE_KEYFIELD( weapon_snark, FIELD_BOOLEAN, "weapon_snark" ),
	DEFINE_KEYFIELD( weapon_tripmine, FIELD_BOOLEAN, "weapon_tripmine" ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( logic_newgame_weapon, CLogicNewGameWeapon );

//------------------------------------------------------------------------------
void CLogicNewGameWeapon::Activate(void)
{
	BaseClass::Activate();
	SetNextThink( gpGlobals->curtime + 0.2 );
}

//-----------------------------------------------------------------------------
void CLogicNewGameWeapon::Think(void)
{
	if ( gpGlobals->eLoadType == MapLoad_NewGame )
	{
		Equip();
	}
	UTIL_Remove(this);
}

//-----------------------------------------------------------------------------
void CLogicNewGameWeapon::Equip(void)
{
	CBasePlayer *pPlayer = AI_GetSinglePlayer();
	if ( pPlayer == NULL )
		return;

//	pPlayer->EquipSuit( false );

	if ( weapon_ak47 ) pPlayer->GiveNamedItem( "weapon_ak47" );
	if ( weapon_chainsaw ) pPlayer->GiveNamedItem( "weapon_chainsaw" );
	if ( weapon_colt1911A1 ) pPlayer->GiveNamedItem( "weapon_colt1911A1" );
	if ( weapon_handgrenade ) pPlayer->GiveNamedItem( "weapon_handgrenade" );
	if ( weapon_machete ) pPlayer->GiveNamedItem( "weapon_machete" );
	if ( weapon_m16 ) pPlayer->GiveNamedItem( "weapon_m16" );
	if ( weapon_m21 ) pPlayer->GiveNamedItem( "weapon_m21" );
	if ( weapon_m60 ) pPlayer->GiveNamedItem( "weapon_m60" );
	if ( weapon_m79 ) pPlayer->GiveNamedItem( "weapon_m79" );
	if ( weapon_870 ) pPlayer->GiveNamedItem( "weapon_870" );
	if ( weapon_rpg7 ) pPlayer->GiveNamedItem( "weapon_rpg7" );
	if ( weapon_snark ) pPlayer->GiveNamedItem( "weapon_snark" );
	if ( weapon_tripmine ) pPlayer->GiveNamedItem( "weapon_tripmine" );
}

//-----------------------------------------------------------------------------

class CLogicNewGameLetter : public CLogicalEntity
{
public:
	DECLARE_CLASS( CLogicNewGameLetter, CLogicalEntity );
	DECLARE_DATADESC();

	void Activate(void);
	void Think(void);
	void Equip(void);

#define MAX_NGE_VALUES 20
	string_t m_Values[MAX_NGE_VALUES];
};

BEGIN_DATADESC( CLogicNewGameLetter )

	DEFINE_KEYFIELD( m_Values[0], FIELD_STRING, "Value01" ),
	DEFINE_KEYFIELD( m_Values[1], FIELD_STRING, "Value02" ),
	DEFINE_KEYFIELD( m_Values[2], FIELD_STRING, "Value03" ),
	DEFINE_KEYFIELD( m_Values[3], FIELD_STRING, "Value04" ),
	DEFINE_KEYFIELD( m_Values[4], FIELD_STRING, "Value05" ),
	DEFINE_KEYFIELD( m_Values[5], FIELD_STRING, "Value06" ),
	DEFINE_KEYFIELD( m_Values[6], FIELD_STRING, "Value07" ),
	DEFINE_KEYFIELD( m_Values[7], FIELD_STRING, "Value08" ),
	DEFINE_KEYFIELD( m_Values[8], FIELD_STRING, "Value09" ),
	DEFINE_KEYFIELD( m_Values[9], FIELD_STRING, "Value10" ),
	DEFINE_KEYFIELD( m_Values[10], FIELD_STRING, "Value11" ),
	DEFINE_KEYFIELD( m_Values[11], FIELD_STRING, "Value12" ),
	DEFINE_KEYFIELD( m_Values[12], FIELD_STRING, "Value13" ),
	DEFINE_KEYFIELD( m_Values[13], FIELD_STRING, "Value14" ),
	DEFINE_KEYFIELD( m_Values[14], FIELD_STRING, "Value15" ),
	DEFINE_KEYFIELD( m_Values[15], FIELD_STRING, "Value16" ),
	DEFINE_KEYFIELD( m_Values[16], FIELD_STRING, "Value17" ),
	DEFINE_KEYFIELD( m_Values[17], FIELD_STRING, "Value18" ),
	DEFINE_KEYFIELD( m_Values[18], FIELD_STRING, "Value19" ),
	DEFINE_KEYFIELD( m_Values[19], FIELD_STRING, "Value20" ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( logic_newgame_letter, CLogicNewGameLetter );

//------------------------------------------------------------------------------
void CLogicNewGameLetter::Activate(void)
{
	BaseClass::Activate();
	SetNextThink( gpGlobals->curtime + 0.2 );
}

//-----------------------------------------------------------------------------
void CLogicNewGameLetter::Think(void)
{
	if ( gpGlobals->eLoadType == MapLoad_NewGame )
	{
		Equip();
	}
	UTIL_Remove(this);
}

//-----------------------------------------------------------------------------
void CLogicNewGameLetter::Equip(void)
{
	CBasePlayer *pPlayer = AI_GetSinglePlayer();
	if ( pPlayer == NULL )
		return;

	CHL2_Player *pHL2Player = dynamic_cast<CHL2_Player*>(pPlayer);
	if ( pHL2Player == NULL )
		return;

	for ( int i = 0; i < MAX_NGE_VALUES; i++ )
	{
		if ( m_Values[i] == NULL_STRING )
			continue;

		const char *szName = STRING(m_Values[i]);
		if ( !szName[0] || !Q_stricmp( szName, "none" ) )
			continue;

		pHL2Player->AddLetter( m_Values[i] );
	}
}

//-----------------------------------------------------------------------------

class CLogicNewGameAmmo : public CLogicalEntity
{
public:
	DECLARE_CLASS( CLogicNewGameAmmo, CLogicalEntity );
	DECLARE_DATADESC();

	void Activate(void);
	void Think(void);
	void Equip(void);
	void GiveAmmo( CBasePlayer *pPlayer, const char *ammoName, float frac );

	float ammo_7_62x39mm_M1943;
	float ammo_Gas;
	float ammo_11_43mm;
	float ammo_grenade;
	float ammo_5_56mm;
	float ammo_m21;
	float ammo_7_62mm;
	float ammo_AR_Grenade;
	float ammo_rpg_round;
	float ammo_Buckshot;
	float ammo_Elephantshot;
	float ammo_Snark;
	float ammo_Tripmine;
};

BEGIN_DATADESC( CLogicNewGameAmmo )

	DEFINE_KEYFIELD( ammo_7_62x39mm_M1943, FIELD_FLOAT, "ammo_7_62x39mm_M1943" ),
	DEFINE_KEYFIELD( ammo_Gas, FIELD_FLOAT, "ammo_Gas" ),
	DEFINE_KEYFIELD( ammo_11_43mm, FIELD_FLOAT, "ammo_11_43mm" ),
	DEFINE_KEYFIELD( ammo_grenade, FIELD_FLOAT, "ammo_grenade" ),
	DEFINE_KEYFIELD( ammo_5_56mm, FIELD_FLOAT, "ammo_5_56mm" ),
	DEFINE_KEYFIELD( ammo_m21, FIELD_FLOAT, "ammo_m21" ),
	DEFINE_KEYFIELD( ammo_7_62mm, FIELD_FLOAT, "ammo_7_62mm" ),
	DEFINE_KEYFIELD( ammo_AR_Grenade, FIELD_FLOAT, "ammo_AR_Grenade" ),
	DEFINE_KEYFIELD( ammo_rpg_round, FIELD_FLOAT, "ammo_rpg_round" ),
	DEFINE_KEYFIELD( ammo_Buckshot, FIELD_FLOAT, "ammo_Buckshot" ),
	DEFINE_KEYFIELD( ammo_Elephantshot, FIELD_FLOAT, "ammo_Elephantshot" ),
	DEFINE_KEYFIELD( ammo_Snark, FIELD_FLOAT, "ammo_Snark" ),
	DEFINE_KEYFIELD( ammo_Tripmine, FIELD_FLOAT, "ammo_Tripmine" ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( logic_newgame_ammo, CLogicNewGameAmmo );

//------------------------------------------------------------------------------
void CLogicNewGameAmmo::Activate(void)
{
	BaseClass::Activate();
	SetNextThink( gpGlobals->curtime + 0.2 );
}

//-----------------------------------------------------------------------------
void CLogicNewGameAmmo::Think(void)
{
	if ( gpGlobals->eLoadType == MapLoad_NewGame )
	{
		Equip();
	}
	UTIL_Remove(this);
}

//-----------------------------------------------------------------------------
void CLogicNewGameAmmo::Equip(void)
{
	CBasePlayer *pPlayer = AI_GetSinglePlayer();
	if ( pPlayer == NULL )
		return;

	GiveAmmo( pPlayer, "7_62x39mm_M1943", ammo_7_62x39mm_M1943 );
	GiveAmmo( pPlayer, "Gas", ammo_Gas );
	GiveAmmo( pPlayer, "11_43mm", ammo_11_43mm );
	GiveAmmo( pPlayer, "grenade", ammo_grenade );
	GiveAmmo( pPlayer, "5_56mm", ammo_5_56mm );
	GiveAmmo( pPlayer, "m21", ammo_m21 );
	GiveAmmo( pPlayer, "7_62mm", ammo_7_62mm );
	GiveAmmo( pPlayer, "AR_Grenade", ammo_AR_Grenade );
	GiveAmmo( pPlayer, "rpg_round", ammo_rpg_round );
	GiveAmmo( pPlayer, "Buckshot", ammo_Buckshot );
	GiveAmmo( pPlayer, "Elephantshot", ammo_Elephantshot );
	GiveAmmo( pPlayer, "Snark", ammo_Snark );
	GiveAmmo( pPlayer, "Tripmine", ammo_Tripmine );
}

//-----------------------------------------------------------------------------
void CLogicNewGameAmmo::GiveAmmo( CBasePlayer *pPlayer, const char *ammoName, float frac )
{
	frac = clamp( frac, 0, 1.0 );
	if ( frac == 0 )
		return;

	int iAmmoType = GetAmmoDef()->Index( ammoName );
	if ( iAmmoType != -1 )
	{
		int max = GetAmmoDef()->MaxCarry( iAmmoType ) * frac;
		int cur = pPlayer->GetAmmoCount( iAmmoType );
		if ( max - cur > 0 )
			pPlayer->GiveAmmo( max - cur, iAmmoType, true );
		return;
	}

	DevWarning( "CLogicNewGameAmmo::GiveAmmo unknown ammo type \"%s\"\n", ammoName );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class CCorpseSolver : public CLogicalEntity, public IMotionEvent
{
	DECLARE_CLASS( CCorpseSolver, CLogicalEntity );
public:
	CCorpseSolver();
	~CCorpseSolver();
	DECLARE_DATADESC();
	void Init( CBaseEntity *pCorpse, CBaseEntity *pPhysicsObject, bool disableCollisions, float separationTime );
	static CCorpseSolver *Create( CBaseEntity *pCorpse, CBaseEntity *pPhysicsObject, bool disableCollisions, float separationTime );

	// CBaseEntity
	virtual void Spawn();
	virtual void UpdateOnRemove();
	virtual void Think();
	virtual void OnRestore()
	{
		BaseClass::OnRestore();
		if ( m_allowIntersection )
		{
			PhysDisableEntityCollisions( m_hCorpse, m_hEntity );
		}
	}

	// IMotionEvent
	virtual simresult_e	Simulate( IPhysicsMotionController *pController, IPhysicsObject *pObject, float deltaTime, Vector &linear, AngularImpulse &angular );

public:
	CCorpseSolver *m_pNext;
private:
	// locals
	void BecomePenetrationSolver();
	bool IsIntersecting();
	bool IsContactOnNPCHead( IPhysicsFrictionSnapshot *pSnapshot, IPhysicsObject *pPhysics, CAI_BaseNPC *pNPC );
	bool CheckTouching();
	friend bool NPCPhysics_SolverExists( CAI_BaseNPC *pNPC, CBaseEntity *pPhysicsObject );

	EHANDLE						m_hCorpse;
	EHANDLE						m_hEntity;
	IPhysicsMotionController	*m_pController;
	float						m_separationDuration;
	bool						m_allowIntersection;
};

LINK_ENTITY_TO_CLASS( hoe_corpse_solver, CCorpseSolver );

BEGIN_DATADESC( CCorpseSolver )

	DEFINE_FIELD( m_hCorpse, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hEntity, FIELD_EHANDLE ),
	DEFINE_FIELD( m_separationDuration, FIELD_FLOAT ),
	DEFINE_FIELD( m_allowIntersection, FIELD_BOOLEAN ),
	DEFINE_PHYSPTR( m_pController ),
	//DEFINE_FIELD( m_pNext, FIELD_CLASSPTR ),

END_DATADESC()

CCorpseSolver *CCorpseSolver::Create( CBaseEntity *pCorpse, CBaseEntity *pPhysicsObject, bool disableCollisions, float separationTime )
{
	CCorpseSolver *pSolver = (CCorpseSolver *)CBaseEntity::CreateNoSpawn( "hoe_corpse_solver", vec3_origin, vec3_angle, NULL );
	pSolver->Init( pCorpse, pPhysicsObject, disableCollisions, separationTime );
	pSolver->Spawn();
	//NDebugOverlay::EntityBounds(pNPC, 255, 255, 0, 64, 0.5f );
	return pSolver;
}

CCorpseSolver::CCorpseSolver()
{
}

CCorpseSolver::~CCorpseSolver()
{
}

void CCorpseSolver::Init( CBaseEntity *pCorpse, CBaseEntity *pPhysicsObject, bool disableCollisions, float separationTime )
{
	m_hCorpse = pCorpse;
	m_hEntity = pPhysicsObject;
	m_pController = NULL;
	m_separationDuration = separationTime;
	m_allowIntersection = disableCollisions;

}

void CCorpseSolver::BecomePenetrationSolver()
{
	CBaseEntity *pEntity = m_hEntity.Get();
	if ( pEntity )
	{
		m_allowIntersection = true;
		IPhysicsObject *pList[VPHYSICS_MAX_OBJECT_LIST_COUNT];
		int listCount = pEntity->VPhysicsGetObjectList( pList, ARRAYSIZE(pList) );
		PhysDisableEntityCollisions( m_hCorpse, pEntity );
		m_pController = physenv->CreateMotionController( this );
		for ( int i = 0; i < listCount; i++ )
		{
			m_pController->AttachObject( pList[i], false );
			pList[i]->Wake();
		}
		m_pController->SetPriority( IPhysicsMotionController::HIGH_PRIORITY );
	}
}

void CCorpseSolver::Spawn()
{
	if ( m_allowIntersection )
	{
		BecomePenetrationSolver();
	}
	else
	{
//		m_hEntity->SetNavIgnore();
	}
	SetNextThink( gpGlobals->curtime );
}

void CCorpseSolver::UpdateOnRemove()
{
	if ( m_allowIntersection )
	{
		physenv->DestroyMotionController( m_pController );
		m_pController = NULL;
		PhysEnableEntityCollisions( m_hCorpse, m_hEntity );
	}
	else
	{
//		if ( m_hEntity.Get() )
//		{
//			m_hEntity->ClearNavIgnore();
//		}
	}
	//NDebugOverlay::EntityBounds(m_hNPC, 0, 255, 0, 64, 0.5f );
	BaseClass::UpdateOnRemove();
}

bool CCorpseSolver::IsIntersecting()
{
	CBaseEntity *pCorpse = m_hCorpse.Get();
	if ( !pCorpse )
		return false;

	CBaseEntity *pPhysics = m_hEntity.Get();
	if ( !pPhysics || !pPhysics->VPhysicsGetObject() )
		return false;

#if 1
	IPhysicsObject *pList[VPHYSICS_MAX_OBJECT_LIST_COUNT];
	int listCount = pCorpse->VPhysicsGetObjectList( pList, ARRAYSIZE(pList) );

	Vector entityOrg;
	QAngle entityAng;
	pPhysics->VPhysicsGetObject()->GetPosition( &entityOrg, &entityAng );
	const CPhysCollide *pEntityCollide = pPhysics->VPhysicsGetObject()->GetCollide();

	for ( int i = 0; i < listCount; i++ )
	{
		// Head might be gone
		if ( !pList[i]->IsCollisionEnabled() )
			continue;

		Vector listOrg;
		QAngle listAng;
		pList[i]->GetPosition( &listOrg, &listAng );

		trace_t tr;
		physcollision->TraceCollide( entityOrg, entityOrg, pEntityCollide, entityAng,
			pList[i]->GetCollide(), listOrg, listAng, &tr );
#if 0
		Vector mins, maxs;
		physcollision->CollideGetAABB( &mins, &maxs, pEntityCollide, entityOrg, entityAng );
		NDebugOverlay::Box( vec3_origin, mins, maxs, 255, 0, 0, 0, 0.05f );
#endif
		if ( tr.startsolid )
			return true;
	}
#else
	if ( pCorpse && pPhysics )
	{
		Ray_t ray;
		// bloated bounds to force slight separation
		Vector mins = pCorpse->WorldAlignMins() - Vector(1,1,1);
		Vector maxs = pCorpse->WorldAlignMaxs() + Vector(1,1,1);

		ray.Init( pCorpse->GetAbsOrigin(), pCorpse->GetAbsOrigin(), mins, maxs );
		trace_t tr;
		enginetrace->ClipRayToEntity( ray, pCorpse->PhysicsSolidMaskForEntity(), pPhysics, &tr );
		if ( tr.startsolid )
			return true;
	}
#endif
	return false;
}

bool CCorpseSolver::CheckTouching()
{
	CBaseEntity *pCorpse = m_hCorpse.Get();
	if ( !pCorpse )
		return false;

	CBaseEntity *pPhysicsEnt = m_hEntity.Get();
	if ( !pPhysicsEnt )
		return false;

	IPhysicsObject *pPhysics = pPhysicsEnt->VPhysicsGetObject();
	IPhysicsObject *pNPCPhysics = pCorpse->VPhysicsGetObject();
	if ( !pNPCPhysics || !pPhysics )
		return false;

	IPhysicsObject *pList[VPHYSICS_MAX_OBJECT_LIST_COUNT];
	int listCount = pCorpse->VPhysicsGetObjectList( pList, ARRAYSIZE(pList) );

	IPhysicsFrictionSnapshot *pSnapshot = pPhysics->CreateFrictionSnapshot();
	bool found = false;
	bool penetrate = false;

	while ( pSnapshot->IsValid() )
	{
		IPhysicsObject *pOther = pSnapshot->GetObject(1);

		for ( int i = 0; i < listCount; i++ )
		{
			if ( pOther == pList[i] )
			{
				found = true;
				break;
			}
		}
		pSnapshot->NextFrictionData();
	}
	pSnapshot->DeleteAllMarkedContacts( true );
	pPhysics->DestroyFrictionSnapshot( pSnapshot );

	// if the object is penetrating something, check to see if it's intersecting this NPC
	// if so, go ahead and switch over to penetration solver mode
	if ( !penetrate && (pPhysics->GetGameFlags() & FVPHYSICS_PENETRATING) )
	{
		penetrate = IsIntersecting();
	}

	if ( penetrate )
	{
//		pPhysicsEnt->ClearNavIgnore();
		BecomePenetrationSolver();
	}

	return found;
}

void CCorpseSolver::Think()
{
	bool finished = m_allowIntersection ? !IsIntersecting() : !CheckTouching();

	if ( finished )
	{
		UTIL_Remove(this);
		return;
	}
	if ( m_allowIntersection )
	{
		IPhysicsObject *pObject = m_hEntity->VPhysicsGetObject();
		if ( !pObject )
		{
			UTIL_Remove(this);
			return;
		}
		pObject->Wake();
	}

	SetNextThink( gpGlobals->curtime );
}

IMotionEvent::simresult_e CCorpseSolver::Simulate( IPhysicsMotionController *pController, IPhysicsObject *pObject, 
													  float deltaTime, Vector &linear, AngularImpulse &angular )
{
	if ( IsIntersecting() )
	{
		const float PUSH_SPEED = 150.0f;

		if ( pObject->GetGameFlags() & FVPHYSICS_PLAYER_HELD )
		{
			CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
			if ( pPlayer )
			{
				pPlayer->ForceDropOfCarriedPhysObjects( m_hEntity );
			}
		}

//		ResetCancelTime();
		angular.Init();
		linear.Init();
		
		// Don't push on vehicles because they won't move
		if ( pObject->GetGameFlags() & FVPHYSICS_MULTIOBJECT_ENTITY )
		{
			if ( m_hEntity->GetServerVehicle() )
				return SIM_NOTHING;
		}

		Vector origin, vel;
		pObject->GetPosition( &origin, NULL );
		pObject->GetVelocity( &vel, NULL );
		Vector dir = origin - m_hCorpse->GetAbsOrigin();
		dir.z = dir.z > 0 ? 0.1f : -0.1f;
		VectorNormalize(dir);
		AngularImpulse angVel;
		angVel.Init();

		// NOTE: Iterate this object's contact points 
		// if it can't move in this direction, try sliding along the plane/crease
		Vector pushImpulse;
		PhysComputeSlideDirection( pObject, dir * PUSH_SPEED, angVel, &pushImpulse, NULL, 0 );

		dir = pushImpulse;
		VectorNormalize(dir);

		if ( DotProduct( vel, dir ) < PUSH_SPEED * 0.5f )
		{
			linear = pushImpulse;
//			if ( pObject->GetContactPoint(NULL,NULL) )
			{
				linear.z += sv_gravity.GetFloat();
			}
		}
		return SIM_GLOBAL_ACCELERATION;
	}
	return SIM_NOTHING;
}


CBaseEntity *CreateCorpseSolver( CBaseEntity *pCorpse, CBaseEntity *pPhysicsObject, bool disableCollisions, float separationDuration )
{
	if ( disableCollisions )
	{
		if ( PhysEntityCollisionsAreDisabled( pCorpse, pPhysicsObject ) )
			return NULL;
	}
	else
	{
//		if ( pPhysicsObject->IsNavIgnored() )
//			return NULL;
	}
	return CCorpseSolver::Create( pCorpse, pPhysicsObject, disableCollisions, separationDuration );
}

/* This creates a collor_correction entity on every level to enable/disable greyscale. */
static void HoeColorCorrectionWeightChanged( IConVar *pConVar, char const *pOldString, float flOldValue )
{
	CBaseEntity *pEnt = gEntList.FindEntityByName( NULL, "hoe_color_correction_entity" );
	if ( pEnt != NULL )
	{
		ConVarRef var( pConVar );
		float weight = clamp( var.GetFloat(), 0.0f, 1.0f );
		pEnt->KeyValue( "maxweight", weight  );
		variant_t value;
		pEnt->AcceptInput( weight > 0 ? "Enable" : "Disable", pEnt, pEnt, value, 0 );
	}
}
ConVar hoe_color_correction_weight( "hoe_color_correction_weight", "0.0", FCVAR_ARCHIVE, "",
								   HoeColorCorrectionWeightChanged );

class CColorCorrectionInit : public CAutoGameSystem
{
public:
	CColorCorrectionInit( char const *name ) : CAutoGameSystem( name )
	{
	}

	virtual void LevelInitPostEntity() 
	{
		CBaseEntity *pEnt = gEntList.FindEntityByName( NULL, "hoe_color_correction_entity" );
		if ( pEnt == NULL )
		{
			pEnt = CreateEntityByName( "color_correction" );
			pEnt->KeyValue( "targetname", "hoe_color_correction_entity" );
			pEnt->KeyValue( "StartDisabled", "1" );
			pEnt->KeyValue( "fadeInDuration", "0.0" );
			pEnt->KeyValue( "fadeOutDuration", "0.0" );
			pEnt->KeyValue( "filename", "materials/colorcorrection.raw" );
			pEnt->KeyValue( "minfalloff", "-1" );
			pEnt->KeyValue( "maxfalloff", "-1" );
			pEnt->KeyValue( "maxweight", "1.0" );
			DispatchSpawn( pEnt );
			pEnt->Activate();
		}

		// CColorCorrection::Spawn (incorrectly?) set the current weight to 1.0, ignoring maxweight.
		variant_t value;
		float weight = clamp( hoe_color_correction_weight.GetFloat(), 0.0f, 1.0f );
		if ( weight > 0 )
		{
			pEnt->KeyValue( "maxweight", weight  );
			pEnt->AcceptInput( "Enable", pEnt, pEnt, value, 0 );
		}
		else
		{
			pEnt->AcceptInput( "Disable", pEnt, pEnt, value, 0 );
		}
	}
	virtual void LevelShutdownPreEntity() 
	{
	}
};

CColorCorrectionInit g_ColorCorrectionInit( "CColorCorrectionInit" );