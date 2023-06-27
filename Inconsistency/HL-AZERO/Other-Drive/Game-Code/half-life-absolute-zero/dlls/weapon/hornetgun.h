

#ifndef HORNETGUN_H
#define HORNETGUN_H


#include "extdll.h"
#include "weapons.h"




enum hgun_e {
	HGUN_IDLE1 = 0,
	HGUN_FIDGETSWAY,
	HGUN_FIDGETSHAKE,
	HGUN_DOWN,
	HGUN_UP,
	HGUN_SHOOT
};

enum hgun_firemode_e
{
	FIREMODE_TRACK = 0,
	FIREMODE_FAST
};



class CHgun : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int iItemSlot(void) { return 4; }
	int GetItemInfo(ItemInfo* p);

	//MODDD
	void customAttachToPlayer(CBasePlayer* pPlayer);

	int AddToPlayer(CBasePlayer* pPlayer);

	void PrimaryAttack(void);
	void SecondaryAttack(void);
	BOOL Deploy(void);
	BOOL IsUseable(void);
	void Holster(int skiplocal = 0);
	void Reload(void);
	void ItemPostFrameThink(void);
	void WeaponIdle(void);
	float m_flNextAnimTime;

	//MODDD - replcaed with a better synch'd var.  or, not?
	float m_flRechargeTime;

	int m_iFirePhase;// don't save me.

	virtual BOOL UseDecrement(void)
	{
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}
private:
	unsigned short m_usHornetFire;
};




#endif //HORNETGUN_H
