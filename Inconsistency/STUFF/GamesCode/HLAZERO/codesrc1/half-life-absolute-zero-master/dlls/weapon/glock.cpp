/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "extdll.h"
#include "glock.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"



EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo)

EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(glockOldReloadLogic)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(glockUseLastBulletAnim)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(wpn_glocksilencer)
//EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar)


extern float global2_wpn_glocksilencer;




LINK_ENTITY_TO_CLASS( weapon_glock, CGlock );
LINK_ENTITY_TO_CLASS( weapon_9mmhandgun, CGlock );

//MODDD - check for these variations by name!
LINK_ENTITY_TO_CLASS( weapon_glocksilencer, CGlock );
LINK_ENTITY_TO_CLASS( weapon_9mmhandgunsilencer, CGlock );




#ifndef CLIENT_DLL

#else
	extern int global2PSEUDO_playerHasGlockSilencer;
#endif




//in case this is ever still needed from outside this class...
float CGlock::getUsingGlockOldReloadLogic(void){
	return EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(glockOldReloadLogic);
}



void CGlock::setExtraBulletFalse(void){
	m_chargeReady &= ~32;
}
void CGlock::setExtraBulletTrue(void){
	m_chargeReady |= 32;
}
BOOL CGlock::getExtraBullet(void){
	return (m_chargeReady & 32) != 0;
}


void CGlock::setFiredSinceReloadFalse(void) {
	m_chargeReady &= ~64;
}
void CGlock::setFiredSinceReloadTrue(void) {
	m_chargeReady |= 64;
}
BOOL CGlock::getFiredSinceReload(void) {
	// FEATURE CANCELED.
	return TRUE;
	//return (m_chargeReady & 64) != 0;
}



CGlock::CGlock(){
	everSent = 0;

	//only for when using the old reload logic (if not, this is always implied!)
	setExtraBulletFalse();
	setFiredSinceReloadFalse();


	//"m_chargeReady" is better than using the custom var "rememberOneLessRound" because the Sync Gods are not offended by m_chargeReady.
	//yup.  I'll just go with that.
		
	//This is, "rememberOneLessRound", better for sync.
	//-1000 = TRUE
	//-2000 = FALSE



	//NEW VARS:
	//pev->body (as a synced var:  m_fireState, follows the same rules)
	//~coord this with whether the silencer is on or not:
	// 0 = no
	// 1 = yes
	//~More importantly: m_fInAttack...
	// 0 = Silencer unavailable, no silencer on.
	// 1 = Silencer available, not on.
	// 2 = Silencer available, on.


	nextAnimBackwards = FALSE;

	timeSinceDeployed = 0;

	
	//g_engfuncs.getView->curstate.body = m_fireState;

	//gEngfuncs.GetViewModel()->curstate.body = 1;


	oldTime = -1;
	currentTime = -1;

	timeDelta = 0;

	legalHoldSecondary = FALSE;
	startedSecondaryHoldAttempt = FALSE;
	holdingSecondaryCurrent = 0;
	animationTime = 0;
	holdingSecondaryTarget0 = 0.7f;

	//holdingSecondaryTarget1 = 2.44f;
	holdingSecondaryTarget1 = 2.44f - 0.32f;

	holdingSecondaryTarget2 = 3.39f;

	//holdingSecondaryTarget3 = 2.72f;
	holdingSecondaryTarget3 = 2.72 - 0.1f;


	holdingSecondaryTarget4 = 3.39f;


	toggledSilencerYet = FALSE;

	m_flStartThrow = -1;
	//animationindex??

	m_flReleaseThrow = -1;
	//animation sequence??


	scheduleGlockDeletion = FALSE;

	//???
	//m_fireState = 1;
}//END OF CGlock constructor



// Save/restore for serverside only!
#ifndef CLIENT_DLL
//MODDD - m_fInAttack tells whether the glock silencer is not available (0), or available but off (1) or on (2).
	//Saved on the weapon, as this var syncs better than one on the player (m_pPlayer->glockSilencerOnVar).
TYPEDESCRIPTION	CGlock::m_SaveData[] =
{
	DEFINE_FIELD(CGlock, m_fInAttack, FIELD_INTEGER),
	//MODDD - new
	DEFINE_FIELD(CGlock, m_fireState, FIELD_INTEGER),
	// m_flReleaseThrow, m_flStartThrow?  probably not

	DEFINE_FIELD(CGlock, includesGlockSilencer, FIELD_BOOLEAN),

};
IMPLEMENT_SAVERESTORE(CGlock, CBasePlayerWeapon);
#endif




