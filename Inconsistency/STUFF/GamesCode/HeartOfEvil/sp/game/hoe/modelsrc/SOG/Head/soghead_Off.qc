// Output .MDL
$modelname "sog/head_off.mdl"

// Directory of .SMD

// Directory of .BMP
$cdmaterials "models\SOG"
$cliptotextures

$attachment "head" "static_prop" 0.4201 3.4162 -2.9356 rotate 0 0 0

$surfaceprop "bloodyflesh"
$staticprop
$autocenter
$scale 1.0

// bodygroups: use relative path from $cd
$bodygroup head
{
studio "ref_Blond_Hair_Off.smd"
studio "ref_Brown_Hair_Off.smd"
studio "ref_Grey_Hair_Off.smd"
studio "ref_Stealth_Off.smd"
studio "ref_Zombie_Off.smd"
}

// sequences: all sequences are in $cd
$sequence idle "seq_idle" loop fps 1

// auto-create a collisionmodel
$collisionmodel "ref_Blond_Hair_Off.smd"
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

