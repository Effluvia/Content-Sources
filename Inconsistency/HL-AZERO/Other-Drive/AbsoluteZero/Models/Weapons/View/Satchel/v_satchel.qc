$modelname ./MDL/v_satchel.mdl
$cd ./SMD
$cdtexture ./Textures

$scale 1.0

$origin 0 0 64

//$origin -1 1.25 63 3



// whole body
$body body "satchel_hand"
$body body "satchel_charge"



$sequence idle1 "satchel_idle1(simple)" loop fps 30
$sequence fidget1 "satchel_fidget1(gripshift)" fps 30
$sequence draw "satchel_draw" frame 0 16 fps 30
$sequence drop "satchel_drop" fps 30
