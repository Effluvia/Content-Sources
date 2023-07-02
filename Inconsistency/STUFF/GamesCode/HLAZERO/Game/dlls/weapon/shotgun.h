
#ifndef SHOTGUN_H
#define SHOTGUN_H

#include "extdll.h"
#include "weapons.h"



// special deathmatch shotgun spreads
// REMEMBER, specific weapon .cpp's are included clientside too so cl_dlls/ev_hldm.cpp already sees
// these vectors in shotgun.cpp.
//MODDD - made into const vectors instead of #define's because blablabla see util_shared.h
extern DLL_GLOBAL const Vector VECTOR_CONE_DM_SHOTGUN;// 10 degrees by 5 degrees
extern DLL_GLOBAL const Vector VECTOR_CONE_DM_DOUBLESHOTGUN; // 20 degrees by 5 degrees

enum shotgun_e {
	SHOTGUN_IDLE = 0,
	SHOTGUN_FIRE,
	SHOTGUN_FIRE2,
	SHOTGUN_RELOAD,
	SHOTGUN_PUMP,
	SHOTGUN_START_RELOAD,
	SHOTGUN_DRAW,
	SHOTGUN_HOLSTER,
	SHOTGUN_IDLE4,
	SHOTGUN_IDLE_DEEP
};



class CShotgun : public CBasePlayerWeapon
{
public:
	int m_fInReload;
	float m_flNextReload;
	int m_iShell;

private:
	unsigned short m_usDoubleFire;
	unsigned short m_usSingleFire;

public:
	BOOL queueReload;


#ifndef CLIENT_DLL
	int	Save(CSave& save);
	int	Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];
#endif

	//MODDD - new
	CShotgun(void);

	BOOL reloadBlockFireCheck(BOOL isPrimary);
	void reloadFinishPump(void);
	BOOL reloadSemi(void);
	void reloadLogic(void);

	void Spawn(void);
	void Precache(void);
	int iItemSlot() { return 3; }
	int GetItemInfo(ItemInfo* p);


	//MODDD
	void customAttachToPlayer(CBasePlayer* pPlayer);

	int AddToPlayer(CBasePlayer* pPlayer);

	//MODDD - new.
	void ItemPreFrame(void);
	void ItemPostFrame(void);
	void ItemPostFrameThink(void);

	void PrimaryAttack(void);
	void SecondaryAttack(void);
	void FireShotgun(BOOL isPrimary);

	//MODDD	
	void Holster(int skiplocal = 0);

	BOOL Deploy();
	void Reload(void);
	void WeaponIdle(void);

	virtual BOOL UseDecrement(void)
	{
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

};


#endif //SHOTGUN_H