void CGlock::customAttachToPlayer(CBasePlayer *pPlayer ){

	//CBasePlayerWeapon::customAttachToPlayer(pPlayer);
	//not necessary, empty in the CBasePlayerWeapon class.  (this was already called by 
	//"AttachToPlayer").
	m_pPlayer->SetSuitUpdate("!HEV_PISTOL", FALSE, SUIT_NEXT_IN_30MIN, 4.4f);

	if(m_pPlayer->recentlyGrantedGlockSilencer == TRUE){
		m_pPlayer->recentlyGrantedGlockSilencer = FALSE;
		if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(wpn_glocksilencer) == 1){
			m_pPlayer->SetSuitUpdate("!HEV_SILENCER", FALSE, SUIT_NEXT_IN_30MIN);
		}
	}

}


BOOL CGlock::ExtractAmmo( CBasePlayerWeapon *pWeapon )
{
	BOOL iReturn = FALSE;

	if ( pszAmmo1() != NULL )
	{
		int adjustedMaxClip = 0;
		if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(glockOldReloadLogic) == 1){
			//round in firing chamber.
			adjustedMaxClip = GLOCK_DEFAULT_GIVE;
		}else{
			adjustedMaxClip = GLOCK_DEFAULT_GIVE - 1;
		}

		//MODDD!!!
		//iReturn = pWeapon->AddPrimaryAmmo( m_iDefaultAmmo, (char *)pszAmmo1(), iMaxClip(), iMaxAmmo1() );
		//ALSO: forces the gun pickup sound.
		iReturn |= pWeapon->AddPrimaryAmmo( m_iDefaultAmmo, (char *)pszAmmo1(), adjustedMaxClip, iMaxAmmo1(), 2 );
		m_iDefaultAmmo = 0;
	}

	if ( pszAmmo2() != NULL )
	{
		iReturn |= pWeapon->AddSecondaryAmmo( 0, (char *)pszAmmo2(), iMaxAmmo2() );
	}

	//easyPrintLine("HOW??? %d %d", includesGlockSilencer, pWeapon->m_pPlayer->hasGlockSilencer );
	if(includesGlockSilencer && pWeapon->m_pPlayer->hasGlockSilencer == FALSE){
		pWeapon->m_pPlayer->hasGlockSilencer = TRUE;

		//not just yet..  ever necessary to turn off?
		//includesGlockSilencer = FALSE;

		
		//easyPrintLine("WELL DO YA??! %d", !pWeapon->m_pPlayer->HasNamedPlayerItem("weapon_9mmhandgun") );

		//say.  Does the player already have this weapon?
		if(!pWeapon->m_pPlayer->HasNamedPlayerItem("weapon_9mmhandgun")){
			//if not, this one will start with the silencer on.
			m_fInAttack = 2;
			m_fireState = 1;

			pWeapon->m_pPlayer->recentlyGrantedGlockSilencer = TRUE;
		}else{
			//if so, play this now.
			if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(wpn_glocksilencer) == 1){
				pWeapon->m_pPlayer->SetSuitUpdate("!HEV_SILENCER", FALSE, SUIT_NEXT_IN_30MIN);
			}
		}

		//let the player know if this is new.   ...nah, do it in the same place the weapon-pickup notice is issued.
		//pWeapon->m_pPlayer->SetSuitUpdate("!HEV_SILENCER", FALSE, SUIT_NEXT_IN_30MIN);
			
		if(iReturn == FALSE){
			// just to say something happened (remove this).  Return value does nothing to ammo, only determines whether to delete this item (used) or not (untouched).
			iReturn = TRUE;
		}

		//Received the silencer!
	}

	return iReturn;
}


//MODDD
BOOL CGlock::weaponCanHaveExtraCheck(CBasePlayer* pPlayer){
	if(pPlayer->hasGlockSilencer == FALSE && this->includesGlockSilencer){
		//looks like I have something you want!
		scheduleGlockDeletion = TRUE;
		//anticipate being removed soon.
		return TRUE;
	}else{
		//the normal way must suffice for this to work.
		return FALSE;
	}
}
BOOL CGlock::weaponPlayPickupSoundException(CBasePlayer* pPlayer){
	return scheduleGlockDeletion;
}


