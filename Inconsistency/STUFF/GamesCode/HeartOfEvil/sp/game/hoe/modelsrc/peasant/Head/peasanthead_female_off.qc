// Output .MDL
$modelname "peasant\head_female_off.mdl"

// Directory of .SMD

// Directory of .BMP
$cdmaterials "models\peasant"
$cliptotextures

$attachment "head" "static_prop" -0.2134 -0.8333 -3.0688 rotate 0 0 0

$surfaceprop "bloodyflesh"
$scale 1.0
$staticprop
$autocenter

// bodygroups: use relative path from $cd
$bodygroup head
{
studio "Female1_Off.smd"
studio "Female2_Off.smd"
studio "Female3_Off.smd"
}

// sequences: all sequences are in $cd
$sequence idle "seq_idle" loop fps 1

// auto-create a collisionmodel
$collisionmodel "Female2_Off.smd"
{
    $mass 10.0
}

$keyvalues
{
    prop_data
    {
        base	Flesh.Small
    }
    world_interactions
    {
        onworldimpact bloodsplat
    }
}

