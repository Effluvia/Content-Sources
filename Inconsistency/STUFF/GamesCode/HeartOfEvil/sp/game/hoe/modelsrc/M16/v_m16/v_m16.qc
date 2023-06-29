$modelname "v_m16.mdl"
$cdmaterials "models\M16\v_m16"
$cliptotextures

$scale 1.0

// 1 attachments
$attachment "MUZZLE" "Gun" -2.00 -19.50 20.50 rotate -42 -100 90
$attachment "BRASS" "Gun" -0.60 -3.50 4.50 rotate 0 175 45

$bodygroup studio
{
studio "ref_m16.smd"
}

// 8 sequences
$sequence longidle "seq_longidle" fps 20 {
    activity ACT_VM_IDLE 1
}
$sequence idle1 "seq_idle1" fps 20 {
    activity ACT_VM_IDLE 1
}
$sequence reload "seq_reload" fps 40 {
    { event AE_CL_PLAYSOUND 25 "Weapon_M16.ClipRelease" }
    { event AE_CL_PLAYSOUND 55 "Weapon_M16.ClipInsert" }
    activity ACT_VM_RELOAD 1
}
$sequence reload_empty "seq_reload_empty" fps 40 {
    { event AE_CL_PLAYSOUND 25 "Weapon_M16.ClipRelease" }
    { event AE_CL_PLAYSOUND 55 "Weapon_M16.ClipInsert" }
    { event AE_CL_PLAYSOUND 96 "Weapon_M16.Slide" }
    activity ACT_VM_RELOAD_EMPTY 1
}
$sequence deploy "seq_deploy" fps 40 {
    snap
    activity ACT_VM_DRAW 1
}
$sequence holster "seq_holster" fps 40 {
    activity ACT_VM_HOLSTER 1
}
$sequence shoot "seq_shoot1" fps 120 {
    snap
    { event AE_MUZZLEFLASH 0 "SMG1 MUZZLE" }
    { event 9011 0 "RifleShellEject" }
    activity ACT_VM_PRIMARYATTACK 1
}

////////////////////////////////////////
$sequence ironsight_attack "seq_shoot1_ironsight" fps 40 {
    snap
    { event AE_MUZZLEFLASH 0 "SMG1 MUZZLE" }
    { event 9011 0 "RifleShellEject" }
    activity ACT_VM_PRIMARYATTACK_IRONSIGHT 1
}
$sequence ironsight_idle "seq_idle_ironsight" fps 1 {
    activity ACT_VM_IDLE_IRONSIGHT 1
}
$sequence ironsight_transition "seq_ironsight_transition" fps 1 {
    activity ACT_VM_IRONSIGHT_TRANSITION 1
}

