$modelname "machete\v_machete\v_machete.mdl"
$cdmaterials "models\machete\v_machete"
$cliptotextures

$scale 1.0

// 0 attachments

// 0 bone controllers

// 2 hit boxes
$hbox 0 "Bip01 R Arm2" 0.000000 -2.120000 -1.980000  11.368422 2.100000 2.180000
$hbox 0 "Bip01 R Hand" 0.000000 -0.870000 -5.930000  7.850000 2.940000 19.950001

$bodygroup studio
{
studio "ref_v_machete.smd"
}

// 11 sequences
$sequence idle1 "seq_idle1" fps 13 {
    activity ACT_VM_IDLE 1
}
$sequence idle2 "seq_idle2" loop fps 15  {
    activity ACT_VM_IDLE 1
}
$sequence idle3 "seq_idle3" loop fps 15 {
    activity ACT_VM_IDLE 1
}
$sequence draw "seq_draw" fps 24 {
    activity ACT_VM_DRAW 1
}
$sequence holster "seq_holster" fps 24 {
    activity ACT_VM_HOLSTER 1
}

$sequence attack1 "seq_attack1" fps 22 {
    activity ACT_VM_HITCENTER 1
}
$sequence miss1 seq_attack1miss fps 22 {
    activity ACT_VM_MISSCENTER 1
}


$sequence attack2 "seq_attack2" fps 22 {
    activity ACT_VM_MISSCENTER 1
}
$sequence attack2hit "seq_attack2hit" fps 22 {
    activity ACT_VM_HITCENTER 1
}

$sequence attack3 "seq_attack3" fps 24 {
    activity ACT_VM_MISSCENTER 1
}
$sequence attack3hit "seq_attack3hit" fps 24 {
   activity ACT_VM_HITCENTER 1
}