void CGlock::Spawn( )
{
	includesGlockSilencer = FALSE;

	const char* classNameTest = STRING(pev->classname);
	if(classNameTest != NULL && stringEndsWith(classNameTest, "silencer")){
		//pretty good sign that we want the silencer.		
		includesGlockSilencer = TRUE;
	}else{

	}

	pev->classname = MAKE_STRING("weapon_9mmhandgun"); // hack to allow for old names
	Precache( );
	m_iId = WEAPON_GLOCK;
	
	if(this->pev->spawnflags & SF_GLOCK_HAS_SILENCER){
		//custom flag 8: gives silencer too.
		includesGlockSilencer = TRUE;
	}


	if(!includesGlockSilencer){
		//default.
		SET_MODEL(ENT(pev), "models/w_9mmhandgun.mdl");
	}else{
		//show the silencer on there!
		SET_MODEL(ENT(pev), "models/w_silencer.mdl");
	}


	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(glockOldReloadLogic) == 1){
		//round in firing chamber.
		m_iDefaultAmmo = GLOCK_DEFAULT_GIVE;
	}else{
		m_iDefaultAmmo = GLOCK_DEFAULT_GIVE - 1;
	}

	
	FallInit();// get ready to fall down.
}


void CGlock::Precache( void )
{
	PRECACHE_MODEL("models/v_9mmhandgun.mdl");
	PRECACHE_MODEL("models/w_9mmhandgun.mdl");
	PRECACHE_MODEL("models/p_9mmhandgun.mdl");

	m_iShell = PRECACHE_MODEL ("models/shell.mdl");// brass shell

	PRECACHE_SOUND("items/9mmclip1.wav");
	//PRECACHE_SOUND("items/9mmclip2.wav");

	PRECACHE_SOUND ("weapons/pl_gun1.wav");//silenced handgun
	PRECACHE_SOUND ("weapons/pl_gun2.wav");//silenced handgun
	PRECACHE_SOUND ("weapons/pl_gun3.wav");//handgun

	precacheGunPickupSound();
	
	m_usFireGlock1 = PRECACHE_EVENT( 1, "events/glock1.sc" );
	m_usFireGlock2 = PRECACHE_EVENT( 1, "events/glock2.sc" );
}

int CGlock::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;

	//    emp part
	//old  X   O
	//cur  X   X

	//== 0 || == 1
	//MODDD - intercepted!?
	//easyPrintLine("SUPAMAN THAT %d %d", usingGlockOldReloadLogic==1, m_chargeReady);
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(glockOldReloadLogic) == 0 || ( getExtraBullet() ) || !getFiredSinceReload()   ) {
		p->iMaxClip = GLOCK_MAX_CLIP - 1;
	}else{
		p->iMaxClip = GLOCK_MAX_CLIP;
	}

	//MODDD
	// w-.. what possessed me to make this direct of an edit.
	// Is it necessary?  It might be?
	// Yes, it is.  Only precaching in util.cpp sets the ItemInfoArray spots
	// with what comes from "GetItemInfo".  So this has to be forced in case of a change since then.
	// Thank you 'past me' for saying fucking nothing here!
	ItemInfoArray[ m_iId ].iMaxClip = p->iMaxClip;


	//p->iMaxClip = GLOCK_MAX_CLIP - 1;

	p->iSlot = 1;
	p->iPosition = 0;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_GLOCK;
	p->iWeight = GLOCK_WEIGHT;

	return 1;
}


BOOL CGlock::Deploy( )
{
	timeSinceDeployed = 0;

	//start in the idle anim at deploy.
	m_flReleaseThrow = -1;


	currentTime = gpGlobals->time;
	//easyPrintLine("DEPLOY2");
	// pev->body = 1;

	/*
	if(m_pPlayer->glockSilencerOnVar == 1){
		m_fireState = 1;
	}else{
		m_fireState = 0;
	}
	*/
	//??? ???
	forceBlockLooping();

	SetBodyFromDefault();

	//MODDD - is including the body here (not forced to 0) okay?
	return DefaultDeploy( "models/v_9mmhandgun.mdl", "models/p_9mmhandgun.mdl", GLOCK_DRAW, "onehanded", /*UseDecrement() ? 1 : 0*/ 0, (m_fireState& ~128), (16.0f/20.0f), -1 );
}

