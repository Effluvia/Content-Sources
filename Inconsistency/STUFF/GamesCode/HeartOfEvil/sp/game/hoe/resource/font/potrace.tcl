console show
cd "C:/Program Files/Steam/SteamApps/SourceMods/hoe/resource/font"
#set INPUT 870_screenshot
set INPUT ak47_screenshot
set MKBITMAP "C:/Program Files/potrace-1.8.win32-i386/mkbitmap.exe"
set POTRACE "C:/Program Files/potrace-1.8.win32-i386/potrace.exe"
exec $MKBITMAP -f 3 -s 1 -t 0.5 $INPUT.bmp
exec $POTRACE --svg $INPUT.pbm
