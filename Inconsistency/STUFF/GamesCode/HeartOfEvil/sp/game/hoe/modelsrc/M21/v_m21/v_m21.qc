$modelname "M21\v_m21\v_m21.mdl"
$cdmaterials "models\M21\v_m21"
$cliptotextures

$scale 1.0

// 1 attachments
$attachment "muzzle" "Bone63" 0.00 0.00 0.00 rotate 0 -90 90

// 0 bone controllers

// 8 hit boxes
$hbox 0 "Bone02" -7.448997 0.000000 -68.967392  10.037575 20.226458 0.000000
$hbox 0 "Bone06" -1.460000 -10.813042 -1.470000  1.950000 0.820000 2.285020
$hbox 0 "Bone08" -0.660000 -0.920000 -0.960000  0.670000 0.150000 1.190000
$hbox 0 "Bone09" -0.580000 -3.238840 -1.210000  0.720000 0.000000 2.400000
$hbox 0 "Bone05" -1.950000 -10.812996 -1.470000  1.460000 0.820000 2.285002
$hbox 0 "Bone07" -0.670000 -0.940000 -0.970000  0.660000 0.150000 1.200000
$hbox 0 "Bone34" -0.690000 -3.239000 -1.210000  0.580000 0.000000 2.310000
$hbox 0 "Bone63" -1.600000 0.000000 -6.220000  0.960000 43.590000 3.160000

$bodygroup studio
{
studio "ref_m21.smd"
}

// 5 sequences
$sequence idle "seq_idle_2010" loop fps 30 {
    activity ACT_VM_IDLE 1
}
$sequence fire1 "seq_fire1" fps 30 {
    snap
//    { event 5001 3 "11" }
    { event AE_MUZZLEFLASH 0 "PISTOL MUZZLE" }
    activity ACT_VM_PRIMARYATTACK 1
}
$sequence reload "seq_reload" fps 45 {
    { event AE_CL_PLAYSOUND 23 "Weapon_M21.ClipOut" }
    { event AE_CL_PLAYSOUND 99 "Weapon_M21.ClipIn" }
    activity ACT_VM_RELOAD 1
}
$sequence deploy "seq_deploy" fps 30 {
    snap
    activity ACT_VM_DRAW 1
}
$sequence holster "seq_holster" fps 30 {
    activity ACT_VM_HOLSTER 1
}

// Set this animation at the start of zooming in or out.
// The movement comes from interpolating the viewmodel origin.
$sequence zooming "seq_zooming" fps 1 {
    activity ACT_SPECIAL_ATTACK1 1
}

// $sequence zoomin "seq_zoom_in" fps 25 {
//     snap
//     fadeout 0
//     activity ACT_SPECIAL_ATTACK1 1
// }
// $sequence zoomout "seq_zoom_out" fps 25 {
//     snap
//     fadeout 0
//     activity ACT_SPECIAL_ATTACK2 1
// }


$sequence ironsight_attack "seq_fire1" fps 120 {
    snap
//    { event 5001 3 "11" }
//    { event AE_MUZZLEFLASH 0 "PISTOL MUZZLE" }
    activity ACT_VM_PRIMARYATTACK_IRONSIGHT 1
}
$sequence ironsight_idle "seq_idle" frame 0 0 loop fps 1 {
    activity ACT_VM_IDLE_IRONSIGHT 1
}

