$modelname "mikeforce\mikeforce.mdl"

$bodygroup head
{
studio "ref_Head_MikeForce.smd"
}

$bodygroup torso
{
studio "ref_Torso_MikeForce.smd"
}

$bodygroup arms
{
studio "ref_Arms.smd"
}

$bodygroup legs
{
studio "ref_Legs.smd"
}

//$sequence crouch_combatidle "seq_crouch_combatidle" loop fps 20
//$sequence crouch_fire_up "seq_crouch_fire_up" loop fps 20
//$sequence crouch_fire_down "seq_crouch_fire_down" loop fps 20
$sequence ragdoll "ragdoll_pose" fps 30.00
