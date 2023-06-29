$modelname "ape\gorilla\gorilla.mdl"
$cdmaterials "models\ape\gorilla"
$cliptotextures

$surfaceprop "flesh"

$scale 1.0

$eyeposition 0.000000 0.000000 55.000000

// 2 bone controllers
//$controller 0 "Bip01 Head" XR -30 30
//$controller mouth "Bone05" XR 0 30

// 20 hit boxes
$hbox 3 "Bip01 Pelvis" -12 -4 -4  12 8 4
$hbox 6 "Bip01 L Leg" -4 -4 0  4 4 16
$hbox 6 "Bip01 L Leg1" -4 -4 0  4 4 16
$hbox 6 "Bip01 L Foot" 0 -4 -2  -12 8 2
$hbox 7 "Bip01 R Leg" -4 -4 0  4 4 16
$hbox 7 "Bip01 R Leg1" -4 -4 0  4 4 16
$hbox 7 "Bip01 R Foot" 0 -8 -2  -12 4 2
$hbox 3 "Bip01 Spine1" -12 -2 -12  12 10 4
$hbox 2 "Bip01 Spine2" -14 -12 -8  14 2 8
$hbox 2 "Bip01 Spine3" -16 -2 -8  16 12 8
$hbox 1 "Bip01 Head" -8 -4 -6  6 8 8
$hbox 4 "Bip01 L Arm1" -4 -4 -4  4 4 24
$hbox 4 "Bip01 L Arm2" -4 -4 0  4 4 24
$hbox 4 "Bip01 L Hand" -4 -4 0  4 4 8
$hbox 5 "Bip01 R Arm1" -4 -4 -4  4 4 24
$hbox 5 "Bip01 R Arm2" -4 -4 0  4 4 24
$hbox 5 "Bip01 R Hand" -4 -4 0  4 4 8

$bodygroup studio
{
studio "ref_gorilla.smd"
}

// 21 sequences
$sequence idle1 "seq_idle1" loop fps 30 ACT_IDLE 1
$sequence beatchest "seq_beatchest" fps 30 ACT_VICTORY_DANCE 1 { event 1004 20 "NPC_Gorilla.BeatChest" } 
$sequence walk "seq_walk" LX loop fps 30 {
    { event AE_NPC_LEFTFOOT 3 "NPC_Gorilla.FootstepLeft" }
    { event AE_NPC_RIGHTFOOT 18 "NPC_Gorilla.FootstepRight" }
    activity ACT_WALK 1
}
$sequence run "seq_run" LX loop fps 30 {
    { event AE_NPC_LEFTFOOT 0 "NPC_Gorilla.FootstepLeft" }
    { event AE_NPC_RIGHTFOOT 3 "NPC_Gorilla.FootstepRight" }
    activity ACT_RUN 1
}
$sequence jump "seq_jump" fps 30 ACT_RANGE_ATTACK1 1 {
    { event AE_GORILLA_JUMPATTACK 0 }
}
$sequence punch "seq_punch" fps 30 {
    { event AE_GORILLA_ATTACK_RIGHT 21 }
    { event AE_GORILLA_ATTACK_LEFT 34 }
    activity ACT_MELEE_ATTACK1 5
} 
$sequence smackdown "seq_smackdown" fps 30  {
    { event AE_GORILLA_ATTACK_BOTH 21 }
    activity ACT_MELEE_ATTACK1 1
}
$sequence eat "seq_eat" loop fps 30 ACT_GORILLA_EAT 1
$sequence smell "seq_smell" fps 30 ACT_DETECT_SCENT 1
$sequence dieforward "seq_dieforward" fps 30 ACT_DIEFORWARD 1 
$sequence diebackward "seq_diebackward" fps 30 ACT_DIEBACKWARD 1 
$sequence diesimple "seq_diesimple" fps 30 ACT_DIESIMPLE 1 
$sequence 180_left "seq_turnleft" fps 10 ACT_TURN_LEFT 1 
$sequence 180_right "seq_turnright" fps 10 ACT_TURN_RIGHT 1 
$sequence smflinch "seq_smflinch" fps 30 ACT_SMALL_FLINCH 1 

$sequence ragdoll "ragdoll_pose" ACT_DIERAGDOLL 1 fps 30.00
$collisionjoints "collision_model.smd" {

	$mass 800.0
	$inertia 1.00
	$damping 0.00
	$rotdamping 0.00
	$rootbone "bip01"

	$jointconstrain "bip01 spine2" x limit -40.00 40.00 0.00
	$jointconstrain "bip01 spine2" y limit -26.00 26.00 0.00
	$jointconstrain "bip01 spine2" z limit -21.00 21.00 0.00

	$jointconstrain "bip01 r arm1" x limit -180.00 60.00 0.00
	$jointconstrain "bip01 r arm1" y limit -75.00 10.00 0.00
	$jointconstrain "bip01 r arm1" z limit -15.00 15.00 0.00

	$jointconstrain "bip01 r arm2" x limit -100.00 1.00 0.00
	$jointconstrain "bip01 r arm2" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 r arm2" z limit 0.00 0.00 0.00

	$jointconstrain "bip01 l arm1" x limit -60.00 180.00 0.00
	$jointconstrain "bip01 l arm1" y limit -75.00 10.00 0.00
	$jointconstrain "bip01 l arm1" z limit -15.00 15.00 0.00

	$jointconstrain "bip01 l arm2" x limit 0.00 100.00 0.00
	$jointconstrain "bip01 l arm2" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 l arm2" z limit 0.00 0.00 0.00

	$jointconstrain "bip01 l hand" x limit -30.00 30.00 0.00
	$jointconstrain "bip01 l hand" y limit -30.00 30.00 0.00
	$jointconstrain "bip01 l hand" z limit -20.00 20.00 0.00

	$jointconstrain "bip01 r hand" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 r hand" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 r hand" z limit -20.00 20.00 0.00

	$jointconstrain "bip01 r leg" x limit -15.00 15.00 0.00
	$jointconstrain "bip01 r leg" y limit -70.00 50.00 0.00
	$jointconstrain "bip01 r leg" z limit -20.00 20.00 0.00

	$jointconstrain "bip01 r leg1" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 r leg1" y limit 0.00 100.00 0.00
	$jointconstrain "bip01 r leg1" z limit 0.00 0.00 0.00

	$jointconstrain "bip01 head" x limit -30.00 20.00 0.00
	$jointconstrain "bip01 head" y limit -40.00 40.00 0.00
	$jointconstrain "bip01 head" z limit -30.00 30.00 0.00

	$jointconstrain "bip01 l leg" x limit -15.00 15.00 0.00
	$jointconstrain "bip01 l leg" y limit -70.00 50.00 0.00
	$jointconstrain "bip01 l leg" z limit -20.00 20.00 0.00

	$jointconstrain "bip01 l leg1" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 l leg1" y limit 0.00 100.00 0.00
	$jointconstrain "bip01 l leg1" z limit 0.00 0.00 0.00

	$jointconstrain "bip01 l foot" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 l foot" y limit -30.00 30.00 0.00
	$jointconstrain "bip01 l foot" z limit -20.00 20.00 0.00

	$jointconstrain "bip01 r foot" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 r foot" y limit -30.00 30.00 0.00
	$jointconstrain "bip01 r foot" z limit -20.00 20.00 0.00
}
