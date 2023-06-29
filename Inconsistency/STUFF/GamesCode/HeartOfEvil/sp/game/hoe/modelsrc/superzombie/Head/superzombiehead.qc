// Output .MDL
$modelname "superzombiehead.mdl"

// Directory of .SMD

// Directory of .BMP
$cdmaterials "models\superzombie\Head"
$cliptotextures

// Write textures into a ???T.mdl

$scale 1.0

// bodygroups: use relative path from $cd
$bodygroup head
{
studio "ref_Knifed.smd"
studio "ref_SwissCheese.smd"
}

// sequences: all sequences are in $cd
$sequence idle "seq_idle" loop fps 1

// auto-create a collisionmodel
$collisionmodel "ref_SwissCheese.smd"
{
    $mass 10.0
}

$surfaceprop "bloodyflesh"

