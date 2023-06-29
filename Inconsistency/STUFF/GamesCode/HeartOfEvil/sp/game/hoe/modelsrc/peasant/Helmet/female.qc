// Output .MDL
$modelname "peasant/hat_female.mdl"

// Directory of .SMD

// Directory of .BMP
$cdmaterials "models\peasant"
$cliptotextures

$attachment "head" "Head" -0.4614 1.6814 -7.2025 rotate 10 0 0

$staticprop
$scale 1.0
$surfaceprop "hoe_helmet_cloth" // I want this to float in water

$bodygroup studio
{
studio "ref_female.smd"
}

// sequences: all sequences are in $cd
$sequence idle "seq_idle" loop fps 1

// auto-create a collisionmodel
$collisionmodel "ref_female.smd"
{
    $mass 0.5
}

$keyvalues
{
    prop_data
    {
        base	Cloth.Small
    }
}

