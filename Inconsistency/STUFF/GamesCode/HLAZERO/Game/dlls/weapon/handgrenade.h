
// To be clear, yes, this (CHandGrenade) is the viewmodel/weapon, not the projectile.

#ifndef HANDGRENADE_H
#define HANDGRENADE_H


#include "extdll.h"
#include "weapons.h"



enum handgrenade_e {
	HANDGRENADE_IDLE = 0,
	HANDGRENADE_FIDGET,
	HANDGRENADE_PINPULL,
	HANDGRENADE_THROW1,	// toss
	HANDGRENADE_THROW2,	// medium
	HANDGRENADE_THROW3,	// hard
	HANDGRENADE_HOLSTER,
	HANDGRENADE_DRAW
};

#define HANDGRENADE_PRIMARY_VOLUME 450





class CHandGrenade : public CBasePlayerWeapon
{
public:



	//MODDD - new.
	CHandGrenade(void);

#ifndef CLIENT_DLL
	int	Save(CSave& save);
	int	Restore(CRestore& restore);
	static TYPEDESCRIPTION m_SaveData[];
#endif


	//MODDD
	void customAttachToPlayer(CBasePlayer* pPlayer);
	BOOL ExtractAmmo(CBasePlayerWeapon* pWeapon);
	BOOL AddPrimaryAmmo(int iCount, char* szName, int iMaxClip, int iMaxCarry);
	BOOL AddPrimaryAmmo(int iCount, char* szName, int iMaxClip, int iMaxCarry, int forcePickupSound);
	


	void Spawn(void);
	void Precache(void);
	int iItemSlot(void) { return 5; }
	int GetItemInfo(ItemInfo* p);

	void PrimaryAttack(void);
	void SecondaryAttack(void);
	void EitherAttack(void);
	BOOL Deploy(void);
	BOOL CanHolster(void);
	void Holster(int skiplocal = 0);
	void WeaponIdle(void);


	void ItemPreFrame(void);
	void ItemPostFrame(void);
	void ItemPostFrameThink(void);


	virtual BOOL UseDecrement(void)
	{
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}
};



#endif //HANDGRENADE_H