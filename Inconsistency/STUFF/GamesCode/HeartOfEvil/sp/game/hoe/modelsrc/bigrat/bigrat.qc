$modelname "bigrat\bigrat.mdl"
$cdmaterials "models\bigrat"
$cliptotextures

$scale 1.0

$eyeposition 0.000000 0.000000 3.000000

// 0 attachments

// 0 bone controllers

// 7 hit boxes
$hbox 0 "Bip01 Pelvis" -3.210000 -2.560000 -2.270000  2.880000 1.040000 2.250000
$hbox 0 "Bip01 Spine" 0.000000 -1.110000 -2.220000  2.116361 1.310000 2.200000
$hbox 0 "Bip01 Spine3" 0.000000 -1.570000 -1.630000  2.010000 2.180000 1.620000
$hbox 0 "Bip01 Neck" -0.130000 -1.870000 -1.540000  1.710000 1.850000 1.520000
$hbox 0 "Bip01 Head" 0.000000 -1.920000 -1.440000  3.360000 1.620000 1.430000
$hbox 0 "Bip01 L Arm1" 0.000000 -1.010000 -0.770000  1.520000 0.660000 0.770000
$hbox 0 "Bip01 R Arm1" 0.000000 -1.010000 -0.790000  1.540000 0.640000 0.730000

$bodygroup studio
{
studio "Rat_Template_Biped1.smd"
}

// 8 sequences
$sequence walk "walk" LX loop fps 15 ACT_WALK 1
$sequence idle4 "idle4" loop fps 15 ACT_IDLE 4 
$sequence idle5 "idle5" loop fps 15 ACT_IDLE 1 
$sequence idle6 "idle6" loop fps 15 ACT_IDLE 1 
$sequence idle7 "idle7" loop fps 15 ACT_IDLE 20 
$sequence walk2 "walk2" LX loop fps 15 ACT_WALK 2 
$sequence jump "jump" loop fps 15 
$sequence eat "eat" loop fps 15 ACT_EAT 1 
$sequence die1 "die1" loop fps 15 ACT_DIESIMPLE 1 

