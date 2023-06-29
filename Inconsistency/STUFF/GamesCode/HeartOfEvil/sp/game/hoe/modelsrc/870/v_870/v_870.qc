$modelname "v_870.mdl"
$cdmaterials "models\870\v_870"
$cliptotextures

$scale 1.0

// 2 attachments
$attachment "MUZZLE" "gun" 0.80 1.20 28.00 rotate -90 -40 0
$attachment "BRASS" "gun" 0.50 1.50 6.00 rotate 0 140 180

$bodygroup studio
{
studio "ref_v_870.smd"
}

$bodygroup shell
{
studio "ref_buckshot.smd"
studio "ref_elephantshot.smd"
blank
}

// 8 sequences
$sequence idle1 "seq_idle1" fps 20 {
    activity ACT_VM_IDLE 1
}
$sequence longidle "seq_longidle" loop fps 20 {
    activity ACT_VM_IDLE 4
}
$sequence sfire1 "seq_sfire1" fps 40 {
    snap
    { event AE_MUZZLEFLASH 0 "SHOTGUN MUZZLE" }
    { event AE_CL_PLAYSOUND 25 "Weapon_870.Pump" }
    { event 6001 25 "3" }
    activity ACT_VM_PRIMARYATTACK 1
}
$sequence bigfire "seq_bigfire" fps 40 {
    snap
    { event AE_MUZZLEFLASH 0 "SHOTGUN MUZZLE" }
    { event AE_CL_PLAYSOUND 40 "Weapon_870.Pump" }
    { event 6001 40 "4" }
    activity ACT_VM_SECONDARYATTACK 1
}

$sequence start_reload "seq_start_reload" fps 20 {
    activity ACT_SHOTGUN_RELOAD_START 1
}
$sequence reload "seq_reload" fps 20 {
    { event 3015 8 }
    activity ACT_VM_RELOAD 1
}
$sequence finish_reload "seq_pump" fps 20 {
    { event AE_CL_PLAYSOUND 7 "Weapon_870.Pump" }
    activity ACT_SHOTGUN_RELOAD_FINISH 1
}

$sequence pump "seq_pump" fps 20 {
    { event AE_CL_PLAYSOUND 7 "Weapon_870.Pump" }
    activity ACT_SHOTGUN_PUMP 1
}
$sequence draw "seq_draw" fps 24 {
    snap
    activity ACT_VM_DRAW 1
}
$sequence holster "seq_holster" fps 30 {
    activity ACT_VM_HOLSTER 1
}

$sequence ironsight_idle "seq_longidle_ironsight" fps 20 {
    activity ACT_VM_IDLE_IRONSIGHT 1
}

