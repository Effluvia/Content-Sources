/*
==============================================================================

mp5 machine gun viewmodel

==============================================================================
*/

$modelname ./MDL_Bare/v_9mmAR.mdl
$cdtexture ./Textures
$cd ./SMD
$scale 1 

// whole body

$body studio "reference"
$body studio "hands_bare"


//$origin	SIDE-SIDE IN-OUT  UP-DOWN
$origin 3 14 8
//$origin 3.75 14.5 7.6

$attachment 0 "Bone09" -14 1 -0.5 X -1

$sequence longidle "long_idle" fps 8
$sequence idle1 "idle1" fps 35 
$sequence grenade "grenadeshoot" fps 30 

$sequence reload "reload" fps 20 {
{ event 5004 5 "items/cliprelease1.wav" } 
{ event 5004 24 "items/clipinsert1.wav" }
origin 3.95 1.81 0.65
//origin 3 1.81 1.5
} 

$sequence deploy "deploy" frame 4 15 fps 12
$sequence shoot "shoot1" fps 10 { event 5001 0 "20" }
$sequence shoot "shoot2" fps 10 { event 5001 0 "20" }
$sequence shoot "shoot3" fps 10 { event 5001 0 "20" }
$sequence holster "holster" frame 0 11 fps 12



