
REM use your own folders for the two vars below, the rest should work out.
REM this will get anything the projects don't copy over for whatever reason, I forget what state I left it in.
REM sometimes you want everything copied over, sometimes you don't.

SET projectdir=C:\Users\blue2\Desktop\SM\HLS1 AZ - gameui dll - point of frustration
SET gamedir=C:\Program Files (x86)\Valve\Half-Life NGHLv3

REM ---------------------------------------


copy  "%projectdir%\vgui2\src\Release\vgui2.dll" "%gamedir%\vgui2.dll" >nul
copy  "%projectdir%\tier0\Release\tier0.dll" "%gamedir%\tier0.dll" >nul
copy  "%projectdir%\vstdlib\Release\vstdlib.dll" "%gamedir%\vstdlib.dll" >nul
copy  "%projectdir%\filesystem\filesystem_stdio\Release\filesystem_stdio.dll" "%gamedir%\filesystem_stdio.dll" >nul


copy  "%projectdir%\vgui2\src\Release\vgui2.pdb" "%gamedir%\vgui2.pdb" >nul
copy  "%projectdir%\tier0\Release\tier0.pdb" "%gamedir%\tier0.pdb" >nul
copy  "%projectdir%\vstdlib\Release\vstdlib.pdb" "%gamedir%\vstdlib.pdb" >nul
copy  "%projectdir%\filesystem\filesystem_stdio\Release\filesystem_stdio.pdb" "%gamedir%\filesystem_stdio.pdb" >nul


pause

