$modelname "w_whisky.mdl"
$cdmaterials "models\mikeforce"
$cliptotextures

$scale 1.0
// $staticprop // rotation

$surfaceprop "popcan"

// 0 attachments

// 0 bone controllers

$bodygroup studio
{
studio "ref_Whisky-XSI.smd"
}

// 1 sequences
$sequence idle "seq_whisky" fps 30 

// auto-create a collision model
$collisionmodel "ref_Whisky-XSI.smd"
{
    $mass 1.0
}

