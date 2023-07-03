

#include "extdll.h"
#include "client_message.h"

//
//#include "util.h"
//#include "cbase.h"

#include "shake.h"
#include "enginecallback.h"

#include "util_printout.h"


//MODDD - messages moved here from player.cpp.
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////


int gmsgShake = 0;
int gmsgFade = 0;
int gmsgSelAmmo = 0;
int gmsgFlashlight = 0;
int gmsgFlashBattery = 0;
int gmsgResetHUD = 0;
int gmsgInitHUD = 0;
int gmsgShowGameTitle = 0;
int gmsgCurWeapon = 0;
int gmsgHealth = 0;
int gmsgDamage = 0;
int gmsgBattery = 0;

//MODDD - new event
int gmsgAntidoteP = 0;
int gmsgAdrenalineP = 0;
int gmsgRadiationP = 0;
int gmsgDrowning = 0;
int gmsgHUDItemFlash = 0;


int gmsgUpdateAirTankAirTime = 0;
int gmsgUpdateLongJumpCharge = 0;

int gmsgUpdatePlayerAlive = 0;
int gmsgClearWeapon = 0;

//int gmsgUpdateCam = 0;
int gmsgUpdBnclStat = 0;

int gmsgUpdateAlphaCrosshair = 0;
int gmsgUpdateFreezeStatus = 0;

int gmsgHasGlockSilencer = 0;
int gmsgTrain = 0;
int gmsgLogo = 0;
int gmsgWeaponList = 0;
int gmsgAmmoX = 0;
int gmsgHudText = 0;
int gmsgDeathMsg = 0;
int gmsgScoreInfo = 0;
int gmsgTeamInfo = 0;
int gmsgTeamScore = 0;
int gmsgGameMode = 0;
int gmsgMOTD = 0;
int gmsgServerName = 0;
int gmsgAmmoPickup = 0;
int gmsgWeapPickup = 0;
int gmsgItemPickup = 0;
int gmsgHideWeapon = 0;

//MODDD - unused. Redundant with gmsgCurWeapon perhaps?
//int gmsgSetCurWeap = 0;

int gmsgSayText = 0;
int gmsgTextMsg = 0;
int gmsgSetFOV = 0;
int gmsgShowMenu = 0;
int gmsgGeigerRange = 0;
int gmsgTeamNames = 0;

int gmsgStatusText = 0;
int gmsgStatusValue = 0; 

int gmsgPrintClient = 0;

int gmsgJukeboxRequest = 0;
int gmsgJukeboxOff = 0;
int gmsgAutoMus = 0;
int gmsgCliTest = 0;

int gmsgUnpause = 0;

int gmsgOnFirstAppearance = 0;

int gmsgUpdateClientCVar = 0;
int gmsgUpdateClientCVarNoSave = 0;
int gmsgResetClientCVar = 0;
int gmsgPrintClientCVar = 0;

//int gmsgTimeWeaponIdleUpdate = 0;
//int gmsgJustThrownUpdate = 0;
int gmsgCurWeaponForceNoSelectOnEmpty = 0;


int gmsgServerDLL_Info = 0;

int gmsgYMG = 0;
int gmsgYMG_Stop = 0;



//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////





