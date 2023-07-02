:begin
@Echo Off
Title Batch compiler for NightFire maps made by 007 Tres 
If Not Exist bbsp.exe Goto errornotfoundf
If Not Exist brad.exe Goto errornotfoundf
If Not Exist bvis.exe Goto errornotfoundf
If Not Exist corrector.exe Goto errornotfoundf
if not exist nfinit.cfg (
If 1%console%==1Yes (Set console=1) Else Set console=0
If 1%screen%==1fullscreen (Set screen=0) Else Set screen=1
If 1%noipx%==1SinglePlayer (Set noipx=0) Else Set noipx=1
If 1%dedserver%==1Yes (Set dedserver=1) Else Set dedserver=0
Del nfinit.cfg /f /q
Echo [Startnf]=>>nfinit.cfg  
Echo console=1=>>nfinit.cfg
Echo screen=0=>>nfinit.cfg
Echo noipx=0=>>nfinit.cfg
Echo dedserver=0=>>nfinit.cfg
Echo hunk=40=>>nfinit.cfg
Echo heap=148=>>nfinit.cfg
Echo comm==>>nfinit.cfg
echo nfdir=none=>>nfinit.cfg
)
For /f "tokens=1,2* delims==" %%a In (nfinit.cfg) Do Set %%a=%%b
If 1%console%==10 (Set console=No) Else Set console=Yes
If 1%dedserver%==10 (Set dedserver=No) Else Set dedserver=Yes
If 1%screen%==10 (Set screen=fullscreen) Else Set screen=windowed
If 1%noipx%==10 (Set noipx=SinglePlayer) Else Set noipx=Multiplayer

Set statnf=No
cls
Echo.
Set cmdln=p%1
If %cmdln%==p Goto muin
Set mapcom=%1 %2 %3 %4 %5 %6 %7 %8 %9
If Exist %mapcom% Echo si
If Exist mapa.txt Del mapa.txt /f /q
Echo %mapcom%>>mapa.txt
For /f "tokens=1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23* delims=.\ " %%a In (mapa.txt) Do If Exist "%%~a\%%b.map" (Set direc=%%~a\) Else If Exist "%%~a\%%b\%%c.map" (Set direc=%%~a\%%b) Else If Exist "%%~a\%%b\%%c\%%d.map" (Set direc=%%~a\%%b\%%c) Else If Exist "%%~a\%%b\%%c\%%d\%%e.map" (Set direc=%%~a\%%b\%%c\%%d\) Else If Exist "%%~a\%%b\%%c\%%d\%%e\%%f.map" (Set direc=%%~a\%%b\%%c\%%d\%%e) Else If Exist "%%~a\%%b\%%c\%%d\%%e\%%f\%%g.map" (Set direc=%%~a\%%b\%%c\%%d\%%e\%%f) Else If Exist "%%~a\%%b\%%c\%%d\%%e\%%f\%%g\%%h.map" (Set direc=%%~a\%%b\%%c\%%d\%%e\%%f\%%g) Else If Exist "%%~a\%%b\%%c\%%d\%%e\%%f\%%g\%%h\%%i.map" (Set direc=%%~a\%%b\%%c\%%d\%%e\%%f\%%g\%%h) Else If Exist "%%~a\%%b\%%c\%%d\%%e\%%f\%%g\%%h\%%i\%%j.map" (Set direc=%%~a\%%b\%%c\%%d\%%e\%%f\%%g\%%h\%%i) Else If Exist "%%~a\%%b\%%c\%%d\%%e\%%f\%%g\%%h\%%i\%%j\%%k.map" (Set direc=%%~a\%%b\%%c\%%d\%%e\%%f\%%g\%%h\%%i\%%j) Else If Exist "%%~a\%%b\%%c\%%d\%%e\%%f\%%g\%%h\%%i\%%j\%%k\%%l.map" (Set direc=%%~a\%%b\%%c\%%d\%%e\%%f\%%g\%%h\%%i\%%j\%%k) Else If Exist "%%~a\%%b\%%c\%%d\%%e\%%f\%%g\%%h\%%i\%%j\%%k\%%l\%%m.map" (Set direc=%%~a\%%b\%%c\%%d\%%e\%%f\%%g\%%h\%%i\%%j\%%k\%%l) Else If Exist "%%~a\%%b\%%c\%%d\%%e\%%f\%%g\%%h\%%i\%%j\%%k\%%l\%%n.map" (Set direc=%%~a\%%b\%%c\%%d\%%e\%%f\%%g\%%h\%%i\%%j\%%k\%%l\%%m) Else If Exist "%%~a\%%b\%%c\%%d\%%e\%%f\%%g\%%h\%%i\%%j\%%k\%%l\%%n\%%o.map" (Set direc=%%~a\%%b\%%c\%%d\%%e\%%f\%%g\%%h\%%i\%%j\%%k\%%l\%%m\%%n) Else If Exist "%%~a\%%b\%%c\%%d\%%e\%%f\%%g\%%h\%%i\%%j\%%k\%%l\%%n\%%o\%%p.map" (Set direc=%%~a\%%b\%%c\%%d\%%e\%%f\%%g\%%h\%%i\%%j\%%k\%%l\%%m\%%n\%%o) Else If Exist "%%~a\%%b\%%c\%%d\%%e\%%f\%%g\%%h\%%i\%%j\%%k\%%l\%%n\%%o\%%p\%%q.map" (Set direc=%%~a\%%b\%%c\%%d\%%e\%%f\%%g\%%h\%%i\%%j\%%k\%%l\%%m\%%n\%%o\%%p)
For /f "tokens=1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23* delims=.\ " %%a In (mapa.txt) Do If Exist "%%~a\%%b.map" (Set map=%%~b) Else If Exist "%%~a\%%b\%%c.map" (Set map=%%~c) Else If Exist "%%~a\%%b\%%c\%%d.map" (Set map=%%~d) Else If Exist "%%~a\%%b\%%c\%%d\%%e.map" (Set map=%%~e) Else If Exist "%%~a\%%b\%%c\%%d\%%e\%%f.map" (Set map=%%~f) Else If Exist "%%~a\%%b\%%c\%%d\%%e\%%f\%%g.map" (Set map=%%~g) Else If Exist "%%~a\%%b\%%c\%%d\%%e\%%f\%%g\%%h.map" (Set map=%%~h) Else If Exist "%%~a\%%b\%%c\%%d\%%e\%%f\%%g\%%h\%%i.map" (Set map=%%~i) Else If Exist "%%~a\%%b\%%c\%%d\%%e\%%f\%%g\%%h\%%i\%%j.map" (Set map=%%~j) Else If Exist "%%~a\%%b\%%c\%%d\%%e\%%f\%%g\%%h\%%i\%%j\%%k.map" (Set map=%%~k) Else If Exist "%%~a\%%b\%%c\%%d\%%e\%%f\%%g\%%h\%%i\%%j\%%k\%%l.map" (Set map=%%~l) Else If Exist "%%~a\%%b\%%c\%%d\%%e\%%f\%%g\%%h\%%i\%%j\%%k\%%l\%%m.map" (Set map=%%~m) Else If Exist "%%~a\%%b\%%c\%%d\%%e\%%f\%%g\%%h\%%i\%%j\%%k\%%l\%%m\%%n.map" (Set map=%%~n) Else If Exist "%%~a\%%b\%%c\%%d\%%e\%%f\%%g\%%h\%%i\%%j\%%k\%%l\%%m\%%n\%%o.map" (Set map=%%~o) Else If Exist "%%~a\%%b\%%c\%%d\%%e\%%f\%%g\%%h\%%i\%%j\%%k\%%l\%%m\%%n\%%o.map" (Set map=%%~o) Else If Exist "%%~a\%%b\%%c\%%d\%%e\%%f\%%g\%%h\%%i\%%j\%%k\%%l\%%m\%%n\%%o\%%p.map" (Set map=%%~p)
Set blorldir=%direc%
Set direc=%blorldir%\
Set mapb=%map%
Set map=%mapb%.map
Echo Folder Is: %direc%
Echo map Is: %map%.map
Del mapa.txt /f /q
Goto tst

