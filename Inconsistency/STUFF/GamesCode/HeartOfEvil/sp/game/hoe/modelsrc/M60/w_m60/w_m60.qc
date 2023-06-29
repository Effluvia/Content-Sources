$modelname "M60\w_m60\w_m60.mdl"
$cdmaterials "models\M60\w_m60"
$cliptotextures

$scale 1.0

// 1 attachment
// The first attachment must be the muzzle of the weapon!!!
$attachment "muzzle" "Bip01 R Hand" 36.80 10.40 7.50 rotate -7 10 0
$attachment "brass" "Bip01 R Hand" 5.50 3.50 4.50 rotate 0 -80 10

// 0 bone controllers

$bodygroup studio
{
studio "ref_w_m60_notripod.smd"
}

$bodygroup tripod
{
studio "ref_tripod_down.smd"
studio "ref_tripod_up.smd"
}

// 2 sequences
$sequence idle "seq_idle1" fps 1 {
    // see CBaseCombatWeapon::SetActivity() for why ACT_VM_IDLE is used by non-viewmodel
    activity ACT_VM_IDLE 1
} 
$sequence attack1 "seq_idle1" fps 30 {
    { event 3002 0 }
    { event AE_NPC_MUZZLEFLASH 0 "SMG1 MUZZLE" }
    { event 9011 0 "RifleShellEject" }
    activity ACT_RANGE_ATTACK_AR2 1
}

// auto-create a collisionmodel
$collisionmodel "ref_w_m60.smd"
{
    $mass 10.0 // HL1 value
}

