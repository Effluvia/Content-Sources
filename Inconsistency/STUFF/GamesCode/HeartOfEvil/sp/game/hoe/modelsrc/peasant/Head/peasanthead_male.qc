// Output .MDL
$modelname "peasanthead_male.mdl"

// Directory of .SMD

// Directory of .BMP
$cdmaterials "models\peasant\Head"
$cliptotextures

$surfaceprop "bloodyflesh"

$scale 1.0

// bodygroups: use relative path from $cd
$bodygroup head
{
studio "SCI3_Template_Biped1(NerdHead).smd"
studio "SCI3_Template_Biped1(LutherHead).smd"
studio "SCI3_Template_Biped1(SlickHead).smd"
}

// sequences: all sequences are in $cd
$sequence idle "seq_idle" loop fps 1

// auto-create a collisionmodel
$collisionmodel "SCI3_Template_Biped1(LutherHead).smd"
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
