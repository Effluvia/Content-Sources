$modelname "M60\v_m60\v_m60.mdl"
$cdmaterials "models\M60\v_m60"
$cliptotextures

$scale 1.0

// 1 attachments
$attachment "MUZZLE" "gun" -0.50 33.50 -1.00 rotate 0 90 -90
$attachment "BRASS" "gun" -1.50 1.00 -1.60 rotate 0 -180 -90

$bodygroup studio
{
studio "ref_v_m60.smd"
}

$bodygroup bullets
{
blank
studio "ref_bullets0.smd"
studio "ref_bullets1.smd"
studio "ref_bullets2.smd"
studio "ref_bullets3.smd"
studio "ref_bullets4.smd"
studio "ref_bullets5.smd"
studio "ref_bullets6.smd"
studio "ref_bullets7.smd"
studio "ref_bullets8.smd"
studio "ref_bullets9.smd"
studio "ref_bullets10.smd"
studio "ref_bullets11.smd"
studio "ref_bullets12.smd"
studio "ref_bullets13.smd"
studio "ref_bullets14.smd"
studio "ref_bullets15.smd"
studio "ref_bullets16.smd"
studio "ref_bullets17.smd"
}

// 8 sequences
$sequence idle1 "seq_idle1" fps 20 {
    activity ACT_VM_IDLE 1
}
$sequence longidle "seq_longidle" fps 20 {
    activity ACT_VM_IDLE 4
}
$sequence reload1 "seq_reload1" fps 30 {
    { event AE_CL_PLAYSOUND 24 "Weapon_M60.ReloadOpen" }
    { event AE_CL_PLAYSOUND 38 "Weapon_M60.ReloadFull" }
    activity ACT_VM_RELOAD 1
}
$sequence reload1_empty "seq_reload1_empty" fps 30 {
    { event AE_CL_PLAYSOUND 24 "Weapon_M60.ReloadOpen" }
    { event AE_CL_PLAYSOUND 60 "Weapon_M60.ReloadEmpty" }
    activity ACT_VM_RELOAD_EMPTY 1
}
$sequence reload2 "seq_reload2" fps 30 {
    { event AE_CL_PLAYSOUND 39 "Weapon_M60.ReloadClose" }
    activity ACT_VM_M60_RELOAD_FINISH 1
}
$sequence deploy "seq_deploy_2010" fps 20 {
    snap
    activity ACT_VM_DRAW 1
}
$sequence shoot "seq_fire1" fps 60 {
    snap
//    { event 5001 0 "20" }
    { event AE_MUZZLEFLASH 0 "SMG1 MUZZLE" }
    { event 9011 0 "RifleShellEject" }
    activity ACT_VM_PRIMARYATTACK 1
}
$sequence holster "seq_holster" fps 20 {
    activity ACT_VM_HOLSTER 1
}

$sequence ironsight_attack "seq_fire1_ironsight" fps 40 {
    snap
    { event AE_MUZZLEFLASH 0 "SMG1 MUZZLE" }
    { event 9011 0 "RifleShellEject" }
    activity ACT_VM_PRIMARYATTACK_IRONSIGHT 1
}
$sequence ironsight_idle "seq_longidle_ironsight" fps 1 {
    activity ACT_VM_IDLE_IRONSIGHT 1
}

