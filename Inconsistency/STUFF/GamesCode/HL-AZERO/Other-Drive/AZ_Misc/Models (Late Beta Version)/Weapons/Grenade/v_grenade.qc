$modelname ./MDL/v_grenade.mdl
$cd ./SMD
$cdtexture ./Textures

$scale 1.0

$origin 0 0 64

// whole body
$body body "grenade"

$body body "grenade_hand"

$sequence idle "grenade_idle1(sway)" loop fps 30
$sequence fidget "grenade_fidget1(thumb)" loop fps 30
$sequence pinpull "grenade_pinpull" fps 45
$sequence throw1 "grenade_throw1(toss)" fps 18 
$sequence throw2 "grenade_throw2(medium)" fps 20
$sequence throw3 "grenade_throw3(hard)" fps 30 
$sequence holster "grenade_holster" fps 30
$sequence draw "grenade_draw" fps 30

