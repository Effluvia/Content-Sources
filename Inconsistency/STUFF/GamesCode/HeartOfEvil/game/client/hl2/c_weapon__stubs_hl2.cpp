//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "c_weapon__stubs.h"
#include "basehlcombatweapon_shared.h"
#include "c_basehlcombatweapon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

STUB_WEAPON_CLASS( cycler_weapon, WeaponCycler, C_BaseCombatWeapon );

STUB_WEAPON_CLASS( weapon_binoculars, WeaponBinoculars, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_bugbait, WeaponBugBait, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_flaregun, Flaregun, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_annabelle, WeaponAnnabelle, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_gauss, WeaponGaussGun, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_cubemap, WeaponCubemap, C_BaseCombatWeapon );
STUB_WEAPON_CLASS( weapon_alyxgun, WeaponAlyxGun, C_HLSelectFireMachineGun );
STUB_WEAPON_CLASS( weapon_citizenpackage, WeaponCitizenPackage, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_citizensuitcase, WeaponCitizenSuitcase, C_WeaponCitizenPackage );

#ifndef HL2MP
STUB_WEAPON_CLASS( weapon_ar2, WeaponAR2, C_HLMachineGun );
STUB_WEAPON_CLASS( weapon_frag, WeaponFrag, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_rpg, WeaponRPG, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_pistol, WeaponPistol, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_shotgun, WeaponShotgun, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_smg1, WeaponSMG1, C_HLSelectFireMachineGun );
STUB_WEAPON_CLASS( weapon_357, Weapon357, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_crossbow, WeaponCrossbow, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_slam, Weapon_SLAM, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_crowbar, WeaponCrowbar, C_BaseHLBludgeonWeapon );
#ifdef HL2_EPISODIC
STUB_WEAPON_CLASS( weapon_hopwire, WeaponHopwire, C_BaseHLCombatWeapon );
//STUB_WEAPON_CLASS( weapon_proto1, WeaponProto1, C_BaseHLCombatWeapon );
#endif
#ifdef HL2_LOSTCOAST
STUB_WEAPON_CLASS( weapon_oldmanharpoon, WeaponOldManHarpoon, C_WeaponCitizenPackage );
#endif
#endif

#ifdef HOE_DLL
//STUB_WEAPON_CLASS( weapon_870, Weapon870, C_BaseHLCombatWeapon );
//STUB_WEAPON_CLASS( weapon_handgrenade, WeaponHandGrenade, C_BaseHLCombatWeapon );
//STUB_WEAPON_CLASS( weapon_letter, WeaponLetter, C_BaseHLCombatWeapon );
#ifndef HOE_THIRDPERSON
STUB_WEAPON_CLASS( weapon_chainsaw, WeaponChainsaw, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_colt1911A1, Colt1911A1, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_ak47, WeaponAK47, C_HLMachineGun );
STUB_WEAPON_CLASS( weapon_m16, WeaponM16, C_HLMachineGun );
STUB_WEAPON_CLASS( weapon_m60, WeaponM60, C_HLMachineGun );
STUB_WEAPON_CLASS( weapon_m79, WeaponM79, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_rpg7, WeaponRPG7, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_machete, WeaponMachete, C_BaseHLBludgeonWeapon );
STUB_WEAPON_CLASS( weapon_tripmine, WeaponTripmine, C_BaseHLCombatWeapon );
#endif
//STUB_WEAPON_CLASS( weapon_m21, WeaponM21, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_snark, WeaponSnark, C_BaseHLCombatWeapon );
#endif // HOE_DLL

#ifdef HOE_THIRDPERSON

#define ACTIVITY_OVERRIDE( className ) \
	Activity className##::ActivityOverride( Activity baseAct, bool *pRequired ) { return BaseClass::ActivityOverride( baseAct, pRequired ); }