void LinkUserMessages( void )
{
	// Already taken care of?
	if ( gmsgSelAmmo )
	{
		return;
	}


	gmsgSelAmmo = REG_USER_MSG("SelAmmo", sizeof(SelAmmo));
	gmsgCurWeapon = REG_USER_MSG("CurWeapon", 3);
	gmsgGeigerRange = REG_USER_MSG("Geiger", 1);
	gmsgFlashlight = REG_USER_MSG("Flashlight", 2);
	gmsgFlashBattery = REG_USER_MSG("FlashBat", 1);
	gmsgHealth = REG_USER_MSG( "Health", 1 );

	//MODDD - size is now 16, needs 4 extra for the additional bitmask.
	//MODDD - another 1 byte added to support sending "raw intended damage" before armor reduction too.
	//gmsgDamage = REG_USER_MSG( "Damage", 12 );
	gmsgDamage = REG_USER_MSG( "Damage", 17 );

	gmsgBattery = REG_USER_MSG( "Battery", 2);




	//MODDD
	gmsgAntidoteP = REG_USER_MSG( "AntidoteP", 2);
	gmsgAdrenalineP = REG_USER_MSG( "AdrenalineP", 2);
	gmsgRadiationP = REG_USER_MSG( "RadiationP", 2);
	gmsgUpdateAirTankAirTime = REG_USER_MSG("UpdTankTime", 2);
	gmsgDrowning = REG_USER_MSG("Drowning", 1);
	gmsgHUDItemFlash = REG_USER_MSG("HUDItemFsh", 1);
	
	gmsgUpdateLongJumpCharge = REG_USER_MSG("UpdLJCharge", 2);
	
	gmsgUpdatePlayerAlive = REG_USER_MSG("UpdPlyA", 2);

	gmsgClearWeapon = REG_USER_MSG("ClearWpn", 0);

	//gmsgUpdateCam = REG_USER_MSG("UpdateCam", 0);
	gmsgUpdBnclStat = REG_USER_MSG("UpdBnclStat", 1);



	gmsgUpdateAlphaCrosshair = REG_USER_MSG("UpdACH", 0);
	gmsgUpdateFreezeStatus = REG_USER_MSG("UpdFrz", 1);

	gmsgHasGlockSilencer = REG_USER_MSG("HasGlockSil", 2);



	gmsgTrain = REG_USER_MSG( "Train", 1);
	gmsgHudText = REG_USER_MSG( "HudText", -1 );
	gmsgSayText = REG_USER_MSG( "SayText", -1 );
	gmsgTextMsg = REG_USER_MSG( "TextMsg", -1 );
	gmsgWeaponList = REG_USER_MSG("WeaponList", -1);
	gmsgResetHUD = REG_USER_MSG("ResetHUD", 1);		// called every respawn
	gmsgInitHUD = REG_USER_MSG("InitHUD", 0 );		// called every time a new player joins the server
	gmsgShowGameTitle = REG_USER_MSG("GameTitle", 1);
	gmsgDeathMsg = REG_USER_MSG( "DeathMsg", -1 );
	gmsgScoreInfo = REG_USER_MSG( "ScoreInfo", 7 );  //MODDD - reduced by 2 since removing the playerclass parameter
	gmsgTeamInfo = REG_USER_MSG( "TeamInfo", -1 );  // sets the name of a player's team
	gmsgTeamScore = REG_USER_MSG( "TeamScore", -1 );  // sets the score of a team on the scoreboard
	gmsgGameMode = REG_USER_MSG( "GameMode", 1 );
	gmsgMOTD = REG_USER_MSG( "MOTD", -1 );
	gmsgServerName = REG_USER_MSG( "ServerName", -1 );
	gmsgAmmoPickup = REG_USER_MSG( "AmmoPickup", 2 );
	gmsgWeapPickup = REG_USER_MSG( "WeapPickup", 1 );
	gmsgItemPickup = REG_USER_MSG( "ItemPickup", -1 );
	gmsgHideWeapon = REG_USER_MSG( "HideWeapon", 1 );
	gmsgSetFOV = REG_USER_MSG( "SetFOV", 1 );
	gmsgShowMenu = REG_USER_MSG( "ShowMenu", -1 );

	// Yes, even this message's handling on clientside.  No idea if any effects of that are interpreted
	// or handled codebase (view.cpp?  camera-someting.cpp?)
	gmsgShake = REG_USER_MSG("ScreenShake", sizeof(ScreenShake));

	gmsgFade = REG_USER_MSG("ScreenFade", sizeof(ScreenFade));
	gmsgAmmoX = REG_USER_MSG("AmmoX", 2);
	gmsgTeamNames = REG_USER_MSG( "TeamNames", -1 );


	//MODDD - these were present in the recent SDK but not the 'ancient' one.
	gmsgStatusText = REG_USER_MSG("StatusText", -1);
	gmsgStatusValue = REG_USER_MSG("StatusValue", 3); 
	
	
	gmsgPrintClient = REG_USER_MSG("PrintCl", -1);
	
	
	gmsgJukeboxRequest = REG_USER_MSG("JBoxReq", -1);
	gmsgJukeboxOff = REG_USER_MSG("JBoxOff", 0);
	gmsgAutoMus = REG_USER_MSG("AutoMus", 0);
	gmsgCliTest = REG_USER_MSG("CliTest", 0);

	gmsgUnpause = REG_USER_MSG("MUnpause", 0);

	gmsgOnFirstAppearance = REG_USER_MSG("FirstAppr", 0);

	gmsgUpdateClientCVar = REG_USER_MSG("UpdClientC", 4);
	gmsgUpdateClientCVarNoSave = REG_USER_MSG("UpdClientN", 4);
	gmsgResetClientCVar = REG_USER_MSG("RstClientC", 0);
	gmsgPrintClientCVar = REG_USER_MSG("PntClientC", 2);

	// Dummied!
	//gmsgTimeWeaponIdleUpdate = REG_USER_MSG("UpdTWI", 2);
	//gmsgJustThrownUpdate = REG_USER_MSG("UpdJT", 1);

	gmsgCurWeaponForceNoSelectOnEmpty = REG_USER_MSG("CWFNSOE", 2);



	gmsgServerDLL_Info = REG_USER_MSG("ServerDLLI", -1);

	

	gmsgYMG = REG_USER_MSG("YMG", 0);
	gmsgYMG_Stop = REG_USER_MSG("YMG_S", 0);

}//END OF LinkUserMessages