void CGlock::Holster( int skiplocal /* = 0 */ ){
	
	//not animating backwards, if we were.
	//m_fireState &= ~128;

	/*
	if(m_pPlayer->glockSilencerOnVar == 1){
		m_fireState = 1;
	}else{
		m_fireState = 0;
	}
	*/

	everSent = 0;

	animationTime = 0;
	//m_fireState = (0 | 128);


		//if(m_fInAttack == 1){
		//	m_fireState = 0;
		//}else if(m_fInAttack == 2){
		//	m_fireState = 1;
		//}

	//interrupting this bit?  Keep the change then.
	if(m_flReleaseThrow == 2){
		//silencer not on, putting it on (is now)
		if(m_fInAttack == 1){
			//m_fireState = 1;
			m_fInAttack = 2;
			m_fireState = 1;
			//pev->body = m_fireState;
			//m_flTimeWeaponIdle = 0;
		}else if(m_fInAttack == 2){
			//m_fireState = 0;
			m_fInAttack = 1;
			m_fireState = 0;
		}
	}else{

		if(m_fInAttack == 1){
			m_fireState = 0;
		}else if(m_fInAttack == 2){
			m_fireState = 1;
		}

	}

	//???
	/*
	m_fInAttack = 1;
	m_fireState = 0;
	animationTime = 0;
	*/

	m_flReleaseThrow = -2;
	m_flStartThrow = -1;

	//restore muzzle flash for the third-person player model.
	m_pPlayer->pev->renderfx &= ~NOMUZZLEFLASH; 

	//m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	//SendWeaponAnim( CROWBAR_HOLSTER );

	//m_fInAttack = 1;
			//SendWeaponAnim( -1, 1, m_fireState );

	DefaultHolster(GLOCK_HOLSTER, skiplocal, (m_fireState& ~128), (16.0f/20.0f) );
	//CBasePlayerWeapon::Holster();

}//END OF Holster

//MODDD - everybody do the swap.
void CGlock::ItemPreFrame(){
	CBasePlayerWeapon::ItemPreFrame();
}


BOOL CanAttackG( float attack_time, float curtime, BOOL isPredicted )
{
#if defined( CLIENT_WEAPONS )
	if ( !isPredicted )
#else
	if ( 1 )
#endif
	{
		return ( attack_time <= curtime ) ? TRUE : FALSE;
	}
	else
	{
		return ( attack_time <= 0.0 ) ? TRUE : FALSE;
	}
}


