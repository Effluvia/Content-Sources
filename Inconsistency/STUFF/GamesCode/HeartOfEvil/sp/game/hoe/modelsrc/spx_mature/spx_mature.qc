$modelname "spx_mature\spx_mature.mdl"
$cdmaterials "models\spx_mature"
$cliptotextures

$scale 1.0

$surfaceprop "alienflesh"

//$eyeposition 0.000000 0.000000 -2.000000
$eyeposition 0.000000 0.000000 48.000000

// 0 attachments
$attachment "mouth" "Head" 0.000000 0.000000 10.000000 

// 0 bone controllers

// 24 hit boxes
// NOTE: the first 5 were HITGROUP_HEAD but that makes these guys too easy to kill
$hbox 0 "Spine01" -5.349998 -4.660000 -5.015873  5.750000 2.580000 3.050000
$hbox 0 "Head" -4.280000 -2.830000 -0.840000  4.340000 1.820000 3.980964
$hbox 0 "Horn" -1.510000 -3.380000 -1.390000  1.710000 4.640000 4.480000
$hbox 0 "Spine02" -5.382999 -3.070000 -4.550000  5.838180 3.080000 2.940000
$hbox 0 "Tail01" -2.820000 -2.530000 -1.030000  2.780000 1.150000 6.101560
$hbox 0 "Tail02" -2.300000 -1.730000 -0.110000  2.110000 1.230000 12.303881
$hbox 0 "Tail03" -1.690000 -0.950000 -0.650000  1.530000 2.150000 8.993323
$hbox 0 "Tail04" -1.410000 -1.410000 -1.240000  1.460000 1.290000 10.170992
$hbox 0 "Tail05" -1.310000 -0.920000 -0.190000  1.150000 0.720000 17.278893
$hbox 0 "Right Hind Leg01" -1.700000 0.000000 0.000000  0.830000 1.050000 5.556110
$hbox 0 "Right Hind Leg02" -1.110000 -0.820000 -2.990000  1.240000 0.410000 18.545860
$hbox 0 "Left Hind Leg01" -1.210000 -1.090000 0.000000  1.080000 0.000000 5.543105
$hbox 0 "Left Hind Leg02" -1.120000 -0.980000 -3.090000  1.230000 0.300000 18.563766
$hbox 0 "Right Arm01" -1.950000 -1.270000 0.000000  1.930000 1.920000 3.956783
$hbox 0 "Right Arm02" -1.000000 -0.780000 -0.650000  0.730000 0.850000 19.151396
$hbox 0 "Right Arm03" -0.530000 -0.480000 -0.340000  1.090000 0.670000 9.270265
$hbox 0 "Right Arm04" -1.481909 -1.261611 -0.330000  1.480000 0.860000 3.692777
$hbox 0 "Left Arm01" -1.500000 -1.910000 0.000000  2.330000 1.310000 4.452791
$hbox 0 "Left Arm02" -1.030000 -0.310000 -1.060000  0.710000 1.360000 19.025133
$hbox 0 "Left Arm03" -0.850000 -0.570000 -0.480000  0.770000 0.560000 9.206153
$hbox 0 "Left Arm04" -1.619408 -0.920000 -0.210000  1.231918 1.706179 3.632080
$hbox 0 "Right Claw 12" -0.490000 -1.040000 -0.570000  0.600000 0.810000 13.062431
$hbox 0 "Right Claw 22" -1.290000 -0.550000 0.000000  1.170000 0.540000 17.268505
$hbox 0 "Left Claw 12" -0.620000 -1.000000 -0.010000  0.430000 0.880000 13.892864

$bodygroup studio
{
studio "ref_spx_mature.smd"
}

$bodygroup wings
{
studio "ref_wings.smd"
blank
}

// 19 sequences
$sequence idle1 "seq_idle1" loop fps 15 ACT_IDLE 4 
$sequence idle2 "seq_idle2" loop fps 15 ACT_IDLE 1 
$sequence forward "seq_forward" loop fps 15 ACT_SPXFLYER_FLY_FORWARD 1
$sequence backward "seq_backward" loop fps 15 ACT_SPXFLYER_FLY_BACKWARD 1
$sequence up "seq_up" loop fps 15 ACT_SPXFLYER_FLY_UP 1
$sequence down "seq_down" loop fps 15 ACT_SPXFLYER_FLY_DOWN 1
$sequence right "seq_right" loop fps 15 ACT_SPXFLYER_FLY_RIGHT 1
$sequence left "seq_left" loop fps 15 ACT_SPXFLYER_FLY_LEFT 1
$sequence hit "seq_hit" loop fps 15 ACT_SPXFLYER_HIT 1
$sequence fall "seq_fall" loop fps 15 
$sequence die1 "seq_die" fps 20 ACT_DIESIMPLE 1 
$sequence shoot "seq_shoot" fps 30 snap ACT_RANGE_ATTACK1 1 { event AE_SPXFLYER_SPIT 0 }
$sequence punch "seq_punch" fps 30 ACT_MELEE_ATTACK1 1 { event AE_SPXFLYER_CLAW_RIGHT 13 } { event AE_SPXFLYER_CLAW_LEFT 25 }
$sequence punchdown "seq_punchdown" fps 30 ACT_MELEE_ATTACK2 1 { event AE_SPXFLYER_CLAW_RIGHT_DOWN 13 } { event AE_SPXFLYER_CLAW_LEFT_DOWN 21 }
$sequence smlflinch "seq_smlflinch" fps 20 ACT_SMALL_FLINCH 1 
//$sequence flinch2 "flinch2" fps 20 ACT_BIG_FLINCH 1 

