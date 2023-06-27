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
$body studio "v_shotgun_reference_HEVHands"
$body studio "shell"

$texrendermode "barrel.bmp" chrome

$origin 0 0 64

//$origin -0.4 -1 63.40 -1.5

//$origin -0.5 -2.25 62.499

//$origin       SIDE-SIDE IN-OUT  UP-DOWN

//$origin -0.348 -2 63.499 -5.5

$attachment 0 "Bip01 R Hand" 30 1 6 X 1

$sequence sm_idle "small_idle" fps 9 loop
$sequence shoot "shoot_reload" fps 20 { event 5001 0 "31" }
$sequence shoot_big "shoot_bigammo" fps 13 frames 0 19 { event 5001 0 "41" }
$sequence reload "reload_short" fps 16
$sequence pump "pump_after_reload_e3" fps 12
$sequence start_reload "start_reload" fps 12
$sequence draw "draw_weapon" fps 24
$sequence reholster "reholster" fps 30
$sequence idle2 "idle2" fps 15 loop
$sequence deepidle "deep_idle" fps 12 loop
