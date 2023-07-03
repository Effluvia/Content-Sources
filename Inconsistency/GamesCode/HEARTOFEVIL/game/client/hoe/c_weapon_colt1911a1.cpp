#include "cbase.h"
#include "c_basehlcombatweapon.h"
#include "c_basehlplayer.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef HOE_THIRDPERSON

#define ACT_SECONDARY_PREFIRE_STAND_COLT ACT_MP_ATTACK_STAND_MELEE_SECONDARY
#define ACT_SECONDARY_ATTACK_STAND_COLT ACT_MP_ATTACK_STAND_SECONDARYFIRE
#define ACT_SECONDARY_PREFIRE_MOVE_COLT ACT_SECONDARY_VM_PULLBACK
#define ACT_SECONDARY_ATTACK_MOVE_COLT ACT_SECONDARY_VM_SECONDARYATTACK

class C_WeaponColt1911A1 : public C_BaseHLCombatWeapon
{
public:
	DECLARE_CLASS( C_WeaponColt1911A1, C_BaseHLCombatWeapon );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_DATADESC();
	DECLARE_ACTTABLE();	

	C_WeaponColt1911A1() {};
	~C_WeaponColt1911A1() {};

	Activity ActivityOverride( Activity baseAct, bool *pRequired );

	bool m_fSecondaryAttack;
};

BEGIN_PREDICTION_DATA( C_WeaponColt1911A1 )
END_PREDICTION_DATA()

acttable_t C_WeaponColt1911A1::m_acttable[] = 
{
//	{ ACT_IDLE,						ACT_IDLE_PISTOL,				true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_PISTOL,			true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_PISTOL,				true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_PISTOL,	true },
	{ ACT_GESTURE_RANGE_ATTACK2,	ACT_SECONDARY_ATTACK_STAND_COLT,		true },
	{ ACT_MP_GRENADE2_DRAW,			ACT_SECONDARY_PREFIRE_STAND_COLT,	false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_PISTOL,		false },
};
IMPLEMENT_ACTTABLE(C_WeaponColt1911A1);

IMPLEMENT_CLIENTCLASS_DT( C_WeaponColt1911A1, DT_Colt1911A1, CColt1911A1 )
	RecvPropBool( RECVINFO( m_fSecondaryAttack ) ),
END_RECV_TABLE()

BEGIN_DATADESC( C_WeaponColt1911A1 )
	DEFINE_FIELD( m_fSecondaryAttack, FIELD_BOOLEAN ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( weapon_colt1911a1, C_WeaponColt1911A1 );

Activity C_WeaponColt1911A1::ActivityOverride( Activity baseAct, bool *pRequired )
{
	C_BaseHLPlayer *pPlayer = ToHL2Player( GetOwner() );
	if ( pPlayer && pPlayer->GetCurrentMainActivity() == ACT_RUN_RIFLE )
	{
		if ( baseAct == ACT_MP_GRENADE2_DRAW && m_fSecondaryAttack )
			return ACT_SECONDARY_PREFIRE_MOVE_COLT;
		if ( baseAct == ACT_GESTURE_RANGE_ATTACK2 && m_fSecondaryAttack )
			return ACT_SECONDARY_ATTACK_MOVE_COLT;
	}
//	if ( baseAct == ACT_RUN_AIM && m_fSecondaryAttack )
//		return ACT_HL2MP_RUN_MELEE;

	//HOE_SWITCH_GESTURE_WITH_MAIN_SEQUENCE
	if ( pPlayer && pPlayer->GetCurrentMainActivity() == ACT_IDLE )
	{
		if ( baseAct == ACT_SECONDARY_PREFIRE_MOVE_COLT )
			return ACT_SECONDARY_PREFIRE_STAND_COLT;
		if ( baseAct == ACT_SECONDARY_ATTACK_MOVE_COLT )
			return ACT_SECONDARY_ATTACK_STAND_COLT;
	}
	if ( pPlayer && pPlayer->GetCurrentMainActivity() == ACT_RUN_RIFLE )
	{
		if ( baseAct == ACT_SECONDARY_PREFIRE_STAND_COLT )
			return ACT_SECONDARY_PREFIRE_MOVE_COLT;
		if ( baseAct == ACT_SECONDARY_ATTACK_STAND_COLT )
			return ACT_SECONDARY_ATTACK_MOVE_COLT;
	}

	return BaseClass::ActivityOverride( baseAct, pRequired );
}
#endif // HOE_THIRDPERSON
