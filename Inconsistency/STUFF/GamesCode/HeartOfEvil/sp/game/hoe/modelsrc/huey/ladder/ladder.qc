$modelname "huey\ladder.mdl"
$cdmaterials "models\huey\ladder"
$cliptotextures

$scale 1.0
//$surfaceprop "popcan"

$bodygroup studio
{
studio "ref_ladder.smd"
}

$attachment "passenger" "bone38" 0.00 -7.00 0.00 rotate 90 90 0

$sequence idle "seq_idle" fps 1
$sequence unwind "seq_unwind" fps 120 
$sequence unwound "seq_unwound" fps 1

