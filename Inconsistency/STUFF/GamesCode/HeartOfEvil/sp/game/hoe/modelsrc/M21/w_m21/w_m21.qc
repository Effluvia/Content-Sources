$modelname M21\w_m21\w_m21.mdl
$cd .
$cdmaterials "models\M21\w_m21"

$scale 1.0

$origin 0 0 0

$bodygroup body
{
studio "ref_w_m21_2010.smd"
}

// The first attachment must be the muzzle of the weapon!!!
$attachment "muzzle" "Bip01 R Hand" 37.50 9.20 7.80 rotate -7 10 90
$attachment "scope" "Bip01 R Hand" 17.00 5.60 7.90 rotate -7 10 90

$sequence idle "idle" loop fps 1 {
    // see CBaseCombatWeapon::SetActivity() for why ACT_VM_IDLE is used by non-viewmodel
    activity ACT_VM_IDLE 1
} 

$sequence attack1 "idle" fps 1 {
    { event 3011 0 }
    { event AE_NPC_MUZZLEFLASH 0 "PISTOL MUZZLE" }
//    { event 6001 0 "0" }
    activity ACT_RANGE_ATTACK_SNIPER_RIFLE 1
}

// auto-create a collisionmodel
$collisionmodel "ref_w_m21_2010.smd"
{
    $mass 6.0 // HL1 value
}

