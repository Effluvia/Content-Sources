/*
==============================================================================

mp5 machine gun viewmodel

==============================================================================
*/

$modelname ./MDL/v_9mmAR_Center.mdl
$cdtexture ./Textures
$cd ./SMD
$scale 1 

// whole body
$body studio "reference_uv"
$body studio "hands_gloved_hev"


//$origin	SIDE-SIDE IN-OUT  UP-DOWN
$origin 0.1 12.9 8

$attachment 0 "Bone09" -14 1 -0.5 X -1

$sequence longidle "long_idle" fps 8
$sequence idle1 "idle1" fps 35 
$sequence grenade "grenadeshoot" fps 30 

$sequence reload "reload" fps 20 {
{ event 5004 5 "items/cliprelease1.wav" } 
{ event 5004 24 "items/clipinsert1.wav" }
origin -0.6 -0.2 -0.5 
}

$sequence deploy "deploy" frame 4 15 fps 12
$sequence shoot "shoot1" fps 10 { event 5001 0 "20" }
$sequence shoot "shoot2" fps 10 { event 5001 0 "20" }
$sequence shoot "shoot3" fps 10 { event 5001 0 "20" }



