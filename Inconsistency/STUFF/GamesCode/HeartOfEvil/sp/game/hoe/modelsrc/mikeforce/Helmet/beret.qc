// Output .MDL
$modelname "mikeforce/helmet_beret.mdl"

// Directory of .SMD

// Directory of .BMP
$cdmaterials "models\mikeforce"
$cliptotextures

$attachment "head" "Head" 0.4201 3.8551 -6.9163 rotate 9 0 0

$staticprop
$scale 1.0
$surfaceprop "hoe_helmet_cloth" // I want this to float in water

$bodygroup studio
{
studio "ref_beret.smd"
}

// sequences: all sequences are in $cd
$sequence idle "seq_idle" loop fps 1

// auto-create a collisionmodel
$collisionmodel "ref_beret.smd"
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

