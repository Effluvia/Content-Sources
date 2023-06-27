
#include "external_lib_include.h"
//#include <stdio.h>
//#include <string.h>
//#include "STDIO.H"
//#include "STDLIB.H"
//#include "MATH.H"




#include "custom_message.h" //what. how were we fine without this.
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
//#include "pm_shared.h"
#include "r_efx.h"

//MODDD - why was this even defined here?  It's already in common/com_model.h.
// Moved to util_shared.h if anywhere even uses this.
//#define MAX_CLIENTS 32


#include "cvar_custom_info.h"
#include "cvar_custom_list.h"

//#include "ammohistory.h"
//#include "vgui_TeamFortressViewport.h"


extern float global2PSEUDO_grabbedByBarancle;

// exists in inputw32.cpp.
extern cvar_t* sensitivity;



EASY_CVAR_EXTERN_HASH_ARRAY

EASY_CVAR_EXTERN_MASS



//EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(canShowWeaponSelectAtDeath)
extern BEAM *pBeam;
extern BEAM *pBeam2;

extern BOOL g_queueCVarHiddenSave;




//MODDD NOTE - kinda.. odd to have to do this, but yea. There is no
// 'in_camera.h', yet we need a method from it.
extern void CAM_ToFirstPerson(void);



//where?
int flag_apply_m_flTimeWeaponIdle = FALSE;
float stored_m_flTimeWeaponIdle = 0;
	
int flag_apply_m_fJustThrown = FALSE;
int stored_m_fJustThrown = 0;



//MODDD - from hud.cpp, at least what they replace was there.
DECLARE_MESSAGE_HUD(Logo)
DECLARE_MESSAGE_HUD(ResetHUD)
DECLARE_MESSAGE_HUD(InitHUD)
DECLARE_MESSAGE_HUD(ViewMode)
DECLARE_MESSAGE_HUD(SetFOV)
DECLARE_MESSAGE_HUD(Concuss)
DECLARE_MESSAGE_HUD(GameMode)
//MODDD - from health.cpp
DECLARE_MESSAGE_HUD(Damage)
DECLARE_MESSAGE_HUD(Drowning)



// gHUD-less methods use this form instead.
PROTOTYPE_MESSAGE(PrintCl);
PROTOTYPE_MESSAGE(JBoxReq);
PROTOTYPE_MESSAGE(JBoxOff);
PROTOTYPE_MESSAGE(AutoMus);
PROTOTYPE_MESSAGE(CliTest);
PROTOTYPE_MESSAGE(FirstAppr);

PROTOTYPE_MESSAGE(UpdClientC);
PROTOTYPE_MESSAGE(UpdClientN);
PROTOTYPE_MESSAGE(RstClientC);
PROTOTYPE_MESSAGE(PntClientC);
PROTOTYPE_MESSAGE(UpdBnclStat);
//PROTOTYPE_MESSAGE(UpdateCam);
PROTOTYPE_MESSAGE(UpdPlyA);
PROTOTYPE_MESSAGE(MUnpause);
PROTOTYPE_MESSAGE(UpdTWI);
PROTOTYPE_MESSAGE(UpdJT);
PROTOTYPE_MESSAGE(CWFNSOE);

PROTOTYPE_MESSAGE(ServerDLLI);
PROTOTYPE_MESSAGE(YMG);
PROTOTYPE_MESSAGE(YMG_S);





