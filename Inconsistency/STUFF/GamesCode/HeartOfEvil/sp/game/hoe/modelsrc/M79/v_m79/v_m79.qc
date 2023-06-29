$modelname "M79\v_m79\v_m79.mdl"
$cdmaterials "models\M79\v_m79"
$cliptotextures

$scale 1.0

// 1 attachments
$attachment "MUZZLE" "gun" 0.00 17.00 0.00 rotate 0 90 -90

$bodygroup studio
{
studio "ref_v_m79.smd"
}

// 8 sequences
$sequence idle1 "seq_idle1" fps 20 {
    activity ACT_VM_IDLE 1
}
$sequence longidle "seq_longidle" fps 20 {
    activity ACT_VM_IDLE 4
}
$sequence reload "seq_reload" fps 30 {
    { event AE_CL_PLAYSOUND 6 "Weapon_M79.Open" }
    { event AE_CL_PLAYSOUND 25 "Weapon_M79.Eject" }
    { event AE_CL_PLAYSOUND 35 "Weapon_M79.ShellDrop" }
    { event AE_CL_PLAYSOUND 53 "Weapon_M79.Insert" }
    { event AE_CL_PLAYSOUND 69 "Weapon_M79.Close" }
    activity ACT_VM_RELOAD 1
}
$sequence deploy "seq_deploy" fps 30 {
    snap
    activity ACT_VM_DRAW 1
}
$sequence holster "seq_holster" fps 30 {
    activity ACT_VM_HOLSTER 1
}
$sequence fire "seq_fire" fps 30 {
    snap
    activity ACT_VM_PRIMARYATTACK 1
}

$sequence ironsight_idle "seq_longidle_ironsight" fps 20 {
    activity ACT_VM_IDLE_IRONSIGHT 1
}

