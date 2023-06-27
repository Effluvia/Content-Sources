$modelname ./MDL/v_crossbow.mdl
$cd ./smd
$cdtexture ./Textures
// THIS VERSION USES THE OLDER MESH WITH THE SNIPER SCOPE

$scale 1.0

$origin -6 .5 63.25 4 

// whole body
$body body "crossbow_bow" reverse
$body body "crossbow_hand"

$sequence idle1 "crossbow_idle1(simple)" loop fps 30
$sequence idle2 "crossbow_idle2(empty)" loop fps 30
$sequence fidget1 "crossbow_fidget(bad pluck)" fps 30
//$sequence fidget1 "crossbow_fidget1(flick)" fps 30
$sequence fidget2 "crossbow_fidget2(empty)" fps 30
$sequence fire1 "crossbow_fire1(standard)" fps 60 {
{ event 5004 66 "weapons/dryfire1.wav" }
}
$sequence fire2 "crossbow_fire2(reload)" fps 30
$sequence fire3 "crossbow_fire3(empty)" fps 30
$sequence reload "crossbow_reload(fromempty)" fps 30 {

{ event 5004 13 "items/gunpickup4.wav" } 
{ event 5004 48 "items/gunpickup3.wav" }
{ event 5004 48 "items/gunpickup3.wav" }
{ event 5004 114 "weapons/dryfire1.wav" }
}
$sequence draw1 "crossbow_draw1(loaded)" fps 30
$sequence draw2 "crossbow_draw2(empty)" fps 30
$sequence holster1 "crossbow_holster1(loaded)" fps 30
$sequence holster2 "crossbow_holster2(empty)" fps 30
