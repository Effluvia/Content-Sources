// Output .MDL
$modelname "armymedic/helmet.mdl"

// Directory of .SMD

// Directory of .BMP
$cdmaterials "models\armymedic\Head"
$cliptotextures

$attachment "head" "Head" 0.1438 3.6461 -6.1102 rotate 9 0 0

$staticprop
$scale 1.0
$surfaceprop "hoe_helmet_metal" // I want this to float in water

$bodygroup studio
{
studio "ref_helmet.smd"
}

// sequences: all sequences are in $cd
$sequence idle "seq_idle" loop fps 1

// auto-create a collisionmodel
$collisionmodel "ref_helmet.smd"
{
    $mass		0.5
}

$keyvalues
{
    prop_data
    {
        base		Metal.Small
    }
}