//MODDD - everybody do the swap.
void CGlock::ItemPostFrame(){
	//int animationIndex = -1;

	oldTime = currentTime;
	currentTime = gpGlobals->time;
	timeDelta = currentTime - oldTime;
	//!!!!! Delta? Isn't that what "gpGlobals->frametime" is for?


	if(oldTime == -1 || timeDelta > 2){
		timeDelta = 0;
		//ignore.  Perhaps this "itemPreFrame" method hasn't been called in a while.  
		//Let "oldTime" update to a more reasonable value with a less extreme difference between that and currentTime.
	}

	timeSinceDeployed += timeDelta;


	const BOOL holdingPrimary = m_pPlayer->pev->button & IN_ATTACK;
	const BOOL holdingSecondary = m_pPlayer->pev->button & IN_ATTACK2;

	if(holdingPrimary){
		holdingSecondaryCurrent = 0;
	}
	//easyPrintLine("GLOCK NEXT ATTACK %.2f, %.2f", m_pPlayer->m_flNextAttack, gpGlobals->time);
	
	//if(m_pPlayer->allowGlockSecondarySilencerMem == 1){
	//if(CVAR_GET_FLOAT("allowGlockSecondarySilencer") == 1){

	int determiner = 0;
	
	// !!!MY VAR NOW!  .. no changed.
	int playerHasGlockYet = 0;


	determiner = EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(wpn_glocksilencer);

#ifndef CLIENT_DLL
	playerHasGlockYet = m_pPlayer->hasGlockSilencer;
#else
	playerHasGlockYet = global2PSEUDO_playerHasGlockSilencer;
#endif

	if( (determiner == 1 && playerHasGlockYet == 1) || (determiner == 2)){
		// silencer available, and...


		if(m_fInAttack == 0){
			// must be 1 or 2.  Default to "1" : off.
			m_fInAttack = 1;
		}

		//easyPrintLine("STAYTUS: %.2f, %d", m_pPlayer->glockSilencerOnVar, m_fInAttack  );
		
		//WARNING: UNWISE!
		/*
		if(m_pPlayer->glockSilencerOnVar == 0){
			//silencer is not being used.
			m_fInAttack = 1;
		}else{
			//silencer is being used.
			m_fInAttack = 2;
		}
		*/
		
		SetBodyFromDefault();

		//----







		


		//BOOL legalHoldSecondary = FALSE;
		BOOL suddenSecondaryRelease = FALSE;


		if(timeSinceDeployed > 1.0 && !m_fInReload && holdingSecondary && !holdingPrimary && m_pPlayer->m_flNextAttack <= gpGlobals->time  ){
			legalHoldSecondary = TRUE;
		}else{
			if(legalHoldSecondary == TRUE){
				suddenSecondaryRelease = TRUE;
			}
			legalHoldSecondary = FALSE;
		}


		if(legalHoldSecondary){
			if(holdingSecondaryCurrent != -1){
				holdingSecondaryCurrent += timeDelta;
			}
		}else{
			holdingSecondaryCurrent = 0;
		}
		//easyPrintLine("GLOCKTEST %.2f, %.2f", animationTime, holdingSecondaryTarget1);
		//easyPrintLine("GLOCK CONDITIONS %d %d", holdingSecondaryCurrent < holdingSecondaryTarget2,  ( (holdingSecondaryCurrent > 0 && holdingSecondaryCurrent < holdingSecondaryTarget2) || (animationTime >= holdingSecondaryTarget1 && animationTime < holdingSecondaryTarget2)) );

		

		/*
#ifndef CLIENT_DLL
		easyPrintLine("glock: ser %.2f", m_flReleaseThrow);
#else
		easyPrintLine("glock: cli %.2f", m_flReleaseThrow);
#endif
		*/



		// LET IT BE KNOWN.  THIS IS WHAT DOES THE GLOCK SILENCER ATTACH/REMOVE ANIMATIONS.
		if(m_flReleaseThrow == -1 && legalHoldSecondary){
			if(m_fInAttack == 2){
				//backwards!
				//easyForcePrintLine("ILL do depraved things! backwards.");
				SendWeaponAnimReverse( GLOCK_ADD_SILENCER, 1, (m_fireState& ~128) );
			}else{ // == 1
				//easyForcePrintLine("ILL do depraved things! forwards.");
				SendWeaponAnim( GLOCK_ADD_SILENCER, 1, (m_fireState& ~128) );
			}
			//SendWeaponAnimBypass(ANIM_NO_UPDATE, m_fireState & ~128);
		}




		if(m_fInAttack == 1){
		//silencer not on.  Putting it on.

			if(m_flReleaseThrow == -2){
					everSent = 0;
				m_flStartThrow = -1;
				if(!legalHoldSecondary){
					m_flReleaseThrow = -1;
				}

			}else if(m_flReleaseThrow == -1){
				m_flStartThrow = -1;
				if(legalHoldSecondary){
					m_flReleaseThrow = 0;
					animationTime = 0;
				}

			}else if(m_flReleaseThrow == 0){
				m_flStartThrow = GLOCK_ADD_SILENCER;
				animationTime += timeDelta;
				m_fireState = 0;
				if(animationTime > holdingSecondaryTarget0){
					m_flReleaseThrow = 1;
				}

			}else if(m_flReleaseThrow == 1){
				m_flStartThrow = GLOCK_ADD_SILENCER;
				animationTime += timeDelta;
				m_fireState = 1;

				if(animationTime > holdingSecondaryTarget1){
					m_flReleaseThrow = 2;
					//m_pPlayer->glockSilencerOnVar = 1;
					m_fireState = 1;
					//m_fInAttack = 2;
					
					//EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "items/9mmclip2.wav", RANDOM_FLOAT(0.52, 0.62), ATTN_IDLE, 0, 93 + RANDOM_LONG(0,0xF));
					
					//MODDD - soundsentencesave
					UTIL_PlaySound(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/glockSilencerOn.wav", RANDOM_FLOAT(1, 1), ATTN_IDLE, 0, 98 + RANDOM_LONG(0,4), FALSE);
				}
			}else if(m_flReleaseThrow == 2){
				m_flStartThrow = GLOCK_ADD_SILENCER;
				animationTime += timeDelta;
				m_fireState = 1;

				if(animationTime > holdingSecondaryTarget2){
					m_flReleaseThrow = -2;
					//m_pPlayer->glockSilencerOnVar = 1;
					m_fInAttack = 2;

					animationTime = 0;
					//idletime = 0;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + randomIdleAnimationDelay();

					//m_pPlayer->pev->weaponanim = ANIM_NO_UPDATE;
					
					// MODDD - IMPORTANT.
					// Need the server to stop sending the signal to play this animation,
					// which happens if it isn't explicitly told to stop doing that at the end
					// I suppose.  I don't get that.
					SendWeaponAnimBypass(ANIM_NO_UPDATE, m_fireState & ~128);

					CBasePlayerWeapon::ItemPostFrame();

					//MODDD - ok to remove this???
					//pev->body = m_fireState;


					//SendWeaponAnim( (int)m_flStartThrow, 1, m_fireState );

					//m_flTimeWeaponIdle = 0; //?
					

					return;
				}
			}
			
		}else if(m_fInAttack == 2){


			//silencer on.  Taking it off.
			//nextAnimBackwards = TRUE;


			if(m_flReleaseThrow == -2){
					everSent = 0;
				m_flStartThrow = -1;
				if(!legalHoldSecondary){
					m_flReleaseThrow = -1;
				}


			}else if(m_flReleaseThrow == -1){
				m_flStartThrow = -1;
				if(legalHoldSecondary){
					m_flReleaseThrow = 0;
					animationTime = 0;
				}
				//!!!!
				//m_fireState |= 128;

			}else if(m_flReleaseThrow == 0){
				m_flStartThrow = (GLOCK_ADD_SILENCER | 0); //!!!
				animationTime += timeDelta;
				m_fireState = (1 | 128);
				if(animationTime > holdingSecondaryTarget0){
					m_flReleaseThrow = 1;
				}
				
				//m_fireState |= 128;
				

			}else if(m_flReleaseThrow == 1){
				/*
				m_flStartThrow = (GLOCK_ADD_SILENCER | 0); //!!!
				animationTime += timeDelta;
				m_fireState = (1 | 128);
				*/

				m_flStartThrow = (GLOCK_ADD_SILENCER | 0); //!!!
				animationTime += timeDelta;
				m_fireState = (1 | 128);

				if(animationTime > holdingSecondaryTarget3){
					m_flReleaseThrow = 2;
					//m_pPlayer->glockSilencerOnVar = 0;
					m_fireState = (0 | 128);
					//YYYYYYYYYOOOOOOOOOOOOOOOOOOOOOOOOUUUUUUUUUUUUUUUUUUUUUU
					//m_fInAttack = 1;
					//EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "items/9mmclip2.wav", RANDOM_FLOAT(0.52, 0.62), ATTN_IDLE, 0, 93 + RANDOM_LONG(0,0xF));
					//MODDD - soundsentencesave
					UTIL_PlaySound(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/glockSilencerOff.wav", RANDOM_FLOAT(1, 1), ATTN_IDLE, 0, 98 + RANDOM_LONG(0,4), FALSE);
				}
				
				//m_fireState |= 128;
			}else if(m_flReleaseThrow == 2){
				m_flStartThrow = (GLOCK_ADD_SILENCER | 0); //!!!
				animationTime += timeDelta;
				m_fireState = (0 | 128);

				//m_fireState |= 128;

				if(animationTime > holdingSecondaryTarget4){
					m_flReleaseThrow = -2;
					m_fInAttack = 1;
					//m_pPlayer->glockSilencerOnVar = 0;

					animationTime = 0;
					//idletime = 0;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + randomIdleAnimationDelay();

					//m_pPlayer->pev->weaponanim = ANIM_NO_UPDATE;

					SendWeaponAnimBypass(ANIM_NO_UPDATE, m_fireState & ~128);

					CBasePlayerWeapon::ItemPostFrame();

					//pev->body = m_fireState;
					//SendWeaponAnim( (int)m_flStartThrow, 1, m_fireState );
					
					//???????????????????????????????????
					//m_flTimeWeaponIdle = 0; //?

					return;
				}
			}
		}


	}else{

		/*
#ifndef CLIENT_DLL
		easyPrintLine("glock: server sees else");
#else
		easyPrintLine("glock: client sees else");
#endif
		*/

		//silencer not available, and silencer off.
		
		m_fireState = 0;
		m_fInAttack = 0;

		m_flReleaseThrow = -1;
		m_flStartThrow = -1;
		animationTime = 0;
	}


	//easyForcePrintLine("My person Im Not making much sense right about now %d", m_fireState);


	//MODDD - EVERYBODY DO THE SWAP
	CBasePlayerWeapon::ItemPostFrame();


	/*
#ifndef CLIENT_DLL
	if(global_testVar == 1){
	easyPrintLine("HOW ARE YOU DOING THIS serv %.2f %d", m_flStartThrow, m_fireState);
	}
#else
	if(global2_testVar == 1){
	easyPrintLine("HOW ARE YOU DOING THIS clie %.2f %d", m_flStartThrow, m_fireState);
	}
#endif
	*/
			
	if(m_flStartThrow != -1){


		//does this even do anything??
		//pev->body = m_fireState;

		//experimental?  Not doing anything methinks.
		//m_chargeReady = -500;

		//this->nextAnimBackwards = TRUE;
		
		if(!everSent){
			//easyPrintLine("YOU SHALL SUFFER MY WRATH %d", (int)everSent);

			//easyForcePrintLine("ILL perhaps not.");
			//no... problems here? interesting.
			//MODDD NOTE - is the "& ~128" bit okay? Just checking.


			//SendWeaponAnim( (int)m_flStartThrow, 1, (m_fireState& ~128) );

			/*
			//Aha!
			if(m_fireState & 128){
				//backwards!

				easyForcePrintLine("eeee");
				SendWeaponAnimReverse( (int)m_flStartThrow, 1, (m_fireState& ~128) );
			}else{
				easyForcePrintLine("aaaa");
				SendWeaponAnim( (int)m_flStartThrow, 1, (m_fireState& ~128) );
			}
			*/
		}
		everSent = 1;
	}else{
		//SendWeaponAnim( GLOCK_IDLE1, 1 );
	}

}

//MODDD - Secondary fire removed altogether.  Can only apply the silencer when right-clicking.
void CGlock::SecondaryAttack( void )
{

	//no silencer available?  we're doing retail's rapid-fire then.
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(wpn_glocksilencer) == 0){
		if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay) == 0){
			GlockFire( 0.1, 0.2, FALSE );
		}else{
			GlockFire( 0.1, EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom), FALSE );
		}
	}
}

