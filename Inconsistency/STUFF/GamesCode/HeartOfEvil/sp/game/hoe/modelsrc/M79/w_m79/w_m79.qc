$modelname "M79\w_m79\w_m79.mdl"
$cdmaterials "models\M79\w_m79"
$cliptotextures

$scale 1.0

// 1 attachment
$attachment "muzzle" "Bip01 R Hand" 24.50 7.20 6.20 rotate -9 10 0

// 0 bone controllers

$bodygroup studio
{
studio "ref_w_m79.smd"
}

// 1 sequences
$sequence idle "seq_idle1" fps 1 {
    // see CBaseCombatWeapon::SetActivity() for why ACT_VM_IDLE is used by non-viewmodel
    activity ACT_VM_IDLE 1
} 
$sequence attack1 "seq_idle1" fps 1 {
    { event 3012 0 }
    activity ACT_RANGE_ATTACK_AR2 1
}

// auto-create a collisionmodel
$collisionmodel "ref_w_m79.smd"
{
    $mass 8.0 // HL1 value
}

