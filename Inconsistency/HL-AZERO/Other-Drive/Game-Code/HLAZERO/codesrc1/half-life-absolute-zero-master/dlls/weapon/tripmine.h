
#ifndef TRIPMINE_H
#define TRIPMINE_H

#include "extdll.h"
#include "weapons.h"


#define TRIPMINE_PRIMARY_VOLUME		450



enum tripmine_e {
	TRIPMINE_IDLE1 = 0,
	TRIPMINE_IDLE2,
	TRIPMINE_ARM1,
	TRIPMINE_ARM2,  //now named 'place' in-model
	TRIPMINE_FIDGET,
	TRIPMINE_HOLSTER,
	TRIPMINE_DRAW,
	TRIPMINE_WORLD,
	TRIPMINE_GROUND,
};





class CTripmine : public CBasePlayerWeapon
{
public:

	//MODDD
	void customAttachToPlayer(CBasePlayer* pPlayer);

	void Spawn(void);
	void Precache(void);
	int iItemSlot(void) { return 5; }
	int GetItemInfo(ItemInfo* p);
	void SetObjectCollisionBox(void)
	{
		//!!!BUGBUG - fix the model!
		pev->absmin = pev->origin + Vector(-16, -16, -5);
		pev->absmax = pev->origin + Vector(16, 16, 28);
	}


	//MODDD - new.
	float holdingSecondaryTarget0;
	float holdingSecondaryTarget1;
	float holdingSecondaryTarget2;

	CTripmine();
	void ItemPreFrame(void);
	void ItemPostFrame(void);
	void ItemPostFrameThink(void);

	void PrimaryAttack(void);
	void SecondaryAttack(void);
	
	BOOL Deploy(void);
	void Holster(int skiplocal = 0);
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
	unsigned short m_usTripFire;

};




#ifndef CLIENT_DLL
class CTripmineGrenade : public CGrenade
{

	float m_flPowerUp;
	Vector m_vecDir;
	Vector m_vecEnd;
	float m_flBeamLength;

	EHANDLE m_hOwner;
	CBeam* m_pBeam;
	Vector m_posOwner;
	Vector m_angleOwner;
	edict_t* m_pRealOwner;// tracelines don't hit PEV->OWNER, which means a player couldn't detonate his own trip mine, so we store the owner here.



	void SetObjectCollisionBox( void )
	{
		//MODDD - will this make me more likely to get hit by bullets?
		pev->absmin = pev->origin + Vector(-12, -12, -12);
		pev->absmax = pev->origin + Vector(12, 12, 12);
		//CGrenade::SetObjectCollisionBox();
	}

	//MODDD - added.
	void Activate(void);

	void Spawn(void);
	void Precache(void);

	virtual int	Save(CSave& save);
	virtual int	Restore(CRestore& restore);

	static	TYPEDESCRIPTION m_SaveData[];

	GENERATE_TRACEATTACK_PROTOTYPE
	GENERATE_TAKEDAMAGE_PROTOTYPE

	void EXPORT WarningThink(void);
	void EXPORT PowerupThink(void);
	void EXPORT BeamBreakThink(void);
	void EXPORT DelayDeathThink(void);

	GENERATE_KILLED_PROTOTYPE

	//Separate from the "Activate" built-in method for all entities(?). This is a quick way to turn the mine on.
	void ActivateMine(BOOL argPlayActivateSound);

	void MakeBeam(void);
	void KillBeam(void);

	float massInfluence(void);
	int GetProjectileType(void);

};
#endif





#endif TRIPMINE_H