:muin
If Exist compiler.cfg Goto nextp
Set direc=.\
Goto questdir

:nextp
attrib -H compiler.cfg > NUL
For /F "tokens=1* delims=*" %%a In (compiler.cfg) Do Set dire=%%a
Set direc=%dire%
Goto questdir

:subp
If %direc%==.\ Goto setitroot
Set directory=%direc%
Goto questdir

:setitroot
Set direc=%Cd%\
Goto questdir

:questdir
If "%direc%"==".\" Goto setitroot
cls
Goto nochange

:changedir
cls
Echo.
Echo  The folder that contains your maps is set to: 
Echo  %direc%
Echo.
Echo  Type the new directory, check that the directory is valid
Echo  For example: C:\NFMaps
Echo  If you want to put the same directory as the tools, type a dot 
Echo.
Set /P directory=:
Set direc=%directory%\
If "%directory%"=="." Goto delcfg
Del compiler.cfg /f /q > NUL
Echo *%direc%*> compiler.cfg
attrib +H compiler.cfg > NUL
Goto maint

:delcfg
Del compiler.cfg /f /q
Set direc=%Cd%\
Goto maint

:novaldir
cls
Echo.
Echo  The directory saved Is Not valid, taking c:\NFEditors\maps as default
Echo.
pause
Set direc=c:\NFEditors\maps\
Echo *%direc%*> compiler.cfg
cls
Echo.
Goto main

:nochange
:maint
Echo %direc% > a.tmp
For /f "delims=\ " %%a In (a.tmp) Do Set drivedst=%%a
Del a.tmp /f /q
wa
Echo %Cd% > b.tmp
For /f "delims=\ " %%a In (b.tmp) Do Set drivetls=%%a
Del b.tmp /f /q
Set toolsdir=%Cd%

cls
Echo.
Goto main

:main
If Not Exist %direc% Goto novaldir
color
Echo  Directory Set To: %direc%
Echo  If you want to change the directory, type CHANGEDIR
Echo  ----------------------------------------------------
Echo  Write the name of the map, without the .map extension, e.g. dm_jail.map
Echo  If you want to see all the .maps available On this directory, type DIR
Echo  type QUIT to exit                                            Final version 3.0
Echo.
Set /P mapb=
Set map=%mapb%.map
Goto tst
:tst
If %mapb%==.map Goto maperr
If %mapb%==dir Goto dirm
If %mapb%==DIR Goto dirm
If %mapb%==quit Goto quit
If %mapb%==CHANGEDIR Goto changedir
If %mapb%==changedir Goto changedir
If %mapb%==QUIT Goto quit
If Exist "%direc%%map%" Goto compile
If Exist ".\%map%" Goto chaange
Goto maperr

:chaange
Set direc=%Cd%\
Goto compile

:maperr
cls
Echo.
Echo The map %direc%%map% does not exist, write it again
Echo.
Goto main

:dirm
Echo %direc% > a.tmp
For /f "delims=\ " %%a In (a.tmp) Do Set drivedst=%%a
Del a.tmp /f /q

Echo %Cd% > b.tmp
For /f "delims=\ " %%a In (b.tmp) Do Set drivetls=%%a
Del b.tmp /f /q
cls
Set toolsdir=%Cd%
%drivedst%
Cd %direc%
Echo List of maps:
dir *.map /b /p
%drivetls%
Cd %toolsdir%
Echo.
Goto main

:detectmap
cls
Echo Off
Echo.
Echo  Detecting which editor made the map...
For /f "tokens=1,2,3,4,5*" %%a In (%mapwpath%) Do If %%a=="mapversion" Set mapv=%%~b
Echo.
Echo  Mapversion: %mapv%
If 1%mapv%==1510 (Echo  Editor: Nightfire Level Editor aka Gearcraft) Else If 1%mapv%==1220 (Echo  Editor: Valve Hammer Editor For Nightfire) Else (Echo  Editor: Unknown/Not valid
																			Set mapv=notv)
:liip
If 1%mapv%==1notv (
	Echo. 
	Echo  The editor version was Not detected, choose one of this options:
	Echo.
	Echo  1 - Made with Gearcraft / Nightfire Level Editor - v510
	Echo  2 - Made with Worldcraft / Valve Hammer - v220
	Echo.
	Set /p ke=:
	If 1%ke%==11 (
		Set mapv=510
		goto preparemap
		)
	If 1%ke%==12 (
		Set mapv=220
		goto preparemap
		)
	If 1%mapv%==1notv Goto liip
	)
:preparemap
Echo Preparing map directories....
echo.
if exist "%direc%%mapb%t.map" (Del "%direc%%mapb%t.map" /F /Q)
if exist "%direc%%mapb%_.map" (Del "%direc%%mapb%_.map" /F /Q)
if exist "%direc%%mapb%\%mapb%_.map" (Del "%direc%%mapb%\%mapb%_.map" /F /Q)
If 1%mapv%==1510 (
	Copy "%direc%%mapb%.map" "%direc%%mapb%_.map"
	if not exist "%direc%%mapb%" (md "%direc%%mapb%")
	Copy "%direc%%mapb%_.map" "%direc%%mapb%"
	) Else (
	Echo  Converting to Gearcraft mapversion....
	corrector.exe -in "%direc%%mapb%.map" -out "%direc%%mapb%_.map" -custom %mapb%
	md "%direc%%mapb%"
	Copy "%direc%%mapb%_.map" "%direc%%mapb%"
	if not exist "%direc%%mapb%\%mapb%_.map" (
		echo corrector.exe -in "%direc%%mapb%.map" -out "%direc%%mapb%_.map" -custom %mapb%
		pause
		)
	)

Set poo=b 
Goto compile

:compile
Set mapwpath=%direc%%map%
Set mapwpathwoutext=%direc%%map%
If Not 1%poo%==1b Goto detectmap
color
cls
Echo.
if 1%mapv%==1510 (Echo    Select MAP QUALITY - Map: %mapwpath% - Gearcraft) else (Echo    Select MAP QUALITY - Map: %mapwpath% - Hammer)
Echo  ****************************************************************
Echo  F - Fast compile (Fast VIS and no Lighting), only for testings
Echo  T - Lighting Test (Fast VIS and fast Lighting)
Echo  N - Normal compile (Fast VIS and normal Lighting)
Echo  Q - High quality compile (For final releases, Full VIS and quality Lighting)
Echo  O - Other (Special compiling options menu)
Echo  ****************************************************************
Echo  C - Custom
Echo  ****************************************************************
Echo  1 - Start Nightfire after compiling: %statnf%
Echo  2 - Nightfire start configuration
if 1%mapv%==1220 (
	echo  ****************************************************************
	echo  3 - Convert this map PERMANENTLY to a GC mapversion
	Echo.
	)
Set /P qual=:

Set qty=1%qual%
If %qty%==1 Goto compile
If %qty%==1F Goto fastcompil
If %qty%==1f Goto fastcompil
If %qty%==1O Goto specmenu
If %qty%==1o Goto specmenu
If %qty%==1t Goto lighttest
If %qty%==1T Goto lighttest
If %qty%==1N Goto normcompil
If %qty%==1n Goto normcompil
If %qty%==1Q Goto qualcompil
If %qty%==1q Goto qualcompil
If %qty%==11 Goto changstart
If %qty%==12 Goto nfinitconf
if %qty%==13 goto convtogc
If %qty%==1c Goto customcompil
If %qty%==1C Goto customcompil
Goto compile

