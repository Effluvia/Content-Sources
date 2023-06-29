$modelname "colt1911a1\w_colt1911a1\w_colt1911A1.mdl"
$cdmaterials "models\colt1911a1\w_colt1911a1"
$cliptotextures

//$scale 1.5
$scale 0.9

// 1 attachments
$attachment "muzzle" "Weapon_Bone" 12.00 4.50 3.50 rotate 0 18 0
$attachment "brass" "Weapon_Bone" 5.50 1.50 3.50 rotate 0 -70 0

// 0 bone controllers

$bodygroup studio
{
studio "ref_w_colt1911A1.smd"
}

// 2 sequences
$sequence idle "seq_idle1" fps 1 {
	// see CBaseCombatWeapon::SetActivity() for why ACT_VM_IDLE is used by non-viewmodel
    activity ACT_VM_IDLE 1
}
$sequence attack1 "seq_idle1" fps 20 {
    { event 3014 0 }
    { event AE_NPC_MUZZLEFLASH 0 "PISTOL MUZZLE" }
	{ event 9011 0 "ShellEject" }
    activity ACT_RANGE_ATTACK_PISTOL 1
}

// Used by barney SOG pull animation.  Ideally the bullet should come out
// when this animation tells it to, not Barney's animation.
$sequence scripted "seq_idle1" fps 20 {
    { event AE_NPC_MUZZLEFLASH 0 "PISTOL MUZZLE" }
	{ event 9011 0 "ShellEject" }
	activity ACT_USE 1
}

// auto-create a collisionmodel
$collisionmodel "ref_w_colt1911A1.smd"
{
    $mass 2.0
}
