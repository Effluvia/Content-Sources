#include "cbase.h"
#include "c_basehlcombatweapon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_WeaponHandGrenade : public C_BaseHLCombatWeapon
{
public:
	DECLARE_CLASS( C_WeaponHandGrenade, C_BaseHLCombatWeapon );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_DATADESC();
#ifdef HOE_THIRDPERSON
	DECLARE_ACTTABLE();	
#endif // HOE_THIRDPERSON

	C_WeaponHandGrenade() {};
	~C_WeaponHandGrenade() {};

	Activity ActivityOverride( Activity baseAct, bool *pRequired )
	{
#if 0
		// Grenade is drawn back for throwing
		if ( baseAct == ACT_IDLE_ANGRY && m_fDrawbackFinished )
			return ACT_HL2MP_IDLE_GRENADE;

		// Grenade is drawn back for throwing
		if ( baseAct == ACT_RUN_AIM && m_fDrawbackFinished )
			return ACT_HL2MP_RUN_GRENADE;
#endif
		return BaseClass::ActivityOverride( baseAct, pRequired );
	}

	bool m_fDrawbackFinished;
};

BEGIN_PREDICTION_DATA( C_WeaponHandGrenade )
END_PREDICTION_DATA()

#ifdef HOE_THIRDPERSON
acttable_t C_WeaponHandGrenade::m_acttable[] = 
{
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_MP_GRENADE1_ATTACK, true },
	{ ACT_GESTURE_RANGE_ATTACK2, ACT_MP_GRENADE2_ATTACK, true },
	{ ACT_MP_GRENADE1_DRAW, ACT_MP_GRENADE1_DRAW, false },
	{ ACT_MP_GRENADE2_DRAW, ACT_MP_GRENADE2_DRAW, false },
	{ ACT_IDLE,				ACT_IDLE_ANGRY_MELEE,	false },
	{ ACT_IDLE_ANGRY,		ACT_IDLE_ANGRY_MELEE,	false },
	{ ACT_RUN_AIM,			ACT_HL2MP_RUN_MELEE, false },
};
IMPLEMENT_ACTTABLE(C_WeaponHandGrenade);
#endif // HOE_THIRDPERSON

IMPLEMENT_CLIENTCLASS_DT( C_WeaponHandGrenade, DT_WeaponHandGrenade, CWeaponHandGrenade )
	RecvPropBool( RECVINFO( m_fDrawbackFinished ) ),
END_RECV_TABLE()

BEGIN_DATADESC( C_WeaponHandGrenade )
	DEFINE_FIELD( m_fDrawbackFinished, FIELD_BOOLEAN ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( weapon_handgrenade, C_WeaponHandGrenade );