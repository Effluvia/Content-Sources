
#ifndef SATCHEL_H
#define SATCHEL_H

#include "extdll.h"
#include "weapons.h"



enum satchel_e {
	SATCHEL_IDLE1 = 0,
	SATCHEL_FIDGET1,
	SATCHEL_DRAW,
	SATCHEL_DROP
};

enum satchel_radio_e {
	SATCHEL_RADIO_IDLE1 = 0,
	SATCHEL_RADIO_FIDGET1,
	SATCHEL_RADIO_DRAW,
	SATCHEL_RADIO_FIRE,
	SATCHEL_RADIO_HOLSTER
};



//MODDD - serverside-only method now, and extern here.  May as well.
#ifndef CLIENT_DLL
extern void DeactivateSatchels(CBasePlayer* pOwner);
#endif


class CSatchel : public CBasePlayerWeapon
{
public:

	BOOL sentOutOfAmmoHolster;



#ifndef CLIENT_DLL
	int	Save(CSave& save);
	int	Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];
#endif

	void Spawn(void);
	void Precache(void);
	int iItemSlot(void) { return 5; }
	int GetItemInfo(ItemInfo* p);

	//MODDD
	CSatchel();
	void customAttachToPlayer(CBasePlayer* pPlayer);

	int AddToPlayer(CBasePlayer* pPlayer);
	void PrimaryAttack(void);
	void SecondaryAttack(void);
	int AddDuplicate(CBasePlayerItem* pOriginal);
	BOOL CanDeploy(void);
	BOOL Deploy(void);
	BOOL IsUseable(void);

	void Holster(int skiplocal = 0);
	void WeaponIdle(void);
	void Throw(void);

	void CheckOutOfAmmo(void);
	// there is also ItemPostFrame, it runs when m_flNextAttack is off.
	void ItemPostFrameThink(void);

	void ReDeploySatchel(void);

	virtual BOOL UseDecrement(void)
	{
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}
};




// physical entities are serverside-only.
#ifndef CLIENT_DLL

class CSatchelCharge : public CGrenade
{
public:
	float nextBounceSoundAllowed;


	void Spawn(void);
	void Precache(void);
	void BounceSound(void);

	void EXPORT SatchelSlide(CBaseEntity* pOther);
	void EXPORT SatchelThink(void);

	float massInfluence(void);
	int GetProjectileType(void);

public:
	void Deactivate(void);
};

#endif


#endif //SATCHEL_H
