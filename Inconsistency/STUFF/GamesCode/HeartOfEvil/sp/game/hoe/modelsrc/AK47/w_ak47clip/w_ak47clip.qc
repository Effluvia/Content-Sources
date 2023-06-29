$modelname "AK47\w_ak47clip\w_ak47clip.mdl"
$cdmaterials "models\AK47\w_ak47clip"
$cliptotextures

$scale 1.5

// 0 attachments

// 0 bone controllers

$bodygroup studio
{
studio "ref_w_ak47clip.smd"
}

// 1 sequences
$sequence idle "seq_idle1" fps 30 

// auto-create a collisionmodel
$collisionmodel "ref_w_ak47clip.smd"
{
    // FIXME: proper mass
    $mass 1.0
}

