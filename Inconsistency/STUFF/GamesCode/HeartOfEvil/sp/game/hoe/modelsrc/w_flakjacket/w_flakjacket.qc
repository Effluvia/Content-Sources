$modelname "w_flakjacket\w_flakjacket.mdl"
$cdmaterials "models\w_flakjacket"
$cliptotextures

$scale 1.0
$staticprop

// 0 attachments

// 0 bone controllers

$bodygroup studio
{
studio "ref_w_flakjacket.smd"
}

// 1 sequences
$sequence idle "idle" fps 30 

// auto-create a collisionmodel
$collisionmodel "ref_w_flakjacket.smd"
{
    $mass 2.0
}

