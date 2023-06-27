$modelname ./MDL/v_gauss.mdl
$cd ./SMD
$cdtexture ./Textures

$scale 1.0

$origin 0 0 64

//$origin -1 -1.5 63 -3

//$origin -3 0 64 5

// whole body
$body body "gauss_gunfixer" reverse
$body body "gauss_hand" 

$attachment 0 "Gauss Master" 17 -1 0 X 1

$sequence idle "gauss_idle_slarty" loop fps 15
$sequence idle2 "Gauss_idle2(backandforth)_beta" loop fps 15
$sequence fidget "gauss_fidget1(spin)" fps 30
$sequence spinup "gauss_spinup_slarty" loop fps 30
$sequence spin "gauss_spincycle_slart" loop fps 30
$sequence fire "gauss_fire1(quick)" fps 30
$sequence fire2 "Gauss_fire2(violent)" fps 30
$sequence holster "slart_holster" frame 0 30 fps 60
$sequence draw "slarty_draw" fps 64