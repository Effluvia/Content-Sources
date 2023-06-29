// Output .MDL
$modelname "mikeforce/helmet_beret_tiger.mdl"

// Directory of .SMD

// Directory of .BMP
$cdmaterials "models\mikeforce"
$cliptotextures

$attachment "head" "Head" 0.2359 3.9289 -6.8766 rotate 9 0 0

$staticprop
$scale 1.0
$surfaceprop "hoe_helmet_cloth" // I want this to float in water

$bodygroup studio
{
studio "ref_beret_tiger.smd"
}

// sequences: all sequences are in $cd
$sequence idle "seq_idle" loop fps 1

// auto-create a collisionmodel
$collisionmodel "ref_beret_tiger.smd"
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

