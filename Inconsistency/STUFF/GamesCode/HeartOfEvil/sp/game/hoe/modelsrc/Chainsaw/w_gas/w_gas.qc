$modelname "Chainsaw\w_gas\w_gas.mdl"
$cdmaterials "models\Chainsaw\w_gas"
$cliptotextures

$scale 1.5
$staticprop

// 0 attachments

// 0 bone controllers

$bodygroup studio
{
studio "ref_w_gas.smd"
}

// 1 sequences
$sequence idle "seq_idle1" fps 30 

// auto-create a collisionmodel
$collisionmodel "ref_w_gas.smd"
{
    $mass 4.0
}

