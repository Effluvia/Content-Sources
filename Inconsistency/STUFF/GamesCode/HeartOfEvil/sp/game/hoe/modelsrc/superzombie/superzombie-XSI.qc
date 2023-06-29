$bodygroup head
{
studio "ref_Head_Knifed.smd"
}

$bodygroup body
{
studio "ref_Body_Knifed.smd"
}

$sequence idle1 "idle1" loop fps 15 ACT_IDLE 1
$sequence lurch "lurch" LX loop fps 30
$sequence fire_hip_up seq_fire_hip_up
$sequence fire_hip_down seq_fire_hip_down
$sequence fire_up seq_fire_up
$sequence fire_down seq_fire_down

