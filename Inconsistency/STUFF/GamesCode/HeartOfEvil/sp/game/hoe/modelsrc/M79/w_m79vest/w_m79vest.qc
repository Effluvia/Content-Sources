$modelname "M79\w_m79vest\w_m79vest.mdl"
$cdmaterials "models\M79\w_m79vest"
$cliptotextures

$scale 1.0
$staticprop

// 0 attachments

// 0 bone controllers

$bodygroup studio
{
studio "ref_w_m79vest.smd"
}

// 1 sequences
$sequence idle "idle" fps 30 

// auto-create a collisionmodel
$collisionmodel "ref_w_m79vest.smd"
{
    $mass 5.0 // ???
}

