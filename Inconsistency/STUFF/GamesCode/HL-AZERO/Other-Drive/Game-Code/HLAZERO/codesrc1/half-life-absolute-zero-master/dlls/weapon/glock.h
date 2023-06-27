
#ifndef GLOCK_H
#define GLOCK_H


#include "extdll.h"
#include "weapons.h"




enum glock_e {
	GLOCK_IDLE1 = 0,
	GLOCK_IDLE2,
	GLOCK_IDLE3,
	GLOCK_SHOOT,
	GLOCK_SHOOT_EMPTY,
	GLOCK_RELOAD,
	GLOCK_RELOAD_NOT_EMPTY,
	GLOCK_DRAW,
	GLOCK_HOLSTER,
	GLOCK_ADD_SILENCER

};



class CGlock : public CBasePlayerWeapon
{
private:
	int m_iShell;

	float oldTime;
	float currentTime;
	float timeDelta;

	BOOL legalHoldSecondary;
	BOOL startedSecondaryHoldAttempt;
	float animationTime;
	float holdingSecondaryCurrent;
	float holdingSecondaryTarget0;
	float holdingSecondaryTarget1;
	float holdingSecondaryTarget2;

	float holdingSecondaryTarget3;
	float holdingSecondaryTarget4;

	float timeSinceDeployed;

	BOOL toggledSilencerYet;

	unsigned short m_usFireGlock1;
	unsigned short m_usFireGlock2;

public:

	BOOL scheduleGlockDeletion;
	BOOL includesGlockSilencer;
	//BOOL playerHasGlockYet;

	int everSent;
	BOOL nextAnimBackwards;






	static float getUsingGlockOldReloadLogic(void);

	BOOL weaponCanHaveExtraCheck(CBasePlayer* pPlayer);
	BOOL weaponPlayPickupSoundException(CBasePlayer* pPlayer);

	void setExtraBulletFalse(void);
	void setExtraBulletTrue(void);
	BOOL getExtraBullet(void);
	void setFiredSinceReloadFalse(void);
	void setFiredSinceReloadTrue(void);
	BOOL getFiredSinceReload(void);

	//MODDD - this is necessary for  saving and loading, apparently.
#ifndef CLIENT_DLL
	int	Save(CSave& save);
	int	Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];
#endif

	//MODDD
	virtual int ExtractAmmo(CBasePlayerWeapon* pWeapon);

	void Spawn(void);
	void Precache(void);
	int iItemSlot(void) { return 2; }
	int GetItemInfo(ItemInfo* p);

	//MODDD
	CGlock();
	void customAttachToPlayer(CBasePlayer* pPlayer);
	void ItemPreFrame(void);
	void ItemPostFrame(void);

	void Holster(int skiplocal = 0);

	void PrimaryAttack(void);
	void SecondaryAttack(void);

	void GlockFire(float flSpread, float flCycleTime, BOOL fUseAutoAim);
	BOOL Deploy(void);
	void Reload(void);
	void WeaponIdle(void);

	//MODDD - NEW, event.
	void OnReloadApply(void);

	void SendWeaponAnim(int iAnim, int skiplocal = 1, int body = 0);

	void SetBodyFromDefault(void);


	virtual BOOL UseDecrement(void)
	{
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}



};



#endif //GLOCK_H