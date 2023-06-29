$modelname "colt1911a1\v_colt1911a1\v_colt1911A1.mdl"
$cdmaterials "models\colt1911a1\v_colt1911a1"
$cliptotextures

$scale 1.0

// 1 attachments
$attachment "muzzle" "gun" -0.20 9.00 -1.50 rotate 0 90 -90
$attachment "brass" "gun" -0.50 2.30 -1.80 rotate 0 -180 -90

$bodygroup studio
{
studio "ref_v_colt1911A1.smd"
}

// 8 sequences
$sequence idle1 "seq_idle1" fps 20 {
    activity ACT_VM_IDLE 1
}
$sequence idleempty "seq_idleempty" fps 20 {
    activity ACT_VM_IDLE_EMPTY 1
}
$sequence reload "seq_reload" fps 30 {
    { event 5004 16 "Weapon_Colt1911A1.Release" }
    { event 5004 35 "Weapon_Colt1911A1.Insert" }
    activity ACT_VM_RELOAD 1
}
$sequence reloadempty "seq_reloadempty" fps 30 {
    { event 5004 16 "Weapon_Colt1911A1.Release" }
    { event 5004 35 "Weapon_Colt1911A1.Insert" }
    { event 5004 43 "Weapon_Colt1911A1.Slide" }
    activity ACT_VM_RELOAD_EMPTY 1
}
$sequence deploy "seq_deploy" fps 30 {
    snap
    activity ACT_VM_DRAW 1
}
$sequence deployempty "seq_deployempty" fps 30 {
    activity ACT_VM_DRAW_EMPTY 1
}
$sequence holster "seq_holster" fps 30 {
    activity ACT_VM_HOLSTER 1
}
$sequence holsterempty "seq_holsterempty" fps 30 {
    activity ACT_COLT1911A1_HOLSTER_EMPTY 1
}
$sequence fire "seq_fire" fps 40 {
    snap
    { event AE_MUZZLEFLASH 0 "PISTOL MUZZLE" } 
    { event 6001 3 "0" }
    activity ACT_VM_PRIMARYATTACK 1
}
$sequence firelast "seq_firelast" fps 40 {
    snap
    { event AE_MUZZLEFLASH 0 "PISTOL MUZZLE" } 
    { event 6001 3 "0" }
    activity ACT_COLT1911A1_PRIMARYATTACK_FIRELAST 1
}

$animation a_whip_haulback "seq_pistolwhip1" frames 0 8
$sequence pistolwhip_haulback "a_whip_haulback" fps 30 {
    activity ACT_VM_HAULBACK 1
    { event 3900 8 } // EVENT_WEAPON_SEQUENCE_FINISHED
    fadeout 0
}

$animation a_whip_swing "seq_pistolwhip1" frames 8 20
$sequence pistolwhip_swing "a_whip_swing" fps 30 {
    activity ACT_VM_SWINGHARD 1
}

////////////////////////////////////////
$sequence ironsight_attack "seq_fire_ironsight" fps 40 {
    snap
    { event AE_MUZZLEFLASH 0 "PISTOL MUZZLE" }
    { event 6001 2 "0" }
    activity ACT_VM_PRIMARYATTACK_IRONSIGHT 1
}
$sequence ironsight_idle "seq_idle1_ironsight" fps 1 {
    activity ACT_VM_IDLE_IRONSIGHT 1
}