#define STUB_WEAPON_CLASS_WITH_ACTTABLE( entityName, className, baseClassName )	\
	class C_##className## : public baseClassName					\
	{																\
		DECLARE_CLASS( C_##className##, baseClassName );			\
	public:															\
		DECLARE_PREDICTABLE();										\
		DECLARE_CLIENTCLASS();										\
		DECLARE_ACTTABLE();											\
		C_##className() {};											\
		Activity ActivityOverride( Activity baseAct, bool *pRequired ); \
	private:														\
		C_##className( const C_##className & );						\
	};																\
	STUB_WEAPON_CLASS_IMPLEMENT( entityName, C_##className );		\
	IMPLEMENT_CLIENTCLASS_DT( C_##className, DT_##className, C##className )	\
	END_RECV_TABLE()

STUB_WEAPON_CLASS_WITH_ACTTABLE( weapon_ak47, WeaponAK47, C_HLMachineGun );
acttable_t C_WeaponAK47::m_acttable[] = 
{
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },
//	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SMG1,			true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_RIFLE,				true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_SMG1,	true },
};
IMPLEMENT_ACTTABLE(C_WeaponAK47);
ACTIVITY_OVERRIDE(C_WeaponAK47);

STUB_WEAPON_CLASS_WITH_ACTTABLE( weapon_chainsaw, WeaponChainsaw, C_BaseHLCombatWeapon );
acttable_t C_WeaponChainsaw::m_acttable[] = 
{
	{ ACT_IDLE_ANGRY,				ACT_CHAINSAW_IDLE_AIM,				true },
	{ ACT_RUN_AIM,					ACT_CHAINSAW_RUN,					true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_CHAINSAW_ATTACK,					true },
};
IMPLEMENT_ACTTABLE(C_WeaponChainsaw);
ACTIVITY_OVERRIDE(C_WeaponChainsaw);

#if 0
STUB_WEAPON_CLASS_WITH_ACTTABLE( weapon_colt1911A1, Colt1911A1, C_BaseHLCombatWeapon );
#define RECV_TABLE()
acttable_t C_Colt1911A1::m_acttable[] = 
{
//	{ ACT_IDLE,						ACT_IDLE_PISTOL,				true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_PISTOL,			true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_PISTOL,				true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_PISTOL,	true },
	{ ACT_GESTURE_RANGE_ATTACK2,	ACT_MP_ATTACK_STAND_SECONDARYFIRE,		true },
	{ ACT_MP_GRENADE2_DRAW,			ACT_MP_ATTACK_STAND_MELEE_SECONDARY,	false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_PISTOL,		false },
};
IMPLEMENT_ACTTABLE(C_Colt1911A1);
#endif

STUB_WEAPON_CLASS_WITH_ACTTABLE( weapon_m16, WeaponM16, C_HLMachineGun );
acttable_t C_WeaponM16::m_acttable[] = 
{
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },
//	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SMG1,			true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_RIFLE,				true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_SMG1,	true },
};
IMPLEMENT_ACTTABLE(C_WeaponM16);
ACTIVITY_OVERRIDE(C_WeaponM16);

#if 0 // m21 has its own client-side file
STUB_WEAPON_CLASS_WITH_ACTTABLE( weapon_m21, WeaponM21, C_HLMachineGun );
acttable_t C_WeaponM21::m_acttable[] = 
{
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },
//	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SMG1,			true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_RIFLE,				true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_SMG1,	true },
};
IMPLEMENT_ACTTABLE(C_WeaponM21);
#endif

STUB_WEAPON_CLASS_WITH_ACTTABLE( weapon_m60, WeaponM60, C_HLMachineGun );
acttable_t C_WeaponM60::m_acttable[] = 
{
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },
	{ ACT_IDLE_ANGRY,				ACT_RANGE_AIM_AR2_LOW,			true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_SHOTGUN,			true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_AR2,	true },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SMG1,		true },
};
IMPLEMENT_ACTTABLE(C_WeaponM60);
ACTIVITY_OVERRIDE(C_WeaponM60);

STUB_WEAPON_CLASS_WITH_ACTTABLE( weapon_m79, WeaponM79, C_BaseHLCombatWeapon );
acttable_t C_WeaponM79::m_acttable[] = 
{
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },
//	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SMG1,			true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_RIFLE,				true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_SMG1,	true },
};
IMPLEMENT_ACTTABLE(C_WeaponM79);
ACTIVITY_OVERRIDE(C_WeaponM79);

STUB_WEAPON_CLASS_WITH_ACTTABLE( weapon_machete, WeaponMachete, C_BaseHLBludgeonWeapon );
acttable_t C_WeaponMachete::m_acttable[] = 
{
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_MELEE_ATTACK_SWING, true },
	{ ACT_IDLE,				ACT_IDLE_ANGRY_MELEE,	false },
	{ ACT_IDLE_ANGRY,		ACT_IDLE_ANGRY_MELEE,	false },
	{ ACT_RUN_AIM,			ACT_HL2MP_RUN_MELEE, false },
};
IMPLEMENT_ACTTABLE(C_WeaponMachete);
ACTIVITY_OVERRIDE(C_WeaponMachete);

STUB_WEAPON_CLASS_WITH_ACTTABLE( weapon_rpg7, WeaponRPG7, C_BaseHLCombatWeapon );
acttable_t C_WeaponRPG7::m_acttable[] = 
{
	{ ACT_IDLE,						ACT_IDLE_RPG,					true },
	{ ACT_GESTURE_RELOAD,			ACT_DOD_RELOAD_BAZOOKA,			true },
//	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SMG1,			true },
	{ ACT_RUN_AIM,					ACT_RUN_RIFLE,					true }, // not aiming
//	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_SMG1,	true },
};
IMPLEMENT_ACTTABLE(C_WeaponRPG7);
ACTIVITY_OVERRIDE(C_WeaponRPG7);

STUB_WEAPON_CLASS_WITH_ACTTABLE( weapon_tripmine, WeaponTripmine, C_BaseHLCombatWeapon );
acttable_t C_WeaponTripmine::m_acttable[] = 
{
	{ ACT_MP_ATTACK_STAND_PREFIRE, ACT_SLAM_TRIPMINE_TO_STICKWALL_ND, false },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_TRIPWIRE, true },
	{ ACT_IDLE,				ACT_IDLE_ANGRY_MELEE,	false },
	{ ACT_IDLE_ANGRY,		ACT_IDLE_ANGRY_MELEE,	false },
	{ ACT_RUN_AIM,			ACT_HL2MP_RUN_MELEE, false },
};
IMPLEMENT_ACTTABLE(C_WeaponTripmine);
ACTIVITY_OVERRIDE(C_WeaponTripmine);

#endif // HOE_THIRDPERSON
