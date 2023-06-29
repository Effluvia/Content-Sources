// Output .MDL
$modelname "mikeforcehead.mdl"

// Directory of .SMD

// Directory of .BMP
$cdmaterials "models\mikeforce\Head"
$cliptotextures

$surfaceprop "bloodyflesh"
$staticprop
$scale 1.0

// bodygroups: use relative path from $cd
$bodygroup head
{
studio "ref_MikeForce.smd"
studio "ref_RT.smd"
studio "ref_Boonie.smd"
studio "ref_LRRP.smd"
studio "ref_Seal.smd"
}

// sequences: all sequences are in $cd
$sequence idle "seq_idle" loop fps 1

// auto-create a collisionmodel
$collisionmodel "ref_MikeForce.smd"
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

