/*
==============================================================================

glock viewmodel

==============================================================================
*/

$modelname ./MDL/v_9mmhandgun_bare.mdl
$cd ./SMD
$cdtexture ./Textures
$scale 1.0 

$body studio "glock_reference"
$body studio "HEV_Hands_Bare"

$bodygroup silencer
{
blank
studio "glock_reference(silencer)"
}

$attachment 0 "Bip01 R Hand" 10 2.25 4.25 X 1

$origin 0.25 -2.5 62.5 0

$sequence idle1 "glock_idle1" fps 0 loop
$sequence idle2 "glock_idle2" fps 0 loop
$sequence idle3 "glock_idle3" fps 0 loop
$sequence shoot "glock_shoot" fps 25 { event 5001 0 "11" }
$sequence shoot_empty "glock_shoot_slideback" fps 25 { event 5001 0 "11" }
$sequence reload "reload" fps 18 {
{ event 5004 4 "items/9mmclip2.wav" }
{ event 5004 23 "items/9mmclip1.wav" }
}

$sequence reload_noshot "reload" fps 18 {
{ event 5004 4 "items/9mmclip2.wav" }
{ event 5004 23 "items/9mmclip1.wav" }
}
$sequence draw "glock_draw" fps 16
$sequence holster "glock_holster" fps 16
$sequence add_silencer "glock_add_silencer" fps 18