// Output .MDL
$modelname "charlie/helmet_Brown.mdl"

// Directory of .SMD

// Directory of .BMP
$cdmaterials "models\charlie"
$cliptotextures

$attachment "head" "Head" 0.0843 2.8505 -5.6825 rotate 9 0 0

$staticprop
$scale 1.0
$surfaceprop "hoe_helmet_cloth" // I want this to float in water

$bodygroup studio
{
studio "ref_Brown.smd"
}

// sequences: all sequences are in $cd
$sequence idle "seq_idle" loop fps 1

// auto-create a collisionmodel
$collisionmodel "ref_Brown.smd"
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

