$modelname "Chainsaw\w_chainsaw\w_chainsaw.mdl"
$cdmaterials "models\Chainsaw\w_chainsaw"
$cliptotextures

$scale 1.0 // was 1.5

$surfaceprop metal

// 0 attachments

// 0 bone controllers

$bodygroup studio
{
studio "ref_w_chainsaw.smd"
}

// 1 sequences
$sequence idle "seq_idle1" fps 30 

// auto-create a collisionmodel
$collisionmodel "ref_w_chainsaw.smd"
{
    $mass 25.0
}

