$modelname "peasant\shackle.mdl"
$cdmaterials "models\lamp"
$cliptotextures

$surfaceprop "metal"

$scale 1.0
$staticprop

$attachment "rope" "static_prop" 0.00 0.00 0.00 rotate 0 0 0

$bodygroup studio
{
studio "ref_shackle.smd"
}

// 1 sequences
$sequence idle "seq_idle" fps 30 