void Init_CustomMessage(void){
	
	//MODDD - from hud.cpp
	HOOK_MESSAGE( Logo );
	HOOK_MESSAGE( ResetHUD );
	HOOK_MESSAGE( InitHUD );
	HOOK_MESSAGE( ViewMode );
	HOOK_MESSAGE( SetFOV );
	HOOK_MESSAGE( Concuss );
	HOOK_MESSAGE( GameMode );
	HOOK_MESSAGE(Damage);
	HOOK_MESSAGE(Drowning);

	
	//JUKEBOX!
	HOOK_MESSAGE(PrintCl);
	HOOK_MESSAGE(JBoxReq);
	HOOK_MESSAGE(JBoxOff);
	HOOK_MESSAGE(AutoMus);
	HOOK_MESSAGE(CliTest);
	HOOK_MESSAGE(FirstAppr);
	
	HOOK_MESSAGE(UpdClientC);
	HOOK_MESSAGE(UpdClientN);
	HOOK_MESSAGE(RstClientC);
	HOOK_MESSAGE(PntClientC);
	HOOK_MESSAGE(UpdBnclStat);
	//HOOK_MESSAGE(UpdateCam);
	HOOK_MESSAGE(UpdPlyA);
	HOOK_MESSAGE(MUnpause);
	HOOK_MESSAGE(UpdTWI);
	HOOK_MESSAGE(UpdJT);
	HOOK_MESSAGE(CWFNSOE);

	HOOK_MESSAGE(ServerDLLI);
	HOOK_MESSAGE(YMG);
	HOOK_MESSAGE(YMG_S);
	

}//END OF Init




///////////////////////////////////////////////////////////////////////////////////////////////////
//MODDD - methods moved from the since-deleted hud_msg.cpp.
///////////////////////////////////////////////////////////////////////////////////////////////////


//MODDD - MsgFunc_Logo and MsgFunc_SetFOV moved from hud.cpp.

int CHud::MsgFunc_Logo(const char *pszName,  int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );

	// update Train data
	m_iLogo = READ_BYTE();

	return 1;
}

//MODDD - just note
//NOTE: while "hud_redraw" constantly forces m_iPlayerFOV to default_fov, let it be known that this method is perhaps almost entirely pointless.
int CHud::MsgFunc_SetFOV(const char *pszName,  int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );

	int newfov = READ_BYTE();

	//MODDD - you know the drill.
	//int def_fov = CVAR_GET_FLOAT( "default_fov" );
	int def_fov = getPlayerBaseFOV();
	

	//Weapon prediction already takes care of changing the fog. ( g_lastFOV ).
	if ( cl_lw && cl_lw->value )
		return 1;

	g_lastFOV = newfov;

	if ( newfov == 0 )
	{
		m_iPlayerFOV = def_fov;
	}
	else
	{
		m_iPlayerFOV = newfov;
	}

	// the clients fov is actually set in the client data update section of the hud

	// Set a new sensitivity
	if ( m_iPlayerFOV == def_fov )
	{  
		// reset to saved sensitivity
		m_flMouseSensitivity = 0;
	}
	else
	{  
		// set a new sensitivity that is proportional to the change from the FOV default
		if(def_fov != 0){
			m_flMouseSensitivity = sensitivity->value * ((float)newfov / (float)def_fov) * CVAR_GET_FLOAT("zoom_sensitivity_ratio");
		}else{
			m_flMouseSensitivity = 0; //safety??
		}
	}

	return 1;
}


int CHud::MsgFunc_ResetHUD(const char *pszName, int iSize, void *pbuf )
{
	//MODDD - this entire method used to be dummied.  Any reason why?
	//!!!
	
	//MODDD - Referred to the ASSERT defned in cl_dlls/parsemsg.h.  Which was dummied.  Renamed here and there to be
	ASSERT_DUMMY( iSize == 0 );

	// clear all hud data
	HUDLIST *pList = m_pHudList;

	while ( pList )
	{
		if ( pList->p )
			pList->p->Reset();
		pList = pList->pNext;
	}

	// reset sensitivity
	m_flMouseSensitivity = 0;

	// reset concussion effect
	m_iConcussionEffect = 0;
	
	return 1;
}

int CHud::MsgFunc_ViewMode( const char *pszName, int iSize, void *pbuf )
{
	CAM_ToFirstPerson();
	return 1;
}

