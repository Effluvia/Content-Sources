//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
//=============================================================================

#include <stdio.h>

#include "VGuiSystemModuleLoader.h"
#include "Sys_Utils.h"
#include "IVGuiModule.h"
#include "ServerBrowser/IServerBrowser.h"

#include <vgui/IPanel.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>
#include <KeyValues.h>

#include <vgui_controls/Controls.h>
#include <vgui_controls/Panel.h>

#include "FileSystem.h"


// instance of class
CVGuiSystemModuleLoader g_VModuleLoader;


//MODDD - CRITICAL!  So do we want the GAMEUI_EXPORTS constant on or not?
// My guess yet is 'yes', but seeing how without it goes first.


#ifdef GAMEUI_EXPORTS

#include "TaskBar.h"
extern CTaskbar *g_pTaskbar; // for SetParent call
#else

//MODDD
// There is nothing rmeotely called this, I don't get what this was suppsoed to be
//#include "..\platform\PlatformMainPanel.h"
//extern CPlatformMainPanel *g_pMainPanel;

// How bout dis
#include "BasePanel.h"

#endif



//_GLOBAL__sub_I_g_VModuleLoader
//IBaseInterface * __CreateCVGuiSystemModuleLoaderIVGuiModuleLoader_interface(void)


// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>


/*
0x0140e334 "VFileSystem009"
0x01ea01ec "GameUI007"
0x01ea01f8 "GameConsole003"
0x01ea0208 "CareerUI001"
0x01ea0214 "VGUI_InputInternal001"
0x01ec5230 "VGUI_ivgui006"
0x01ec5220 "VGUI_Panel007"
0x01ec5200 "VGUI_Scheme009"

0x01ec51f0 "VGUI_System009"
0x01ec51e0 "VGUI_Input004"
0x01ec51cc "VGUI_Localize003"

0x01ec51ac "KeyValues003"
0x057f4f94 "VGUI_Surface027"
*/


// demoplayer001 ?
// why no VGuiModuleLoader003 called???


//MODDD - expandde
//EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CVGuiSystemModuleLoader, IVGuiModuleLoader, VGUIMODULELOADER_INTERFACE_VERSION, g_VModuleLoader);
static void* __CreateCVGuiSystemModuleLoaderIVGuiModuleLoader_interface() {
	return (IVGuiModuleLoader *)&g_VModuleLoader;
}
static InterfaceReg __g_CreateCVGuiSystemModuleLoaderIVGuiModuleLoader_reg(__CreateCVGuiSystemModuleLoaderIVGuiModuleLoader_interface, VGUIMODULELOADER_INTERFACE_VERSION);


void _GLOBAL__sub_I_g_VModuleLoader(void){
	return;
}



void _GLOBAL__sub_I_TheCareerGame(void)
{
	return;
}


class CCareerGame {
	void bastard1(void);
	void bastard2(void);
	void bastard3(void);
	void bastard4(void);
	void bastard5(void);
	void bastard6(void);
	void bastard7(void);
	void bastard8(void);
};


void CCareerGame::bastard1(void){

}
void CCareerGame::bastard2(void){

}
void CCareerGame::bastard3(void){

}
void CCareerGame::bastard4(void){

}
void CCareerGame::bastard5(void){

}
void CCareerGame::bastard6(void){

}
void CCareerGame::bastard7(void){

}
void CCareerGame::bastard8(void){

}


