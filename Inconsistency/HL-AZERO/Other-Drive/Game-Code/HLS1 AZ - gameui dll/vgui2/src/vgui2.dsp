# Microsoft Developer Studio Project File - Name="dlls" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=dlls - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "dlls.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dlls.mak" CFG="dlls - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dlls - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dlls - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dlls - Win32 Profile" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "dlls - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /MT /W3 /GX /Zi /O2 /I "..\config\\" /I "." /I ".." /I "..\game_shared\util" /I "..\dlls" /I "..\engine" /I "..\common" /I "..\game_shared" /I "..\pm_shared" /I "..\\" /I "..\dlls\entity" /I "..\dlls\weapon" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "QUIVER" /D "VOXEL" /D "QUAKE2" /D "VALVE_DLL" /D "CLIENT_WEAPONS" /Fr /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o".\Release/az.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /map /debug /machine:I386 /def:".\dlls.def" /out:".\Release/az.dll"
# SUBTRACT LINK32 /profile

!ELSEIF  "$(CFG)" == "dlls - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\az___Win"
# PROP BASE Intermediate_Dir ".\az___Win"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /MTd /W3 /Gm /GR /GX /ZI /Od /I "..\config" /I "." /I ".." /I "..\game_shared\util" /I "..\dlls" /I "..\engine" /I "..\common" /I "..\game_shared" /I "..\pm_shared" /I "..\\" /I "..\dlls\entity" /I "..\dlls\weapon" /I "..\config\\" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "QUIVER" /D "VOXEL" /D "QUAKE2" /D "VALVE_DLL" /D "CLIENT_WEAPONS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "..\engine" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o".\Debug/az.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /def:".\dlls.def" /out:".\Debug/az.dll"
# SUBTRACT LINK32 /profile

!ELSEIF  "$(CFG)" == "dlls - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\az___Win"
# PROP BASE Intermediate_Dir ".\az___Win"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Profile"
# PROP Intermediate_Dir ".\Profile"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /Zi /O2 /I "..\engine" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "QUIVER" /D "VOXEL" /D "QUAKE2" /D "VALVE_DLL" /YX /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /G5 /MT /W3 /GX /Zi /O2 /I "..\config\\" /I "." /I ".." /I "..\game_shared\util" /I "..\dlls" /I "..\engine" /I "..\common" /I "..\game_shared" /I "..\pm_shared" /I "..\\" /I "..\dlls\entity" /I "..\dlls\weapon" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "QUIVER" /D "VOXEL" /D "QUAKE2" /D "VALVE_DLL" /D "CLIENT_WEAPONS" /Fr /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o".\Profile/az.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /debug /machine:I386 /def:".\dlls.def"
# SUBTRACT BASE LINK32 /profile
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /profile /debug /machine:I386 /def:".\dlls.def" /out:".\Profile/az.dll"

!ENDIF 

# Begin Target

# Name "dlls - Win32 Release"
# Name "dlls - Win32 Debug"
# Name "dlls - Win32 Profile"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;for;f90"
# Begin Group "Shared"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\game_shared\util\util_entity.cpp
# End Source File
# Begin Source File

SOURCE=..\game_shared\util\util_printout.cpp
# End Source File
# Begin Source File

SOURCE=..\game_shared\util\util_shared.cpp
# End Source File
# Begin Source File

SOURCE=..\game_shared\util\util_version.cpp
# End Source File
# Begin Source File