void CGlock::PrimaryAttack( void )
{
	//MODDD - only allow firing if not playing a silencer add / remove anim.
	if(m_flReleaseThrow == -1 || m_flReleaseThrow == -2){

	}else{
		return;
	}

	//easyPrintLine("PRIMARY2");

	//MODDD
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay) == 0){
		GlockFire( 0.01, 0.3, TRUE );
	}else{
		GlockFire( 0.01, EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom), TRUE );
	}
}

void CGlock::GlockFire( float flSpread , float flCycleTime, BOOL fUseAutoAim )
{
	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
		}

		return;
	}

	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip) == 0){
		m_iClip --;
	}


	//MODDD - making this depend on the silencer.
	//m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;
	if(m_fInAttack == 2){
		//silencer on.
		m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) & ~EF_MUZZLEFLASH;
	}else{
		//silencer off.
		m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;
	}


	int flags;

#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	//MODDD - Better: use this instead.
	//if (pev->body == 1)
	if(m_fInAttack == 2)
	{
		//MODDD - stats altered.
		//m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
		//m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

		m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME- 50;
		m_pPlayer->m_iWeaponFlash = 0;
		//m_pPlayer->pev->effects |= 256;    not advisable.  Makes the player invisible and shows a bright light around where the player is?
		//m_pPlayer->pev->renderfx |= 128;  //No side-effects from what I can tell, going with this.

		//m_pPlayer->pev->deadf

	}else
	{
		// non-silenced
		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;
		//m_pPlayer->pev->effects &= ~256;
		//m_pPlayer->pev->renderfx &= ~128;
	}

	//MODDD - used to be above, but would it not make more sense to play the animation after relevant settings have been established?
	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );


	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming;
	
	if ( fUseAutoAim )
	{
		vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );
	}
	else
	{
		vecAiming = gpGlobals->v_forward;
	}

	Vector vecDir;
	vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 8192, BULLET_PLAYER_9MM, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );


	//PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), fUseAutoAim ? m_usFireGlock1 : m_usFireGlock2, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 );


	//MODDD - the "shoot_empty" anim condition also requires "glockUseLastBulletAnim" to be true.
	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), fUseAutoAim ? m_usFireGlock1 : m_usFireGlock2, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, m_fInAttack, 0, ( m_iClip == 0 && EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(glockUseLastBulletAnim) == 1 ) ? 1 : 0, 0 );

	// well gee, we fired, didn't we?
	setFiredSinceReloadTrue();

	//MODDD - schedule idle for the end of the firing animation (only one possibility for animation time; no determination ahead of time needed)
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (15.0 / 25.0) + randomIdleAnimationDelay();


	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
	


	if (!m_iClip && PlayerPrimaryAmmoCount() <= 0) {
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}


	//MODDD - no.  ,,,???
	//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}


