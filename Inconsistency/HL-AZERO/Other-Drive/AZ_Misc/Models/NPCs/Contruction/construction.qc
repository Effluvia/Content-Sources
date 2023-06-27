$modelname ".\MDL\construction.mdl"
$cd ".\SMD"
$cdtexture ".\Textures"
$scale 1.0

$body "studio" "construction_reference"

$controller 0 "Bip01 Head" XR -60.000000 60.000000

$sequence "inspect_floor" "inspect_floor" fps 12 loop 
$sequence "c1a4_bottom" "c1a4_bottom" fps 12 
$sequence dying_barneyidle "dying_barney_c1a3(idle)" loop fps 12 scale 0.96
$sequence "idle1" "BARNEY_Idle1" fps 18 loop ACT_IDLE 50 
$sequence "idle2" "BARNEY_Idle2" fps 15 ACT_IDLE 1 
$sequence "idle3" "BARNEY_Idle3(soft&subtle)" fps 15 ACT_IDLE 1 
$sequence "idle5" "BARNY_Idle5(Turn)" fps 12 ACT_IDLE 1 

$sequence walk "BARNEY_Walk(New&Improved)1" LX fps 30  loop ACT_WALK 1 {
	{ event 1004 122 "common/npc_step1.wav" }
	{ event 1004 137 "common/npc_step3.wav" }
}
$sequence run "BARNEY_Run(New&Improved)2" LX fps 30 loop  ACT_RUN 1 {
	{ event 1004 5 "common/npc_step2.wav" }
	{ event 1004 13 "common/npc_step4.wav" }
}

$sequence diesimple "BARNEY_Die(Simple)1" X fps 15  ACT_DIE_HEADSHOT 1 { event 2001 5 }
$sequence dieviolent "BARNEY_Die(Violent)1" X fps 20 frames 0 26  ACT_DIE_HEADSHOT 1 { event 2001 10 }
$sequence dieforward "BARNEY_Die(Forward)1" X fps 18  ACT_DIEFORWARD 1 { event 2001 8 }
$sequence diebackward "BARNEY_Die(Backward)1" X fps 15  ACT_DIEBACKWARD 1 { event 2001 19 }
$sequence diecrump "BARNEY_Die(Forward)1" X fps 18  ACT_DIESIMPLE 2 { event 2001 20 }

$sequence barnaclehit "barney_barn_jerk" fps 15  ACT_BARNACLE_HIT 1
$sequence barnaclepull "barney_barn_jerk2" loop fps 45  ACT_BARNACLE_PULL 1
$sequence barnaclecrunch "barney_barn_jerk3" fps 5  ACT_BARNACLE_CHOMP 1
$sequence barnaclechew "barney_barn_jerk4" loop fps 5  ACT_BARNACLE_CHEW 1

$sequence lying_on_back "dead_pose_onback" fps 1 
$sequence lying_on_side "dead_pose_onside" fps 1 
$sequence lying_on_stomach "dead_pose_onstomach" fps 1 
