
#ifndef EGON_H
#define EGON_H

#include "weapons.h"


#define EGON_BEAM_SPRITE		"sprites/xbeam1.spr"
#define EGON_FLARE_SPRITE		"sprites/XSpark1.spr"
#define EGON_SOUND_OFF			"weapons/egon_off1.wav"
#define EGON_SOUND_RUN			"weapons/egon_run3.wav"
#define EGON_SOUND_STARTUP		"weapons/egon_windup2.wav"

#define EGON_PRIMARY_VOLUME		450


enum egon_e {
	EGON_IDLE1 = 0,
	EGON_FIDGET1,
	EGON_ALTFIREON,
	EGON_ALTFIRECYCLE,
	EGON_ALTFIREOFF,
	EGON_FIRE1,
	EGON_FIRE2,
	EGON_FIRE3,
	EGON_FIRE4,
	EGON_DRAW,
	EGON_HOLSTER
};



enum EGON_FIRESTATE { FIRE_OFF, FIRE_CHARGE };
enum EGON_FIREMODE { FIRE_NARROW, FIRE_WIDE};



class CEgon : public CBasePlayerWeapon
{
public:
	BOOL effectsExist;;
	int fireStatePrev;
	Vector recentFireDirection;
	Vector recentHitPlaneNormal;
	float nextScorchInterval;

	BOOL previousScorchLocSet;
	Vector previousScorchLoc;
	CBaseEntity* recentlyDamagedEntity;

	float ignoreIdleTime;

	float m_flAmmoUseTime;// since we use < 1 point of ammo per update, we subtract ammo on a timer.

	float fuser1_store;

	CBeam* m_pBeam;
	CBeam* m_pNoise;
	CSprite* m_pSprite;

	unsigned short m_usEgonStop;

private:
	float			m_shootTime;
	EGON_FIREMODE	m_fireMode;
	float			m_shakeTime;
	BOOL			m_deployed;

	unsigned short m_usEgonFire;

public:

	BOOL fireExceptionPrev;
	float fireExceptionStartTime;
	BOOL lockedFireState;

	BOOL canStartSecondary;
	BOOL secondarySwitched;

	//BOOL waitForAnimReset;
	int queueAnim;

	BOOL legalHoldSecondary;
	BOOL startedSecondaryHoldAttempt;
	float animationTime;
	float holdingSecondaryCurrent;
	float holdingSecondaryTarget0;
	float holdingSecondaryTarget1;
	float holdingSecondaryTarget2;

	//MODDD - also new.
	float timeSinceDeployed;

	BOOL toggledYet;
	BOOL altFireOn;
	int animationSequence;

	float oldTime;
	float currentTime;

	float timeDelta;
	int lastSentAnim;
	BOOL blockContinualFireSound;



#ifndef CLIENT_DLL
	int	Save(CSave& save);
	int	Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];
#endif

	void onFreshFrame(void);



	CEgon::CEgon(void);



	//MODDD - ??
	void ItemPreFrame(void);
	void ItemPostFrame(void);
	void ItemPostFrameThink(void);

	void UpdateHitCloud(void);

	void PrimaryNotHeld(void);
	void SecondaryNotHeld(void);
	void NeitherHeld(void);
	void BothHeld(void);

	void Spawn(void);
	void Precache(void);
	int iItemSlot(void) { return 4; }
	int GetItemInfo(ItemInfo* p);

	//MODDD
	void customAttachToPlayer(CBasePlayer* pPlayer);

	int AddToPlayer(CBasePlayer* pPlayer);

	BOOL Deploy(void);
	void Holster(int skiplocal = 0);

	void UpdateEffect(const Vector& startPoint, const Vector& endPoint, float timeBlend);

	void CreateEffect(void);
	void DestroyEffect(void);

	void EndAttack(void);
	void Attack(void);
	void PrimaryAttack(void);
	//MODDD -added
	void SecondaryAttack(void);

	void WeaponIdle(void);

	float GetPulseInterval(void);
	float GetDischargeInterval(void);

	void Fire(const Vector& vecOrigSrc, const Vector& vecDir);

	BOOL HasAmmo(void);

	void UseAmmo(int count);


	virtual BOOL UseDecrement(void)
	{
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

};


#endif //EGON_H