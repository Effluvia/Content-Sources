@echo off 
set MAPSPATH=d:\[Mapping]\maps
set MAPNAME=1 
set CSPATH=d:\Games\Half-life\valve\maps
 
d:\[Mapping]\Zhlt\hlcsg.exe -texdata 8192 d:\[Mapping]\maps\1.map 
d:\[Mapping]\Zhlt\hlbsp.exe -texdata 8192 -estimate d:\[Mapping]\maps\1.map
d:\[Mapping]\Zhlt\hlvis.exe -texdata 8192 -full -estimate d:\[Mapping]\maps\1.map
d:\[Mapping]\Zhlt\hlrad.exe -texdata 8192 -extra -estimate d:\[Mapping]\maps\1.map  

echo COMPILE END
