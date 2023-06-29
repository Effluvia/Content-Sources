$modelname "palm\palm3\palm3.mdl"
$cdmaterials "models\palm\palm3"
$cliptotextures

$scale 1.0
//$staticprop

// 0 attachments

// 0 bone controllers

// 1 hit boxes
$hbox 0 "joint1" -10 -14 0 10 7 266

$bodygroup studio
{
// when exporting from XSI, rotate X=90 Z=180, then freeze transforms
studio "palm3-xsi.smd"
}

// 1 sequences
$sequence idle1 "idle1" loop fps 13 

$surfaceprop "Wood_Solid"

