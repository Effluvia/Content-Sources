$modelname "kophadult.mdl"
$cdmaterials "models\kophyaeger\adult"
$cliptotextures

$eyeposition 0.000000 0.000000 72.000000

// attachments
$attachment "mouth" "Head" 0.00 10.00 5.00 rotate -0 -0 0

// bone controllers

// 29 hit boxes
$hbox 3 "Center" -4.200000 -7.490000 -6.470000  4.560000 6.239997 6.460000
$hbox 7 "R Leg" -1.210000 -4.980000 0.000000  2.480000 4.380000 21.160000
$hbox 7 "R Leg1" -0.900000 -1.130000 0.000000  0.990000 2.900000 13.602428
$hbox 7 "R Leg2" -1.270000 -0.540000 0.000000  1.230000 1.210000 16.549999
$hbox 6 "L Leg" -1.510000 -3.980000 0.000000  2.540000 4.920000 21.200001
$hbox 6 "L Leg1" -0.900000 -1.130000 0.000000  0.990000 2.900000 13.602428
$hbox 6 "L Leg2" -1.270000 -0.540000 0.000000  1.230000 1.210000 16.549999
$hbox 3 "Spine1" -6.030000 -5.480000 -7.260000  6.460000 11.290000 11.180000
$hbox 3 "Spine2" -11.130000 -16.480000 0.000000  8.740000 6.620000 6.723954
$hbox 2 "Spine3" -5.780000 -8.430000 -0.790000  5.420000 8.300000 5.263671
$hbox 2 "Neck" -4.807695 -5.230000 0.000000  4.611399 7.800000 8.940563
$hbox 1 "Head" -4.170000 -26.059999 -14.310000  3.940000 11.440000 8.880000
$hbox 5 "R Arm" -3.460000 -1.120000 0.000000  1.030000 1.820000 16.430000
$hbox 5 "R Arm1" -1.800000 -2.860000 0.000000  4.050000 2.750000 14.190000
$hbox 5 "R Arm2" -0.810000 -1.070000 0.000000  1.430000 1.310000 20.459999
$hbox 5 "R Hand" 0.000000 -2.720000 0.000000  1.370000 3.320000 3.420000
$hbox 5 "R Finger1" 0.000000 -2.410000 0.000000  4.660000 0.000000 10.570000
$hbox 5 "R Finger3" 0.000000 0.000000 0.000000  3.560000 3.150000 5.090000
$hbox 4 "L Arm" -3.290000 -2.230000 0.000000  1.130000 0.430000 16.040001
$hbox 4 "L Arm1" -2.460000 -2.390000 0.000000  3.500000 3.200000 13.990000
$hbox 4 "L Arm2" -1.030000 -0.950000 0.000000  1.170000 1.420000 20.450001
$hbox 4 "L Hand" 0.000000 -2.980000 0.000000  1.148325 3.120000 3.500000
$hbox 4 "L Finger1" 0.000000 0.000000 0.000000  4.390000 2.830000 10.520000
$hbox 4 "L Finger3" 0.000000 -2.880000 0.000000  4.050000 0.000000 5.010000
$hbox 0 "Tail" -2.720000 -2.500000 0.000000  2.110000 3.050000 28.590000
$hbox 0 "Tail1" -1.770000 -2.200000 0.000000  0.740000 2.460000 23.160000
$hbox 0 "Tail2" -1.790000 -1.200000 0.000000  0.000000 1.700000 22.930000

// Put this *after* hitboxes otherwise they get scaled as well
$scale 3.0

$surfaceprop "alienflesh"

$bodygroup studio
{
studio "ref_kophadult.smd"
}

// 11 sequences
$sequence idle1 "seq_idle1" loop fps 10 ACT_IDLE 10 
$sequence idle2 "seq_idle2" fps 10 ACT_IDLE 1 
$sequence walk "seq_walk" LX loop fps 60 ACT_WALK 1
$sequence run "seq_run" LX loop fps 40 ACT_RUN 1
$sequence flinch "seq_flinch" fps 30 ACT_SMALL_FLINCH 1 
$sequence dieback "seq_diesimple" fps 30 ACT_DIESIMPLE 1 
$sequence 180_left "seq_turn_left" fps 30 ACT_TURN_LEFT 1 
$sequence 180_right "seq_turn_right" fps 30 ACT_TURN_RIGHT 1 
$sequence jump "seq_jump" fps 30 ACT_RANGE_ATTACK1 2 { event AE_KOPHYAEGER_JUMPATTACK 0 } 
$sequence eat "seq_eat" fps 10 ACT_KOPHYAEGER_EAT 1 { event 1008 0 "NPC_Kophyaeger.Eat" } 
$sequence sniff "seq_sniff" fps 10 ACT_KOPHYAEGER_DETECT_SCENT 1 

$sequence bite1 "seq_bite1" fps 30 activity ACT_MELEE_ATTACK1 1 { event AE_KOPHYAEGER_BITE 10 }
$sequence bite2 "seq_bite2" fps 30 activity ACT_MELEE_ATTACK1 1 { event AE_KOPHYAEGER_BITE 10 }
$sequence bite3 "seq_bite3" fps 30 activity ACT_MELEE_ATTACK1 1 { event AE_KOPHYAEGER_BITE 10 }

