/*
==============================================================================

glock viewmodel

==============================================================================
*/

$modelname ./MDL/v_9mmhandgun.mdl
$cd ./SMD
$cdtexture ./Textures
$scale 1.0 

// whole body

$body studio "glock_reference"
$body studio "clip_reference"

$bodygroup silencer
{
blank
studio "glock_reference(silencer)"
}

$body studio "HEV_Hands_Gloved"

$attachment 0 "Bip01 R Hand" 10 2.25 4.25 X 1

//$origin       SIDE-SIDE IN-OUT  UP-DOWN

$origin 0.75 -4 62.25 1
$sequence idle1 "glock_idle1" fps 0 loop
$sequence idle2 "glock_idle2" fps 0 loop
$sequence idle3 "glock_idle3" fps 0 loop
$sequence shoot "glock_shoot" fps 25
$sequence shoot_empty "glock_shoot_slideback" fps 25
$sequence reload "Reload" fps 18 {
}

$sequence reload_noshot "reload_empty" fps 18 {
}
$sequence draw "glock_draw" fps 16
$sequence holster "glock_holster" fps 16
$sequence add_silencer "glock_add_silencer" fps 18