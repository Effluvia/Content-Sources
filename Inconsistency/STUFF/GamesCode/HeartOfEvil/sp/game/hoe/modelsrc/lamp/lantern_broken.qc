$modelname "lantern_broken.mdl"
$cdmaterials "models\lamp"
$cliptotextures

$surfaceprop "glass"

$scale 1.0
$staticprop

$attachment "light" "static_prop" 0.00 0.00 7.00 rotate 0 0 0
$attachment "rope" "static_prop" 0.00 0.00 22.00 rotate 0 0 0

$bodygroup studio
{
studio "ref_lantern_broken.smd"
}

$texturegroup skinfamilies
{
 {
   "generic039"
   "drkmtlt_bord11"
   "lab1_stair2b"
   "lantern_on"
 }
 {
   "generic039_off"
   "drkmtlt_bord11_off"
   "lab1_stair2b_off"
   "lantern_off"
 }
}

// 1 sequences
$sequence idle "seq_idle" fps 30 

// $keyvalues
// {
//     prop_data
//     {
//         base Wooden.Small
//     }
//     "physgun_interactions"
//     {
//         "preferred_carryangles" "0 180 0"
// //        "carry_distance_offset" "-32"
//     }
// }

// auto-create a collisionmodel
$collisionmodel "ref_lantern_broken.smd"
{
    $mass 4.0
}


