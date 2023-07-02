




#include "const.h"
#include "progdefs.h"




//Make the messages public to the world, why not.

extern int gmsgJukeboxRequest;
extern int gmsgJukeboxOff;
extern int gmsgAutoMus;
extern int gmsgCliTest;

extern int gmsgUnpause;

extern int gmsgOnFirstAppearance;

extern int gmsgUpdateClientCVar;
extern int gmsgUpdateClientCVarNoSave;
extern int gmsgResetClientCVar;
extern int gmsgPrintClientCVar;

extern int gmsgShake;
extern int gmsgFade;
extern int gmsgSelAmmo;
extern int gmsgFlashlight;
extern int gmsgFlashBattery;
extern int gmsgResetHUD;
extern int gmsgInitHUD;
extern int gmsgShowGameTitle;
extern int gmsgCurWeapon;
extern int gmsgHealth;
extern int gmsgDamage;
extern int gmsgBattery;

//MODDD - new event
extern int gmsgAntidoteP;
extern int gmsgAdrenalineP;
extern int gmsgRadiationP;
extern int gmsgDrowning;
extern int gmsgHUDItemFlash;


extern int gmsgUpdateAirTankAirTime;
extern int gmsgUpdateLongJumpCharge;

extern int gmsgUpdatePlayerAlive;
extern int gmsgClearWeapon;

//extern int gmsgUpdateCam;
extern int gmsgUpdBnclStat;

extern int gmsgUpdateAlphaCrosshair;
extern int gmsgUpdateFreezeStatus;

extern int gmsgHasGlockSilencer;
extern int gmsgTrain;
extern int gmsgLogo;
extern int gmsgWeaponList;
extern int gmsgAmmoX;
extern int gmsgHudText;
extern int gmsgDeathMsg;
extern int gmsgScoreInfo;
extern int gmsgTeamInfo;
extern int gmsgTeamScore;
extern int gmsgGameMode;
extern int gmsgMOTD;
extern int gmsgServerName;
extern int gmsgAmmoPickup;
extern int gmsgWeapPickup;
extern int gmsgItemPickup;
extern int gmsgHideWeapon;

//extern int gmsgSetCurWeap;

extern int gmsgSayText;
extern int gmsgTextMsg;
extern int gmsgSetFOV;
extern int gmsgShowMenu;
extern int gmsgGeigerRange;
extern int gmsgTeamNames;

extern int gmsgStatusText;
extern int gmsgStatusValue;

extern int gmsgPrintClient;

extern int gmsgTimeWeaponIdleUpdate;
extern int gmsgJustThrownUpdate;
extern int gmsgCurWeaponForceNoSelectOnEmpty;



extern int gmsgServerDLL_Info;

extern int gmsgYMG;
extern int gmsgYMG_Stop;



//MODDD - new, let this be available to whatever wants it.
extern void LinkUserMessages( void );


extern void submitJukeboxRequest(entvars_t* pev, const char* cmdStringSend);
extern void submitJukeboxRequest(edict_t* pev, const char* cmdStringSend);
extern void submitJukeboxOff(entvars_t* pev);
extern void submitJukeboxOff(edict_t* pev);
extern void submitUnpauseRequest(entvars_t* pev);
extern void submitUnpauseRequest(edict_t* pev);

//extern void sendTimeWeaponIdleUpdate(edict_t* pev, float argVal);
//extern void sendJustThrown(edict_t* pev, int argVal);

extern void message_ymg(edict_t* pev);
extern void message_ymg_stop(edict_t* pev);


