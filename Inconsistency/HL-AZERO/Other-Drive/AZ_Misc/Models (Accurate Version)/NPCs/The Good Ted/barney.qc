/*
----------------------------

barney.qc

----------------------------
*/

$modelname ./MDL/barney.mdl
$cd ./SMD
$cdtexture ./Textures
$externaltextures


$scale 1.05

$controller 0 "Bip01 Head" XR -60 60
$controller mouth "Bone05" ZR 0 15


// whole body
$body studio "Ted"

$bodygroup gun
{
studio "BARNEY2_Template_Biped5(Hand&Ghlstr)"
studio "BARNEY2_Template_Biped2(WithGun)" 
studio "BARNEY2_Template_Biped5(Hlstr)"
}

$eyeposition 0 0 63
//$eyeposition 0 6 63

$include ".//barney_shared.qc"
