#include "cbase.h"
#include "c_weapon_870.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_PREDICTION_DATA( C_Weapon870 )
END_PREDICTION_DATA()

IMPLEMENT_CLIENTCLASS_DT( C_Weapon870, DT_Weapon870, CWeapon870 )
	RecvPropArray3( RECVINFO_ARRAY(m_AmmoQueue), RecvPropInt(RECVINFO(m_AmmoQueue[0]))),
END_RECV_TABLE()

LINK_ENTITY_TO_CLASS( weapon_870, C_Weapon870 );

#ifdef HOE_THIRDPERSON
acttable_t C_Weapon870::m_acttable[] = 
{
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },
//	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SMG1,			true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_RIFLE,				true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_SHOTGUN,	true },
};
IMPLEMENT_ACTTABLE(C_Weapon870);
#endif // HOE_THIRDPERSON

C_Weapon870::C_Weapon870()
{
}