// Output .MDL
$modelname "soghead.mdl"

// Directory of .SMD

// Directory of .BMP
$cdmaterials "models\SOG\Head"
$cliptotextures

$surfaceprop "bloodyflesh"
$staticprop
$scale 1.0

// bodygroups: use relative path from $cd
$bodygroup head
{
studio "ref_Blond_Hair.smd"
studio "ref_Brown_Hair.smd"
studio "ref_Grey_Hair.smd"
studio "ref_Stealth.smd"
studio "ref_Zombie.smd"
}

// sequences: all sequences are in $cd
$sequence idle "seq_idle" loop fps 1

// auto-create a collisionmodel
$collisionmodel "ref_Blond_Hair.smd"
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

