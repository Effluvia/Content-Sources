$modelname ./MDL/v_crossbow.mdl
$cd ./smd
$cdtexture ./Textures
// THIS VERSION USES THE OLDER MESH WITH THE SNIPER SCOPE

$scale 1.0

$origin -6 0 64

//$origin -6.325 .5 62.85 -1.2 // *to move it back to center, the X-offset is -6
//$origin -6.125 .5 62.85 1 // *to move it back to center, the X-offset is -6

// whole body
$body body "crossbow_bow1" reverse
$body body "crossbow_hand_reduced"
$body body "crossbow_bow2"
$body body "crossbow_lens"

$sequence idle1 "crossbow_idle1(simple)" loop fps 30
$sequence idle2 "crossbow_idle2(empty)" loop fps 30
$sequence fidget1 "crossbow_fidget(bad pluck)" fps 30
$sequence fidget2 "crossbow_fidget2(empty)" fps 30
//old
//$sequence fire1 "crossbow_fire1(standard)" fps 60 {
//{ event 5004 30 "weapons/xbow_reload1.wav" }
//{ event 5004 70 "weapons/dryfire1.wav" }
//}
$sequence fire1 "crossbow_fire_old" fps 30 {
{ event 5004 25 "weapons/xbow_reload1.wav" }
{ event 5004 60 "weapons/dryfire1.wav" }
}
$sequence fire2 "crossbow_fire2(reload)" fps 30
$sequence fire3 "crossbow_fire3(empty)" fps 30
$sequence reload "crossbow_reload(fromempty)" fps 30 {
{ event 5004 17 "items/gunpickup4.wav" } 
{ event 5004 48 "items/gunpickup3.wav" }
{ event 5004 74 "weapons/xbow_reload1.wav" }
{ event 5004 105 "weapons/dryfire1.wav" }
}
$sequence draw1 "crossbow_draw1(loaded)" fps 30
$sequence draw2 "crossbow_draw2(empty)" fps 30
$sequence holster1 "crossbow_holster1(loaded)" fps 30
$sequence holster2 "crossbow_holster2(empty)" fps 30
