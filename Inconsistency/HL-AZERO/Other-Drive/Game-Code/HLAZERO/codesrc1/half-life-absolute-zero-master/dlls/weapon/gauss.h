
#ifndef GAUSS_H
#define GAUSS_H

#include "weapons.h"


#define GAUSS_PRIMARY_CHARGE_VOLUME	256// how loud gauss is while charging
#define GAUSS_PRIMARY_FIRE_VOLUME	450// how loud gauss is when discharged

enum gauss_e {
	GAUSS_IDLE = 0,
	GAUSS_IDLE2,
	GAUSS_FIDGET,
	GAUSS_SPINUP,
	GAUSS_SPIN,
	GAUSS_FIRE,
	GAUSS_FIRE2,
	GAUSS_HOLSTER,
	GAUSS_DRAW
};




class CGauss : public CBasePlayerWeapon
{
public:
	unsigned short m_usGaussFire;
	unsigned short m_usGaussSpin;
	int m_iBalls;
	int m_iGlow;
	int m_iBeam;
	int m_iSoundState; // don't save this
	// was this weapon just fired primary or secondary?
	// we need to know so we can pick the right set of effects. 
	BOOL m_fPrimaryFire;

	float fuser1_store;
	float ignoreIdleTime;
	int inAttackPrev;


#ifndef CLIENT_DLL
	int		Save(CSave& save);
	int		Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];
#endif

	CGauss(void);

	void customAttachToPlayer(CBasePlayer* pPlayer);

	void Spawn(void);
	void Precache(void);
	int iItemSlot(void) { return 4; }
	int GetItemInfo(ItemInfo* p);
	int AddToPlayer(CBasePlayer* pPlayer);

	BOOL Deploy(void);
	void Holster(int skiplocal = 0);

	void _PrimaryAttack(void);
	void _SecondaryAttack(void);


	void onFreshFrame(void);

	void ItemPreFrame(void);
	void ItemPostFrame(void);
	void ItemPostFrameThink(void);

	void WeaponIdle(void);

	void StartFire(void);
	void Fire(Vector vecOrigSrc, Vector vecDirShooting, float flDamage);
	float GetFullChargeTime(void);


	virtual BOOL UseDecrement(void)
	{
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

};



#endif //GAUSS_H