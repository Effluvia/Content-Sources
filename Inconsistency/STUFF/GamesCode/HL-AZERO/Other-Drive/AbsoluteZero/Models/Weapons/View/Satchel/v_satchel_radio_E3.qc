$modelname ./MDL_E3/v_satchel_radio.mdl
$cd ./SMD
$cdtexture ./Textures

$scale 1.0

$origin 0 0 64

// whole body
$body body "satchel_radio_hand_E3"
$body body "satchel_radio_detonator"

$sequence idle1 "satchel_radio_idle1(simple)" loop fps 30
$sequence fidget1 "satchel_radio_fidget1(lidshake)" fps 30
$sequence draw "satchel_radio_draw" fps 30
$sequence fire "satchel_radio_fire" fps 30
$sequence holster "satchel_radio_holster" fps 30