void CGlock::Reload( void )
{

	if ( m_pPlayer->ammo_9mm <= 0 )
		 return;
	
	int iResult;
	if (m_iClip == 0){
		//MODDD - used to refer to "17" for what is now "GLOCK_MAX_CLIP".  This syncs it better, should
		//the constant change again.

		iResult = DefaultReload( GLOCK_MAX_CLIP - 1, GLOCK_RELOAD, (37.0/18.0) );
		//MODDD - ALSO.  Max is one above the usual (without a round in the firing chamber).
		setExtraBulletTrue();
	}else{
		if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(glockOldReloadLogic) == 1 && getFiredSinceReload() ){
			//one extra round for reloading on not completely empty.
			iResult = DefaultReload( GLOCK_MAX_CLIP, GLOCK_RELOAD_NOT_EMPTY, (37.0/18.0) );
		}else{
			iResult = DefaultReload( GLOCK_MAX_CLIP - 1, GLOCK_RELOAD_NOT_EMPTY, (37.0/18.0) );
		}
		setExtraBulletFalse();
	}
	//easyPrintLine("MAGGOTS %d %d", m_iClip, rememberOneLessRound);


	/*
	if (m_iClip == 0) {
		setExtraBulletTrue();
	}
	else {
		setExtraBulletFalse();
	}
	*/


	//MODDD - No.
	/*
	if (iResult)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
	}
	*/

}




