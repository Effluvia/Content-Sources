$modelname "v_squeak.mdl"
$cdmaterials "models\snark\v_squeak"
$cliptotextures

$scale 1.0

// 0 attachments

// 0 bone controllers

// 10 hit boxes
$hbox 0 "Bip01 L Arm2" 0.000000 -2.800000 -1.730000  11.680000 3.020000 4.000000
$hbox 0 "Bip01 L Hand" 0.000000 -0.870000 -1.940000  3.890000 1.030000 1.790000
$hbox 0 "Bip01 R Arm2" 0.000000 -2.800000 -4.000000  11.368423 3.020000 1.730000
$hbox 0 "Bip01 R Hand" 0.000000 -0.870000 -1.790000  3.890000 2.959115 2.020000
$hbox 0 "Bip01 R Finger0" 0.000000 -0.590000 -0.630000  2.570000 0.440000 0.630000
$hbox 0 "Bip02 Spine" -1.900000 -4.380000 -2.040000  6.010000 1.850000 2.230000
$hbox 0 "Bip02 Neck" -0.630000 -0.230000 -1.380000  0.954035 1.681428 1.570000
$hbox 0 "Bip02 R Arm2" -0.190000 -0.510000 -0.450000  3.220000 0.520000 0.640000
$hbox 0 "Bip02 Neck1" 0.000000 -0.840000 -1.260000  1.410000 1.410000 1.450000
$hbox 0 "Bip02 Head" 0.000000 -0.720000 -0.460000  1.430000 0.630000 0.680000

$bodygroup body
{
studio "SQUEAK_Template_Biped(Hand)1.smd"
}

$bodygroup body
{
studio "SQUEAK_Template_Biped(Weapon)1.smd"
}

// 6 sequences
$sequence idle1 "seq_idle1" loop fps 16 {
    activity ACT_VM_IDLE 8
}
$sequence fidgetfit "seq_fidgetfit" fps 16 {
    activity ACT_VM_IDLE 1
}
$sequence fidgetnip "seq_fidgetnip" fps 16 {
    activity ACT_VM_IDLE 1
}
$sequence down "seq_down" fps 16 {
    activity ACT_VM_HOLSTER 1
}
$sequence up "seq_up" fps 30 {
    snap
    activity ACT_VM_DRAW 1
}
$sequence throw "seq_throw" fps 24 {
    snap
    activity ACT_VM_THROW 1
}

