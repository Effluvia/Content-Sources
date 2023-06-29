$modelname "minibus.mdl"
$cdmaterials "models\minibus"
$cliptotextures

$scale 1.0
//$staticprop // crashes with this
$surfaceprop "metal"

$bodygroup studio
{
studio "minibus.smd"
}

$bodygroup tire_fl
{
studio "tire_fl.smd"
blank
}

$bodygroup tire_fr
{
studio "tire_fr.smd"
blank
}

$bodygroup tire_rl
{
studio "tire_rl.smd"
blank
}

$bodygroup tire_rr
{
studio "tire_rr.smd"
blank
}

// 1 sequences
$sequence idle "seq_idle" fps 30 

// Expensive collisionmodel, compile with -fullcollide
$collisionmodel "collision_model.smd"
{
    $concave
    $mass 1000.0
    $maxconvexpieces 30
}

