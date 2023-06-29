// Output .MDL
$modelname "charlie/helmet_VC1.mdl"

// Directory of .SMD

// Directory of .BMP
$cdmaterials "models\charlie"
$cliptotextures

$attachment "head" "Head" -0.1038 2.8621 -5.7643 rotate 9 0 0

$staticprop
$scale 1.0
$surfaceprop "hoe_helmet_cloth" // I want this to float in water

$bodygroup studio
{
studio "ref_VC1.smd"
}

// sequences: all sequences are in $cd
$sequence idle "seq_idle" loop fps 1

// auto-create a collisionmodel
$collisionmodel "ref_VC1.smd"
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

