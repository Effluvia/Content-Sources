/*
==============================================================================

shared info between barney and g_barney

==============================================================================
*/

//$sequencegroupsize 64

// muzzleflash
$attachment 0 "bip01 R hand" 15 3.5 5 X 1

$hbox 3 "Bip01 Pelvis" -9.89 -5.66 -8.60  2.94 6.52 6.79

$hbox 6 "Bip01 L Leg" 0.00 -5.92 -4.00  19.37 3.78 3.73
$hbox 6 "Bip01 L Leg1" 0.00 -5.69 -3.62  18.83 3.07 2.94
$hbox 6 "Bip01 L Foot" 0.00 -2.99 -4.04  6.42 8.68 1.61

$hbox 7 "Bip01 R Leg" 0.00 -5.93 -3.87  19.27 3.75 3.79
$hbox 7 "Bip01 R Leg1" -0.33 -5.73 -2.98  18.95 2.96 3.19
$hbox 7 "Bip01 R Foot" 0.00 -3.24 -2.32  6.21 8.44 3.28

$hbox 3 "Bip01 Spine" 0.00 -4.89 -6.97  8.37 8.00 6.73
$hbox 2 "Bip01 Spine2" 0.00 -3.46 -8.32  6.11 8.00 7.53
$hbox 2 "Bip01 Spine3" -1.00 -5.99 -8.22  5.96 8.00 8.48

$hbox 4 "Bip01 L Arm" -0.40 -4.18 0.00  7.20 4.75 4.20
$hbox 4 "Bip01 L Arm1" -2.52 -4.57 -3.92  9.86 2.28 3.43
$hbox 4 "Bip01 L Arm2" 0.00 -2.85 -2.08  11.90 2.85 2.75
$hbox 4 "Bip01 L Hand" 0.00 -1.32 -2.29  4.66 2.60 1.65

$hbox 5 "Bip01 R Arm" 0.00 -4.15 -4.19  7.53 4.12 4.24
$hbox 5 "Bip01 R Arm1" -2.69 -4.65 -3.42  9.47 2.28 3.76
$hbox 5 "Bip01 R Arm2" 0.00 -2.73 -2.56  11.00 3.14 2.19
$hbox 5 "Bip01 R Hand" 0.00 -1.32 -2.91  10.38 3.23 4.00

//$hbox 1 "Bip01 Head" -0.66 -4.25 -5.24  11.19 7.97 4.63

$hbox 1 "Bip01 Head" -0.66  2.25 -4.24  6.00 7.97 3.63
$hbox 10 "Bip01 Head" -0.66 -4.25 -5.24  6.00 2.25 4.63
$hbox 10 "Bip01 Head"  6.00 -4.25 -5.24  11.19 7.97 4.63

//$hbox 0 "Bone05" 0.00 -2.47 -2.29  3.73 0.00 2.12
//$hbox 0 "Bone03" -2.69 -14.48 -6.05  1.60 4.30 2.29

/*
==============================================================================
COMMON ANIMATIONS
==============================================================================
*/
$sequence idle1 "BARNEY_Idle1" fps 15 loop  ACT_IDLE 1
$sequence idle2 "BARNEY_Idle2" fps 15  ACT_IDLE 1
$sequence idle3 "BARNEY_Idle3(soft&subtle)" fps 15  ACT_IDLE 50 loop
$sequence idle4 "BARNEY_Idle4(Subtle_Look)" fps 12  ACT_IDLE 10
$sequence walk "BARNEY_Walk(New&Improved)1" LX fps 30  loop ACT_WALK 1 {
	{ event 1004 0 "common/npc_step1.wav" }
	{ event 1004 12 "common/npc_step3.wav" }
}
$sequence run "BARNEY_Run(New&Improved)2" LX fps 30 loop  ACT_RUN 1 {
	{ event 1004 0 "common/npc_step2.wav" }
	{ event 1004 8 "common/npc_step4.wav" }
}
$sequence shootgun "barno2_shoot_down45" "barno2_shoot_up45" fps 8 blend XR -50 50 ACT_RANGE_ATTACK1 1 { event 5001 0 "21" } { event 3 0 }
//$sequence shootgun2 "BARNEY2_shoot1" fps 20 blend XR -50 50 ACT_RANGE_ATTACK1 2 { event 5001 0 "21" } { event 3 0 }
$sequence draw "BARNEY2_NEW_Draw1" fps 20  ACT_ARM 1 { event 2 7 } 
$sequence disarm "Barno_Reholster_Final" fps 10  ACT_DISARM 1 { event 4 15 }
$sequence reload "BARNEY2_Reload" fps 15  ACT_RELOAD 1 
$sequence turnleft "BARNEY2_180-Left1" fps 15  ACT_TURN_LEFT 1
$sequence turnright "BARNEY2_180-Right1" fps 15  ACT_TURN_RIGHT 1
//$sequence laflinch "BARNEY-X_SFCH_larm1" fps 16  ACT_FLINCH_LEFTARM 1
//$sequence raflinch "BARNEY-X_SFCH_rarm1" fps 16  ACT_FLINCH_RIGHTARM 1
//$sequence llflinch "BARNEY-X_SFCH_lleg1" fps 20  ACT_FLINCH_LEFTLEG 1
//$sequence rlflinch "BARNEY-X_SFCH_rleg1" fps 20  ACT_FLINCH_RIGHTLEG 1
$sequence smlflinch "barney_sml_flinch" fps 30  ACT_SMALL_FLINCH 1 scale 0.96

