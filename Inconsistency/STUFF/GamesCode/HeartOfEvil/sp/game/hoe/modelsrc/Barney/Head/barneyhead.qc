// Output .MDL
$modelname "barneyhead.mdl"

// Directory of .SMD

// Directory of .BMP
$cdmaterials "models\Barney\Head"
$cliptotextures

$surfaceprop "bloodyflesh"
$staticprop
$scale 1.0

// bodygroups: use relative path from $cd
$bodygroup head
{
studio "ref_Head.smd"
}

// sequences: all sequences are in $cd
$sequence idle "seq_idle" loop fps 1

// auto-create a collisionmodel
$collisionmodel "ref_Head.smd"
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

