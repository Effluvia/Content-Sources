/*
==============================================================================

shotgun viewmodel

==============================================================================
*/

$modelname ./MDL/v_shotgun.mdl
$cd ./SMD
$cdtexture ./Textures
$scale 1.0 

// whole body
$body studio "v_shotgun_reference_full"
$body studio "shell"

//$origin       SIDE-SIDE IN-OUT  UP-DOWN


//this was an attempt to restore the old origin seen on controller_test_map.jpg
//it's off, but close at the same time
//pretty sure that 3 wasn't used
$origin -1 -1.5 63 3

$attachment 0 "Bip01 R Hand" 30 1 6 X 1

//$sequence sm_idle "small_idle" fps 9 loop
$sequence sm_idle "small_idle" fps 9 frame 6 6 loop
$sequence shoot "shoot_reload" fps 20 { event 5001 0 "31" }
$sequence shoot_big "shoot_bigammo" fps 13 { event 5001 0 "41" }
$sequence reload "reload_short" fps 16
$sequence pump "pump_after_reload" fps 13
$sequence start_reload "start_reload" fps 12
$sequence draw "draw_weapon" fps 24
$sequence reholster "reholster" fps 30
$sequence idle2 "idle2" fps 15 loop
$sequence deepidle "deep_idle" fps 12 loop
