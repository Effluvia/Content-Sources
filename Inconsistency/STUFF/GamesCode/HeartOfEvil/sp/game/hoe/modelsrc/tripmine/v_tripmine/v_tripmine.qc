$modelname "tripmine\v_tripmine\v_tripmine.mdl"
$cdmaterials "models\tripmine\v_tripmine"
$cliptotextures

$scale 1.0

// 0 attachments

// 0 bone controllers

// 5 hit boxes
$hbox 0 "Bip01 L Arm2" 0.000000 -1.770000 -1.570000  11.680000 1.030000 2.620000
$hbox 0 "Bip01 L Hand" -0.070000 -0.870000 -1.950000  3.890000 1.030000 1.700000
$hbox 0 "Bip01 R Arm2" 0.000000 -1.770000 -2.590000  11.368423 1.000000 1.570000
$hbox 0 "Bip01 R Hand" -0.070000 -0.870000 -1.790000  3.890000 1.030000 1.950000
$hbox 0 "Dummy22" -11.440000 -3.910000 -4.220000  11.460000 7.320000 5.430000

$bodygroup hands
{
studio "tripmine_hands.smd"
}

$scale 0.5
$bodygroup mine
{
//studio "tripmine_mine.smd"
studio "tripmine_boned_world.smd"
blank
}
$scale 1.0

// 9 sequences
// NOTE: "place" and "draw" specify "snap", otherwise the mine is seen flying
// to/from the hands. The alternative is to call SetBodygroup() to hide
// the mine.
$sequence idle1 "seq_idle1" loop fps 30 ACT_VM_IDLE 1
$sequence idle2 "seq_idle2" loop fps 30 ACT_VM_IDLE 1
$sequence arm1 "seq_arm1" fps 60 ACT_VM_TRIPMINE_ARM1 1
$sequence place "seq_place" fps 30 snap ACT_VM_TRIPMINE_PLACE 1
$sequence fidget "seq_fidget" loop fps 30 ACT_VM_FIDGET 3
$sequence holster "seq_holster" fps 30 ACT_VM_HOLSTER 1
$sequence draw "seq_draw" fps 30 snap ACT_VM_DRAW 1
$sequence world "seq_world" loop fps 30 ACT_VM_TRIPMINE_WORLD 1
$sequence ground "seq_ground" loop fps 30 ACT_VM_TRIPMINE_GROUND 1

