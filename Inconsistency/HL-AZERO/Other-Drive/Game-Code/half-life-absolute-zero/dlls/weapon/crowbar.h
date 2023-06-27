
#ifndef CROWBAR_H
#define CROWBAR_H

#include "extdll.h"
#include "weapons.h"


//NOTE - these look to be for the AI logic 'sound', as in how far away something can still hear
// this weapon firing (from only going to 'm_pPlayer->m_iWeaponVolume').
// Not the played (user-heard) volume.
#define CROWBAR_BODYHIT_VOLUME 128
#define CROWBAR_WALLHIT_VOLUME 512


enum crowbar_e {
	CROWBAR_IDLE = 0,
	CROWBAR_DRAW,
	CROWBAR_HOLSTER,
	CROWBAR_ATTACK1HIT,
	CROWBAR_ATTACK1MISS,
	CROWBAR_ATTACK2MISS,
	CROWBAR_ATTACK2HIT,
	CROWBAR_ATTACK3MISS,
	CROWBAR_ATTACK3HIT,
	//MODDD - two extras present in the model, may as well make available.
	CROWBAR_IDLE2,
	CROWBAR_IDLE3
};



class CCrowbar : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int iItemSlot(void) { return 1; }
	void EXPORT SwingAgain(void);
	void EXPORT Smack(void);
	int GetItemInfo(ItemInfo* p);

	void PrimaryAttack(void);
	int Swing(int fFirst);
	BOOL Deploy(void);
	void Holster(int skiplocal = 0);
	int m_iSwing;

	//MODDD
	int m_iSwingMiss;

	TraceResult m_trHit;

	//MODDD - added.
	void WeaponIdle(void);

	virtual BOOL UseDecrement(void)
	{
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}
private:
	unsigned short m_usCrowbar;
};


#endif //CROWBAR_H
