$modelname "w_battery\w_battery.mdl"
$cdmaterials "models\w_battery"
$cliptotextures

$scale 2.0
$staticprop
$surfaceprop "metal"

// 0 attachments

// 0 bone controllers

$bodygroup studio
{
studio "ref_w_battery.smd"
}

// 1 sequences
$sequence idle "seq_idle1" fps 30 

$collisionmodel "ref_w_battery.smd"
{
    $mass 2.0
}

