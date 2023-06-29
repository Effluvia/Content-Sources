$modelname "RPG7\v_rpg7\v_rpg.mdl"
$cdmaterials "models\RPG7\v_rpg7"
$cliptotextures

$scale 1.0

$origin 0 0 0

// 0 attachments

// 0 bone controllers


$bodygroup studio
{
studio "rpg7_reference.smd"
}
$bodygroup nade
{
studio "nade_reference.smd"
blank
}

// 1 sequences
$sequence idle "seq_idle" fps 30  {
    activity ACT_VM_IDLE 1
}
$sequence fire "seq_fire" fps 30 {
    snap
    activity ACT_VM_PRIMARYATTACK 1
}
$sequence reload "seq_reload" fps 30 {
    { event AE_SV_BODYGROUP_SET_VALUE 49 "nade 0" }
    { event AE_CL_PLAYSOUND 73 "Weapon_RPG7.Reload" }
    { event 3015 77 }
    activity ACT_VM_RELOAD 1
}
$animation a_deploy "seq_reload" frame 100 120 fps 30
$sequence deploy "a_deploy" fps 30 {
    snap
    activity ACT_VM_DRAW 1
}
//$sequence holster "seq_holster" fps 30 {
//    activity ACT_VM_HOLSTER 1
//}
$sequence empty "seq_idle_empty" fps 30  {
    activity ACT_VM_IDLE_EMPTY 1
} 

