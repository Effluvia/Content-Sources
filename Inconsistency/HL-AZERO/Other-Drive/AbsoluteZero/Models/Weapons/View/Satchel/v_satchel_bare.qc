$modelname ./MDL_Bare/v_satchel.mdl
$cd ./SMD
$cdtexture ./Textures

$scale 1.0

$origin 0 0 64

// whole body
$body body "satchel_hand_bare"
$body body "satchel_charge"

$sequence idle1 "satchel_idle1(simple)" loop fps 30
$sequence fidget1 "satchel_fidget1(gripshift)" fps 30
$sequence draw "satchel_draw" fps 30
$sequence drop "satchel_drop" fps 30
