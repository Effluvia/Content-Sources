$modelname "them\them_head.mdl"
$cdmaterials "models\them"
$cliptotextures

$surfaceprop "alienflesh"

$scale 1.0

$eyeposition 0.000000 0.000000 8.000000

// 2 bone controllers
$controller 0 "Bip01 Head" XR -30 30
//$controller mouth "Bone05" ZR 0 15

// 1 hit box
$hbox 1 "Bip01 Head" -1.0000 -1.5000 -3.000000  9.0000 7.000000 3.000000

$bodygroup head
{
  studio "ref_head.smd"
}

$sequence idle "ref_head" loop fps 1 activity ACT_IDLE 1

// mouth stuff taken from HL:S
$poseparameter mouth 0.00 90.00
$animation pose_subtract "mouth_close" fps 30.000000
$animation mouth_close "mouth_close" fps 30.000000 subtract pose_subtract 0
$animation mouth_open "mouth_open" fps 30.000000 subtract pose_subtract 0
$sequence barney_mouth "mouth_close" fps 30.00 {
  blendwidth 2
  blend mouth 0.000000 90.000000
  delta
  autoplay
 mouth_open
}

