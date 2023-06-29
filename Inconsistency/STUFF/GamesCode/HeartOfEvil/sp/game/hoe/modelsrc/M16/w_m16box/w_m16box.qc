$modelname "w_m16box.mdl"
$cdmaterials "models\M16\w_m16box"
$cliptotextures

$scale 1.0
$staticprop

// 0 attachments

// 0 bone controllers

// 1 hit boxes
//$hbox 0 "Box01" -16.950001 -5.760000 -0.430000  10.070000 12.770000 14.180000

$bodygroup studio
{
studio "ammo_box.smd"
}

// 1 sequences
$sequence idle "idle" fps 30 

// auto-create a collisionmodel
$collisionmodel "ammo_box.smd"
{
    $mass 2.0
}

