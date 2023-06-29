$modelname "palm\palm2\palm2.mdl"
$cdmaterials "models\palm\palm2"
$cliptotextures

$scale 1.0
//$staticprop

// 0 attachments

// 0 bone controllers

// 1 hit boxes
$hbox 0 "joint1" -8 -6 0 8 8 205

$bodygroup studio
{
// when exporting from XSI, rotate X=90 Z=180, then freeze transforms
studio "palm2-xsi.smd"
}

// 1 sequences
$sequence idle1 "idle1" loop fps 13 

$surfaceprop "Wood_Solid"

