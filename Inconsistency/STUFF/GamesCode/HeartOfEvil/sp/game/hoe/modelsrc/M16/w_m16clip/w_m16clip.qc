$modelname "w_m16clip.mdl"
$cdmaterials "models\M16\w_m16clip"
$cliptotextures

$scale 1.5
//$staticprop // rotation

// 0 attachments

// 0 bone controllers

$bodygroup studio
{
studio "ref_w_m16clip.smd"
}

// 1 sequences
$sequence idle "seq_idle1" fps 30 

// auto-create a collisionmodel
$collisionmodel "ref_w_m16clip.smd"
{
    $mass 2.0
}

