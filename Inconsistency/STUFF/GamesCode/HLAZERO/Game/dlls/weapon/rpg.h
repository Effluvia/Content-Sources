
#ifndef RPG_H
#define RPG_H

#include "extdll.h"
#include "weapons.h"


class CLaserSpot;


enum rpg_e {
	RPG_IDLE = 0,
	RPG_RELOAD,
	RPG_FIRE,
	RPG_HOLSTER,
	RPG_DRAW,
	RPG_IDLE_EMPTY,
	RPG_HOLSTER_EMPTY,
	RPG_DRAW_EMPTY,

};

/*
enum rpg_e {
	RPG_IDLE = 0,
	RPG_FIDGET,     // NEW COMMENT: loaded fidget.
	RPG_RELOAD,		// to reload
	RPG_FIRE2,		// to empty
	RPG_HOLSTER1,	// loaded
	RPG_DRAW1,		// loaded
	RPG_HOLSTER2,	// unloaded
	RPG_DRAW_UL,	// unloaded
	RPG_IDLE_UL,	// unloaded idle

	//MODDD NOTICE - this anim just does not exist, no anim of this index in the model.
	//               Should we make a special one.?
	//RPG_FIDGET_UL,	// unloaded fidget
};
*/


class CRpg : public CBasePlayerWeapon
{
public:
	CLaserSpot* m_pSpot;
	int m_fSpotActive;
	int m_cActiveRockets;// how many missiles in flight from this launcher right now?
	unsigned short m_usRpg;
#ifndef CLIENT_DLL
	float forceHideSpotTime;
#endif


public:
	//MODDD - constructor.
	CRpg(void);

#ifndef CLIENT_DLL
	int	Save(CSave& save);
	int	Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];
#endif

	int GetClip(void);
	int GetClipMax(void);
	void OnReloadApply(void);
	void OnAddPrimaryAmmoAsNewWeapon(void);
	BOOL RPGReload(int iClipSize, int iAnim, float fDelay, int body = 0);

	void Spawn(void);
	void Precache(void);
	void Reload(void);

	//MODDD - moved down.
	//int iItemSlot( void ) { return 4; }
	int iItemSlot(void) { return 3; }

	int GetItemInfo(ItemInfo* p);

	//MODDD
	void customAttachToPlayer(CBasePlayer* pPlayer);

	int AddToPlayer(CBasePlayer* pPlayer);

	BOOL Deploy(void);
	BOOL CanHolster(void);
	void Holster(int skiplocal = 0);

	void PrimaryAttack(void);
	void SecondaryAttack(void);

	void onFreshFrame(void);
	void ItemPostFrameThink(void);
	void WeaponIdle(void);

	void UpdateSpot(void);
	BOOL ShouldWeaponIdle(void) { return TRUE; };

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

class CLaserSpot : public CBaseEntity
{
public:
	void Spawn(void);
	void Precache(void);

	int ObjectCaps(void) { return FCAP_DONT_SAVE; }

	void Suspend(float flSuspendTime);
	void EXPORT Revive(void);
	static CLaserSpot* CreateSpot(void);
};


class CRpgRocket : public CGrenade
{
public:

	int m_iTrail;
	float m_flIgniteTime;
	CRpg* m_pLauncher;// pointer back to the launcher that fired me. 

	BOOL ignited;

	//MODDD - new
	Vector vecMoveDirectionMemory;
	BOOL alreadyDeleted;
	BOOL forceDumb;



	//MODDD - new
	CRpgRocket(void);

	int	Save(CSave& save);
	int	Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];
	void Spawn(void);
	void Precache(void);
	void EXPORT FollowThink(void);
	void EXPORT IgniteThink(void);
	void EXPORT RocketTouch(CBaseEntity* pOther);

	void onDelete(void);
	void commonPreDelete(void);

	float massInfluence(void);
	int GetProjectileType(void);

	Vector GetVelocityLogical(void);
	void SetVelocityLogical(const Vector& arg_newVelocity);
	void OnDeflected(CBaseEntity* arg_entDeflector);


	static CRpgRocket* CreateRpgRocket(Vector vecOrigin, Vector vecAngles, CBaseEntity* pOwner, CRpg* pLauncher);
	static CRpgRocket* CreateRpgRocket(Vector vecOrigin, Vector vecAngles, Vector arg_vecMoveDirection, CBaseEntity* pOwner, CRpg* pLauncher);

};

#endif



#endif //RPG_H
