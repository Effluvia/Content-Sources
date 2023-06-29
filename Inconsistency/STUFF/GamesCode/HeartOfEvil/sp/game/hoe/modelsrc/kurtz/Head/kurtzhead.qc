// Output .MDL
$modelname "kurtzhead.mdl"

// Directory of .SMD

// Directory of .BMP
$cdmaterials "models\kurtz\Head"
$cliptotextures

$attachment "head" "static_prop" -0.635 3.8816 -2.3365 rotate 0 0 0

$surfaceprop "bloodyflesh"
$staticprop
$scale 1.0

// bodygroups: use relative path from $cd
$bodygroup head
{
studio "ref_Bald.smd"
studio "ref_Beret.smd"
}

// sequences: all sequences are in $cd
$sequence idle "seq_idle" loop fps 1

// auto-create a collisionmodel
$collisionmodel "ref_Bald.smd"
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

