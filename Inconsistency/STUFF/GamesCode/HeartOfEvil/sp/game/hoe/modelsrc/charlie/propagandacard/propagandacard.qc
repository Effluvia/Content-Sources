// Output .MDL
$modelname "propagandacard.mdl"

// Directory of .SMD

// Directory of .BMP
$cdmaterials "models\charlie\propagandacard"
$cliptotextures

// Write textures into a ???T.mdl
//$externaltextures

$staticprop
$surfaceprop "paper"

$keyvalues
{
    "prop_data"
    {
        "base" "Cardboard.Small"
        "AIWalkable" "0"
        "physicsmode" "3"
    }
    "physgun_interactions"
    {
        "preferred_carryangles" "90 180 180"
        "carry_distance_offset" "-32"
    }
}

$scale 2.0

// bodygroups: use relative path from $cd
$bodygroup body
{
studio "ref_propagandacard.smd"
}

// sequences: all sequences are in $cd
$sequence idle "seq_idle" loop fps 30 ACT_IDLE 1

$scale 1.0

// had to scale this up a bit so studiomdl wouldn't discard it
$collisionmodel "collision_model.smd"
{
    $mass 0.1
    $damping 3.0
    $rotdamping 3.0
    $drag 1.5
}