$sequence cower_stand_idle "barney_cower_stand" fps 14 loop scale 0.96
$sequence locked_door "barney_locked_door" fps 28 loop 
$sequence fall_loop "barney_fall_loop" fps 28 loop scale 0.96
$sequence barn_wave "barney_wave" fps 22 scale 0.96
$sequence beat_grunt "barney_beats_grunt" fps 14
$sequence beat_gruntidle "barney_beats_grunt(idle)" loop fps 14
$sequence flashlight "barney_flashlight" loop fps 14  {
	{ event 1004 5 "*buttons/latchunlocked2.wav" }
}
/*
==============================================================================
DEATH ANIMATIONS
==============================================================================
*/
$sequence diesimple "BARNEY_Die(Simple)1" X fps 15  ACT_DIE_HEADSHOT 1 { event 2001 5 }
$sequence dieviolent "BARNEY_Die(Violent)1" X fps 15 frames 0 26  ACT_DIE_HEADSHOT 1 { event 2001 10 }
//$sequence diegutshot "BARNEY-X_SDTH_GUTSHOT1" fps 15  ACT_DIE_GUTSHOT 1 { event 2001 33 }
$sequence dieforward "BARNEY_Die(Forward)1" X fps 15  ACT_DIEFORWARD 1 { event 2001 8 }
$sequence diebackward "BARNEY_Die(Backward)1" X fps 15  ACT_DIEBACKWARD 1 { event 2001 19 }
$sequence diecrump "BARNEY_Die(Forward)1" X fps 15  ACT_DIESIMPLE 2 { event 2001 20 }
/*
==============================================================================
GETTING DEVOURED BY BARNACLE
==============================================================================
*/
$sequence barnaclehit "barney_barn_jerk" fps 15  ACT_BARNACLE_HIT 1
$sequence barnaclepull "barney_barn_jerk2" loop fps 45  ACT_BARNACLE_PULL 1
$sequence barnaclecrunch "barney_barn_jerk3" fps 5  ACT_BARNACLE_CHOMP 1
$sequence barnaclechew "barney_barn_jerk4" loop fps 5  ACT_BARNACLE_CHEW 1
/*
==============================================================================
DEAD POSES
==============================================================================
*/
$sequence lying_on_back "dead_pose_onback" fps 1 
$sequence lying_on_side "dead_pose_onside" fps 1 
$sequence lying_on_stomach "dead_pose_onstomach" fps 1 
$sequence stuffed_in_vent "dead_pose_vent" fps 1 
/*
==============================================================================
SCRIPTED ANIMATIONS
==============================================================================
*/
//$sequence standing_idle "relaxed_stance_c1a0" loop fps 15 
$sequence standing_idle "barn_attention_idle" loop fps 15
$sequence standing_looking "barn_attention_looking" loop fps 5
$sequence standing_halt "barn_attention_halt" fps 15
$sequence cprbarney "cpr_barney" loop fps 20 
$sequence cprbarneyrevive "cpr_barney(revive)" fps 20  origin 0 3 0
$sequence barney_dragvent "barney_dragvent" fps 15  origin 664 -904 16 {
	{ event 1000 1 }
}
$sequence dying_barney "dying_barney_c1a3" fps 18 scale 0.96 {
	{ event 1000 1 }
}
$sequence dying_barneyidle "dying_barney_c1a3(idle)" loop fps 12 scale 0.96
$sequence dying_friend "dying_barneys_friend" fps 18 scale 0.96
$sequence dying_friendidle "dying_barneys_friend(idle)" loop fps 12 scale 0.96
$sequence c1a3_bidle "scene_barney_getpulled(idle)" fps 3  {
        origin 0 14 0
	AX AY AZR
}
$sequence c1a3_ventb "scene_barney_getpulled" fps 18  {
	{ event 1000 1 }
        origin 0 14 0
	AX AY AZR
}
$sequence c1a3_emergeidle "scene_barney_emerge(idle)" loop fps 16  {
        origin -113 -60 0
	AX AY AZR
}
$sequence c1a3_emerge "scene_barney_emerge" fps 16  {
	{ event 1000 1 }
        origin -113 -60 0
	AX AY AZR
}
$sequence haulbarney "haul_barney" fps 18  {
	origin -16 32 0
	{ event 1000 1 }
	AX AY AZR
}
$sequence intropush "intro_pushbutton" fps 16  {
	{ event 1004 15 "*buttons/blip1.wav" }
	{ event 1004 24 "*buttons/blip1.wav" }
	{ event 1004 34 "*buttons/blip1.wav" }
	{ event 1004 44 "*buttons/blip1.wav" }
} scale 0.94
$sequence fence "electrified_fence" fps 24  {
	{ event 1000 70 }
	{ event 1003 70 "lightning" }
	{ event 1001 70 }
	{ event 1004 72 "weapons/electro4.wav" }
	{ event 1008 100 "barney/ba_pain2.wav" }
	AX AY AZR
} scale 0.96
$sequence sit1 "sit_desk_idle1" loop fps 20  origin 0 -40 36 {
	{ event 1003 50 "introchair" }
	{ event 1003 102 "introchair" }
}
$sequence almostidle "c1a1_dyingmedkit(idle)" loop fps 16 
$sequence almost "c1a1_dyingmedkit" fps 26  {
	{ event 1000 1 }
}
$sequence "barney_fight" "barney_zombie_fight_B" AX AY AZR fps 17 {
  { event 1000 1 } 
  { event 1004 131 "weapons/cbar_hitbod1.wav" } 
  { event 1004 147 "weapons/cbar_hitbod2.wav" } 
  { event 1004 159 "weapons/cbar_hitbod3.wav" } 
  { event 1004 169 "weapons/cbar_hitbod1.wav" } 
  { event 1004 216 "weapons/xbow_hitbod1.wav" } 
  { event 1003 216 "splatbz" } 
 }
 
