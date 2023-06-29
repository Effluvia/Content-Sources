$modelname "w_870.mdl"
$cdmaterials "models\870\w_870"
$cliptotextures

$scale 1.0

// 1 attachments
$attachment "muzzle" "Bip01 R Hand" 32.00 8.50 7.50 rotate -10 10 0
$attachment "brass" "Bip01 R Hand" 11.00 4.00 3.50 rotate -10 -80 10

// 0 bone controllers

$bodygroup studio
{
studio "ref_w_870.smd"
}

// 2 sequences
$sequence idle "seq_idle1" fps 1 {
    // see CBaseCombatWeapon::SetActivity() for why ACT_VM_IDLE is used by non-viewmodel
    activity ACT_VM_IDLE 1
} 
$sequence attack1 "seq_idle1" fps 1 {
    { event 3004 0 }
    { event AE_NPC_MUZZLEFLASH 0 "SHOTGUN MUZZLE" }
//    { event 9011 0 "ShotgunShellEject" }
    { event 6001 0 "3" }
    activity ACT_RANGE_ATTACK_SHOTGUN 1
}

// auto-create a collisionmodel
$collisionmodel "ref_w_870.smd"
{
    $mass 2.0
}