int CHud::MsgFunc_InitHUD( const char *pszName, int iSize, void *pbuf )
{
	//MODDDMIRROR
	numMirrors = 0;
	
	// prepare all hud data
	HUDLIST *pList = m_pHudList;
	
	while (pList)
	{
		if ( pList->p )
			pList->p->InitHUDData();
		pList = pList->pNext;
	}
	
	//Probably not a good place to put this.
	pBeam = pBeam2 = NULL;
	return 1;
}

int CHud::MsgFunc_GameMode(const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	m_Teamplay = READ_BYTE();

	return 1;
}

//MODDD - MsgFunc_Damage removed.  This is likely stuck in an earlier phase of development,
// differnet intentions.
// Makes sense that it could not work at the HOOK_MESSAGE step (absent in hud.cpp unlike all
// other messages here), the message's name must be unique (globally) and this collides with
// the "MsgFunc_Damage" one in health.cpp (now moved to this same file interestingly enough).
/*
int CHud::MsgFunc_Damage(const char *pszName, int iSize, void *pbuf )
{
	int	armor, blood;
	Vector	from;
	int	i;
	float count;
	
	BEGIN_READ( pbuf, iSize );
	armor = READ_BYTE();
	blood = READ_BYTE();

	for (i=0 ; i<3 ; i++)
		from[i] = READ_COORD();

	count = (blood * 0.5) + (armor * 0.5);

	if (count < 10)
		count = 10;

	// TODO: kick viewangles,  show damage visually

	return 1;
}
*/


int CHud::MsgFunc_Concuss( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	m_iConcussionEffect = READ_BYTE();
	if (m_iConcussionEffect)
		this->m_StatusIcons.EnableIcon("dmg_concuss",255,160,0);
	else
		this->m_StatusIcons.DisableIcon("dmg_concuss");
	return 1;
}



// Helpful for the new CPain gui too (script moved from health.cpp)
int CHud::MsgFunc_Damage(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int armor = READ_BYTE();	// armor
	int damageTaken = READ_BYTE();	// health
	int rawDamageTaken = READ_BYTE();	// MODDD NEW - raw damage, before armor reduction (what was intended)
	long bitsDamage = READ_LONG(); // damage bits

	//MODDD
	long bitsDamageMod = READ_LONG(); // damage bits

	vec3_t vecFrom;

	for (int i = 0; i < 3; i++)
		vecFrom[i] = READ_COORD();

	int damageBlockedByArmor = rawDamageTaken - damageTaken;

	//MODDD
	//UpdateTiles(gHUD.m_flTime, bitsDamage);
	gHUD.m_Health.UpdateTiles(gHUD.m_flTime, bitsDamage, bitsDamageMod);

	gHUD.recentDamageBitmask = bitsDamage;

	if (bitsDamage & DMG_DROWN) {
		//if this is "drown" damage, get how to draw pain differnetly (default is nothing at all)
		if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(painFlashDrownMode) == 2) {
			//just do this.
			gHUD.m_Pain.cumulativeFadeDrown = 1.0f;
			//return 1;
		}
		else if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(painFlashDrownMode) == 3) {
			//m_fAttackFront = m_fAttackRear = m_fAttackRight = m_fAttackLeft = 1;
			//MODDD TODO - ditto.
			const float damageFlashModTotal = damageTaken + damageBlockedByArmor * EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(painFlashArmorBlock);

			gHUD.m_Pain.m_fAttackFront = gHUD.m_Pain.m_fAttackRear = gHUD.m_Pain.m_fAttackRight = gHUD.m_Pain.m_fAttackLeft = 0.5;
			gHUD.m_Pain.setUniformDamage(damageFlashModTotal);
			return 1;
		}
	}

	//...is "armor" unused?  It comes from "pev->dmg_save". Does it have any purpose than to
	//trigger a damage draw on any "takeDamage" event, even if the damage is 0?

	if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(painFlashPrintouts) == 1)easyForcePrintLine("RAW DAMAGE %d %d", armor, damageTaken);
	// took damage
	//if ( damageTaken > 0 || armor > 0 )
	if (damageTaken > 0 || (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(painFlashArmorBlock) > 0 && damageBlockedByArmor > 0) || armor > 0)
		gHUD.m_Pain.CalcDamageDirection(vecFrom, damageTaken, rawDamageTaken);

	return 1;
}

