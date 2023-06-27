/*
==============================================================================

shotgun viewmodel

==============================================================================
*/

$modelname ./mdl/v_shotgun.mdl
$cd ./smd
$cdtexture ./textures
$scale 1.0 

// whole body
$body studio "shotgun_reference"
$body studio "hands_gloved"
$body studio "shell"



//$origin       SIDE-SIDE IN-OUT  UP-DOWN

$origin 0.152 -2.5 62.499 3

$attachment 0 "Bip01 R Hand" 30 1 6 X 1

$sequence sm_idle "small_idle" fps 9 loop
$sequence shoot "new_shoot(edited)" fps 20 { event 5001 0 "31" }
$sequence shoot_big "shoot_bigammo" fps 13 { event 5001 0 "41" }
$sequence reload "reload_short" fps 16
$sequence pump "pump_after_reload" fps 13
$sequence start_reload "start_reload" fps 12
$sequence draw "draw_weapon" fps 24
$sequence reholster "reholster" fps 30
$sequence idle2 "idle2" fps 20 loop
$sequence deepidle "deep_idle" fps 12 loop
