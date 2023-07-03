

#ifndef MP5_H
#define MP5_H

#include "extdll.h"
#include "weapons.h"


enum mp5_e
{
	MP5_LONGIDLE = 0,
	MP5_IDLE1,
	MP5_LAUNCH,
	MP5_RELOAD,
	MP5_DEPLOY,
	MP5_FIRE1,
	MP5_FIRE2,
	MP5_FIRE3,
	//MODDD - new
	MP5_HOLSTER

};






class CMP5 : public CBasePlayerWeapon
{
private:
	unsigned short m_usMP5;
	unsigned short m_usMP52;

public:
	//MODDD
	void customAttachToPlayer(CBasePlayer* pPlayer);

	void Spawn(void);
	void Precache(void);
	int iItemSlot(void) { return 3; }
	int GetItemInfo(ItemInfo* p);
	int AddToPlayer(CBasePlayer* pPlayer);

	void PrimaryAttack(void);
	void SecondaryAttack(void);
	
	BOOL Deploy(void);
	//MODDD - new
	void Holster(int skiplocal = 0);

	void Reload(void);
	void WeaponIdle(void);
	//MODDD - UNUSED VAR, removed.
	//float m_flNextAnimTime;
	int m_iShell;

	virtual BOOL UseDecrement(void)
	{
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

};




#endif //MP5_H