int CHud::MsgFunc_Drowning(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int x = READ_BYTE();
	gHUD.m_Pain.playerIsDrowning = x;
	return 1;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////


IMPLEMENT_MESSAGE(PrintCl) {
	BEGIN_READ(pbuf, iSize);
	char* formattedString = READ_STRING();
	// print the string in console.   That's it.
	gEngfuncs.Con_Printf(formattedString);
	return 1;
}

//JUKEBOX!
IMPLEMENT_MESSAGE(JBoxReq){
	BEGIN_READ( pbuf, iSize );
	char* strReceive = READ_STRING();
	//Do the jukebox!

	easyPrintLine("DEBUG: JBoxReq received: %s", strReceive);

	//CLIENT_COMMAND(tempEd, "mp3 play media/Half-Life11.mp3");
	//gEngfuncs.pfnClientCmd("mp3 play media/Half-Life11.mp3");
	gEngfuncs.pfnClientCmd(strReceive);
	//use strReceive in there next!
	return 1;
}

//JUKEBOX!   Just send a "stop" request.
IMPLEMENT_MESSAGE(JBoxOff){
	gEngfuncs.pfnClientCmd("mp3 stop");
	return 1;
}

extern BOOL hasAutoMus;
IMPLEMENT_MESSAGE(AutoMus){
	if(hasAutoMus == FALSE){return 1;}  //don't even bother.

	//BEGIN_READ( pbuf, iSize );
	//char* strReceive = READ_STRING();
	gEngfuncs.pfnClientCmd("mp3 play media/AutoMus.mp3");
	return 1;
}

IMPLEMENT_MESSAGE(CliTest){
	gEngfuncs.pfnClientCmd("mp3 pause media/AutoMus.mp3");
	return 1;
}





IMPLEMENT_MESSAGE(FirstAppr){
	// Keep vars the server gets from the client in-check!
	lateCVarInit();

	// Not turned on by the Health msg_func, so may as well be turned on by this user message.
	// Turns out FirstAppr is also called after a transition finishes (map to map).
	gHUD.m_Pain.m_iFlags |= HUD_ACTIVE;

	return 1;
}//END OF MsgFunc_FirstAppr




IMPLEMENT_MESSAGE(UpdClientC){
#ifdef _DEBUG
// !!! This comment may be out of date.
//nothing to do here. This should never be called, this feature is unused for Debug mode.
//It already treats everything as ordinary CVars.
//CHANGE.  Even CVars have to be broadcasted from the server to the client for multipalyer to work right for
// players that join a game (as opposed to single-player or a player running the non-dedicated server).
// Point is, non-server running players don't get access to server-registered CVars, so they need to receive
// up-to-date values of them anytime they're changed for access (being cached clientside) for stuff that needs
// access to the same CVars clientside read-only, like almost any CVar involved with weapons, animations, submodels
// (wpn_allowGlockSilencer or whatever), etc.

	BEGIN_READ(pbuf, iSize);

	int argID = READ_SHORT();
	int argPRE = READ_SHORT();
	float arg = ((float)(argPRE)) / 100.0f;

	//????????????????

	*(aryCVarHash[argID]) = arg;


	// Strangely, the developer-requiriing printout method for clientside just isn't working in WON,
	// so non-forced calls don't show up at all.   Weird.
	// Either replace it or,  'eh'.   I don't really test on WON very often though.
	//easyForcePrintLine("AHA THIS MESSAGE IS FORCED dev:%.2f", CVAR_GET_FLOAT("developer"));
	
	easyPrintLine("CVAR DEBUG: Client: found ID %d. Set CVar %s to %.2f", argID, aryCVarHashName[argID], arg);
	
	//if (argID == 0) {
	//	gEngfuncs.pfnClientCmd("tcs_init_link");
	//}

#else
	// Need to update hidden CVars meant to be broadcasted to clients. Receive the new value(s) here.

	BEGIN_READ( pbuf, iSize );
	
	int argID = READ_SHORT();
	int argPRE = READ_SHORT();
	float arg = ((float)(argPRE)) / 100.0f;
	
	//????????????????


	//if(EASY_CVAR_GET_DEBUGONLY(hiddenMemPrintout) == 1)easyPrintLine("CVAR DEBUG: Client: Received ID %d, newval %.2f", argID, arg);


	*(aryCVarHash[argID]) = arg;

	// no need for the hiddenMemPrintout check, having "developer" on or off shows enough intent.
	//if(EASY_CVAR_GET_DEBUGONLY(hiddenMemPrintout)==1)
	easyPrintLine("CVAR DEBUG: Client: found ID %d. Set CVar %s to %.2f", argID, aryCVarHashName[argID], arg);
	

	//Save. Is this ok?
	// no, but queue it for the end of this frame to do so in case a lot of changes come through in a short time.
	g_queueCVarHiddenSave = TRUE;
	//saveHiddenCVars();



	//this allows all others to follow the formula of "start with else... okay to end with nothing".
	/*
	
	if(FALSE){

	}
	EASY_CVAR_CLIENTSENDOFF_LIST_CLIENT
	*/
	//...

#endif
	return 1;
}//END OF MsgFunc_UpdClientC





// Similar, but don't save the CVar to hidden mem.
// The initial sendoff at startup and broadcasts (debugonly or not) don't warrant saving.
IMPLEMENT_MESSAGE(UpdClientN){
#ifdef _DEBUG

	BEGIN_READ(pbuf, iSize);

	int argID = READ_SHORT();
	int argPRE = READ_SHORT();
	float arg = ((float)(argPRE)) / 100.0f;

	//float* testRef = aryCVarHash[argID-1];

	*(aryCVarHash[argID]) = arg;

	easyPrintLine("CVAR DEBUG: Client, no-save: found ID %d. Set CVar %s to %.2f", argID, aryCVarHashName[argID], arg);

#else
	//Need to update hidden CVars meant to be broadcasted to clients. Receive the new value(s) here.

	BEGIN_READ( pbuf, iSize );

	int argID = READ_SHORT();
	int argPRE = READ_SHORT();
	float arg = ((float)(argPRE)) / 100.0f;

	*(aryCVarHash[argID]) = arg;

	// no need for the hiddenMemPrintout check, having "developer" on or off shows enough intent.
	//if(EASY_CVAR_GET_DEBUGONLY(hiddenMemPrintout)==1)
	easyPrintLine("CVAR DEBUG: Client, no-save: found ID %d. Set CVar %s to %.2f", argID, aryCVarHashName[argID], arg);


	// Save. Is this ok?
	// no.
	//::saveHiddenCVars();


#endif
	return 1;
}//END OF MsgFunc_UpdClientN





IMPLEMENT_MESSAGE(RstClientC){

	EASY_CVAR_SET(hud_logo, DEFAULT_hud_logo);
	EASY_CVAR_SET(cl_fvox, DEFAULT_cl_fvox);
	EASY_CVAR_SET(cl_holster, DEFAULT_cl_holster);
	EASY_CVAR_SET(cl_breakholster, DEFAULT_cl_breakholster);
	EASY_CVAR_SET(cl_ladder, DEFAULT_cl_ladder);

	// still needed.
	resetModCVarsClientOnly();
	
	return 1;
}//END OF MsgFunc_RstClientC


IMPLEMENT_MESSAGE(PntClientC){
#ifdef _DEBUG

//nothing to do here. This should never be called, this feature is unused for Debug mode.
//It already treats everything as ordinary CVars.

#else
	//Need to update hidden CVars meant to be broadcasted to clients. Receive the new value(s) here.

	BEGIN_READ( pbuf, iSize );
	
	int argID = READ_SHORT();
	
	easyForcePrintLine("\"%s\" is %.2f. My ID was %d.",aryCVarHashName[argID], *aryCVarHash[argID], argID);\

#endif
	return 1;
}//END OF MsgFunc_PntClientC




IMPLEMENT_MESSAGE(UpdBnclStat){
	BEGIN_READ( pbuf, iSize );
	int arg = READ_SHORT();


	global2PSEUDO_grabbedByBarancle = (float)arg;

	return 1;
}


//MODDD - new way of updating player status (dead or alive).
IMPLEMENT_MESSAGE(UpdPlyA){

	BEGIN_READ( pbuf, iSize );
	int x = READ_SHORT();


	//This works because BOOLs are just integers that can only be 0 (off) or 1 (on).
	gHUD.m_fPlayerDead = !x;


	//if dead, and cannot show weapon select... force it off just in case.
	if(gHUD.m_fPlayerDead && EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(canShowWeaponSelectAtDeath) == 0){
		gHUD.m_Ammo.gWR.gpLastSel = gHUD.m_Ammo.gWR.gpActiveSel;
		gHUD.m_Ammo.gWR.gpActiveSel = NULL;
	}
	

	return 1;
}

IMPLEMENT_MESSAGE(MUnpause){
	//BEGIN_READ( pbuf, iSize );
	//???
	
	//that is all.
	gEngfuncs.pfnClientCmd("unpause");
	

	return 1;
}


//MODDD - Dummied, don't call!
IMPLEMENT_MESSAGE(UpdTWI){
	/*
	BEGIN_READ( pbuf, iSize );
	int x = READ_SHORT();

	//unpack.
	float newWeaponIdleTime = ((float)x) / 1000;
	
	flag_apply_m_flTimeWeaponIdle = TRUE;
	stored_m_flTimeWeaponIdle = newWeaponIdleTime;
	*/
	return 1;
}

//MODDD - Dummied, don't call!
IMPLEMENT_MESSAGE(UpdJT){
	
	/*
	BEGIN_READ( pbuf, iSize );
	int x = READ_BYTE();
	
	//unpack... no action needed, straight int.
	int newJustThrown = x;
	
	flag_apply_m_fJustThrown = TRUE;
	stored_m_fJustThrown = newJustThrown;
	*/
	return 1;
}


// Short for, CurWeaponForceNoSelectOnEmpty
IMPLEMENT_MESSAGE(CWFNSOE) {
	BEGIN_READ(pbuf, iSize);

	int iId = READ_CHAR();
	WEAPON* pWeapon = gHUD.m_Ammo.gWR.GetWeapon(iId);

	int resultVal = READ_BYTE();

	if (!pWeapon)
		return 0;

	pWeapon->fForceNoSelectOnEmpty = resultVal;

	return 1;
}

//gmsgCurWeaponForceNoSelectOnEmpty = REG_USER_MSG("CWFNSOE", 0);


// Get the server's DLL info for easy reference.
IMPLEMENT_MESSAGE(ServerDLLI) {
	BEGIN_READ(pbuf, iSize);

	strcpy(&globalbuffer_sv_mod_version_cache[0], READ_STRING());
	strcpy(&globalbuffer_sv_mod_date_cache[0], READ_STRING());

	// ifll it out
	sprintf(&globalbuffer_sv_mod_display[0], "SV: %s - %s", &globalbuffer_sv_mod_version_cache[0], &globalbuffer_sv_mod_date_cache[0]);

	return 1;
}




IMPLEMENT_MESSAGE(YMG){
	//BEGIN_READ( pbuf, iSize );
	//???

	playingMov = TRUE;
	movieStartTime = gHUD.recentTime;
	

	return 1;
}

IMPLEMENT_MESSAGE(YMG_S){
	//BEGIN_READ( pbuf, iSize );
	//???

	playingMov = FALSE;
	
	

	return 1;
}