/*
********************************************
* vtable for CCareerGame                   *
********************************************
_ZTV11CCareerGame                    XREF[1]: Entry Point(*)  
CCareerGame::vtable
00174b40 00                                          ??              00h
00174b41 00                                          ??              00h
00174b42 00                                          ??              00h
00174b43 00                                          ??              00h
00174b44 0c 4b 17 00                                 addr            CCareerGame::typeinfo                       = 001e00f0
PTR_~CCareerGame_00174b48            XREF[2]: ~CCareerGame:000c15de(
	CCareerGame:000c236e(*
		00174b48 d0 15 0c 00                                 addr            CCareerGame::~CCareerGame
		00174b4c 20 19 0c 00                                 addr            CCareerGame::~CCareerGame
		00174b50 e0 13 0c 00                                 addr            CCareerGame::IsPlayingMatch
		00174b54 00 14 0c 00                                 addr            CCareerGame::GetCurrentTaskVec
		00174b58 90 57 0c 00                                 addr            CCareerGame::PlayAsCT
		00174b5c a0 57 0c 00                                 addr            CCareerGame::GetReputationGained
		00174b60 b0 57 0c 00                                 addr            CCareerGame::GetNumMapsUnlocked
		00174b64 00 13 0c 00                                 addr            CCareerGame::DoesWinUnlockAll
		00174b68 b0 12 0c 00                                 addr            CCareerGame::GetRoundTimeLength
		00174b6c c0 57 0c 00                                 addr            CCareerGame::GetWinfastLength
		00174b70 d0 57 0c 00                                 addr            CCareerGame::GetDifficulty
		00174b74 40 19 0c 00                                 addr            CCareerGame::GetCurrentMapTriplet
		00174b78 20 14 0c 00                                 addr            CCareerGame::OnRoundEndMenuOpen
		00174b7c 80 14 0c 00                                 addr            CCareerGame::OnMatchEndMenuOpen
		00174b80 60 15 0c 00                                 addr            CCareerGame::OnRoundEndMenuClose
		00174b84 e0 14 0c 00                                 addr            CCareerGame::OnMatchEndMenuClose
*/







CCareerGame g_CareerGame;

#define CCAREERGAME_INTERFACE_VERSION "CareerUI001"

static void* __CreateCCareerGameICareerUI_interface() {
	return (CCareerGame*)&g_CareerGame;
}
static InterfaceReg __g_CreateCCareerGameICareerUI_reg(__CreateCCareerGameICareerUI_interface, CCAREERGAME_INTERFACE_VERSION);