$weightlist wings WingLF 1.0 WingRF 1.0 WingLR 1.0 WingRR 1.0
$sequence wings seq_wings fps 30 weightlist wings autoplay hidden

// move-and-shoot; FIXME walk-aim/run-aim not correct
$sequence move_shoot "seq_shoot" subtract move_shoot 0 fps 30 activity ACT_GESTURE_RANGE_ATTACK1 1 delta { event AE_SPXFLYER_SPIT 0 }
$sequence walk_aim "seq_forward" fps 30 activity ACT_WALK_AIM 1
$sequence run_aim "seq_forward" fps 30 activity ACT_RUN_AIM 1

$sequence ragdoll "ragdoll_pose" FPS 30 activity ACT_DIERAGDOLL 1
$collisionjoints collision_model {

	$mass 80.0
	$inertia 1.00
	$damping 0.00
	$rotdamping 0.00
	$rootbone "spine01"

	$jointconstrain "left arm02" x limit -30.00 80.00 0.00
	$jointconstrain "left arm02" y limit -80.00 80.00 0.00
	$jointconstrain "left arm02" z limit 0.00 0.00 0.00

	$jointconstrain "left arm03" x limit 0.00 90.00 0.00
	$jointconstrain "left arm03" y limit 0.00 0.00 0.00
	$jointconstrain "left arm03" z limit 0.00 0.00 0.00

	$jointconstrain "right arm02" x limit -80.00 30.00 0.00
	$jointconstrain "right arm02" y limit -80.00 80.00 0.00
	$jointconstrain "right arm02" z limit 0.00 0.00 0.00

	$jointconstrain "right arm03" x limit -90.00 0.00 0.00
	$jointconstrain "right arm03" y limit 0.00 0.00 0.00
	$jointconstrain "right arm03" z limit 0.00 0.00 0.00

	$jointconstrain "right arm04" x limit -75.00 50.00 0.00
	$jointconstrain "right arm04" y limit -30.00 30.00 0.00
	$jointconstrain "right arm04" z limit 0.00 0.00 0.00

	$jointconstrain "left hind leg02" x limit -80.00 10.00 0.00
	$jointconstrain "left hind leg02" y limit -80.00 10.00 0.00
	$jointconstrain "left hind leg02" z limit 0.00 0.00 0.00

	$jointconstrain "right hind leg02" x limit -10.00 80.00 0.00
	$jointconstrain "right hind leg02" y limit -80.00 10.00 0.00
	$jointconstrain "right hind leg02" z limit 0.00 0.00 0.00

	$jointconstrain "tail01" x limit 0.00 88.00 0.00
	$jointconstrain "tail01" y limit -20.00 20.00 0.00
	$jointconstrain "tail01" z limit 0.00 0.00 0.00

	$jointconstrain "tail02" x limit -30.00 0.00 0.00
	$jointconstrain "tail02" y limit -20.00 20.00 0.00
	$jointconstrain "tail02" z limit 0.00 0.00 0.00

	$jointconstrain "tail03" x limit -60.00 0.00 0.00
	$jointconstrain "tail03" y limit -20.00 20.00 0.00
	$jointconstrain "tail03" z limit 0.00 0.00 0.00

	$jointconstrain "tail04" x limit -20.00 20.00 0.00
	$jointconstrain "tail04" y limit -15.00 15.00 0.00
	$jointconstrain "tail04" z limit 0.00 0.00 0.00

	$jointconstrain "tail05" x limit -15.00 15.00 0.00
	$jointconstrain "tail05" y limit -10.00 10.00 0.00
	$jointconstrain "tail05" z limit 0.00 0.00 0.00

	$jointconstrain "left arm04" x limit -50.00 75.00 0.00
	$jointconstrain "left arm04" y limit -30.00 30.00 0.00
	$jointconstrain "left arm04" z limit 0.00 0.00 0.00
}

