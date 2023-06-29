$modelname "M60\w_m60box\w_m60box.mdl"
$cdmaterials "models\M60\m60ammobox"
$cliptotextures

$scale 1.0
$staticprop
$surfaceprop "metal"

// 0 attachments

// 0 bone controllers

// 1 hit boxes
//$hbox 0 "Box01" -16.950001 -5.760000 -0.430000  10.070000 12.770000 14.180000

$bodygroup studio
{
studio "ref_w_m60box.smd"
}

// 1 sequences
$sequence idle "idle" fps 30 

// auto-create a collisionmodel
$collisionmodel "ref_w_m60box.smd"
{
    $mass 2.0
}

