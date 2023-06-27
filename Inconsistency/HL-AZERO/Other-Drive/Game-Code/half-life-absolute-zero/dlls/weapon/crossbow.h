
#ifndef CROSSBOW_H
#define CROSSBOW_H

#include "extdll.h"
//#include "util.h"
//#include "cbase.h"
#include "weapons.h"




enum crossbow_e {
	CROSSBOW_IDLE1 = 0,	// full
	CROSSBOW_IDLE2,		// empty
	CROSSBOW_FIDGET1,	// full
	CROSSBOW_FIDGET2,	// empty
	CROSSBOW_FIRE1,		// full
	CROSSBOW_FIRE2,		// reload
	CROSSBOW_FIRE3,		// empty
	CROSSBOW_RELOAD,	// from empty
	CROSSBOW_DRAW1,		// full
	CROSSBOW_DRAW2,		// empty
	CROSSBOW_HOLSTER1,	// full
	CROSSBOW_HOLSTER2,	// empty
};



class CCrossbow : public CBasePlayerWeapon
{
public:

	//int testcrossbowvar;

	//MODDD - new.
	CCrossbow(void);

	
	/*
#ifndef CLIENT_DLL
	int	Save(CSave& save);
	int	Restore(CRestore& restore);
	static TYPEDESCRIPTION m_SaveData[];
#endif
	*/


	void Spawn(void);
	void Precache(void);
	int iItemSlot() { return 3; }
	int GetItemInfo(ItemInfo* p);

	void FireBolt(void);
	void FireSniperBolt(void);
	void PrimaryAttack(void);
	void SecondaryAttack(void);
	//MODDD
	void customAttachToPlayer(CBasePlayer* pPlayer);
	int AddToPlayer(CBasePlayer* pPlayer);
	BOOL Deploy();
	void Holster(int skiplocal = 0);
	void Reload(void);
	void WeaponIdle(void);

	int m_fInZoom; // don't save this

	//MODDD - new.
	float crossbowReloadSoundTimer;

	//MODDD - new.
	void ItemPostFrameThink(void);
	void ItemPostFrame(void);


	virtual BOOL UseDecrement(void)
	{
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usCrossbow;
	unsigned short m_usCrossbow2;
};




#endif //CROSSBOW_H