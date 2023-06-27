$modelname ./mdl/v_gauss.mdl
$cd ./smd
$cdtexture ./textures

$scale 1.0

$origin -1 -1 63 -3

// whole body
$body body "gauss_gunfixer" reverse
$body body "gauss_hand" 

$attachment 0 "Gauss Master" 17 -1 0 X 1

$sequence idle "idle1" loop fps 8
$sequence idle2 "idle2" loop fps 15
$sequence fidget "gauss_fidget1(spin)" fps 30
$sequence spinup "gauss_spinup" loop fps 30
$sequence spin "gauss_spincycle" loop fps 30
$sequence fire "Gauss_fire1(quick)" fps 30
$sequence fire2 "Gauss_fire2(violent)" fps 30
$sequence holster "holster" fps 25
$sequence draw "draw" fps 22