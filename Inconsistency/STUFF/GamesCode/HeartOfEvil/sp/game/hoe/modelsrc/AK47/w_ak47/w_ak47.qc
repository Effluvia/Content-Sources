$modelname "AK47\w_ak47\w_ak47.mdl"
$cdmaterials "models\AK47\w_ak47"
$cliptotextures

$scale 1.0

// 1 attachments
$attachment "muzzle" "Bip01 R Hand" 26.50 7.50 7.20 rotate -10 10 90
$attachment "brass" "Bip01 R Hand" 8.00 4.00 5.00 rotate 0 -80 10

// 0 bone controllers

$bodygroup studio
{
studio "ref_w_ak47.smd"
}

// 2 sequences
$sequence idle "seq_idle1" fps 1 {
    // see CBaseCombatWeapon::SetActivity() for why ACT_VM_IDLE is used by non-viewmodel
    activity ACT_VM_IDLE 1
} 
$sequence attack1 "seq_idle1" fps 20 {
    { event 3002 0 }
    { event AE_NPC_MUZZLEFLASH 0 "SMG1 MUZZLE" }
    { event 9011 0 "RifleShellEject" }
    activity ACT_RANGE_ATTACK_SMG1 1
}

// auto-create a collisionmodel
$collisionmodel "ref_w_ak47.smd"
{
    $mass 11.0
}
