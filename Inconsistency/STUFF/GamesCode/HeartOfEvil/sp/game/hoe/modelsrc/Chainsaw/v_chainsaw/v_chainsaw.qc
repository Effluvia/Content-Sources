$modelname "Chainsaw\v_chainsaw\v_chainsaw.mdl"
$cdmaterials "models\Chainsaw\v_chainsaw"
$cliptotextures

$scale 1.0

// 0 attachments
$attachment 0 "chainsaw" 0.0000 18.00000 2.00000

// 0 bone controllers

// 2 hit boxes
$hbox 0 "Bip01 R Arm2" 0.000000 -2.120000 -1.980000  11.368422 2.100000 2.180000
$hbox 0 "Bip01 R Hand" 0.000000 -0.870000 -5.930000  7.850000 2.940000 19.950001

$bodygroup studio
{
studio "ref_v_chainsaw.smd"
}

// 11 sequences
$sequence idleoff "seq_idleoff" fps 16 {
    activity ACT_CHAINSAW_IDLE_OFF -1
}
$sequence idle1 "seq_idle1" fps 16 {
    activity ACT_CHAINSAW_IDLE -1
}
$sequence draw "seq_deploy" fps 24 {
    activity ACT_VM_DRAW -1
}
$sequence holster "seq_holster" fps 24 {
    activity ACT_VM_HOLSTER -1
}
$sequence idle_hit "seq_idle_hit" fps 16 {
    activity ACT_CHAINSAW_IDLE_HIT -1
}
$sequence attack1 "seq_attack1" fps 16 {
    activity ACT_CHAINSAW_ATTACK 1
}
$sequence attack2 "seq_attack2" fps 16 {
    activity ACT_CHAINSAW_ATTACK 1
}
$sequence attack3 "seq_attack3" fps 16 {
    activity ACT_CHAINSAW_ATTACK 1
}
$sequence attack4 "seq_attack4" fps 16 {
    activity ACT_CHAINSAW_ATTACK 1
}
$sequence startready "seq_startready" fps 16 {
    activity ACT_CHAINSAW_START_READY -1
}
$sequence startfail "seq_startfail" fps 16 {
    activity ACT_CHAINSAW_START_FAIL -1
}
$sequence startempty "seq_startempty" fps 16 {
    activity ACT_CHAINSAW_START_EMPTY -1
}
$sequence start	"seq_start" fps 21 {
    activity ACT_CHAINSAW_START -1
}