//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CVGuiSystemModuleLoader::CVGuiSystemModuleLoader()
{
	m_bModulesInitialized = false;
	m_bPlatformShouldRestartAfterExit = false;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CVGuiSystemModuleLoader::~CVGuiSystemModuleLoader()
{
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the module loader has acquired the platform mutex and loaded the modules
//-----------------------------------------------------------------------------
bool CVGuiSystemModuleLoader::IsPlatformReady()
{
	return m_bModulesInitialized;
}

//-----------------------------------------------------------------------------
// Purpose: sets up all the modules for use
//-----------------------------------------------------------------------------
void CVGuiSystemModuleLoader::InitializeAllModules(CreateInterfaceFn *factorylist, int factorycount)
{
	int i;

	// Init vgui in the modules
	for (i = 0; i < m_Modules.Size(); i++)
	{
		if (!m_Modules[i].moduleInterface->Initialize(factorylist, factorycount))
		{
			vgui::ivgui()->DPrintf2("Platform Error: module failed to initialize\n");
		}
	}

	// create a table of all the loaded modules
	
	//MODDD - avoid alloca.
	//CreateInterfaceFn *moduleFactories = (CreateInterfaceFn *)_alloca(sizeof(CreateInterfaceFn) * m_Modules.Size());
	CreateInterfaceFn *moduleFactories = (CreateInterfaceFn *)malloc(sizeof(CreateInterfaceFn) * m_Modules.Size());

	for (i = 0; i < m_Modules.Size(); i++)
	{
		moduleFactories[i] = Sys_GetFactory(m_Modules[i].module);
	}

	// give the modules a chance to link themselves together
	for (i = 0; i < m_Modules.Size(); i++)
	{

		module_t& thisMod = m_Modules[i];

		if (!m_Modules[i].moduleInterface->PostInitialize(moduleFactories, m_Modules.Size()))
		{
			vgui::ivgui()->DPrintf2("Platform Error: module failed to initialize\n");
		}
		
#ifdef GAMEUI_EXPORTS
		m_Modules[i].moduleInterface->SetParent(g_pTaskbar->GetVPanel());
		g_pTaskbar->AddTask(m_Modules[i].moduleInterface->GetPanel());
#else
		//MODDD - ahee
		//m_Modules[i].moduleInterface->SetParent(g_pMainPanel->GetVPanel());		
		m_Modules[i].moduleInterface->SetParent( GetGameUIBasePanel() );		
#endif
	}

	m_bModulesInitialized = true;
}

//-----------------------------------------------------------------------------
// Purpose: Loads and initializes all the modules specified in the platform file
//-----------------------------------------------------------------------------
void CVGuiSystemModuleLoader::LoadPlatformModules(CreateInterfaceFn *factorylist, int factorycount, bool useSteamModules)
{
	// !!! WASNT COMMENTED OUT BEFORE
	
	// load platform menu
	KeyValues *kv = new KeyValues("Platform");
	if (!kv->LoadFromFile(vgui::filesystem(), "resource/PlatformMenu.vdf", "PLATFORM"))
		return;

	// walk the platform menu loading all the interfaces
	KeyValues *menuKeys = kv->FindKey("Menu", true);
	for (KeyValues *it = menuKeys->GetFirstSubKey(); it != NULL; it = it->GetNextKey())
	{
		// see if we should skip steam modules
		if (!useSteamModules && it->GetInt("SteamApp"))
			continue;

		// get copy out of steam cache
		const char *dllPath = it->GetString("dll");
		vgui::filesystem()->GetLocalCopy(dllPath);

		// load the dll
		char szDir[512];
		//MODDD - GetLocalPath LAZY DEFAULT   oh wait, not so lazy now
		if (!vgui::filesystem()->GetLocalPath(dllPath, szDir, 512))
		{
			vgui::ivgui()->DPrintf2("Platform Error: couldn't find %s, not loading\n", it->GetString("dll"));
			continue;
		}

		// make sure it's a valid dll
		CSysModule *mod = Sys_LoadModule(szDir);
		if (!mod)
		{
			vgui::ivgui()->DPrintf2("Platform Error: bad module '%s', not loading\n", it->GetString("dll"));
			continue;
		}

		// make sure we get the right version
		IVGuiModule *moduleInterface = (IVGuiModule *)Sys_GetFactory(mod)(it->GetString("interface"), NULL);
		if (!moduleInterface)
		{
			vgui::ivgui()->DPrintf2("Platform Error: module version ('%s, %s) invalid, not loading\n", it->GetString("dll"), it->GetString("interface"));
			continue;
		}

		// store off the module
		int newIndex = m_Modules.AddToTail();
		m_Modules[newIndex].module = mod;
		m_Modules[newIndex].moduleInterface = moduleInterface;
		m_Modules[newIndex].data = it;
	}


	//!!! SECTION FOUND COMMENTED OUT.  leave?
/*
    // find all the AddOns
	FileFindHandle_t findHandle = NULL;
	const char *filename = vgui::filesystem()->FindFirst("AddOns/*.", &findHandle);

    for ( ; filename != NULL ; filename = vgui::filesystem()->FindNext(findHandle))
//    while (filename)
    {
        // skip the . directories
        if (vgui::filesystem()->FindIsDirectory ( findHandle ) && filename[0] != '.')
        {
            // add this to list
            const char *gameName = filename;
            KeyValues* kv = new KeyValues(gameName);
            char fileName[512];
            sprintf(fileName, "AddOns/%s/%s.vdf", gameName, gameName);
            if (!kv->LoadFromFile(vgui::filesystem(), fileName, true, "PLATFORM"))
                continue;

            sprintf(fileName, "AddOns/%s/%s.dll", gameName, gameName);
            vgui::filesystem()->GetLocalCopy(fileName);
            CSysModule *mod = Sys_LoadModule(fileName);
            if (!mod)
                continue;

            // make sure we get the right version
            IVGuiModule *moduleInterface = (IVGuiModule *)Sys_GetFactory(mod)(kv->GetString("interface"), NULL);
            if (!moduleInterface)
                continue;

			// hide it from the Steam Platform Menu
			kv->SetInt("Hidden", 1);

            // store off the module
            int newIndex = m_Modules.AddToTail();
            m_Modules[newIndex].module = mod;
            m_Modules[newIndex].moduleInterface = moduleInterface;
            m_Modules[newIndex].data = kv;
        }
    }
	vgui::filesystem()->FindClose(findHandle);
*/

	// !!! WASNT COMMENTED OUT BEFORE
	InitializeAllModules(factorylist, factorycount);
	




	// SRC2007 WAY
	/*
	bool bSuccess = true;

	// load platform menu
	KeyValues *kv = new KeyValues("Platform");
	// g_pFullFileSystem
	if (!kv->LoadFromFile(vgui::filesystem(), "steam/games/PlatformMenu.vdf", "PLATFORM"))
	{
		kv->deleteThis();
		//return false;
		return;
	}

	// walk the platform menu loading all the interfaces
	KeyValues *menuKeys = kv->FindKey("Menu", true);
	for (KeyValues *it = menuKeys->GetFirstSubKey(); it != NULL; it = it->GetNextKey())
	{
		// see if we should skip steam modules
		if (!useSteamModules && it->GetInt("SteamApp"))
			continue;

		const char *pchInterface = it->GetString("interface");

		// don't load friends if we are using Steam Community
		//if ( !Q_stricmp( pchInterface, "VGuiModuleTracker001" )) && bSteamCommunityFriendsVersion )
		//	continue;

		// get copy out of steam cache
		const char *dllPath = it->GetString("dll");

		// load the module (LoadModule calls GetLocalCopy() under steam)
		CSysModule* mod = vgui::filesystem()->LoadModule(dllPath);//, "EXECUTABLE_PATH");
		if (!mod)
		{
			Error("Platform Error: bad module '%s', not loading\n", it->GetString("dll"));
			bSuccess = false;
			continue;
		}

		// make sure we get the right version
		IVGuiModule *moduleInterface = (IVGuiModule *)Sys_GetFactory(mod)(pchInterface, NULL);
		if (!moduleInterface)
		{
			Warning("Platform Error: module version ('%s, %s) invalid, not loading\n", it->GetString("dll"), it->GetString("interface"));
			bSuccess = false;
			continue;
		}

		// store off the module
		int newIndex = m_Modules.AddToTail();
		m_Modules[newIndex].module = mod;
		m_Modules[newIndex].moduleInterface = moduleInterface;
		m_Modules[newIndex].data = it;
	}

	//m_pPlatformModuleData = kv;
	//return InitializeAllModules(factorylist, factorycount) && bSuccess;
	InitializeAllModules(factorylist, factorycount);
	*/

}

//-----------------------------------------------------------------------------
// Purpose: gives all platform modules a chance to Shutdown gracefully
//-----------------------------------------------------------------------------
void CVGuiSystemModuleLoader::ShutdownPlatformModules()
{
	int i;
	// static include guard to prevent recursive calls
	static bool runningFunction = false;
	if (runningFunction)
		return;

	runningFunction = true;

	// deactivate all the modules first
	DeactivatePlatformModules();

	// give all the modules notice of quit
	for (i = 0; i < m_Modules.Size(); i++)
	{
		vgui::ivgui()->PostMessage(m_Modules[i].moduleInterface->GetPanel(), new KeyValues("Command", "command", "Quit"), NULL, 0);
	}

	for (i = 0; i < m_Modules.Size(); i++)
	{
		m_Modules[i].moduleInterface->Shutdown();
	}

	runningFunction = false;
}

//-----------------------------------------------------------------------------
// Purpose: Deactivates all the modules (puts them into in inactive but recoverable state)
//-----------------------------------------------------------------------------
void CVGuiSystemModuleLoader::DeactivatePlatformModules()
{
	for (int i = 0; i < m_Modules.Size(); i++)
	{
		m_Modules[i].moduleInterface->Deactivate();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Reenables all the deactivated platform modules
//-----------------------------------------------------------------------------
void CVGuiSystemModuleLoader::ReactivatePlatformModules()
{
	for (int i = 0; i < m_Modules.Size(); i++)
	{
		m_Modules[i].moduleInterface->Reactivate();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Disables and unloads platform
//-----------------------------------------------------------------------------
void CVGuiSystemModuleLoader::UnloadPlatformModules()
{
	for (int i = 0; i < m_Modules.Count(); i++)
	{
		Sys_UnloadModule(m_Modules[i].module);
	}

	m_Modules.RemoveAll();
}

//-----------------------------------------------------------------------------
// Purpose: Called every frame
//-----------------------------------------------------------------------------
void CVGuiSystemModuleLoader::RunFrame()
{
}

//-----------------------------------------------------------------------------
// Purpose: returns number of modules loaded
//-----------------------------------------------------------------------------
int CVGuiSystemModuleLoader::GetModuleCount()
{
	return m_Modules.Size();
}

//-----------------------------------------------------------------------------
// Purpose: returns the string menu name (unlocalized) of a module
//			moduleIndex is of the range [0, GetModuleCount())
//-----------------------------------------------------------------------------
const char *CVGuiSystemModuleLoader::GetModuleLabel(int moduleIndex)
{
	return m_Modules[moduleIndex].data->GetString("MenuName", "< unknown >");
}

//-----------------------------------------------------------------------------
// Purpose: brings the specified module to the foreground
//-----------------------------------------------------------------------------
bool CVGuiSystemModuleLoader::IsModuleHidden(int moduleIndex)
{
	return m_Modules[moduleIndex].data->GetInt("Hidden", 0);
}

//-----------------------------------------------------------------------------
// Purpose: brings the specified module to the foreground
//-----------------------------------------------------------------------------
bool CVGuiSystemModuleLoader::ActivateModule(int moduleIndex)
{
	if (!m_Modules.IsValidIndex(moduleIndex))
		return false;

	m_Modules[moduleIndex].moduleInterface->Activate();

#ifdef GAMEUI_EXPORTS
	if (g_pTaskbar)
	{
		wchar_t *wTitle;
		wchar_t w_szTitle[1024];

		wTitle = vgui::localize()->Find(m_Modules[moduleIndex].data->GetName());

		if(!wTitle)
		{
			vgui::localize()->ConvertANSIToUnicode(m_Modules[moduleIndex].data->GetName(),w_szTitle,sizeof(w_szTitle));
			wTitle = w_szTitle;
		}

		g_pTaskbar->SetTitle(m_Modules[moduleIndex].moduleInterface->GetPanel(),wTitle);
	
	}
#endif
	
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: activates a module by name
//-----------------------------------------------------------------------------
bool CVGuiSystemModuleLoader::ActivateModule(const char *moduleName)
{
	for (int i = 0; i < GetModuleCount(); i++)
	{
		if (!stricmp(GetModuleLabel(i), moduleName) || !stricmp(m_Modules[i].data->GetName(), moduleName))
		{
			ActivateModule(i);
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: returns a modules interface factory
//-----------------------------------------------------------------------------
CreateInterfaceFn CVGuiSystemModuleLoader::GetModuleFactory(int moduleIndex)
{
	return Sys_GetFactory(m_Modules[moduleIndex].module);
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CVGuiSystemModuleLoader::PostMessageToAllModules(KeyValues *message)
{
	for (int i = 0; i < m_Modules.Size(); i++)
	{
		vgui::ivgui()->PostMessage(m_Modules[i].moduleInterface->GetPanel(), message->MakeCopy(), NULL, 0);
	}
	message->deleteThis();
}

//-----------------------------------------------------------------------------
// Purpose: sets the the platform should update and restart when it quits
//-----------------------------------------------------------------------------
void CVGuiSystemModuleLoader::SetPlatformToRestart()
{
	m_bPlatformShouldRestartAfterExit = true;
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
bool CVGuiSystemModuleLoader::ShouldPlatformRestart()
{
	return m_bPlatformShouldRestartAfterExit;
}
