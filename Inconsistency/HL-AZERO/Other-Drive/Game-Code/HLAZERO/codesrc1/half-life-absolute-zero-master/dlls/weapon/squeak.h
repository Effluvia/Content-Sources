
// To be clear, "CSqueak" is the player weapon (throwable form).
// CSqueakGrenade is the 'projectile' (monster).

#pragma once
#ifndef SQUEAKGRENADE_H
#define SQUEAKGRENADE_H

#include "extdll.h"
#include "weapons.h"


// world model version (thing that runs around, makes noise and bites)
enum w_squeak_e {
	WSQUEAK_IDLE1 = 0,
	WSQUEAK_FIDGET,
	WSQUEAK_JUMP,
	WSQUEAK_RUN,
};

// viewmodel version
enum squeak_e {
	SQUEAK_IDLE1 = 0,  //31, 16
	SQUEAK_FIDGETFIT,  //71, 16
	SQUEAK_FIDGETNIP,  //81, 16
	SQUEAK_DOWN,  //21, 16
	SQUEAK_UP,  //51, 30
	SQUEAK_THROW  //44, 30
};





//TODO: make it move??  OR would it make more sense to make a monster that acts as if the player touched this?
// ...........what.
class CSqueak : public CBasePlayerWeapon
{
	static int numberOfEyeSkins;

private:
	unsigned short m_usSnarkFire;
public:
	// Var dropped, no reason not to rely on an already-synched var instead.
	//int m_fJustThrown;

	CSqueak();

#ifndef CLIENT_DLL
	int	Save(CSave& save);
	int	Restore(CRestore& restore);
	static TYPEDESCRIPTION m_SaveData[];
#endif

	const char* GetPickupWalkerName(void);
	virtual void Spawn(void);
	void Precache(void);
	void setModel(void);
	void setModel(const char* m);
	int iItemSlot(void) { return 5; }
	int GetItemInfo(ItemInfo* p);

	//MODDD
	void customAttachToPlayer(CBasePlayer* pPlayer);


	void PrimaryAttack(void);
	void SecondaryAttack(void);
	void Throw(void);
	BOOL Deploy(void);
	void Holster(int skiplocal = 0);

	float randomIdleAnimationDelay(void);

	//MODDD - new
	void ItemPostFrame(void);
	void ItemPostFrameThink(void);
	
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





//////////////////////////////////////////////////////////////////////////////////////
// Same as spawning by "weapon_snark", but gives the same effect as the 
// SF_PICKUP_NOREPLACE spawnflag.
// It doesn't add the spawnflag, just having a classname that ends in "_noreplace" is enough.
class CSqueak_NoReplace : public CSqueak
{
public:
	//MODDD
	CSqueak_NoReplace();

	void Spawn(void);
};

//////////////////////////////////////////////////////////////////////////////////////







#ifndef CLIENT_DLL
// Used to have a parent of "CGrenade", now CBaseMonster.
// Turns out this offered no relied-on features that CBaseMonster didn't already have.
// I got nothing on that one.
class CSqueakGrenade : public CBaseMonster
{

	//MODDD - why no public?
public:
	CSqueakGrenade();
	BOOL usesSoundSentenceSave(void);

	void Spawn(void);
	void Precache(void);
	void setModel(void);
	void setModel(const char* m);
	int  Classify(void);
	void EXPORT SuperBounceTouch(CBaseEntity* pOther);
	void EXPORT HuntThink(void);
	int  BloodColor(void) { return BLOOD_COLOR_YELLOW; }

	GENERATE_KILLED_PROTOTYPE

	BOOL isOrganic(void);

	float massInfluence(void);
	int GetProjectileType(void);

	Vector GetVelocityLogical(void);
	void SetVelocityLogical(const Vector& arg_newVelocity);

	void OnDeflected(CBaseEntity* arg_entDeflector);


	GENERATE_GIBMONSTER_PROTOTYPE

	virtual int	Save(CSave& save);
	virtual int	Restore(CRestore& restore);

	static	TYPEDESCRIPTION m_SaveData[];

	static float m_flNextBounceSoundTime;

	// CBaseEntity *m_pTarget;
	float m_flDie;
	Vector m_vecTarget;
	float m_flNextHunt;
	float m_flNextHit;
	Vector m_posPrev;
	EHANDLE m_hOwner;
	int  m_iMyClass;
	//MODDD - NEW, since CBaseMonster lost m_flNextAttack.
	// (not that this inherits from there anymore either).
	// Not bothering with save/restore, as m_flNextAttack was saved, since this is such a 
	// negligible amount of time to keep track of.
	float m_flNextAttack;
};
#endif


#endif SQUEAKGRENADE_H