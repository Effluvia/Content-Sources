/*
==============================================================================

glock viewmodel

==============================================================================
*/

$modelname ./MDL/v_9mmhandgun_test.mdl
$cd ./SMD
$cdtexture ./Textures
$scale 1.0 

// whole body

$body studio "glock_reference_test"
$body studio "clip_reference"

$bodygroup silencer
{
blank
studio "glock_reference(silencer)"
}

$body studio "HEV_Hands_Gloved"



$attachment 0 "Bip01 R Hand" 10 2.25 4.25 X 1

//$origin       SIDE-SIDE IN-OUT  UP-DOWN

$origin 0 -1 64
$sequence idle1 "glock_idle1" fps 16 loop
$sequence idle2 "glock_idle2" fps 16 loop
$sequence idle3 "glock_idle3" fps 14 loop
$sequence shoot "glock_shoot" fps 25 { event 5001 0 "11" }
$sequence shoot_empty "glock_shoot_slideback" fps 25 { event 5001 0 "11" }
$sequence reload "Reload" fps 18 {
{ event 5004 7 "items/9mmclip2.wav" }
{ event 5004 12 "items/9mmclip1.wav" }
{ event 5004 20 "items/guncock1.wav" } 
}

$sequence reload_noshot "reload_empty" fps 18 {
{ event 5004 7 "items/9mmclip2.wav" }
{ event 5004 12 "items/9mmclip1.wav" }
{ event 5004 20 "items/guncock1.wav" } 
}
$sequence draw "glock_draw" fps 16
$sequence holster "glock_holster" fps 16
$sequence add_silencer "glock_add_silencer" fps 18