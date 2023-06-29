// Output .MDL
$modelname "charliehead.mdl"

// Directory of .SMD

// Directory of .BMP
$cdmaterials "models\charlie"
$cliptotextures

$surfaceprop "bloodyflesh"
$staticprop
$scale 1.0

// bodygroups: use relative path from $cd
$bodygroup head
{
studio "ref_NVA_Brown.smd"
studio "ref_NVA_Green.smd"
studio "ref_VC1.smd"
studio "ref_VC2.smd"
}

// sequences: all sequences are in $cd
$sequence idle "seq_idle" loop fps 1

// auto-create a collisionmodel
$collisionmodel "ref_VC2.smd"
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