void CGlock::WeaponIdle( void )
{


	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );
	
	//easyPrintLine("WHAT???? %d %d", (m_chargeReady & 32), (m_chargeReady & 64) );

	/*
#ifndef CLIENT_DLL
	easyPrintLine("DOO SER %.2f : %.2f, %.2f", m_flReleaseThrow, m_flTimeWeaponIdle,UTIL_WeaponTimeBase() );
#else
	easyPrintLine("DOO CLI %.2f : %.2f, %.2f", m_flReleaseThrow, m_flTimeWeaponIdle,UTIL_WeaponTimeBase() );
#endif
	*/

	
	//print that out???
	//MODD - added.  Do not interrupt the glock silencer animation.
	if(m_flReleaseThrow == -1 || m_flReleaseThrow == -2){

	}else{
		return;
	}

	
	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;



	//MODDD - condition removed!  Note that leaving the brackets is okay.
	//~Brackets in a method with no condition / loop-specification in front will just always occur
	// when execution reaches them.

	
	//COMMENT'D: only idle if the slid isn't back
	//MODDD - wait, why was this commented again?
	if (m_iClip != 0)
	{
		int iAnim;
		float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0.0, 1.0 );

		//if (flRand <= 0.3 + 0 * 0.75)
		if (flRand <= 0.6)
		{
			iAnim = GLOCK_IDLE3;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 49.0 / 16 + randomIdleAnimationDelay();
		}
		//else if (flRand <= 0.6 + 0 * 0.875)
		else if (flRand <= 0.78)
		{
			iAnim = GLOCK_IDLE1;
			//MODDD - better time.
			//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 61.0 / 16.0;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 58.0 / 16.0 + randomIdleAnimationDelay();
		}
		else
		{
			iAnim = GLOCK_IDLE2;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 40.0 / 16.0 + randomIdleAnimationDelay();
		}
		
		forceBlockLooping();
		



		//m_pPlayer->pev->weaponanim = ANIM_NO_UPDATE;
		//iAnim = ANIM_NO_UPDATE;

		//don't render backwards unless told to do so.
		//m_fireState &= ~128;
		SendWeaponAnim( iAnim, 1, m_fireState &~128 );
	}
}



//MODDD - NEW, event.
void CGlock::OnReloadApply(void) {

	setFiredSinceReloadFalse();

}//END OF OnReloadApply



void CGlock::SendWeaponAnim(int iAnim, int skiplocal, int body){
	

	/*
	if(nextAnimBackwards == TRUE){
		//don't render backwards unless told to do so.
		m_fireState |= 128;
	}else{
		m_fireState &= ~128;
	}
	*/
	//nextAnimBackwards = FALSE;

	CBasePlayerWeapon::SendWeaponAnim(iAnim, skiplocal, body);
}


//MODDD - set the body choice (given by m_fireState) from m_fInAttack, which is saved.
void CGlock::SetBodyFromDefault(void) {

	//defaults.
	if (m_fInAttack == 1) {
		//silencer off.
		m_fireState = 0;
		m_pPlayer->pev->renderfx &= ~NOMUZZLEFLASH;  //restore muzzle flash.
	}
	else if (m_fInAttack == 2) {
		//silencer on.
		m_fireState = 1;
		//pev->renderfx &= ~NOMUZZLEFLASH;
		m_pPlayer->pev->renderfx |= NOMUZZLEFLASH;
		//m_pPlayer->pev->iuser2 = 200;
	}
}//SetBodyFromDefault









class CGlockAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_9mmclip.mdl");
		precacheAmmoPickupSound();
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 

		int ammoChoice = 0;


		/*
		if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(glockOldReloadLogic) == 1){
			//with old reload logic, a clip is the + 1.
			ammoChoice = AMMO_GLOCKCLIP_GIVE;
		}else{
			//yes.
			ammoChoice = AMMO_GLOCKCLIP_GIVE - 1;
		}
		*/
		//ammo will always give the default 12.
		ammoChoice = AMMO_GLOCKCLIP_GIVE - 1;

		if (pOther->GiveAmmo( ammoChoice, "9mm", _9MM_MAX_CARRY ) != -1)
		{

			//MODDD - filtered.
			playAmmoPickupSound();

			//MODDD
			if(pOther->IsPlayer()){
				CBasePlayer* pPlayer = (CBasePlayer*)pOther;
				pPlayer->SetSuitUpdate("!HEV_9MM", FALSE, SUIT_NEXT_IN_20MIN);
			}

			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_glockclip, CGlockAmmo );
LINK_ENTITY_TO_CLASS( ammo_9mmclip, CGlockAmmo );











