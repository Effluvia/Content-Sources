$modelname "spx_baby\spx_baby.mdl"
$cdmaterials "models\spx_baby"
$cliptotextures

$scale 1.0

$surfaceprop "alienflesh"

$eyeposition 0.000000 0.000000 6.000000

$bbox -12.000000 -12.000000 0.000000 12.000000 12.000000 24.000000

// 0 attachments

// 0 bone controllers

$bodygroup studio
{
studio "ref_spx_baby.smd"
}

// 21 sequences
$sequence idle1 "seq_idle1" loop fps 20 ACT_IDLE 15 
//$sequence idle2 "idle2" fps 20 ACT_IDLE 1 
//$sequence idle3 "idle3" fps 20 ACT_IDLE 1 
$sequence walk "seq_walk" LX loop fps 30 ACT_WALK 1 
$sequence run "seq_run" LX loop fps 30 ACT_RUN 1
//$sequence angry "angry" loop fps 30 
$sequence flinch "seq_smlflinch" fps 30 ACT_SMALL_FLINCH 1 
$sequence diesimple "seq_diesimple" fps 30 ACT_DIESIMPLE 1 
$sequence diebackward "seq_diebackward" fps 30 ACT_DIEBACKWARD 1 
$sequence 180_left "seq_turnleft" loop fps 15 ACT_TURN_LEFT 1 
$sequence 180_right "seq_turnright" loop fps 15 ACT_TURN_RIGHT 1 
$sequence jump "seq_jump" fps 30 ACT_RANGE_ATTACK1 2 { event AE_SPXBABY_JUMPATTACK 0 } 
//$sequence jump_variation1 "jump_variation1" fps 30 ACT_RANGE_ATTACK1 1 { event 2 0 } 
//$sequence jump_variation2 "jump_variation2" fps 30 ACT_RANGE_ATTACK1 1 { event 2 0 } 
//$sequence yaw_adjustment "yaw_adjustment" loop fps 30 ACT_MELEE_ATTACK2 1 
//$sequence headcrabbed#1 "headcrabbed#1" fps 20 { event 1000 1 } { event 1003 110 "splat" } { event 1 109 } 
//$sequence headcrabbed#2 "headcrabbed#2" fps 20 { event 1000 1 } { event 1 165 } 
//$sequence headidle "headidle" loop fps 18 
//$sequence crashidle "crashidle" loop fps 34 
//$sequence crash "crash" fps 20 { event 1000 1 } 
//$sequence hstruggleidle "hstruggleidle" loop fps 18 
//$sequence hstruggle "hstruggle" fps 28 { event 1000 1 } 

$sequence idle2 "seq_idle2" fps 15 {
	activity ACT_IDLE 1
}
$sequence idle3 "seq_idle3" fps 40 {
	activity ACT_IDLE 1
}
$sequence threatdisplay "seq_threatdisplay" fps 30 {
	activity ACT_SPXBABY_THREAT_DISPLAY 1
}

$collisionjoints collision_model.smd {

	$mass 20.0
	$inertia 1.00
	$damping 0.00
	$rotdamping 0.00
	$rootbone "spine 02"

	$jointconstrain "right1 hind leg 01" x limit -60.00 30.00 0.00
	$jointconstrain "right1 hind leg 01" y limit -60.00 0.00 0.00
	$jointconstrain "right1 hind leg 01" z limit -30.00 30.00 0.00

	$jointconstrain "left hind leg 01" x limit -30.00 60.00 0.00
	$jointconstrain "left hind leg 01" y limit -60.00 0.00 0.00
	$jointconstrain "left hind leg 01" z limit -30.00 30.00 0.00

	$jointconstrain "left hind leg 02" x limit -60.00 60.00 0.00
	$jointconstrain "left hind leg 02" y limit -79.00 59.00 0.00
	$jointconstrain "left hind leg 02" z limit -20.00 20.00 0.00

	$jointconstrain "left front leg 01" x limit -60.00 40.00 0.00
	$jointconstrain "left front leg 01" y limit -60.00 30.00 0.00
	$jointconstrain "left front leg 01" z limit -30.00 30.00 0.00

	$jointconstrain "left front leg 02" x limit -80.00 40.00 0.00
	$jointconstrain "left front leg 02" y limit -60.00 60.00 0.00
	$jointconstrain "left front leg 02" z limit -20.00 20.00 0.00

	$jointconstrain "right front leg 01" x limit -40.00 60.00 0.00
	$jointconstrain "right front leg 01" y limit -60.00 30.00 0.00
	$jointconstrain "right front leg 01" z limit -30.00 30.00 0.00

	$jointconstrain "right front leg 02" x limit -40.00 80.00 0.00
	$jointconstrain "right front leg 02" y limit -60.00 60.00 0.00
	$jointconstrain "right front leg 02" z limit -20.00 20.00 0.00

	$jointconstrain "tail 02" x limit -40.00 15.00 0.00
	$jointconstrain "tail 02" y limit -30.00 30.00 0.00
	$jointconstrain "tail 02" z limit -40.00 40.00 0.00

	$jointconstrain "tail 03" x limit -35.00 15.00 0.00
	$jointconstrain "tail 03" y limit -30.00 30.00 0.00
	$jointconstrain "tail 03" z limit -30.00 30.00 0.00

	$jointconstrain "right1 hind leg 02" x limit -60.00 60.00 0.00
	$jointconstrain "right1 hind leg 02" y limit -80.00 60.00 0.00
	$jointconstrain "right1 hind leg 02" z limit -20.00 20.00 0.00

	$jointconstrain "tail 04" x limit -20.00 20.00 0.00
	$jointconstrain "tail 04" y limit -30.00 30.00 0.00
	$jointconstrain "tail 04" z limit -30.00 30.00 0.00
}
$sequence ragdoll "ragdoll_pose" FPS 30 activity ACT_DIERAGDOLL 1

