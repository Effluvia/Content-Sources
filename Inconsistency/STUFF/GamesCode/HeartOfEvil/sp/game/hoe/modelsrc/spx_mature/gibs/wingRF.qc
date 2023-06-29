$modelname "spx_mature\gibs\wingRF.mdl"
$cdmaterials "models\spx_mature"
//$cliptotextures

$scale 1

$staticprop
$autocenter

$bodygroup studio
{
studio "wingRF.smd"
}

$sequence idle "seq_idle" loop fps 1 ACT_IDLE 1

// 4 identical wings, but this one doesn't create a collisionmodel...
$collisionmodel "wingRF_phys.smd"
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

