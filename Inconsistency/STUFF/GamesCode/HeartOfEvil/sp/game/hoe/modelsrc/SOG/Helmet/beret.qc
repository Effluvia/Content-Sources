// Output .MDL
$modelname "SOG/helmet_beret.mdl"

// Directory of .SMD

// Directory of .BMP
$cdmaterials "models\SOG"
$cliptotextures

$attachment "head" "Head" 0.6762 3.6293 -6.964 rotate 9 0 0

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