SOURCE=..\game_shared\voice_gamemgr.cpp
# End Source File
# End Group
# Begin Group "entity"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\entity\aflock.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\agrunt.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\airtank.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\animating.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\apache.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\archer.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\archer_ball.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\barnacle.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\barney.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\bigmomma.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\bmodels.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\bullsquid.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\buttons.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\chumtoad.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\controller.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\controller_head_ball.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\controller_zap_ball.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\crossbowbolt.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\doors.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\effects.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\explode.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\floater.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\flyingmonster.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\friendly.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\func_break.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\func_door_health.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\func_tank.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\gargantua.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\genericmonster.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\ggrenade.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\gib.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\gman.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\h_battery.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\h_cine.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\h_cycler.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\hassassin.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\hassault.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\headcrab.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\healthkit.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\hgrunt.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\hornet.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\hornet_kingpin.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\houndeye.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\ichthyosaur.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\islave.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\items.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\kingpin.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\kingpin_ball.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\leech.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\lights.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\maprules.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\monstermaker.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\mortar.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\nihilanth.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\nodes.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\osprey.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\panthereye.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\pathcorner.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\pickupwalker.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\plats.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\player.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\player_extra.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\rat.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\roach.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\scientist.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\scripted.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\snapbug.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\sound.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\soundent.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\spectator.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\squadmonster.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\squidspit.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\stukabat.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\subs.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\talkmonster.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\tentacle.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\triggers.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\turret.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\world.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\xen.cpp
# End Source File
# Begin Source File

SOURCE=.\entity\zombie.cpp
# End Source File
# End Group
# Begin Group "weapon"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\weapon\chumtoadweapon.cpp
# End Source File
# Begin Source File

SOURCE=.\weapon\crossbow.cpp
# End Source File
# Begin Source File

SOURCE=.\weapon\crowbar.cpp
# End Source File
# Begin Source File

SOURCE=.\weapon\egon.cpp
# End Source File
# Begin Source File

SOURCE=.\weapon\gauss.cpp
# End Source File
# Begin Source File

SOURCE=.\weapon\glock.cpp
# End Source File
# Begin Source File

SOURCE=.\weapon\handgrenade.cpp
# End Source File
# Begin Source File

SOURCE=.\weapon\hornetgun.cpp
# End Source File
# Begin Source File

SOURCE=.\weapon\mp5.cpp
# End Source File
# Begin Source File

SOURCE=.\weapon\python.cpp
# End Source File
# Begin Source File

SOURCE=.\weapon\rpg.cpp
# End Source File
# Begin Source File

SOURCE=.\weapon\satchel.cpp
# End Source File
# Begin Source File

SOURCE=.\weapon\shotgun.cpp
# End Source File
# Begin Source File

SOURCE=.\weapon\squeak.cpp
# End Source File
# Begin Source File

SOURCE=.\weapon\tripmine.cpp
# End Source File
# End Group
# Begin Group "common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\common\vector.cpp
# End Source File
# End Group
# Begin Group "_root"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\config\external_lib_include.cpp
# End Source File
# End Group
# Begin Group "pm_shared"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\pm_shared\pm_debug.c
# End Source File
# Begin Source File

SOURCE=..\pm_shared\pm_math.c
# End Source File
# Begin Source File

SOURCE=..\pm_shared\pm_shared.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\basemonster.cpp
# End Source File
# Begin Source File

SOURCE=.\cbase.cpp
# End Source File
# Begin Source File

SOURCE=.\client.cpp
# End Source File
# Begin Source File

SOURCE=.\client_message.cpp
# End Source File
# Begin Source File

SOURCE=.\combat.cpp
# End Source File
# Begin Source File

SOURCE=.\decals.cpp
# End Source File
# Begin Source File

SOURCE=.\defaultai.cpp
# End Source File
# Begin Source File

SOURCE=.\game.cpp
# End Source File
# Begin Source File

SOURCE=.\gamerules.cpp
# End Source File
# Begin Source File

SOURCE=.\gamerules_multiplay.cpp
# End Source File
# Begin Source File

SOURCE=.\gamerules_singleplay.cpp
# End Source File
# Begin Source File

SOURCE=.\gamerules_teamplay.cpp
# End Source File
# Begin Source File

SOURCE=.\globals.cpp
# End Source File
# Begin Source File

SOURCE=.\h_export.cpp
# End Source File
# Begin Source File

SOURCE=.\healthmodule.cpp
# End Source File
# Begin Source File

SOURCE=.\monstersavestate.cpp
# End Source File
# Begin Source File

