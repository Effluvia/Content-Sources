

$modelname MDL/v_HGun.mdl
$cd ./SMD
$cdtexture ./Textures

$scale 1.0

$origin 0 0 0

// whole body
$body body "v_Hgun_Template_Biped1"
$sequence idle1 "v_HGUN_Idle(Subtle)1" loop fps 16
$sequence fidgetSway "v_HGUN_Fidget1(Sway)" fps 16
$sequence fidgetShake "v_HGUN_Fidget1(Shake)" fps 16
$sequence down "v_HGUN_Down1" fps 16
$sequence up "v_HGUN_Up1" fps 30
$sequence Shoot "v_HGUN_Shoot1" fps 24 