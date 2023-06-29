$modelname "spx_mature\gibs\wingRR.mdl"
$cdmaterials "models\spx_mature"
//$cliptotextures

$scale 1

$staticprop
$autocenter

$bodygroup studio
{
studio "wingRR.smd"
}

$sequence idle "seq_idle" loop fps 1 ACT_IDLE 1

// auto-create a collisionmodel
$collisionmodel "wingRR.smd"
{
    $mass 0.5
    $drag 3.5
}

$keyvalues
{
    prop_data
    {
        base Plastic.Small
    }
}