SOURCE=.\monsterstate.cpp
# End Source File
# Begin Source File

SOURCE=.\plane.cpp
# End Source File
# Begin Source File

SOURCE=.\schedule.cpp
# End Source File
# Begin Source File

SOURCE=.\skill.cpp
# End Source File
# Begin Source File

SOURCE=.\util.cpp
# End Source File
# Begin Source File

SOURCE=.\util_debugdraw.cpp
# End Source File
# Begin Source File

SOURCE=.\util_model.cpp
# End Source File
# Begin Source File

SOURCE=.\weapons.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Group "Shared H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\game_shared\util\util_entity.h
# End Source File
# Begin Source File

SOURCE=..\game_shared\util\util_printout.h
# End Source File
# Begin Source File

SOURCE=..\game_shared\util\util_shared.h
# End Source File
# Begin Source File

SOURCE=..\game_shared\util\util_version.h
# End Source File
# End Group
# Begin Group "weapon H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\weapon\chumtoadweapon.h
# End Source File
# Begin Source File

SOURCE=.\weapon\crossbow.h
# End Source File
# Begin Source File

SOURCE=.\weapon\crowbar.h
# End Source File
# Begin Source File

SOURCE=.\weapon\egon.h
# End Source File
# Begin Source File

SOURCE=.\weapon\gauss.h
# End Source File
# Begin Source File

SOURCE=.\weapon\glock.h
# End Source File
# Begin Source File

SOURCE=.\weapon\handgrenade.h
# End Source File
# Begin Source File

SOURCE=.\weapon\hornetgun.h
# End Source File
# Begin Source File

SOURCE=.\weapon\mp5.h
# End Source File
# Begin Source File

SOURCE=.\weapon\python.h
# End Source File
# Begin Source File

SOURCE=.\weapon\rpg.h
# End Source File
# Begin Source File

SOURCE=.\weapon\satchel.h
# End Source File
# Begin Source File

SOURCE=.\weapon\shotgun.h
# End Source File
# Begin Source File

SOURCE=.\weapon\squeak.h
# End Source File
# Begin Source File

SOURCE=.\weapon\tripmine.h
# End Source File
# End Group
# Begin Group "entity H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\entity\animating.h
# End Source File
# Begin Source File

SOURCE=.\entity\archer.h
# End Source File
# Begin Source File

SOURCE=.\entity\archer_ball.h
# End Source File
# Begin Source File

SOURCE=.\entity\barnacle.h
# End Source File
# Begin Source File

SOURCE=.\entity\basebutton.h
# End Source File
# Begin Source File

SOURCE=.\entity\basetoggle.h
# End Source File
# Begin Source File

SOURCE=.\entity\chumtoad.h
# End Source File
# Begin Source File

SOURCE=.\entity\controller_head_ball.h
# End Source File
# Begin Source File

SOURCE=.\entity\controller_zap_ball.h
# End Source File
# Begin Source File

SOURCE=.\entity\crossbowbolt.h
# End Source File
# Begin Source File

SOURCE=.\entity\doors.h
# End Source File
# Begin Source File

SOURCE=.\entity\effects.h
# End Source File
# Begin Source File

SOURCE=.\entity\explode.h
# End Source File
# Begin Source File

SOURCE=.\entity\floater.h
# End Source File
# Begin Source File

SOURCE=.\entity\flyingmonster.h
# End Source File
# Begin Source File

SOURCE=.\entity\friendly.h
# End Source File
# Begin Source File

SOURCE=.\entity\func_break.h
# End Source File
# Begin Source File

SOURCE=.\entity\func_door_health.h
# End Source File
# Begin Source File

SOURCE=.\entity\gib.h
# End Source File
# Begin Source File

SOURCE=.\entity\hassault.h
# End Source File
# Begin Source File

SOURCE=.\entity\healthkit.h
# End Source File
# Begin Source File

SOURCE=.\entity\hgrunt.h
# End Source File
# Begin Source File

SOURCE=.\entity\hornet.h
# End Source File
# Begin Source File

