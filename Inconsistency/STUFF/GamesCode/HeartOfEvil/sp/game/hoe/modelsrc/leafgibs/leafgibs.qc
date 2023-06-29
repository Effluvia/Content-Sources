// Output .MDL
$modelname "leafgibs\leafgibs.mdl"

// Directory of .SMD

// Directory of .BMP
$cdmaterials "models\leafgibs"
$cliptotextures

// Write textures into a ???T.mdl
//$externaltextures

$scale 1.0
$staticprop

// bodygroups: use relative path from $cd
$bodygroup body
{
studio "leaf1.smd"
studio "leaf2.smd"
studio "leaf3.smd"
studio "leaf4.smd"
studio "leaf5.smd"
}

// sequences: all sequences are in $cd
$sequence idle "idle" loop fps 30 ACT_IDLE 1

//$keyvalues
//{
//    "prop_data"
//    {
//        "base" "Cardboard.Small"
//        "AIWalkable" "0"
//        "physicsmode" "3"
//    }
//}

//// auto-create a collisionmodel
//$collisionmodel "leaf1.smd"
//{
//    $mass 0.01
//    $damping 3.0
//    $rotdamping 3.0
//    $drag 1.5
//}

