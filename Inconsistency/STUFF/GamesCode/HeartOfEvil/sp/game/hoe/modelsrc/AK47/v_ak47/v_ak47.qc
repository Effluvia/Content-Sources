$modelname "AK47\v_ak47\v_ak47.mdl"
$cdmaterials "models\AK47\v_ak47"
$cliptotextures

$scale 1.0

// 1 attachments
$attachment "MUZZLE" "ak47" -0.40 20.00 -0.30 rotate -0 90 -90
$attachment "BRASS" "ak47" -0.40 3.00 -1.70 rotate 0 -180 -90

$bodygroup studio
{
studio "ref_v_ak47.smd"
}

// 6 sequences
$sequence longidle "seq_longidle" fps 20 {
    activity ACT_VM_IDLE 1
}
$sequence idle1 "seq_idle1" fps 20 {
    activity ACT_VM_IDLE 1
}
$sequence reload "seq_reload" fps 40 {
    { event AE_CL_PLAYSOUND 21 "Weapon_AK47.ClipRelease" }
    { event AE_CL_PLAYSOUND 62 "Weapon_AK47.ClipInsert" }
    activity ACT_VM_RELOAD 1
}
$sequence reload_empty "seq_reload_empty" fps 40 {
    { event AE_CL_PLAYSOUND 21 "Weapon_AK47.ClipRelease" }
    { event AE_CL_PLAYSOUND 62 "Weapon_AK47.ClipInsert" }
    { event AE_CL_PLAYSOUND 92 "Weapon_AK47.Slide" }
    activity ACT_VM_RELOAD_EMPTY 1
}
$sequence deploy "seq_deploy" fps 30 {
    snap
    activity ACT_VM_DRAW 1
}
$sequence holster "seq_holster" fps 30 {
    activity ACT_VM_HOLSTER 1
}
$sequence fire "seq_fire1" fps 40 {
    snap
    { event AE_MUZZLEFLASH 0 "SMG1 MUZZLE" }
    { event 9011 0 "RifleShellEject" }
    activity ACT_VM_PRIMARYATTACK 1
}

$sequence ironsight_attack "seq_fire1_ironsight" fps 40 {
    snap
    { event AE_MUZZLEFLASH 0 "SMG1 MUZZLE" } 
    { event 9011 0 "RifleShellEject" }
    activity ACT_VM_PRIMARYATTACK_IRONSIGHT 1
}
$sequence ironsight_idle "seq_longidle_ironsight" fps 20 {
    activity ACT_VM_IDLE_IRONSIGHT 1
}

