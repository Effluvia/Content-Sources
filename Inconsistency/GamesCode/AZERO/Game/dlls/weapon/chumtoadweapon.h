


#ifndef CHUMTOADWEAPON_H
#define CHUMTOADWEAPON_H



#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"




enum chumtoadweapon_e {  //key: frames, FPS
	CHUMTOADWEAPON_IDLE1 = 0, //31, 16
	CHUMTOADWEAPON_FIDGETLICK, //31, 16
	CHUMTOADWEAPON_FIDGETCROAK, //51, 16
	CHUMTOADWEAPON_DOWN, //21, 21
	CHUMTOADWEAPON_UP, //36, 36
	CHUMTOADWEAPON_THROW, //16, 24

};

//how far into the throw animation to do another check and spawn the chumtoad.
//#define CHUMTOAD_THROW_DELAY (20.0f / 24.0f)
//instant now.
#define CHUMTOAD_THROW_DELAY 0





class CChumToadWeapon : public CBasePlayerWeapon{

public:
	unsigned short m_usChumToadFire;
	static int numberOfEyeSkins;

	BOOL waitingForChumtoadThrow;
	float chumtoadThrowReverseDelay;
	int antiGravityPositionY;


	//MODDD
	CChumToadWeapon(void);

#ifndef CLIENT_DLL
	int	Save(CSave& save);
	int	Restore(CRestore& restore);
	static TYPEDESCRIPTION m_SaveData[];
#endif

	BOOL usesSoundSentenceSave(void);




	void EXPORT FallThinkCustom ( void );
	const char* GetPickupWalkerName(void);
	CBaseEntity* pickupWalkerReplaceCheck(void);
	virtual void Spawn( void );
	//void AttachToPlayer ( CBasePlayer *pPlayer );
	void FallInit( void );
	void EXPORT ItemRotate ( void );
	void Precache( void );
	void setModel(void);
	void setModel(const char* m);
	int iItemSlot( void ) { return 5; }
	int GetItemInfo(ItemInfo *p);

	//MODDD
	void customAttachToPlayer(CBasePlayer *pPlayer );

	BOOL checkThrowValid(Vector trace_origin, float* minFractionStore);
	void ThrowChumtoad(Vector vecSpawnPoint);

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );

	float randomIdleAnimationDelay(void);
	//MODDD - new
	void ItemPostFrame(void);
	void ItemPostFrameThink(void);
	
	void WeaponIdle( void );

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

};//END OF CChumToadWeapon




//////////////////////////////////////////////////////////////////////////////////////
// Same as spawning by "weapon_chumtoad", but gives the same effect as the 
// SF_PICKUP_NOREPLACE spawnflag.
// It doesn't add the spawnflag, just having a classname that ends in "_noreplace" is enough.
class CChumToadWeapon_NoReplace : public CChumToadWeapon
{
public:
	//MODDD
	CChumToadWeapon_NoReplace();

	void Spawn(void);
};

//////////////////////////////////////////////////////////////////////////////////////




#endif //END OF #ifdef CHUMTOADWEAPON_H
