$modelname "RPG7\w_rpg7\w_rpg.mdl"
$cdmaterials "models\RPG7\w_rpg7"
$cliptotextures

$scale 0.85

// attachments
$attachment "muzzle" "Bip01 R Hand" 22.00 7.50 7.00 rotate -10 10 0
$attachment "missile" "Bip01 R Hand" 17.00 6.50 6.00 rotate -10 10 0
$attachment "exhaust" "Bip01 R Hand" -23.00 -0.50 -1.00 rotate 10 -170 0

// 0 bone controllers


$bodygroup body
{
studio "RPG7_REF.smd"
}
$bodygroup rocket
{
studio "ROCKET_REF.smd"
blank
}

// 2 sequences
$sequence idle "idle" loop fps 1 {
    // see CBaseCombatWeapon::SetActivity() for why ACT_VM_IDLE is used by non-viewmodel
    activity ACT_VM_IDLE 1
} 
$sequence attack "idle" fps 30 {
    { event AE_NPC_MUZZLEFLASH 0 "RPG MUZZLE" }
    { event 3002 0 }
    activity ACT_RANGE_ATTACK_RPG 1
}

// auto-create a collisionmodel
$collisionmodel "RPG7_REF.smd"
{
    $mass 2.0 // seems a bit light?
}

