$modelname "machete\w_machete\w_machete.mdl"
$cdmaterials "models\machete\w_machete"
$cliptotextures

$scale 1.0 // was 1.5 before adding to player

$surfaceprop "metal"

// 0 attachments

// 0 bone controllers

$bodygroup studio
{
studio "ref_w_machete2010.smd"
}

// 1 sequences
$sequence idle "seq_idle1" fps 30 

// auto-create a collisionmodel
$collisionmodel "ref_w_machete2010.smd"
{
    $mass 4.0 // HL1 value
}