$sequence threatdisplay "seq_threatdisplay" fps 30 ACT_KOPHYAEGER_THREAT_DISPLAY 1

$sequence ragdoll "../ragdoll_pose" ACT_DIERAGDOLL 1 fps 30.00
$collisionjoints "../collision_model.smd" {

	$mass 100.0
	$inertia 10.00
	$damping 0.01
	$rotdamping 1.50
	$rootbone "Center"

	$jointconstrain "tail" x limit -95.00 30.00 0.00
	$jointconstrain "tail" y limit -40.00 40.00 0.00
	$jointconstrain "tail" z limit -15.00 15.00 0.00

	$jointconstrain "tail1" x limit -40.00 20.00 0.00
	$jointconstrain "tail1" y limit -30.00 30.00 0.00
	$jointconstrain "tail1" z limit -15.00 15.00 0.00

	$jointconstrain "tail2" x limit -40.00 40.00 0.00
	$jointconstrain "tail2" y limit -30.00 30.00 0.00
	$jointconstrain "tail2" z limit -15.00 15.00 0.00

	$jointconstrain "tail3" x limit -70.00 15.00 0.00
	$jointconstrain "tail3" y limit -30.00 30.00 0.00
	$jointconstrain "tail3" z limit -15.00 15.00 0.00

	$jointconstrain "spine2" x limit -40.00 45.00 0.00
	$jointconstrain "spine2" y limit -30.00 30.00 0.00
	$jointconstrain "spine2" z limit -50.00 50.00 0.00

	$jointconstrain "l arm1" x limit -40.00 50.00 0.00
	$jointconstrain "l arm1" y limit -20.00 20.00 0.00
	$jointconstrain "l arm1" z limit -15.00 15.00 0.00

	$jointconstrain "l arm2" x limit 0.00 60.00 0.00
	$jointconstrain "l arm2" y limit 0.00 0.00 0.00
	$jointconstrain "l arm2" z limit -30.00 30.00 0.00

	$jointconstrain "r arm1" x limit -40.00 50.00 0.00
	$jointconstrain "r arm1" y limit -20.00 20.00 0.00
	$jointconstrain "r arm1" z limit -15.00 15.00 0.00

	$jointconstrain "r arm2" x limit -60.00 0.00 0.00
	$jointconstrain "r arm2" y limit 0.00 0.00 0.00
	$jointconstrain "r arm2" z limit -30.00 30.00 0.00

	$jointconstrain "r hand" x limit 0.00 0.00 0.00
	$jointconstrain "r hand" y limit 0.00 0.00 0.00
	$jointconstrain "r hand" z limit 0.00 0.00 0.00

	$jointconstrain "head" x limit -30.00 20.00 0.00
	$jointconstrain "head" y limit -15.00 15.00 0.00
	$jointconstrain "head" z limit -50.00 50.00 0.00

	$jointconstrain "l hand" x limit 0.00 0.00 0.00
	$jointconstrain "l hand" y limit 0.00 0.00 0.00
	$jointconstrain "l hand" z limit 0.00 0.00 0.00

	$jointconstrain "l leg" x limit -60.00 40.00 0.00
	$jointconstrain "l leg" y limit -10.00 30.00 0.00
	$jointconstrain "l leg" z limit -25.00 25.00 0.00

	$jointconstrain "l leg1" x limit -35.00 35.00 0.00
	$jointconstrain "l leg1" y limit -20.00 20.00 0.00
	$jointconstrain "l leg1" z limit -15.00 15.00 0.00

	$jointconstrain "l leg2" x limit 0.00 0.00 0.00
	$jointconstrain "l leg2" y limit 0.00 0.00 0.00
	$jointconstrain "l leg2" z limit 0.00 0.00 0.00

	$jointconstrain "l foot" x limit 0.00 0.00 0.00
	$jointconstrain "l foot" y limit 0.00 0.00 0.00
	$jointconstrain "l foot" z limit 0.00 0.00 0.00

	$jointconstrain "r leg" x limit -40.00 60.00 0.00
	$jointconstrain "r leg" y limit -10.00 30.00 0.00
	$jointconstrain "r leg" z limit -25.00 25.00 0.00

	$jointconstrain "r leg1" x limit -35.00 35.00 0.00
	$jointconstrain "r leg1" y limit -20.00 20.00 0.00
	$jointconstrain "r leg1" z limit -15.00 15.00 0.00

	$jointconstrain "r leg2" x limit -30.00 30.00 0.00
	$jointconstrain "r leg2" y limit -30.00 60.00 0.00
	$jointconstrain "r leg2" z limit -30.00 30.00 0.00

	$jointconstrain "r foot" x limit -30.00 30.00 0.00
	$jointconstrain "r foot" y limit -30.00 30.00 0.00
	$jointconstrain "r foot" z limit -30.00 30.00 0.00

}
