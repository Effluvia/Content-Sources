$modelname "870\w_shot_10shells\w_shot_10shells.mdl"
$cdmaterials "models\870\w_shot_10shells"
$cliptotextures

$scale 0.75

// 0 attachments

// 0 bone controllers

$bodygroup studio
{
studio "w_shot_10shells.smd"
}

// 1 sequences
$sequence idle "idle" fps 30 

// auto-create a collisionmodel
$collisionmodel "w_shot_10shells.smd"
{
    // FIXME: proper mass
    $mass 2.0
}