void submitJukeboxRequest(entvars_t* pev, const char* cmdStringSend){
	submitJukeboxRequest(ENT(pev), cmdStringSend);
}
void submitJukeboxRequest(edict_t* pev, const char* cmdStringSend){
	MESSAGE_BEGIN( MSG_ONE, gmsgJukeboxRequest, NULL, pev );
		WRITE_STRING(cmdStringSend);
	MESSAGE_END();
}

void submitJukeboxOff(entvars_t* pev){
	submitJukeboxOff(ENT(pev));
}
void submitJukeboxOff(edict_t* pev){
	MESSAGE_BEGIN( MSG_ONE, gmsgJukeboxOff, NULL, pev );
	MESSAGE_END();
}


void submitUnpauseRequest(entvars_t* pev){
	submitUnpauseRequest(ENT(pev));
}
void submitUnpauseRequest(edict_t* pev){
	MESSAGE_BEGIN( MSG_ONE, gmsgUnpause, NULL, pev );
	MESSAGE_END();
}


// REMOVED. not doing this anymore.  Use properly synched vars or bitflags.
// And why was updating m_flTimeWeaponIdle even a thing?  Isn't that already synched?
/*
void sendTimeWeaponIdleUpdate(edict_t* pev, float argVal){

	int tempcon = (int)(argVal * 1000);

	MESSAGE_BEGIN( MSG_ONE, gmsgTimeWeaponIdleUpdate, NULL, pev );
		WRITE_SHORT( tempcon );
	MESSAGE_END();
}

void sendJustThrown(edict_t* pev, int argVal){

	//whole number
	int tempcon = argVal;

	MESSAGE_BEGIN( MSG_ONE, gmsgJustThrownUpdate, NULL, pev );
		WRITE_BYTE( tempcon );
	MESSAGE_END();
}
*/

void message_ymg(edict_t* pev){
	MESSAGE_BEGIN( MSG_ONE, gmsgYMG, NULL, pev );
	MESSAGE_END();
}
void message_ymg_stop(edict_t* pev){
	MESSAGE_BEGIN( MSG_ONE, gmsgYMG_Stop, NULL, pev );
	MESSAGE_END();
}