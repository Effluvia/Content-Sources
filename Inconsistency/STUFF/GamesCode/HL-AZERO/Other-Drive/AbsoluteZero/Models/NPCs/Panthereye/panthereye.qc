//pantherEye

$modelname ".\MDL\Panthereye.mdl"
$cd ".\smd"
$cdtexture ".\textures"
$scale 1.0
$cliptotextures

$controller 0 "Bip01 Spine" YR 90 -90
$controller 1 "Bip01 Spine" ZR 0 50

$bbox 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000
$cbox 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000
$eyeposition 0.000000 0.000000 25.000000

//reference mesh(es)
$body "studio" "DIABLO_Template_Model1"

$sequence subtle_motion "DIABLO_Crouch_Idle(Subtle)1" fps 15
$sequence idle_figit "DIABLO_Crouch_Idle(Twitch)2" fps 15
$sequence get_bug "DIABLO_Idle(Getbug)1" fps 15
$sequence itch "DIABLO_Idle(Itch)1" fps 15
$sequence shakes "DIABLO_Idle(TheShakes)1" fps 15

$sequence walk "diablo_walk2" ACT_WALK 1 loop LX fps 15
$sequence run "DIABLO_Run1(Final)" ACT_RUN 1 loop LX fps 15
$sequence turn_left "DIABLO_Turn_Left1" ACT_TURN_LEFT 1 fps 15
$sequence turn_right "DIABLO_Turn_Right1" ACT_TURN_RIGHT 1 fps 15

$sequence crawl_on_belly "DIABLO_Crawl1" loop LX fps 15
$sequence crouch_to_crawl "DIABLO_Crouch_to_Crawl1" loop LX fps 15
$sequence crouch_to_jump "DIABLO_Crouch-Jump1" X fps 15

$sequence flinch "diablo_flinch(light)1" fps 15 ACT_SMALL_FLINCH 1
$sequence flinch_hard "DIABLO_Flinch(Hard)1" fps 15 ACT_BIG_FLINCH 1 

$sequence death_drop "DIABLO_Die(Drop)1" fps 15 ACT_DIESIMPLE 1
$sequence death_simple "DIABLO_Die(simple)1" fps 15 ACT_DIESIMPLE 1
$sequence death_violent "DIABLO_Die(violent)1" fps 15 ACT_DIEVIOLENT 1

$sequence attack_primary "DIABLO_primary_attack1" fps 15 { event 1 8 11 }
$sequence attack_main_claw "DIABLO_Attack1(Main_claw)" fps 15 { event 1 9 }
$sequence attack_simple_claw "DIABLO_Attack2(Simple_Claw)" fps 20 { event 1 10 }

$sequence walk_to_crouch "DIABLO_Walk_to_Crouch1" loop LX fps 15
$sequence walk_to_stand "DIABLO_Walk_to_stand1" loop LX fps 15
$sequence walk_to_stand "DIABLO_Walk_to_Run1" loop LX fps 15

$sequence fall_c1a1b "panther_fall_c1a1b" {
	origin -24 48 0
	AX AY AZR
	fps 30
	{ event 1 10 }
}
