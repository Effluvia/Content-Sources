$modelname "w_m16.mdl"
$cdmaterials "models\M16\w_m16"
$cliptotextures

$scale 1.0

// attachments
// The first attachment must be the muzzle of the weapon!!!
$attachment "muzzle" "Bip01 R Hand" 27.00 7.80 5.60 rotate -5 10 90
$attachment "brass" "Bip01 R Hand" 8.00 4.00 4.00 rotate -5 -80 5

// 0 bone controllers

$bodygroup studio
{
studio "ref_w_m16.smd"
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

// Used by namd3 superzombie animation.  Ideally the bullet should come out
// when this animation tells it to, not the SZ's animation.
$sequence scripted "seq_idle1" fps 20 {
    { event AE_NPC_MUZZLEFLASH 0 "SMG1 MUZZLE" }
    { event 9011 0 "RifleShellEject" }
    activity ACT_USE 1
}

// auto-create a collisionmodel
$collisionmodel "ref_w_m16.smd"
{
    $mass 15.0 // HL1 value
}

