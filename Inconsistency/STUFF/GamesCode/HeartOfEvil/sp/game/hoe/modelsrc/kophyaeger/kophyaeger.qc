$modelname "kophyaeger.mdl"
$cdmaterials "models\kophyaeger"
$cliptotextures

$scale 1.0

$surfaceprop "alienflesh"

$eyeposition 0.000000 0.000000 26.000000

// 0 bone controllers

// 13 hit boxes
$hbox 1 "Head" -2 -4 -2 2 4 2
$hbox 2 "Spine3" -3 -3 -4 3 3 2
$hbox 3 "Center" -2 -4 -2 2 4 2
$hbox 4 "L Arm1" -1 -1 -3 1 1 5
$hbox 4 "L Arm2" -1 -1 0 1 1 8
$hbox 5 "R Arm1" -1 -1 -3 1 1 5
$hbox 5 "R Arm2" -1 -1 0 1 1 8
$hbox 6 "L Leg1" -1 -1 -1 1 1 5
$hbox 6 "L Leg2" -1 -1 0 1 1 6
$hbox 7 "R Leg1" -1 -1 -1 1 1 5
$hbox 7 "R Leg2" -1 -1 0 1 1 6
$hbox 0 "Tail" -1.5 -1.5 0 1.5 1.5 10
$hbox 0 "Tail1" -1 -1 0 1 1 8
$hbox 0 "Tail2" -1 -1 0 1 1 7

$bodygroup studio
{
studio "ref_kophyaeger.smd"
}

// 21 sequences
$sequence idle1 "seq_idle1" loop fps 10 ACT_IDLE 10
$sequence idle2 "seq_idle2" fps 10 ACT_IDLE 1
$sequence walk "seq_walk" LX loop fps 60 ACT_WALK 1 
$sequence run "seq_run" LX loop fps 60 ACT_RUN 1
$sequence flinch "seq_flinch" fps 30 ACT_SMALL_FLINCH 1 
$sequence dieback "seq_diesimple" fps 30 ACT_DIESIMPLE 1 
$sequence 180_left "seq_turn_left" fps 30 ACT_TURN_LEFT 1 
$sequence 180_right "seq_turn_right" fps 30 ACT_TURN_RIGHT 1 
$sequence jump "seq_jump" fps 30 ACT_RANGE_ATTACK1 2 { event AE_KOPHYAEGER_JUMPATTACK 0 } 
$sequence eat "seq_eat" fps 10 ACT_KOPHYAEGER_EAT 1 { event 1004 0 "NPC_Kophyaeger.Eat" } 
$sequence sniff "seq_sniff" fps 10 ACT_KOPHYAEGER_DETECT_SCENT 1

$sequence threatdisplay "seq_threatdisplay" fps 30 ACT_KOPHYAEGER_THREAT_DISPLAY 1

$sequence ragdoll "ragdoll_pose" ACT_DIERAGDOLL 1 fps 30.00
$collisionjoints "collision_model.smd" {

	$mass 10.0
	$inertia 10.00
	$damping 0.01
	$rotdamping 1.50
	$rootbone "center"

	$jointconstrain "tail" x limit -95.00 30.00 0.00
	$jointconstrain "tail" y limit -40.00 40.00 0.00
	$jointconstrain "tail" z limit -15.00 15.00 0.00

	$jointconstrain "tail1" x limit -40.00 20.00 0.00
	$jointconstrain "tail1" y limit -30.00 30.00 0.00
	$jointconstrain "tail1" z limit -15.00 15.00 0.00

	$jointconstrain "tail2" x limit -40.00 40.00 0.00
	$jointconstrain "tail2" y limit -30.00 30.00 0.00
	$jointconstrain "tail2" z limit -15.00 15.00 0.00

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

	$jointconstrain "l leg2" x limit -30.00 30.00 0.00
	$jointconstrain "l leg2" y limit -30.00 60.00 0.00
	$jointconstrain "l leg2" z limit -30.00 30.00 0.00

	$jointconstrain "l foot" x limit -30.00 30.00 0.00
	$jointconstrain "l foot" y limit -30.00 30.00 0.00
	$jointconstrain "l foot" z limit -30.00 30.00 0.00

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

	$jointconstrain "tail3" x limit -70.00 15.00 0.00
	$jointconstrain "tail3" y limit -30.00 30.00 0.00
	$jointconstrain "tail3" z limit -15.00 15.00 0.00
}

