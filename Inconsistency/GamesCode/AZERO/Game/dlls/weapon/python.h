

#ifndef PYTHON_H
#define PYTHON_H

#include "extdll.h"
#include "weapons.h"
#include "rpg.h"   // for CLaserSpot


enum python_e {
	PYTHON_IDLE1 = 0,
	PYTHON_FIDGET,
	PYTHON_FIRE1,
	PYTHON_RELOAD,
	PYTHON_HOLSTER,
	PYTHON_DRAW,
	PYTHON_IDLE2,
	PYTHON_IDLE3
};




class CPython : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int iItemSlot(void) { return 2; }
	int GetItemInfo(ItemInfo* p);
	int AddToPlayer(CBasePlayer* pPlayer);
	void PrimaryAttack(void);
	void SecondaryAttack(void);
	BOOL Deploy(void);
	void Holster(int skiplocal = 0);
	void Reload(void);

	//MODDD - added.
	void customAttachToPlayer(CBasePlayer* pPlayer);
	CPython();

	void ItemPostFrameThink(void);

	void ItemPostFrame();
	void ItemPreFrame();
	void spotDeleteCheck();
	void updateModel();
	void UpdateSpot();


	void WeaponIdle(void);

	float m_flSoundDelay;

	//MODDD - added.
	int m_fSpotActive;
	int m_iShell;
	CLaserSpot* m_pSpot;


	BOOL m_fInZoom;// don't save this. 

	virtual BOOL UseDecrement(void)
	{
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usFirePython;
};



#endif //PYTHON_H
