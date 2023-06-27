/*
----------------------------

barney.qc

----------------------------
*/

$modelname ./MDL/g_barney.mdl
$cd ./SMD
$cdtexture ./Textures
$texrendermode "blue_metal.bmp" "chrome" 
$texrendermode "white_metal.bmp" "chrome" 

$scale 1.05

$controller 0 "Bip01 Head" XR -60 60
//$controller mouth "Bone05" ZR 0 15


// whole body
$body studio "GERMAN_Reference1"

$bodygroup gun
{
studio "BARNEY2_Template_Biped5(Hand&Ghlstr)"
studio "BARNEY2_Template_Biped2(WithGun)" 
studio "BARNEY2_Template_Biped5(Hlstr)"
}

$eyeposition 0 0 63
//$eyeposition 0 6 63

$include ".//barney_shared.qc"