$sequence "sterilizer_idle" "c2a4_sterilizer_Barney_idle" AX AY AZR loop fps 17 scale 0.96
$sequence "sterilizer" "barney_c2a4_sterilizer" fps 17
$sequence "c1a0_lean_idle" "c1a0_lean(idle)" AX AY AZR fps 17 scale 0.96 loop
$sequence "c1a0_lean" "c1a0_lean" AX AY AZR fps 17 scale 0.96
$sequence "stuka_warn" "stuka_warn" AX AY AZR fps 17


//C1a4 train scripted animations

$sequence "train_assist_idle" "barney2_squatting_kibbitz" AX AY AZR fps 17
$sequence "train_assist" "barney2_c1a4_traintrack_assist" AX AY AZR fps 17
$sequence "train_flung" "c1a4_barney_flung" AX AY AZR fps 36 scale 0.96

//Construction is going to play this
//$sequence "train_escape_idle" "inspect_floor" AX AY AZR fps 17
//$sequence "train_escape" "barney2_c1a4_traintrack_escape" AX AY AZR fps 17

//$sequence laseridle "lasercut_pose" fps 6
//$sequence laser_top "lasercut_top" fps 6 {
//	{ origin 0 2 24 }
//	{ event 1000 0 }
//}
//$sequence laser_bottom "lasercut_bottom" fps 6 {
//	{ origin 0 -2 0 }
//	{ event 1000 0 }
//}

//$sequence barneyfallidle "barney(idle)" loop fps 33 
//$sequence barneyfall "barneyfall" fps 32  {
	{ event 1000 1 }
	{ event 1008 2 "barney/ba_pain1.wav" }
	{ event 1008 24 "barney/ba_pain2.wav" }
	{ event 1003 55 "bsplat" }
	{ event 1008 55 "debris/bustflesh1.wav" }

//}
//$sequence c3a2_draw "c3a2_suspectdraw" fps 16 {
//	{ event 2 13 }
//}
//$sequence unlatch "barney_unlatch" fps 12  {
//	{ event 1003 7 "barney_door_lock" }
//}
//$sequence retina "retinal_scanner" fps 20  origin 0 -7 0
//$sequence relaxstand "relaxed_stance_c1a0" loop fps 10 
//$sequence assassinated "barney_Dieforward" X fps 18  {
//	{ event 1000 1 }
//	{ event 1003 1 "blood1" }
//	{ event 1003 2 "blood2" }
//}
//$sequence trackswitch "track_switch" fps 22  {
//	{ event 1003 14 "trainarmswitch" }
//}
$sequence pepsiswing "pepsiswing" fps 25 {
	{ event 1003 1 "die" }
	{ event 1003 8 "swing" }
}
$sequence pepsipush "pepsipush" X fps 18 {
	{ event 1003 51 "topple" }
	{ event 1003 90 "horror" }
}
$sequence hambone "hambone" fps 30 scale 1
//$sequence buttonpush "button2" fps 20 {
//	{ event 1003 7 "button" }
//}