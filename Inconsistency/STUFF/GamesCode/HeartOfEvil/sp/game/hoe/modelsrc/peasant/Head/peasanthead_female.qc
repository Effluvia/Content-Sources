// Output .MDL
$modelname "peasanthead_female.mdl"

// Directory of .SMD

// Directory of .BMP
$cdmaterials "models\peasant\Head"
$cliptotextures

$surfaceprop "bloodyflesh"

$scale 1.0

// bodygroups: use relative path from $cd
$bodygroup head
{
studio "SCI3_Template_Biped1(Female_Head_1).smd"
studio "SCI3_Template_Biped1(Female_Head_2).smd"
studio "SCI3_Template_Biped1(Female_Head_3).smd"
}

// sequences: all sequences are in $cd
$sequence idle "seq_idle" loop fps 1

// auto-create a collisionmodel
$collisionmodel "SCI3_Template_Biped1(Female_Head_1).smd"
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

