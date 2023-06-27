/*
==============================================================================

shotgun viewmodel

==============================================================================
*/

$modelname ./MDL_E3/v_shotgun.mdl
$cd ./SMD
$cdtexture ./Textures_E3
$scale 1.0 

// whole body
$body studio "v_shotgun_reference_E3"
$body studio "v_shotgun_reference_HEVHands"
$body studio "shell"

$texrendermode "barrel_chrome.bmp" chrome

$origin 2.152 -3 61.499 3

$attachment 0 "Bip01 R Hand" 30 1 6 X 1

$sequence sm_idle "small_idle" fps 9 loop
$sequence shoot "new_shoot" fps 20 { event 5001 0 "31" }
$sequence shoot_big "shoot_bigammo" fps 13 frames 0 19 { event 5001 0 "41" }
$sequence reload "reload_short" fps 16
$sequence pump "pump_after_reload_e3" fps 12
$sequence start_reload "start_reload" fps 12
$sequence draw "draw_weapon" fps 24
$sequence reholster "reholster" fps 30
$sequence idle2 "idle2" fps 15 loop
$sequence deepidle "deep_idle" fps 12 loop
