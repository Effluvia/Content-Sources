$modelname "medkit\w_medkit_NVA.mdl"
$cdmaterials "models\medkit"
$cliptotextures

$scale 1.5
$staticprop

// 0 attachments

// 0 bone controllers

$bodygroup studio
{
studio "ref_w_medkit_NVA.smd"
}

// 1 sequences
$sequence idle "idle" fps 30 

$collisionmodel "collision_model_NVA.smd"
{
    $mass 10.0
    $concave
}

