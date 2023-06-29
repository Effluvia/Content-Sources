$modelname "v_letter.mdl"
$cdmaterials "models\grenade\v_grenade"
$cliptotextures

$scale 1.0

// 0 attachments
$attachment "controlpanel0_ll" "Dummy22" 12.00 -4.00 -14.00 rotate -27 150 79
$attachment "controlpanel0_ur" "Dummy22" 5.00 -5.00 10.00 rotate -27 150 79

// 0 bone controllers

// 6 hit boxes
//$hbox 0 "Bip01 R Arm1" 0.000000 -2.810000 -3.310000  12.350000 3.490000 2.620000
//$hbox 0 "Bip01 R Arm2" 0.000000 -2.610000 -3.140000  11.368420 3.210000 2.340000
//$hbox 0 "Bip01 R Hand" 0.000000 -0.870000 -1.790000  3.890000 1.030000 1.940000
//$hbox 0 "Dummy22" -1.010000 -1.220000 -2.130000  1.620000 1.920000 3.110000
//$hbox 0 "Bip01 L Arm2" 0.000000 -2.280000 -2.120000  11.680000 2.740000 2.380000
//$hbox 0 "Bip01 L Hand" 0.000000 -0.870000 -1.940000  3.890000 1.030000 1.790000

$bodygroup body
{
studio "ref_letter.smd"
}

$bodygroup body
{
studio "ref_hands.smd"
}

// 8 sequences
$sequence idle "seq_idle" loop fps 30 activity ACT_VM_IDLE 4
//$sequence fidget "seq_fidget" loop fps 30 activity ACT_VM_IDLE 1
//$sequence pinpull "seq_pinpull" fps 45  { event 3900 24 } activity ACT_VM_PULLBACK_HIGH 1
//$sequence throw1 "seq_throw1" fps 13 { event 3005 2 } activity ACT_VM_THROW 1
//$sequence throw2 "seq_throw2" fps 20 { event 3013 2 }
//$sequence throw3 "seq_throw3" fps 30 { event 3016 2 }
//$sequence holster "seq_holster" fps 30 activity ACT_VM_HOLSTER 1
//$sequence draw "seq_draw" fps 30 activity ACT_VM_DRAW 1

