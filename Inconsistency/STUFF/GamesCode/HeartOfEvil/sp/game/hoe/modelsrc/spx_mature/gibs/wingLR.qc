$modelname "spx_mature\gibs\wingLR.mdl"
$cdmaterials "models\spx_mature"
//$cliptotextures

$scale 1

$staticprop
$autocenter

$bodygroup studio
{
studio "wingLR.smd"
}

$sequence idle "seq_idle" loop fps 1 ACT_IDLE 1

// auto-create a collisionmodel
$collisionmodel "wingLR.smd"
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

