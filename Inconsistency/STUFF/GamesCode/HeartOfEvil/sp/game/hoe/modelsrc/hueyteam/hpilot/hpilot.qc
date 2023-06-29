$modelname "hpilot.mdl"
$cdmaterials "models\hueyteam\hpilot"
$cliptotextures

$scale 1.0

$eyeposition 0.000000 0.000000 63.000000

// 3 attachments
//$attachment 0 "Bip01 R Hand" 13.000000 5.500000 8.500000 
//$attachment 1 "Bip01 Head" 0.000000 0.000000 0.000000 
//$attachment 2 "Bip01 Neck" 0.500000 1.000000 0.000000 

// 1 bone controllers
//$controller 0 "Bip01 Head" XR -30 30

// These are needed for head-turning
$attachment "eyes" "Bip01 Head" 0.00 -0.00 -0.00 rotate -0.00 81.00 90.00
$attachment "forward" "Bip01 Spine" 0.00 -0.00 0.00 rotate -0.00 81.00 90.00

// 20 hit boxes
$hbox 3 "Bip01 Pelvis" -4.690000 -6.440000 -6.750000  5.000000 5.560000 6.750000
$hbox 6 "Bip01 L Leg" 2.660000 -3.690000 -3.090000  18.160000 4.880000 3.310000
$hbox 6 "Bip01 L Leg1" 0.380000 -3.970000 -2.840000  17.600000 4.000000 2.940000
$hbox 6 "Bip01 L Foot" -0.590000 -2.340000 -2.630000  3.790000 8.000000 2.190000
$hbox 7 "Bip01 R Leg" 2.470000 -3.690000 -3.160000  18.129999 4.880000 3.380000
$hbox 7 "Bip01 R Leg1" 0.310000 -3.970000 -2.840000  17.600000 3.940000 2.970000
$hbox 7 "Bip01 R Foot" -0.560000 -2.340000 -2.190000  3.810000 8.000000 2.660000
$hbox 3 "Bip01 Spine1" -1.250000 -8.500000 -5.500000  5.830000 5.500000 5.500000
$hbox 2 "Bip01 Spine2" -0.060000 -8.530000 -7.000000  7.000000 7.000000 7.000000
$hbox 2 "Bip01 Spine3" -2.250000 -4.000000 -6.310000  5.500000 5.090000 6.310000
$hbox 1 "Bip01 Neck" -3.110000 -1.500000 -3.000000  2.050000 3.500000 3.000000
$hbox 1 "Bip01 Head" -1.000000 -1.500000 -3.000000  9.000000 7.000000 3.000000
$hbox 4 "Bip01 L Arm" 0.940000 -4.880000 -4.130000  5.440000 2.280000 3.500000
$hbox 4 "Bip01 L Arm1" -2.160000 -4.340000 -2.560000  11.560000 1.410000 2.380000
$hbox 4 "Bip01 L Arm2" 0.590000 -1.810000 -2.190000  12.750000 2.840000 2.410000
$hbox 4 "Bip01 L Hand" 3.000000 -1.000000 -2.000000  6.000000 1.500000 3.500000
$hbox 5 "Bip01 R Arm" 1.380000 -4.840000 -3.560000  5.690000 2.310000 4.310000
$hbox 5 "Bip01 R Arm1" -1.880000 -4.470000 -2.160000  11.810000 1.280000 2.810000
$hbox 5 "Bip01 R Arm2" 0.440000 -1.970000 -2.340000  12.590000 2.720000 2.220000
$hbox 5 "Bip01 R Hand" 3.000000 -1.000000 -2.000000  6.000000 1.500000 3.500000

$bodygroup body
{
studio "hpilot.smd"
}

// 1 sequences
$sequence idle "idle" loop fps 20 ACT_IDLE 1 
$sequence sit_in_huey "seq_sit_in_huey" loop fps 20 

// head movement from HL:S
$poseparameter head_yaw -90.00 90.00
$animation neck_subtract "neck_center" fps 30
$animation neck_right "neck_right" fps 30.000000 subtract neck_subtract 0
$animation neck_center "neck_center" fps 30.000000 subtract neck_subtract 0
$animation neck_left "neck_left" fps 30.000000 subtract neck_subtract 0
$sequence barney_neckturn "neck_right" fps 30.00 {
  blendwidth 3
  blend head_yaw -90.000000 90.000000
  delta
  autoplay
 neck_center neck_left
}

