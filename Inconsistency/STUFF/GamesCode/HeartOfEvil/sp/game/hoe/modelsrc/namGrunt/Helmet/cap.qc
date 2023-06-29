// Output .MDL
$modelname "namGrunt/helmet_cap.mdl"

// Directory of .SMD

// Directory of .BMP
$cdmaterials "models\namGrunt\Head"
$cliptotextures

$attachment "head" "Head" 0.1812 4.5553 -5.3567 rotate 9 0 0

$staticprop
$scale 1.0
$surfaceprop "hoe_helmet_cloth" // I want this to float in water

$bodygroup studio
{
studio "ref_cap.smd"
}

// sequences: all sequences are in $cd
$sequence idle "seq_idle" loop fps 1

// auto-create a collisionmodel
$collisionmodel "ref_cap.smd"
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