SOURCE=.\entity\hornet_kingpin.h
# End Source File
# Begin Source File

SOURCE=.\entity\items.h
# End Source File
# Begin Source File

SOURCE=.\entity\kingpin.h
# End Source File
# Begin Source File

SOURCE=.\entity\kingpin_ball.h
# End Source File
# Begin Source File

SOURCE=.\entity\lights.h
# End Source File
# Begin Source File

SOURCE=.\entity\maprules.h
# End Source File
# Begin Source File

SOURCE=.\entity\nodes.h
# End Source File
# Begin Source File

SOURCE=.\entity\panthereye.h
# End Source File
# Begin Source File

SOURCE=.\entity\pickupwalker.h
# End Source File
# Begin Source File

SOURCE=.\entity\plats.h
# End Source File
# Begin Source File

SOURCE=.\entity\player.h
# End Source File
# Begin Source File

SOURCE=.\entity\player_extra.h
# End Source File
# Begin Source File

SOURCE=.\entity\scripted.h
# End Source File
# Begin Source File

SOURCE=.\entity\snapbug.h
# End Source File
# Begin Source File

SOURCE=.\entity\soundent.h
# End Source File
# Begin Source File

SOURCE=.\entity\spectator.h
# End Source File
# Begin Source File

SOURCE=.\entity\squadmonster.h
# End Source File
# Begin Source File

SOURCE=.\entity\squidspit.h
# End Source File
# Begin Source File

SOURCE=.\entity\stukabat.h
# End Source File
# Begin Source File

SOURCE=.\entity\talkmonster.h
# End Source File
# Begin Source File

SOURCE=.\entity\trains.h
# End Source File
# Begin Source File

SOURCE=.\entity\turret.h
# End Source File
# End Group
# Begin Group "common H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\common\vector.h
# End Source File
# End Group
# Begin Group "_root H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\config\external_lib_include.h
# End Source File
# End Group
# Begin Group "pm_shared H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\pm_shared\pm_debug.h
# End Source File
# Begin Source File

SOURCE=..\pm_shared\pm_shared.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\activity.h
# End Source File
# Begin Source File

SOURCE=.\activitymap.h
# End Source File
# Begin Source File

SOURCE=.\basemonster.h
# End Source File
# Begin Source File

SOURCE=.\cbase.h
# End Source File
# Begin Source File

SOURCE=.\cdll_dll.h
# End Source File
# Begin Source File

SOURCE=.\client.h
# End Source File
# Begin Source File

SOURCE=.\client_message.h
# End Source File
# Begin Source File

SOURCE=.\decals.h
# End Source File
# Begin Source File

SOURCE=.\defaultai.h
# End Source File
# Begin Source File

SOURCE=.\enginecallback.h
# End Source File
# Begin Source File

SOURCE=.\extdll.h
# End Source File
# Begin Source File

SOURCE=.\game.h
# End Source File
# Begin Source File

SOURCE=.\gamerules.h
# End Source File
# Begin Source File

SOURCE=.\gamerules_teamplay.h
# End Source File
# Begin Source File

SOURCE=.\healthmodule.h
# End Source File
# Begin Source File

SOURCE=.\linkedlist_ehandle.h
# End Source File
# Begin Source File

SOURCE=.\monsterevent.h
# End Source File
# Begin Source File

SOURCE=.\monstersavestate.h
# End Source File
# Begin Source File

SOURCE=.\plane.h
# End Source File
# Begin Source File

SOURCE=.\saverestore.h
# End Source File
# Begin Source File

SOURCE=.\schedule.h
# End Source File
# Begin Source File

SOURCE=.\scriptevent.h
# End Source File
# Begin Source File

SOURCE=.\skill.h
# End Source File
# Begin Source File

SOURCE=.\util.h
# End Source File
# Begin Source File

SOURCE=.\util_debugdraw.h
# End Source File
# Begin Source File

SOURCE=.\util_model.h
# End Source File
# Begin Source File

SOURCE=.\weapons.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