:convtogc
if not 1%mapv%==1220 goto compile
cls
echo.
echo  WARNING: This map wont be able to be edited through Hammer again (textures won't appear again)
echo.
echo If you want to continue the process, press any key, if you want to cancel, close this window.
echo.
pause >NUL
set statnf=No
Copy "%direc%%mapb%_.map" "%direc%%mapb%.map" /Y
echo Map converted sucessfully. Press any key to exit
echo.
pause >NUL
goto quit


:nfinitconf
Goto initmenu

:initmenu
color f0
cls
Echo.
Echo  ******************************************************************************
Echo  * NF Start options - Enter the number to change the value                    *
Echo  ******************************************************************************
Echo    Booleans:                                                       Value
Echo    1 - Console:                                                    %console% 
Echo    2 - Fullscreen / Windowed:                                      %screen%
Echo    3 - Create a Single player server / Multiplayer                 %noipx%
Echo    4 - Open map on a dedicated server                              %dedserver%
Echo  ******************************************************************************
Echo    Integers / Numbers / Strings:                                   Value
Echo    5 - Hunk (related to memory):                                   %hunk%
Echo    6 - Heap (related to memory):                                   %heap%
Echo    7 - Extra command-line start options for NF: %comm%
Echo  ******************************************************************************
echo    Advanced:
echo    A - Nightfire directory change: %nfdir%
Echo  ******************************************************************************
Echo  * M - Go To Compile Menu                                                     *
Echo  ******************************************************************************
Echo.
Set /p valuebsp=:
If 1%valuebsp%==1M Goto savsetts
If 1%valuebsp%==1m Goto savsetts

If 1%valuebsp%==11 Goto console
If 1%valuebsp%==12 Goto screen
If 1%valuebsp%==13 Goto noipx
If 1%valuebsp%==14 Goto dedserver

If 1%valuebsp%==15 Goto hunk
If 1%valuebsp%==16 Goto heap
If 1%valuebsp%==17 Goto comm

if 1%valuebsp%==1a goto nfdirchg
if 1%valuebsp%==1A goto nfdirchg
Goto initmenu


:savsetts
If 1%console%==1Yes (Set console=1) Else Set console=0
If 1%screen%==1fullscreen (Set screen=0) Else Set screen=1
If 1%noipx%==1SinglePlayer (Set noipx=0) Else Set noipx=1
If 1%dedserver%==1Yes (Set dedserver=1) Else Set dedserver=0
Del nfinit.cfg /f /q
Echo [Startnf]=>>nfinit.cfg  
Echo console=%console%=>>nfinit.cfg
Echo screen=%screen%=>>nfinit.cfg
Echo noipx=%noipx%=>>nfinit.cfg
Echo dedserver=%dedserver%=>>nfinit.cfg
Echo hunk=%hunk%=>>nfinit.cfg
Echo heap=%heap%=>>nfinit.cfg
Echo comm=%comm%=>>nfinit.cfg
echo nfdir=%nfdir%=>>nfinit.cfg
Goto compile

:console
Set booled=console
Set booledval=%console%
Set togo=initmenu
Goto boolean

:dedserver
Set booled=dedserver
Set booledval=%dedserver%
Set togo=initmenu
Goto boolean

:hunk
Set opti=hunk
Set optival=%hunk%
Set defopti=48
Set info=Put higher values with HEAP If you have an Error starting Nightfire saying: Error: Hunk_alloc failed On xxxxxx bytes
Set togo=initmenu
Goto introduce

:comm
Set opti=comm
Set optival=%comm%
Set defopti=No Defaults
Set info=Put everything you want To be runned On game, put the - commands first And the + commands after.
Set togo=initmenu
Goto introduce

:heap
Set opti=heap
Set optival=%heap%
Set defopti=128
Set info=Put higher values with HUNK If you have an Error starting Nightfire saying: Error: Hunk_alloc failed On xxxxxx bytes
Set togo=initmenu
Goto introduce

:nfdirchg
cls
echo.
if exist "%nfdir%\bond.exe" (echo  Status: Nightfire detected at that directory!) else (echo Status: Error: Nightfire was not detected at the specified directory below) 
echo.
echo  Selected directory: %nfdir%
echo.
echo Choose one of this options:
echo 1 - Autodetect Nightfire's directory through registry
echo 2 - Type the directory manually
echo B - Go to previous menu
echo.
set /p optnfdir=:
if 1%optnfdir%==1b goto initmenu
if 1%optnfdir%==1B goto initmenu
if 1%optnfdir%==11 goto detectregnf
if 1%optnfdir%==12 goto nfdirmanu
goto nfdirchg

:detectregnf
REG QUERY HKLM\SOFTWARE\GEARBOX\NIGHTFIRE /v directory>> nfdir.txt
For /f "tokens=1,2*" %%a In (nfdir.txt) Do If %%b==REG_SZ (Set nfdir=%%c)
Del nfdir.txt /f /q
goto nfdirchg

:nfdirmanu
cls
echo.
echo Type the directory without the last slash: e.g.: c:\Program Files\EA Games\Nightfire
echo.
set /p nfdir=Directory: 
goto nfdirchg

:noipx
If %noipx%==SinglePlayer (Set noipx=Multiplayer) Else If %noipx%==Multiplayer (Set noipx=SinglePlayer)
Goto initmenu

:screen
If %screen%==windowed (Set screen=fullscreen) Else If %screen%==fullscreen (Set screen=windowed)
Goto initmenu

:changstart
If %statnf%==No Goto yesnf
If %statnf%==Yes Goto nonf
Goto compile 

:yesnf
if not exist "%nfdir%\bond.exe" goto nonfwrn
Set statnf=
Set statnf=Yes
Goto compile 

:nonfwrn
cls
echo.
echo  Nightfire's directory is not correct, go to Nightfire start configuration to set it up.
echo.
pause
goto compile 

:nonf
Set statnf=
Set statnf=No
Goto compile 

:specmenu
cls
Echo.
Echo    Select MAP compiling special options - Map: %mapwpath%
Echo  ****************************************************************
Echo  E - Update only the entities from the .map to the .bsp
Echo  Q - Make a map with fast vis And High Quality Lighting
Echo  R - Make a map with no lighting (normal vis)
Echo  L - Make a map with no lighting (fast vis)
Echo  B - Back to normal menu
Echo.
Set /P qualp=:

Set qtyn=1%qualp%

If %qtyn%==1 Goto specmenu
If %qtyn%==1E Goto entsonly
If %qtyn%==1e Goto entsonly
If %qtyn%==1Q Goto radvisfast
If %qtyn%==1q Goto radvisfast
If %qtyn%==1R Goto norad
If %qtyn%==1r Goto norad
If %qtyn%==1L Goto noradvisfast
If %qtyn%==1l Goto noradvisfast
If %qtyn%==1b Goto compile
If %qtyn%==1B Goto compile
Goto specmenu

:noradvisfast
bbsp -verbose -estimate "%direc%%mapb%\%mapb%_.map" 
bvis -estimate -fast "%direc%%mapb%\%mapb%_.map"
Del "%direc%%mapb%\%mapb%_.map" /f /q
Copy "%direc%%mapb%\%mapb%_.bsp" "%direc%%mapb%\%mapb%.bsp" /Y
Del "%direc%%mapb%\%mapb%_.bsp" /f /q
If Exist "%direc%%mapb%\%mapb%_.err" Goto errnorm
If Exist "%direc%%mapb%\%mapb%.bsp" Goto done
Goto errnorm2


:radvisfast
bbsp -verbose -estimate "%direc%%mapb%\%mapb%_.map" 
bvis -estimate -fast "%direc%%mapb%\%mapb%_.map"
brad -verbose -estimate -extra -smooth 100 -bounce 2 "%direc%%mapb%\%mapb%_.map"
Del "%direc%%mapb%\%mapb%_.map" /f /q
Copy "%direc%%mapb%\%mapb%_.bsp" "%direc%%mapb%\%mapb%.bsp" /Y
Del "%direc%%mapb%\%mapb%_.bsp" /f /q
If Exist "%direc%%mapb%\%mapb%_.err" Goto errnorm
If Exist "%direc%%mapb%\%mapb%.bsp" Goto done
Goto errnorm2

:norad
bbsp -verbose -estimate "%direc%%mapb%\%mapb%_.map" 
bvis -estimate "%direc%%mapb%\%mapb%_.map"
Del "%direc%%mapb%\%mapb%_.map" /f /q
Copy "%direc%%mapb%\%mapb%_.bsp" "%direc%%mapb%\%mapb%.bsp" /Y
Del "%direc%%mapb%\%mapb%_.bsp" /f /q
If Exist "%direc%%mapb%\%mapb%_.err" Goto errnorm
If Exist "%direc%%mapb%\%mapb%.bsp" Goto done
Goto errnorm2

:entsonly
Del "%direc%%mapb%\%mapb%_.bsp" /f /q
Copy "%direc%%mapb%\%mapb%.bsp" "%direc%%mapb%\%mapb%_.bsp" 
cls
bbsp -verbose -estimate -onlyents "%direc%%mapb%\%mapb%_.map" 
Del "%direc%%mapb%\%mapb%_.map" /f /q
Copy "%direc%%mapb%\%mapb%_.bsp" "%direc%%mapb%\%mapb%.bsp" /Y
Del "%direc%%mapb%\%mapb%_.bsp" /f /q
If Exist "%direc%%mapb%\%mapb%_.err" Goto errnorm
If Exist "%direc%%mapb%\%mapb%.bsp" Goto done
Goto errnorm2

:recustomcfg
cls
Echo.
Echo  Your Custom.cfg Is corrupted And you will need To put a new config.cfg file
Echo  You wont be able To use the Custom Option till you fix this incident.
Echo.
Echo Press any key To go To the main menu
pause >nul
cls
Goto main
:customcompil

If Exist custom.cfg Goto readcustomcfg
cls
Echo.
Echo  You deleted the custom.cfg that Is necessary To run with the Custom compiler
Echo  Press any key To go To the main menu
pause >nul
cls
Goto main

:readcustomcfg
Set save=Yes
Set temp=No
For /f "tokens=1,2* delims==" %%a In (custom.cfg) Do Set %%a=%%b
Rem **************** If %%a==threads (Set threads=%%b) Else If %%a==dev (Set dev=%%b) Else If %%a==onlyents (Set onlyents=%%b) Else If %%a==leakonly (Set leakonly=%%b) Else If %%a==showbevels (Set showbevels=%%b) Else If %%a==notjunc (Set notjunc=%%b) Else If %%a==noclip (Set noclip=%%b) Else If %%a==nofill (Set nofill=%%b) Else If %%a==nolighting (Set nolighting=%%b) Else If %%a==chart (Set chart=%%b) Else If %%a==nowater (Set nowater=%%b) Else If %%a==noinfo (Set noinfo=%%b) Else If %%a==blscale (Set blscale=%%b) Else If %%a==ilscale (Set ilscale=%%b) Else If %%a==maxnodesize (Set maxnodesize=%%b) Else If %%a==full (Set full=%%b) Else If %%a==fast (Set fast=%%b) Else  If %%a==sparse (Set sparse=%%b) Else If %%a==nomatrix (Set nomatrix=%%b) Else If %%a==extra (Set extra=%%b) Else If %%a==circus (Set circus=%%b) Else 
If 1%leakonly%==1 Goto recustomcfg
If 1%leakonly%==10 (Set leakonly=No) Else Set leakonly=Yes
If 1%showbevels%==10 (Set showbevels=No) Else Set showbevels=Yes
If 1%notjunc%==10 (Set notjunc=No) Else Set notjunc=Yes
If 1%noclip%==10 (Set noclip=No) Else Set noclip=Yes
If 1%nofill%==10 (Set nofill=No) Else Set nofill=Yes
If 1%nolighting%==10 (Set nolighting=No) Else Set nolighting=Yes
If 1%chart%==10 (Set chart=No) Else Set chart=Yes
If 1%nowater%==10 (Set nowater=No) Else Set nowater=Yes
If 1%noinfo%==10 (Set noinfo=No) Else Set noinfo=Yes

If 1%sparse%==10 (Set sparse=No) Else Set sparse=Yes
If 1%nomatrix%==10 (Set nomatrix=No) Else Set nomatrix=Yes
If 1%extra%==10 (Set extra=No) Else Set extra=Yes
If 1%circus%==10 (Set circus=No) Else Set circus=Yes
If 1%nohunt%==10 (Set nohunt=No) Else Set nohunt=Yes
If 1%noslide%==10 (Set noslide=No) Else Set noslide=Yes
If 1%nolerp%==10 (Set nolerp=No) Else Set nolerp=Yes
If 1%noskyfix%==10 (Set noskyfix=No) Else Set noskyfix=Yes
If 1%incremental%==10 (Set incremental=No) Else Set incremental=Yes
If 1%dump%==10 (Set dump=No) Else Set dump=Yes
If 1%notexscale%==10 (Set notexscale=No) Else Set notexscale=Yes
Goto generalmenu

:boolean
If 1%booledval%==1Yes (Set %booled%=No ) Else Set %booled%=Yes 
Goto %togo%

:introduce
color 0f
Set valed=
Echo Info: %info%> h.tmp
Del n.tmp /f /q
cls
Echo.
Echo  Type the number/string of the Option: %opti% And press enter
Echo  The actual value of %opti% Is: %optival%
Echo  The default value Is: %defopti%
Echo.
For /f "tokens=1,2*" %%a In (h.tmp) Do If Not 1%%b==1 (Echo %%a %%b %%c) Else Echo.
Echo.
Set /P valed=:
Del h.tmp /f /q
Echo %valed%> n.tmp
For /f "tokens=1,2*" %%a In (n.tmp) Do If 1%%a==1Echo (Goto %togo%) Else If 1%%a==1 Goto %togo%
Set %opti%=%valed%
Del n.tmp /f /q
Goto %togo%


:generalmenu
color f0
cls
Echo.
Echo  ******************************************************************************
Echo  * General Configuration - Enter the number to change the value               *
Echo  ******************************************************************************
Echo    Booleans:                                                       Value
Echo    1 - Display BSP Stats:                                          %chart% 
Echo    2 - Do not show tool configuration information:                 %noinfo%
Echo    3 - Save the custom configuration for the next time it opens:   %save%
Echo    4 - Save the temporary files of the compilation:                %temp%
Echo  ******************************************************************************
Echo    Integers / Numbers:                                             Value
Echo    4 - Manually specify the number of threads to run:              %threads%
Echo    5 - Compile with developer messages:                            %dev%
Echo  ******************************************************************************
Echo  * N - Continue to next compile menu, BBSP - Solid                            *
Echo  * M - Go to compile menu                                                     *
Echo  ******************************************************************************
Echo.
Set /p valuebsp=:
If 1%valuebsp%==1M Goto compile
If 1%valuebsp%==1m Goto compile

If 1%valuebsp%==1N Goto bspmenu
If 1%valuebsp%==1n Goto bspmenu

If 1%valuebsp%==11 Goto chart
If 1%valuebsp%==12 Goto noinfo
If 1%valuebsp%==13 Goto save

If 1%valuebsp%==15 Goto threads
If 1%valuebsp%==16 Goto dev
Goto generalmenu

:chart
Set booled=chart
Set booledval=%chart%
Set togo=generalmenu
Goto boolean

:save
Set booled=save
Set booledval=%save%
Set togo=generalmenu
Goto boolean

:noinfo
Set booled=noinfo
Set booledval=%noinfo%
Set togo=generalmenu
Goto boolean

:threads
Set opti=threads
Set optival=%threads%
Set defopti=1
Set info=
Set togo=generalmenu
Goto introduce

:dev
Set opti=dev
Set optival=%dev%
Set defopti=0
Set info=
Set togo=generalmenu
Goto introduce

:bspmenu
color f0
cls
Echo.
Echo  ******************************************************************************
Echo  * BBSP (Solid) Configuration - Enter the number to change the value          *
Echo  ******************************************************************************
Echo    Booleans:                                                       Value
Echo    1 - Run BSP only For LEAKs check:                               %leakonly%
Echo    2 - Export a .map with the expanded brushes generated bevels:   %showbevels%
Echo    3 - Don't break edges on T-Junctions:                           %notjunc%
Echo    4 - Dont Process the clipping hull:                             %noclip%
Echo    5 - Don't fill outside (will mask leaks):                       %nofill%
Echo    6 - Do Not build lighting BSP:                                  %nolighting%
Echo    7 - Do Not create water models (smd):                           %nowater%
Echo  ******************************************************************************
Echo    Integers / Numbers:                                             Value
Echo    8 - Base lightmap scale:                                        %blscale%
Echo    9 - Incremental lightmap scale:                                 %ilscale%
Echo    10 - Maximum portal node size (from 64 to 4096):                %maxnodesize%
Echo  ******************************************************************************
Echo  * C - Compile with BBSP Only (Not recommended)                               *
Echo  * N - Continue to next compile menu, BVIS - Visibility Calculation           *
Echo  * M - Go to compile menu                                                     *
Echo  ******************************************************************************
Echo.
Set /p valuebsp=:
If 1%valuebsp%==1M Goto compile
If 1%valuebsp%==1m Goto compile

If 1%valuebsp%==1c Goto compbsp
If 1%valuebsp%==1C Goto compbsp

If 1%valuebsp%==1N Goto vismenu
If 1%valuebsp%==1n Goto vismenu

If 1%valuebsp%==11 Goto leakonly
If 1%valuebsp%==12 Goto showbevels
If 1%valuebsp%==13 Goto notjunc
If 1%valuebsp%==14 Goto noclip
If 1%valuebsp%==15 Goto nofill
If 1%valuebsp%==16 Goto nolighting
If 1%valuebsp%==17 Goto nowater

If 1%valuebsp%==18 Goto blscale
If 1%valuebsp%==19 Goto ilscale
If 1%valuebsp%==110 Goto maxnodesize
Goto bspmenu

:leakonly
Set booled=leakonly
Set booledval=%leakonly%
Set togo=bspmenu
Goto boolean

:showbevels
Set booled=showbevels
Set booledval=%showbevels%
Set togo=bspmenu
Goto boolean

:notjunc
Set booled=notjunc
Set booledval=%notjunc%
Set togo=bspmenu
Goto boolean

:noclip
Set booled=noclip
Set booledval=%noclip%
Set togo=bspmenu
Goto boolean

:nofill
Set booled=nofill
Set booledval=%nofill%
Set togo=bspmenu
Goto boolean

:nolighting
Set booled=nolighting
Set booledval=%nolighting%
Set togo=bspmenu
Goto boolean

:nowater
Set booled=nowater
Set booledval=%nowater%
Set togo=bspmenu
Goto boolean

:blscale
Set opti=blscale
Set optival=%blscale%
Set defopti=16
Set info=
Set togo=bspmenu
Goto introduce

:ilscale
Set opti=ilscale
Set optival=%ilscale%
Set defopti=16
Set info=
Set togo=bspmenu
Goto introduce

:maxnodesize
Set opti=maxnodesize
Set optival=%maxnodesize%
Set defopti=1024
Set info=
Set togo=bspmenu
Goto introduce

:vismenu
color f0
cls
Echo.
Echo  ******************************************************************************
Echo  * BVIS (Visibility) Configuration - Enter the number to change the value     *
Echo  ******************************************************************************
Echo   Choice:                                                          Value
Echo    1 - Speed / Quality of VIS Calculation:                         %visspeed%
Echo  ******************************************************************************
Echo  * C - Compile with BVIS Only                                                 *
Echo  * N - Continue to next compile menu, BRAD - Radiosity Lighting               *
Echo  * M - Go to compile menu                                                     *
Echo  ******************************************************************************
Echo.
Set /p valuebsp=:
If 1%valuebsp%==1M Goto compile
If 1%valuebsp%==1m Goto compile

If 1%valuebsp%==1c Goto compvis
If 1%valuebsp%==1C Goto compvis

If 1%valuebsp%==1N Goto radmenu
If 1%valuebsp%==1n Goto radmenu

If 1%valuebsp%==11 Goto visspeed
Goto vismenu

:visspeed
If %visspeed%==fast (Set visspeed=normal) Else If %visspeed%==normal (Set visspeed=full) Else If %visspeed%==full Set visspeed=fast
Goto vismenu

:radmenu
color f0
cls
Echo.
Echo  ******************************************************************************
Echo  * BRAD (Lighting) Configuration - Enter the number to change the value       *
Echo  ******************************************************************************
Echo    Booleans:                                                       Value
Echo    1 - Enable low memory vismatrix algorithm:                      %sparse%
Echo    2 - Disable usage of vismatrix entirely:                        %nomatrix%
Echo    3 - Improve lighting quality by doing 9 point oversampling:     %extra%
Echo    4 - Enable 'circus' mode for locating unlit lightmaps:          %circus%
Echo    5 - Skip lightmap texel hunt For world (For fast compiles):     %nohunt%
Echo    6 - Skip sliding lightmap texels into its polygon bounds:       %noslide%
Echo    7 - Disable radiosity interpolation, nearest point instead:     %nolerp%
Echo    8 - Disable light_environment being global:                     %noskyfix%
Echo    9 - Use Or create an incremental transfer list file:            %incremental%
Echo    10 - Dumps light patches To a file For brad debugging info:     %dump%
Echo    11 - Do Not scale radiosity patches with texture scale:         %notexscale%
Echo  ******************************************************************************
Echo    Integers / Numbers / Strings:                                   Value
Echo    12 - Set number of radiosity bounces:                           %bounce%
Echo    13 - Set ambient world light (0.0 To 1.0, r g b):         %ambient%
Echo    14 - Set maximum light intensity value:                         %maxlight%
Echo    15 - Set smoothing threshold For blending (In degrees):         %smooth%
Echo    16 - Set radiosity patch size For normal textures (chop):       %chop%
Echo    17 - Set radiosity patch size For texture faces (texchop):      %texchop%
Echo    18 - Set lighting threshold before blackness:                   %coring%
Echo    19 - Set direct lighting threshold:                             %dlight%
Echo    20 - Set global fade (larger values = shorter lights):          %fade%
Echo    21 - Set global falloff mode (1 = inv linear, 2 = inv square):  %falloff%
Echo    22 - Set direct light scaling value:                            %dscale%
Echo    23 - Set bounced light scaling value:                           %bscale%
Echo    24 - Set global light scaling value:                            %scale%
Echo    25 - Set global gamma value:                                    %gamma%
Echo    26 - Set ambient sunlight contribution In the shade outside:    %sky%
Echo  ******************************************************************************
Echo   C - Compile now
Echo   M - Go To Compile Menu
Echo.
Set /p valuebsp=:
If 1%valuebsp%==1M Goto compile
If 1%valuebsp%==1m Goto compile

If 1%valuebsp%==1c Goto comprad
If 1%valuebsp%==1C Goto comprad

If 1%valuebsp%==11 Goto sparse
If 1%valuebsp%==12 Goto nomatrix
If 1%valuebsp%==13 Goto extra
If 1%valuebsp%==14 Goto circus
If 1%valuebsp%==15 Goto nohunt
If 1%valuebsp%==16 Goto noslide
If 1%valuebsp%==17 Goto nolerp
If 1%valuebsp%==18 Goto noskyfix
If 1%valuebsp%==19 Goto incremental
If 1%valuebsp%==110 Goto dump
If 1%valuebsp%==111 Goto notexscale

If 1%valuebsp%==112 Goto bounce
If 1%valuebsp%==113 Goto ambient
If 1%valuebsp%==114 Goto maxlight
If 1%valuebsp%==115 Goto smooth
If 1%valuebsp%==116 Goto chop
If 1%valuebsp%==117 Goto texchop
If 1%valuebsp%==118 Goto coring
If 1%valuebsp%==119 Goto dlight
If 1%valuebsp%==120 Goto fade
If 1%valuebsp%==121 Goto falloff
If 1%valuebsp%==122 Goto dscale
If 1%valuebsp%==123 Goto bscale
If 1%valuebsp%==124 Goto scale
If 1%valuebsp%==125 Goto gamma
If 1%valuebsp%==126 Goto sky
Goto radmenu

:sparse
Set booled=sparse
Set booledval=%sparse%
Set togo=radmenu
Goto boolean

:nomatrix
Set booled=nomatrix
Set booledval=%nomatrix%
Set togo=radmenu
Goto boolean

:extra
Set booled=extra
Set booledval=%extra%
Set togo=radmenu
Goto boolean

:circus
Set booled=circus
Set booledval=%circus%
Set togo=radmenu
Goto boolean

:nohunt
Set booled=nohunt
Set booledval=%nohunt%
Set togo=radmenu
Goto boolean

:noslide
Set booled=noslide
Set booledval=%noslide%
Set togo=radmenu
Goto boolean

:nolerp
Set booled=nolerp
Set booledval=%nolerp%
Set togo=radmenu
Goto boolean

:noskyfix
Set booled=noskyfix
Set booledval=%noskyfix%
Set togo=radmenu
Goto boolean

:incremental
Set booled=incremental
Set booledval=%incremental%
Set togo=radmenu
Goto boolean

:dump
Set booled=dump
Set booledval=%dump%
Set togo=radmenu
Goto boolean

:notexscale
Set booled=notexscale
Set booledval=%notexscale%
Set togo=radmenu
Goto boolean

:bounce
Set opti=bounce
Set optival=%bounce%
Set defopti=1
Set info=Dont put too much bounces Or your map wont have much radiation lighting relativity, it will seem False cheap lighting.
Set togo=radmenu
Goto introduce

:ambient
Set opti=ambient
Set optival=%ambient%
Set defopti=0.000 0.000 0.000
Set info=Type the number as a RGB Value from 0.0 To 1.0 Each value, e.g.: 0.125 red 0.010 green 0.100 blue = 0.125 0.010 0.100
Set togo=radmenu
Goto introduce

:maxlight
Set opti=maxlight
Set optival=%maxlight%
Set defopti=256
Set info=
Set togo=radmenu
Goto introduce

:smooth
Set opti=smooth
Set optival=%smooth%
Set defopti=50
Set info=
Set togo=radmenu
Goto introduce

:chop
Set opti=chop
Set optival=%chop%
Set defopti=64
Set info=Change this value To a higher number If you had an Error like this: Error: Exceeded MAX_PATCHES. The maximum patches are 65535.
Set togo=radmenu
Goto introduce

:texchop
Set opti=texchop
Set optival=%texchop%
Set defopti=32
Set info=Change this value To a higher number If you had an Error like this: Error: Exceeded MAX_PATCHES. The maximum patches are 65535.
Set togo=radmenu
Goto introduce

:coring
Set opti=coring
Set optival=%coring%
Set defopti=1.000
Set info=
Set togo=radmenu
Goto introduce

:dlight
Set opti=dlight
Set optival=%dlight%
Set defopti=25.000
Set info=
Set togo=radmenu
Goto introduce

:fade
Set opti=fade
Set optival=%fade%
Set defopti=1.000
Set info=With an higher fade value, it will have less radiosity Each light And it will be darker.
Set togo=radmenu
Goto introduce

:falloff
Set opti=falloff
Set optival=%falloff%
Set defopti=2
Set info=You can only choose 1 Or 2, 1 For inverse linear And 2 For inverse square. If you choose other number, RAD will show Error.
Set togo=radmenu
Goto introduce

:dscale
Set opti=dscale
Set optival=%dscale%
Set defopti=2.000
Set info=
Set togo=radmenu
Goto introduce

:bscale
Set opti=bscale
Set optival=%bscale%
Set defopti=1.000
Set info=
Set togo=radmenu
Goto introduce

:scale
Set opti=scale
Set optival=%scale%
Set defopti=1.000
Set info=
Set togo=radmenu
Goto introduce

:gamma
Set opti=gamma
Set optival=%gamma%
Set defopti=2.200
Set info=
Set togo=radmenu
Goto introduce

:sky
Set opti=sky
Set optival=%sky%
Set defopti=1.000
Set info=
Set togo=radmenu
Goto introduce

Rem **********************************************

:compbsp
Set vis=no
Set rad=no
Goto preparenumbers

:compvis
Set vis=yes
Set rad=no
Goto preparenumbers

:comprad
Set vis=yes
Set rad=yes
Goto preparenumbers

:preparenumbers
If 1%leakonly%==1Yes (Set leakonly=1) Else Set leakonly=0
If 1%showbevels%==1Yes (Set showbevels=1) Else Set showbevels=0
If 1%notjunc%==1Yes (Set notjunc=1) Else Set notjunc=0
If 1%noclip%==1Yes (Set noclip=1) Else Set noclip=0
If 1%nofill%==1Yes (Set nofill=1) Else Set nofill=0
If 1%nolighting%==1Yes (Set nolighting=1) Else Set nolighting=0
If 1%chart%==1Yes (Set chart=1) Else Set chart=0
If 1%nowater%==1Yes (Set nowater=1) Else Set nowater=0
If 1%noinfo%==1Yes (Set noinfo=1) Else Set noinfo=0
If 1%sparse%==1Yes (Set sparse=1) Else Set sparse=0
If 1%nomatrix%==1Yes (Set nomatrix=1) Else Set nomatrix=0
If 1%extra%==1Yes (Set extra=1) Else Set extra=0
If 1%circus%==1Yes (Set circus=1) Else Set circus=0
If 1%nohunt%==1Yes (Set nohunt=1) Else Set nohunt=0
If 1%noslide%==1Yes (Set noslide=1) Else Set noslide=0
If 1%nolerp%==1Yes (Set nolerp=1) Else Set nolerp=0
If 1%noskyfix%==1Yes (Set noskyfix=1) Else Set noskyfix=0
If 1%incremental%==1Yes (Set incremental=1) Else Set incremental=0
If 1%dump%==1Yes (Set dump=1) Else Set dump=0
If 1%notexscale%==1Yes (Set notexscale=1) Else Set notexscale=0

If %save%==No Goto compcustnow
Goto savecustom

:savecustom
Del custom.cfg /f /q

Echo [Generals]=>>custom.cfg  
Echo :Booleans=>>custom.cfg
Echo threads=%threads%=>>custom.cfg
Echo dev=%dev%=>>custom.cfg
Echo.>>custom.cfg
Echo :Integers=>>custom.cfg
Echo chart=%chart%=>>custom.cfg 
Echo noinfo=%noinfo%=>>custom.cfg
Echo.>>custom.cfg
Echo [BBSP]=>>custom.cfg
Echo :Booleans=>>custom.cfg
Echo onlyents=%onlyents%=>>custom.cfg
Echo leakonly=%leakonly%=>>custom.cfg
Echo showbevels=%showbevels%=>>custom.cfg
Echo notjunc=%notjunc%=>>custom.cfg
Echo noclip=%noclip%=>>custom.cfg
Echo nofill=%nofill%=>>custom.cfg
Echo nolighting=%nolighting%=>>custom.cfg
Echo nowater=%nowater%=>>custom.cfg
Echo.>>custom.cfg
Echo :Integers=>>custom.cfg
Echo blscale=%blscale%=>>custom.cfg
Echo ilscale=%ilscale%=>>custom.cfg
Echo maxnodesize=%maxnodesize%=>>custom.cfg
Echo.>>custom.cfg
Echo [BVIS]=>>custom.cfg
Echo :Booleans=>>custom.cfg
Echo visspeed=%visspeed%=>>custom.cfg
Echo.>>custom.cfg
Echo [BRAD]=>>custom.cfg
Echo :Booleans=>>custom.cfg
Echo sparse=%sparse%=>>custom.cfg
Echo nomatrix=%nomatrix%=>>custom.cfg
Echo extra=%extra%=>>custom.cfg
Echo circus=%circus%=>>custom.cfg
Echo nohunt=%nohunt%=>>custom.cfg
Echo noslide=%noslide%=>>custom.cfg
Echo nolerp=%nolerp%=>>custom.cfg
Echo noskyfix=%noskyfix%=>>custom.cfg
Echo incremental=%incremental%=>>custom.cfg
Echo dump=%dump%=>>custom.cfg
Echo notexscale=%notexscale%=>>custom.cfg
Echo.>>custom.cfg
Echo :Integers=>>custom.cfg
Echo bounce=%bounce%=>>custom.cfg
Echo maxlight=%maxlight%=>>custom.cfg
Echo smooth=%smooth%=>>custom.cfg
Echo chop=%chop%=>>custom.cfg
Echo texchop=%texchop%=>>custom.cfg
Echo coring=%coring%=>>custom.cfg
Echo dlight=%dlight%=>>custom.cfg
Echo fade=%fade%=>>custom.cfg
Echo falloff=%falloff%=>>custom.cfg
Echo dscale=%dscale%=>>custom.cfg
Echo bscale=%bscale%=>>custom.cfg
Echo scale=%scale%=>>custom.cfg
Echo gamma=%gamma%=>>custom.cfg
Echo ambient=%ambient%=>>custom.cfg
Goto compcustnow

:compcustnow
If 1%leakonly%==1 Goto recustomcfg
If 1%leakonly%==10 (Set leakonly= ) Else Set leakonly=-leakonly
If 1%onlyents%==10 (Set onlyents= ) Else Set onlyents=-onlyents
If 1%showbevels%==10 (Set showbevels= ) Else Set showbevels=-showbevels
If 1%notjunc%==10 (Set notjunc= ) Else Set notjunc=-notjunc
If 1%noclip%==10 (Set noclip= ) Else Set noclip=-noclip
If 1%nofill%==10 (Set nofill= ) Else Set nofill=-nofill
If 1%nolighting%==10 (Set nolighting= ) Else Set nolighting=-nolighting
If 1%chart%==10 (Set chart= ) Else Set chart=-chart
If 1%nowater%==10 (Set nowater= ) Else Set nowater=-nowater
If 1%noinfo%==10 (Set noinfo= ) Else Set noinfo=-noinfo

If 1%sparse%==10 (Set sparse= ) Else Set sparse=-sparse
If 1%nomatrix%==10 (Set nomatrix= ) Else Set nomatrix=-nomatrix
If 1%extra%==10 (Set extra= ) Else Set extra=-extra
If 1%circus%==10 (Set circus= ) Else Set circus=-circus
If 1%nohunt%==10 (Set nohunt= ) Else Set nohunt=-nohunt
If 1%noslide%==10 (Set noslide= ) Else Set noslide=-noslide
If 1%nolerp%==10 (Set nolerp= ) Else Set nolerp=-nolerp
If 1%noskyfix%==10 (Set noskyfix= ) Else Set noskyfix=-noskyfix
If 1%incremental%==10 (Set incremental= ) Else Set incremental=-incremental
If 1%dump%==10 (Set dump= ) Else Set dump=-dump
If 1%notexscale%==10 (Set notexscale= ) Else Set notexscale=-notexscale

Set pdd=1%textf%
If pdd==1 Set textf=%mapb%
bbsp -verbose -estimate -threads %threads% -dev %dev% -blscale %blscale% -ilscale %ilscale% -maxnodesize %maxnodesize%  %chart% %noinfo% %onlyents% %leakonly% %showbevels% %notjunc% %noclip% %nofill% %nolighting% %nowater% "%direc%%mapb%\%mapb%_.map"
If %visspeed%==normal Set visspeed= 
If %visspeed%==fast Set visspeed=-fast
If %visspeed%==full Set visspeed=-full
bvis -estimate -threads %threads% -dev %dev% %visspeed% %chart% %noinfo% "%direc%%mapb%\%mapb%_.map"
If 1%rad%==1no Goto delcustom
brad -verbose -estimate -threads %threads% -dev %dev% -bounce %bounce% -maxlight %maxlight% -smooth %smooth% -chop %chop% -texchop %texchop% -coring %coring% -dlight %dlight% -fade %fade% -falloff %falloff% -dscale %dscale% -bscale %bscale% -scale %scale% -gamma %gamma% -ambient %ambient% %chart% %noinfo% %sparse% %nomatrix% %extra% %circus% %nohunt% %noslide% %nolerp% %noskyfix% %incremental% %dump% %notexscale% "%direc%%mapb%\%mapb%_.map"
Goto delcustom


:delcustom
Del "%direc%%mapb%\%mapb%_.map" /f /q
Copy "%direc%%mapb%\%mapb%_.bsp" "%direc%%mapb%\%mapb%.bsp" /Y
Del "%direc%%mapb%\%mapb%_.bsp" /f /q
If Exist "%direc%%mapb%\%mapb%_.err" Goto errnorm
If Not Exist "%direc%%mapb%\%mapb%.bsp" Goto errnorm
If 1%temp%==1Yes Goto quit
Goto done

:fastcompil
cls
bbsp -verbose -estimate "%direc%%mapb%\%mapb%_.map" 
bvis -estimate -fast "%direc%%mapb%\%mapb%_.map"
Del "%direc%%mapb%\%mapb%_.map" /f /q
Copy "%direc%%mapb%\%mapb%_.bsp" "%direc%%mapb%\%mapb%.bsp" /Y
Del "%direc%%mapb%\%mapb%_.bsp" /f /q
If Exist "%direc%%mapb%\%mapb%_.err" Goto errnorm
If Exist "%direc%%mapb%\%mapb%.bsp" Goto done
Goto errnorm2


:qualcompil
cls
bbsp -verbose -estimate "%direc%%mapb%\%mapb%_.map" 
bvis -estimate -full "%direc%%mapb%\%mapb%_.map"
brad -verbose -estimate -extra -smooth 100 -bounce 2 "%direc%%mapb%\%mapb%_.map"
Del "%direc%%mapb%\%mapb%_.map" /f /q
Copy "%direc%%mapb%\%mapb%_.bsp" "%direc%%mapb%\%mapb%.bsp" /Y
Del "%direc%%mapb%\%mapb%_.bsp" /f /q
If Exist "%direc%%mapb%\%mapb%_.err" Goto errnorm
If Exist "%direc%%mapb%\%mapb%.bsp" Goto done
Goto errnorm2

:done
cls
Echo.
Echo Deleting temporary files....
Del "%direc%%mapb%\*.lbsp" /f /q
Del "%direc%%mapb%\*.lin" /f /q
Del "%direc%%mapb%\*.log" /f /q
Del "%direc%%mapb%\*.map" /f /q
Del "%direc%%mapb%\*.p0" /f /q
Del "%direc%%mapb%\*.p1" /f /q
Del "%direc%%mapb%\*.p2" /f /q
Del "%direc%%mapb%\*.p3" /f /q
Del "%direc%%mapb%\*.p4" /f /q
Del "%direc%%mapb%\*.prt" /f /q
Del "%direc%%mapb%\*.pts" /f /q
Del "%direc%%mapb%_.map" /f /q
If 1%statnf%==1Yes Goto startnf
color f1
cls
Echo.
Echo  Map compiled sucessfully, press any key to exit
Echo  ***********************************************
Echo         Compiler utility made by 007 Tres
echo       Corrector.exe made by FordGT90Concept
Echo.
pause > nul
Goto quit

:lighttest
cls
bbsp -verbose -estimate "%direc%%mapb%\%mapb%_.map" 
bvis -estimate -fast "%direc%%mapb%\%mapb%_.map"
brad -verbose -estimate -chop 384 -texchop 256 "%direc%%mapb%\%mapb%_.map"
Del "%direc%%mapb%\%mapb%_.map" /f /q
Copy "%direc%%mapb%\%mapb%_.bsp" "%direc%%mapb%\%mapb%.bsp" /Y
Del "%direc%%mapb%\%mapb%_.bsp" /f /q
If Exist "%direc%%mapb%\%mapb%_.err" Goto errnorm
If Exist "%direc%%mapb%\%mapb%.bsp" Goto done
Goto errnorm2


:normcompil
cls
bbsp -verbose -estimate "%direc%%mapb%\%mapb%_.map" 
bvis -estimate -fast "%direc%%mapb%\%mapb%_.map"
brad -verbose -estimate "%direc%%mapb%\%mapb%_.map"
Del "%direc%%mapb%\%mapb%_.map" /f /q
Copy "%direc%%mapb%\%mapb%_.bsp" "%direc%%mapb%\%mapb%.bsp" /Y
Del "%direc%%mapb%\%mapb%_.bsp" /f /q
If Exist "%direc%%mapb%\%mapb%_.err" Goto errnorm
If Exist "%direc%%mapb%\%mapb%.bsp" Goto done
Goto errnorm2



:errnorm2
set statnf=No
Echo An error happened while compiling, check the map and try again
Echo.
Echo Press any key To Exit

Goto quit

:errnorm
set statnf=no
For /f "tokens=1,2,3,4,5,6,7,8,9,10,11,12,13,14,15* delims=,:'() " %%a In (%direc%%mapb%\%mapb%_.err) Do (
If 1%%d==1UAxis (
	Set errorno=1
	Set ent=%%f
	Set brush=%%h
	goto errdetct
	) Else If 1%%d==1version (
	Set errorno=2
	Set mapver=%%f
	goto errdetct
	) Else If 1%%f==1incomplete (
	Set errorno=3
	Set line=%%d
	goto errdetct
	) Else If 1%%d==1opening (
	Set errorno=4
	goto errdetct
	) Else If 1%%i==1plane (
	Set errorno=5
	Set ent=%%d
	Set brush=%%f
	goto errdetct
	) Else If 1%%i==1has (
	Set errorno=6
	Set ent=%%d
	Set brush=%%f
	goto errdetct
	) Else If 1%%b==1LEAK (
	Set errorno=7
	goto errdetct
	) Else If 1%%e==1contains (
	Set errorno=8
	Set ent=%%d
	goto errdetct
	) Else If 1%%d==1MAX_PATCHES (
	Set errorno=9
	goto errdetct
	) Else If 1%%h==1world (
	Set errorno=a
	Set ent=%%d
	Set brush=%%f
	goto errdetct
	)
)

cls
echo ---------------------->>err.log
echo.>>err.log
echo Open Log - %DATE% %TIME%>>err.log
echo tags: a:%%a b:%%b c:%%c d:%%d e:%%e f:%%f g:%%g h:%%h i:%%i j:%%j>>err.log
echo.>>err.log
type "%direc%%mapb%\%mapb%_.err">>err.log
echo.>>err.log
echo Close Log - %DATE% %TIME%>>err.log

Echo.
Echo  Error not SAVED in LIST, this section is in beta testings.
echo  This program has created a file called err.log that is in %CD%
echo  if you want this error reported as solution in forward releases
echo  just mail the file to triomphe3@hotmail.com, Thanks.
Echo.
type "%direc%%mapb%\%mapb%_.err"
Echo.
Del "%direc%%mapb%\%mapb%_.err" /f /q
Echo Press any key to exit
pause > NUL
Goto quit

:errdetct
cls
Echo.
Echo  This Error happened While compiling:
type "%direc%%mapb%\%mapb%_.err"

If 1%errorno%==12 (
	Echo Solution: Put a number of the map version -510 For GC And 220 For Hammer-, you can Do this In GC/Hammer going To Map-Map Properties, Then Select Or create the key mapversion, And put a value there - 510 For GC And 220 For Hammer -
	) Else If 1%errorno%==11 (
	Echo Solution: Bad .map version, Or Not from HL And expansions, this can be too a bug of BBSP, try To delete the brush In Hammer/GC In the menu Map-Go To Brush Number, Then type In Entity # the number %ent% And In Brush # the number %brush%. You will see that the editor will Select an object, it shall be erroneus Or invalid. You can try To fix it Or delete it And try To compile this map again. 
	) Else If 1%errorno%==13 (
	Echo Solution: The map seems totally invalid, try at GC/Hammer the menu Map- Check For problems Or opening the .map file with wordpad And checking the line %line%.
	) Else If 1%errorno%==14 (
	Echo Solution: No solution available, try putting your map into c:\NFEditors\maps
	) Else If 1%errorno%==15 (
	Echo Solution: This can be too a bug of BBSP, try To delete the brush In Hammer/GC In the menu Map-Go To Brush Number, Then type In Entity # the number %ent% And In Brush # the number %brush%. You will see that the editor will Select an object, it shall be erroneus Or invalid. You can try To fix it Or delete it And try To compile this map again. 
	) Else If 1%errorno%==16 (
	Echo Solution: This can be too a bug of BBSP, try To delete the brush In Hammer/GC In the menu Map-Go To Brush Number, Then type In Entity # the number %ent% And In Brush # the number %brush%. You will see that the editor will Select an object, it shall be erroneus Or invalid. You can try To fix it Or delete it And try To compile this map again. 
	) 
If 1%errorno%==17 (
	Echo Solution: Make a skybox that involves all your map, Or try To hide all your map from outside 
	) Else If 1%errorno%==18 (
	Echo Solution: You used the 17Origin texture without another block For a specific entity -like func_door_rotating Or func_button_rotating.- Put another block, group them And Then tie both To an entity. If you dont know which entity Is refering the Error, go To map-Go To brush number And type In entity the number %ent% And In brush type the number 0, the block will be selected.
	)
If 1%errorno%==19 (
	Echo Solution: The terrain\map that you created is too complex to parse the lighting, or you selected a very low chop/texchop value. You can compiling this map on higher chop/texchops value with CUSTOM COMPILING or just remove those complex brush structures of the map.
	)
If 1%errorno%==1a (
	Echo Solution: You may made some brush that goes outside the map, or just a decompiler bug. To fix it, try To delete the brush In Hammer/GC In the menu Map-Go To Brush Number, Then type In Entity # the number %ent% And In Brush # the number %brush%. You will see that the editor will Select an object, it shall be erroneus Or invalid. You can try To fix it Or delete it And try To compile this map again.
	)

echo.
echo Error code: %errorno%
Del "%direc%%mapb%\%mapb%_.err" /f /q
Echo.
Echo Press any key To Exit
pause > NUL
Goto quit

:startnf
If Not Exist "%nfdir%\bond.exe" Goto errnf
Copy /Y "%direc%%mapb%\%mapb%.bsp" "%nfdir%\bond\maps\%mapb%.bsp"
Echo %nfdir%> nfdir.txt
For /f "tokens=1,2* delims=\ " %%a In (nfdir.txt) Do Set drive=%%a
%drive%c:
Cd %nfdir%
Del nfdir.txt /f /q
start /w bond.exe %noipx% %screen% %dedserver% -hunk %hunk% -heap %heap% %comm% +map %mapb%
Set statnf=No
Goto quit

:errornotfoundf
if 1%bloo%==1boo goto finnit
if exist c:\NFEditors\tools\bbsp.exe (
	set bloo=boo
	c:
	cd c:\NFEditors\tools
	goto begin
	)
goto finnit 

:finnit
cls
Echo.
Echo Some of the compile files are missing for this program To work, please, reinstall the program
Echo.
pause
Goto quit 

:errnf
Echo.
Echo Nightfire does not appear installed Or installation Is invalid (Registry Values) And can't be started.
Echo.
Set statnf=No
pause
Goto quit


:quit
If %statnf%==Yes Goto startnf
Del nfdir.txt