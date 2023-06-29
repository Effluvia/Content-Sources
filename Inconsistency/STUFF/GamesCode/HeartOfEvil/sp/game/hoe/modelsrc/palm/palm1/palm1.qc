$modelname "palm\palm1\palm1.mdl"
$cdmaterials "models\palm\palm1"
$cliptotextures

$scale 1.0
//$staticprop

// 0 attachments

// 0 bone controllers

// 1 hit boxes
$hbox 0 "joint1" -6 -6 -0  6 6 155

$bodygroup studio
{
// when exporting from XSI, rotate X=90 Z=180, then freeze transforms
studio "palm1-xsi.smd"
}

// 1 sequences
$sequence idle1 "idle1" loop fps 13 

$surfaceprop "Wood_Solid"
