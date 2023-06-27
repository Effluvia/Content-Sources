$bbox -16 -16 0 16 16 72

//Tuned hit boxes. Do not fiddle with!

$hbox 6 "Bip02 L Leg" 0.00 -5.69 -3.55  18.71 3.61 3.43
$hbox 6 "Bip02 L Leg1" 0.00 -3.77 -2.97  16.96 2.21 2.41
$hbox 6 "Bip02 L Foot" -0.07 -2.29 -3.14  3.62 8.17 1.45

$hbox 7 "Bip02 R Leg" 0.00 -5.96 -3.77  18.64 3.58 3.39
$hbox 7 "Bip02 R Leg1" 0.00 -3.85 -2.58  16.97 2.16 2.98
$hbox 7 "Bip02 R Foot" -0.11 -2.29 -1.38  3.61 8.17 2.99

$hbox 3 "Bip02 Pelvis" -6.67 -4.40 -6.46  5.12 5.17 6.83
//$hbox 3 "Bip02 Spine" 0.00 -3.29 -6.83  5.32 0.89 6.91

$hbox 3 "Bip02 Spine" 0.00 -3.29 -6.83  5.32 5.0 6.91

$hbox 2 "Bip02 Spine1" 0.00 -3.90 -6.20  5.70 5.67 5.91
$hbox 2 "Bip02 Spine2" 0.00 -3.65 -5.97  8.99 4.93 5.44

$hbox 4 "Bip02 L Arm" 0.00 -3.17 -1.87  5.47 2.95 1.58
$hbox 4 "Bip02 L Arm1" 0.00 -3.40 -1.95  12.95 2.06 2.46
$hbox 4 "Bip02 L Arm2" 0.00 -2.88 -2.28  11.55 2.10 1.70
$hbox 4 "Bip02 L Hand" 0.00 -1.64 -1.96  4.12 1.79 1.77

$hbox 5 "Bip02 R Arm" 0.00 -3.08 -1.44  5.89 2.97 2.45
$hbox 5 "Bip02 R Arm1" 0.00 -3.35 -2.71  12.92 2.40 2.11
$hbox 5 "Bip02 R Arm2" 0.00 -2.95 -1.97  11.25 2.22 2.45
$hbox 5 "Bip02 R Hand" 0.00 -1.85 -1.77  3.93 1.59 2.08

$hbox 1 "Bip02 Head" 0.00 -3.96 -4.86  10.02 6.23 5.07

//$hbox 0 "Bone01" -4.66 -2.26 -2.25  0.00 0.17 2.04

$eyeposition 0 0 60

/*
==============================================================================
COMMON ANIMATIONS
==============================================================================
*/

$sequence walk "Scientist_Normal_Walk1" lx loop fps 20 origin 0 0 0 scale 1.08 ACT_WALK 1 { event 1004 0 "common/npc_step1.wav" } { event 1004 9 "common/npc_step1.wav" }
$sequence walk_scary "SCI2_Walk_Scared(Footsteps)2" lx loop fps 30 origin 0 -65 0 scale 1.08 ACT_WALK_SCARED 1 { event 1004 0 "common/npc_step1.wav" } { event 1004 8 "common/npc_step1.wav" }
$sequence run "Scientist_Run1" lx loop fps 30 origin 0 0 0 scale 1.08 ACT_RUN 1 { event 1004 0 "common/npc_step1.wav" } { event 1004 8 "common/npc_step1.wav" }
$sequence run_scary "SCI2_Run2(Scared)" lx loop fps 25 origin 0 -15 0 scale 1.08 ACT_RUN_SCARED 1 { event 1004 0 "common/npc_step1.wav" } { event 1004 8 "common/npc_step1.wav" }
$sequence 180_Left "scientist_180-Left1" lx loop fps 30 scale 1.00 ACT_TURN_LEFT 1
$sequence 180_Right "scientist_180-Right1" lx loop fps 15 scale 1.00 ACT_TURN_RIGHT 1
$sequence flinch "scientist_flinch(the_duck)1" fps 30 scale 1.08  ACT_BIG_FLINCH 1
$sequence flinch1 "Scientist_smallflinch2" fps 20 scale 1.0 ACT_SMALL_FLINCH 1
$sequence laflinch "sci2_sfch_larm1" fps 15 scale 1.08  ACT_FLINCH_LEFTARM 1
$sequence raflinch "sci2_sfch_rarm1" fps 15 scale 1.08  ACT_FLINCH_RIGHTARM 1
$sequence llflinch "sci2_sfch_lleg1" fps 15 scale 1.08  ACT_FLINCH_LEFTLEG 1
$sequence rlflinch "sci2_sfch_rleg1" fps 15 scale 1.08  ACT_FLINCH_RIGHTLEG 1
$sequence idle1 "SCI2_Idle1(Subtle)" loop fps 25 scale 1.08 ACT_IDLE 90
$sequence idle_subtle_alpha "Scientist_Idle(Subtle)1" loop fps 20 scale 1.08  ACT_IDLE 20
$sequence idle_brush "Scientist_Idle(Brush_Off)1" fps 20 scale 1.08  ACT_IDLE 3
$sequence idle_look "Scientist_Idle(Look)2" fps 15 scale 1.08  ACT_IDLE 4
$sequence idle_adjust "SCI2_Fidget1(Suit_Adjust)" fps 25 scale 1.08  ACT_IDLE 2
$sequence idle_yawn "SCI2_fidget3(yawn)" fps 20 scale 1.08  ACT_IDLE 1
$sequence crouchstand "SCI2_FearCrouch(up)1" fps 18 scale 1.08  ACT_STAND 1
$sequence crouch_idle "SCI2_FearCrouch1(Idle)" loop fps 18 scale 1.08  ACT_CROUCHIDLE 1
$sequence crouch_idle2 "Scientist_XFCrouch1" loop fps 18 scale 1.00 ACT_CROUCH 1
$sequence crouch_idle3 "Scientist_XFCrouch2" loop fps 18 scale 1.00 ACT_CROUCH 1
$sequence panic "Scientist_XPanic1" fps 18 scale 1.08 ACT_EXCITED 1
$sequence fear1 "SCI2_Reaction1" fps 18 scale 1.08 ACT_FEAR_DISPLAY 1
$sequence fear2 "SCI2_Reaction(Terror)1" fps 18 scale 1.08 ACT_FEAR_DISPLAY 1
$sequence eye_wipe "SCI2_Reaction(Eye_Wipe)1" fps 18 scale 1.08 ACT_FEAR_DISPLAY 1
$sequence pull_needle "SCI2_Pull_Needle1" fps 15 scale 1.08 ACT_ARM 1 { event 2 25 }
$sequence return_needle "SCI2_Return_Needle1" fps 15 scale 1.08 ACT_DISARM 1 { event 3 11 }
$sequence give_shot "SCI2_Give_Shot1" fps 15 scale 1.08 ACT_MELEE_ATTACK1 1 { event 1 17 }
$sequence punch "scientist_punch" fps 30 scale 1.0
/*
==============================================================================
DEATH ANIMATIONS
==============================================================================
*/

$sequence diesimple "die_simple" fps 24 scale 1.0 ACT_DIESIMPLE 1 { event 2001 12 }
$sequence dieheadshot "SCI2_SDTH_Headsht1" fps 15 scale 1.08 ACT_DIE_HEADSHOT 1 { event 2001 18 }
$sequence dieviolent "Scientist_Die(Violent)1" fps 15 scale 1.08 ACT_DIEVIOLENT 1 { event 2001 12 }
$sequence diebackward "Scientist_Die(Backward)1" fps 20 scale 1.08 ACT_DIEBACKWARD 1 { event 2001 32 }
$sequence dieforward "Scientist_Die(Forward)1" fps 20 scale 1.08 ACT_DIEFORWARD 1 { event 2001 35 }
$sequence diefast "scientist_diefast" fps 20 scale 1.00 ACT_DIE_GUTSHOT 1 { event 2001 17 }

/*
==============================================================================
DEAD POSES
==============================================================================
*/
$sequence lying_on_back "dead_pose_onback" scale 1.0
$sequence lying_on_stomach "dead_pose_onstomach" scale 1.0
$sequence dead_sitting "dead_pose_sitting" scale 1.0
$sequence dead_table1 "dead_tablepose_1" scale 1.0
$sequence dead_table2 "dead_tablepose_2" scale 1.0
$sequence dead_table3 "dead_tablepose_3" scale 1.0
/*
==============================================================================
GETTING DEVOURED BY BARNACLE ANIMATIONS
==============================================================================
*/
$sequence barnacled1 "scientist_barnacled1" fps 18 scale 1.08 ACT_BARNACLE_HIT 1
$sequence barnacled2 "scientist_barnacled2" loop fps 28 scale 1.08 ACT_BARNACLE_PULL 1
$sequence barnacled3 "scientist_barnacled3" fps 18 scale 1.08 ACT_BARNACLE_CHOMP 1
$sequence barnacled4 "scientist_barnacled4" loop fps 15 scale 1.08 ACT_BARNACLE_CHEW 1

/*
==============================================================================
PRE DISASTER ANIMATIONS
==============================================================================
*/
$sequence console "console_crazy" loop fps 18 scale 1.0
$sequence checktie "check_tie" fps 12 scale 1.08 {
	{ event 1008 1 "scientist/weartie.wav" }
}
$sequence dryhands "dry_hands" fps 12 scale 1.08
$sequence tieshoe "tie_shoe" fps 12 scale 1.08
$sequence writeboard "scientist_write_board" fps 14 scale 1.08
$sequence studycart "study_pushcart" fps 12 scale 1.08
$sequence lean "lean_computer" fps 12 scale 1.0
$sequence pondering "scientist_pondering" fps 12 scale 1.0
$sequence pondering2 "scientist_pondering2" fps 15 scale 1.0
$sequence pondering3 "scientist_pondering3" fps 12 scale 1.08
$sequence buysoda "scientist_buysoda" fps 15 scale 1.08 {
  { event 1004 50 "*buttons/blip1.wav" }
  { event 1004 81 "*buttons/latchunlocked2.wav" }
  { event 1004 88 "*buttons/latchunlocked1.wav" }
 }
$sequence pause "scientist_pause" fps 18 scale 1.08
$sequence yes "scientist_yes" fps 16 scale 1.00
$sequence no "scientist_no" fps 16 scale 1.00
$sequence push_button "scientist_push_button" fps 16 scale 1.0
$sequence converse1 "scientist_converse1"  fps 13 scale 1.0
$sequence converse2 "scientist_converse2"  fps 13 scale 1.0
$sequence retina "retina_scan" fps 18 scale 1.0
$sequence talkleft "c1a0_idleleft" loop fps 12 scale 1.0
$sequence talkright "c1a0_idleright" loop fps 14 scale 1.0
$sequence deskidle "c1a0_desk_idle" loop fps 20 scale 1.0
$sequence coffee "coffee_break" loop fps 14
$sequence franticbutton "frantic_pushbuttons" fps 14 scale 1.08
$sequence startle "startle" fps 16 scale 1.08
/*
==============================================================================
SITTING ANIMATIONS
==============================================================================
*/
$sequence sitlookleft "scientist_sitlookleft" fps 10 scale 1.0 origin 0 0 39
$sequence sitlookright "scientist_sitlookright" fps 10 scale 1.0 origin 0 0 39
$sequence sitscared "scientist_sitscared" fps 25 scale 1.0 origin 0 0 39
$sequence sitting2 "scientist_sitting2" fps 20 scale 1.0 origin 0 0 39
$sequence sitting3 "scientist_sitting3" fps 15 scale 1.0 origin 0 0 39

/*
==============================================================================
SCRIPTED/CUT/UNUSED ANIMS
==============================================================================
*/

//$sequence scientist_panic1 "SCI2_Panic1" fps 10 scale 1.0
//$sequence scientist_scarywalk "SCI2_Walk_Scared(Footsteps)2" lx fps 30 origin 0 -65 0 scale 1.0
//$sequence scientist_normalwalk "Scientist_Normal_Walk1" lx fps 30 origin 0 0 0 scale 1.0
//$sequence scientist_alphawalk "Scientist_Walk1" lx fps 30 origin 0 0 0 scale 1.0
//$sequence scientist_run1 "SCI2_Run1" lx fps 30 origin 0 -15 0 scale 1.0
//$sequence scientist_run2scared "SCI2_Run2(Scared)" lx fps 30 origin 0 -15 0 scale 1.0
//$sequence scientist_oldrun1 "Scientist_Run1" lx fps 30 origin 0 0 0 scale 1.0

//C1A1 - Going to make a wild guess and say that all the non-labelled anims were meant for early chapters or unnamed ones
$sequence pantherscientist_fall "scientist_fall_c1a1b" AX AY AZR fps 25 { event 1000 1 } { event 1008 52 "*scientist/scream7.wav" } { event 1008 71 "*scientist/sci_pain7.wav" } { event 2001 71 }
//Panthereye, original version
$sequence bulljumpscientist "scientist_bulljump" AX AY AZR fps 25 { event 1000 1 } { event 1008 52 "*scientist/scream7.wav" } { event 1008 71 "*scientist/sci_pain7.wav" } { event 2001 71 }
//Bullsquid version, edited from the panthereye animation

$sequence cowering_in_corner "scientist_cowering" loop fps 18 scale 1.08 
// Animated coat bones, original qci points for a gameover? wut

$sequence c1a0_catwalkidle "c1a0_fallcatwalk(idle)" loop fps 12 scale 1.0
$sequence c1a0_catwalk "c1a0_fallcatwalk" AX AY AZR fps 16 {
  { event 1000 1 } 
  { event 1008 1 "*scientist/c1a0_sci_stayback.wav" } 
  { event 1008 43 "*scientist/c1a0_sci_catscream.wav" } 
} 
$sequence fall "fall" fps 20 { event 1000 0 } { event 1008 20 "*scientist/scream3.wav" } scale 0.972


//Pre map rename label, kinda neato


$sequence sstruggleidle "headcrab_struggle(idle)" loop fps 18
$sequence sstruggle "headcrab_struggle" fps 28 {
	{ event 1000 1 }
}
$sequence headcrabbed "headcrab_scientist" {
	origin 0 0 0 
	fps 20 
	{ event 1000 165 } 
	scale 1.08
}
$sequence get_stomped "scientist_getting_stomped" AX AY AZR fps 24 { event 1000 47 } { event 1001 0 }
$sequence scicrashidle "scientist(idle)" fps 34 loop
$sequence scicrash "scientist(crash)" fps 20 { event 1000 1 }
$sequence scientist_idlewall "scientist_idlewall" fps 14 loop origin 0 -36 0
$sequence scientist_get_pulled "scientist_get_pulled" AX AY AZR fps 17 { event 1000 1 } origin 0 -36 0
$sequence sitidle "c1a1b_sit(idle)" loop fps 14 scale 1.0 origin 0 -16 0
$sequence sitstand "c1a1b_sit(stand)" fps 18 scale 1.0 origin 0 -16 0
$sequence sitstand_supplies "c1a1b_sit(stand)" fps 18 scale 1.0 origin 0 -16 0 {
{ event 1008 22 "*scientist/hellothere.wav" } 
{ event 1008 65 "*scientist/protectme.wav" } //Did I tell you how bad I am at scripted_sentences? - LeonelC
}

//C1A2 - Going to make a wild guess and say that all the non-labelled anims were meant for early chapters or unnamed ones

$sequence ceiling_dangle "ceiling_dangle" AX AY AZ fps 15 {
  origin 1032 -312 -472
  { event 1000 1 } 
  { event 1001 1 } 
  { event 1004 5 "*scientist/sci_dragoff.wav" } 
} scale 1.0

$sequence crawlwindowdead "Scientist_crawlwindow" fps 20 scale 1.0
$sequence crawlwindow "Scientist_crawlwindow2" fps 20 scale 1.0
$sequence locked_door "Scientist_berserk_door" fps 20 scale 1.00 {
	{ event 1004 11 "buttons/latchlocked1.wav" }
	{ event 1004 21 "buttons/latchlocked2.wav" }
	{ event 1004 31 "buttons/latchlocked1.wav" }
}
$sequence pulldoor "scientist_pulldoor" fps 18 scale 1.08  {
	{ event 1004 11 "buttons/latchlocked2.wav" }
	{ event 1004 26 "buttons/latchlocked1.wav" }
}
$sequence jumpshockidle "jump_shockwater(idle)" fps 14 loop scale 1.0
$sequence jumpshock "jump_shockwater" fps 26 {
  { event 1000 0 } 
  { event 1004 31 "weapons/electro4.wav" } 
  { event 1004 38 "*debris/zap4.wav" } 
}

$sequence ventpull1 "high_ventpull(sci1)" AX AY AZ fps 15 {
  { event 1000 1 } 
  { event 1001 1 } 
  { event 1004 8 "*player/pl_metal1.wav" } 
  { event 1004 16 "*player/pl_metal2.wav" } 
  { event 1004 28 "*player/pl_metal4.wav" } 
  { event 1004 31 "*player/pl_metal1.wav" } 
} scale 1.0

$sequence ventpull2 "high_ventpull(sci2)" AX AY AZ fps 15 	origin 0 -28 77 { event 1000 1 } { event 1001 1 } scale 1.0
$sequence ventpullidle1 "high_ventpull(idle1)" fps 15 loop scale 1.0
$sequence ventpullidle2 "high_ventpull(idle2)" 	origin 0 -28 77 fps 15 loop scale 1.0

$sequence dying_friend "dying_barney_c1a3" fps 18 {
	{ event 1000 1 }
}
$sequence dying_friendidle "dying_barney_c1a3(idle)" loop fps 12

//BOOO YOU STINK
//$sequence c1a2_dying_speech "c1a4_dying_speech" fps 15 scale 1.0 {
//  { event 1000 1 } 
//  { event 1008 0 "*scientist/c1a2_sci_elevator.wav" } 
//} 

//Custon anim since LeonelC cannot into scripted_sentences

$sequence beatdoor "scientist_beatdoor" fps 16 scale 1.0 {
	{ event 1004 8 "*buttons/latchunlocked1.wav" }
	{ event 1004 19 "*buttons/latchunlocked2.wav" }
}

$sequence divewindow "scientist_divewindow" fps 30 scale 1.08 { event 1001 1 }
$sequence fall_elevator "fall_elevator" fps 20 { event 1000 0 } { event 1008 34 "*scientist/scream3.wav" }
$sequence cprscientist "cpr_scientist" fps 20 loop origin 20 6 0
$sequence cprscientistrevive "cpr_scientist(revive)" fps 20 origin 20 6 0

//C1A3

$sequence bodydrag "sci_dragbody_fixed" fps 30 { event 1000 1 }

//C1A4

$sequence tentacle_grab "Scientist_tentacle_grab" {
	origin -406.115570 340.254669 419.5
	rotate -15
	fps 15 
	{ event 1000 1 }
	{ event 1004 2 "*scientist/scream02.wav" }
	{ event 1004 65 "*scientist/c1a4_sci_tent.wav" }
}
Scientist_tentacle_grab
$sequence c1a4_wounded_idle "c1a4_wounded_idle" fps 12 loop scale 1.0

$sequence c1a4_dying_speech "c1a4_dying_speech" fps 15 scale 1.0 {
  { event 1000 1 } 
  { event 1008 10 "*scientist/c1a4_sci_rocket.wav" } 
  { event 1008 105 "*scientist/sci_pain4.wav" } 
}


//??? - Not only unknown anims, but general scripted animations

$sequence "headhole" "headhole" fps 15
// so old it doesn't even have die events
$sequence "corpsefloat1" "corpsefloat1(back)" fps 10 loop
$sequence "corpsefloat2" "corpsefloat2(stomach)" fps 10 loop
$sequence "hide_in_vent" "hide_in_vent" AX AY AZR fps 24
$sequence scientist_pepsi_idle "pepsi_balance(idle)" fps 20 scale 1.00 
// could be meant for C2A5

$sequence scientist_pepsi "pepsi_balance" fps 20 scale 1.00 
// could be meant for C2A5

$sequence scientist_rocketcrawl "rocket_crawl" fps 20 scale 1.00 
$sequence haulscientist "haul_scientist" fps 18 scale 1.08 {
	origin 15 0 0
	{ event 1000 1 }
	{ event 1001 1 }
}
// C1A4-C2A2
$sequence scientist_leanhandrailidle "lean_handrail(idle)" fps 20 scale 1.00 
//could be C1A1 since it's between the first lines of the .qci

$sequence scientist_leanhandrail "lean_handrail" fps 20 scale 1.00 
//could be C1A1 since it's between the first lines of the .qci

$sequence scientist_slump "deadslump" fps 20 scale 1.00 { event 1000 1 } 
//??? - Corpse resting on a horizontal openable door, moving the corpse along with it, could be for C1A3

$sequence wave "scientist_wave" fps 22 scale 1.0
//Could be anything, ai anim, c0a0 anim, who knows? currently used on our C1A2 and C1A1

$sequence hanging_idle "hang_idle" fps 20 loop
//Leaving it here since it's being used in both C1A1 and C1A2

$sequence assassin "assassin_scientist" AX AY AZR fps 15 {
{ event 1008 31 "scientist/sci_pain1.wav" } 
{ event 1000 1 }
}
//Everywhere from C1A3 onwards

$sequence "body_be_dragged" "Sci_bodydrag" fps 30 { event 1000 0 }
//Everywhere from C1A3 onwards, tart fix the mouth bone


$sequence "keypad" "Scientist_Keypad_New" fps 18 {
  { event 1008 15 "*buttons/blip1.wav" } 
  { event 1008 24 "*buttons/blip1.wav" } 
  { event 1008 34 "*buttons/blip1.wav" } 
  { event 1008 44 "*buttons/blip1.wav" } 
 }
$sequence quicklook "scientist_quicklook" fps 16 scale 1.0
$sequence "bangdoor" "bangdoor" fps 20 scale 1.0
//C1A2? Seems he is banging from a door with a window to see through, C1A2 and C2A4 comes to mind

//C2A2
$sequence "kneel" "kneel_dead" fps 22

//C2A3

$sequence slip_n_fall "c2a3_slip_n_fall" fps 36 {
  { event 1000 160 } 
  { event 1008 184 "*Player/pl_slosh3.wav" } 
  { event 1008 250 "*Player/pl_slosh4.wav" } 
  { event 1008 310 "*Player/pl_slosh1.wav" } 
 }
 
$sequence swimming_and_eaten "c2a3_swimming_and_eaten" fps 30
//Empty frames at the end of the anim, looks like a follow-up of slip_n_fall

$sequence compthrow_idle "c2a3_comp_throw_idle_sci" AX AY AZR fps 30 loop rotate 180 { event 1008 12 "*scientist/sci_pain4.wav" } { event 1001 1 }
//Minor offset issues
$sequence compthrow "c2a3_comp_throw_sci" AX AY AZR fps 30 rotate 180 {
  { event 1001 1 } 
  { event 1000 1 } 
  { event 1008 9 "*scientist/sci_pain2.wav" } 
  { event 1004 32 "*debris/zap6.wav" } 
}
//Minor offset issues

$sequence ickycage_scientist "ickyjump_scientist" fps 15 scale 1.00 rotate -90 {
	{ event 1000 1 }
	{ event 1001 1 }
}

//C2A5

$sequence windive "c2a5e_windive" fps 30 scale 1.08
$sequence scientist_windive "windive" fps 20 scale 1.08 

//C3A2
$sequence gluonidle "c3a2_gluonidle" AX AY AZR fps 24 scale 1.08
$sequence gluoncrawl "c3a2_gluon_crawl" AX AY AZR fps 24 scale 1.08
$sequence gluonshow "c3a2_gluon_intro" fps 18 scale 1.08
$sequence gluonup "c3a2_gluonup" AX AY AZR fps 24 scale 1.08
$sequence psstidle "psstidle" fps 5 loop
$sequence psst "c3a2_psst" AX AY AZR fps 24
$sequence portal "portal_c3a2d" fps 20 loop {
  { event 1008 19 "*buttons/blip1.wav" } 
  { event 1008 22 "*buttons/blip1.wav" } 
  { event 1008 25 "*buttons/blip2.wav" } 
  { event 1008 32 "*buttons/blip2.wav" } 
  { event 1008 47 "*buttons/blip1.wav" } 
  { event 1008 98 "*buttons/blip2.wav" } 
  { event 1008 102 "*buttons/blip1.wav" } 
  { event 1008 110 "*buttons/blip2.wav" } 
  { event 1008 119 "*buttons/blip1.wav" } 
} scale 1.0

$sequence "pratfall" "pratfall" fps 36 {
  { event 1008 15 "*scientist/sci_fear5.wav" } 
  { event 1008 55 "*scientist/sci_pain1.wav" } 
  { event 1008 65 "*common/bodydrop4.wav" } 
  { event 1000 60 } 
 } scale 0.972

/*
==============================================================================
JUNK
==============================================================================

//$sequence walk "Scientist_XWalk1" lx loop fps 20 origin 0 -7 0 scale 1.00 ACT_WALK 1 {
//	{ event 1004 0 "NPC.StepLeftWalk" }
//	{ event 1004 8 "NPC.StepRightWalk" }
// }

//$sequence walk_scared "Scientist_XScared-Walk1" lx loop fps 20 origin 0 -7 0 scale 1.00 ACT_WALK_SCARED 1 {
//	{ event 1004 0 "NPC.StepLeftWalk" }
//	{ event 1004 8 "NPC.StepRightWalk" }
// }

//$sequence run "Scientist_XXRun1" lx loop fps 25 scale 1.00 ACT_RUN 1 {
//	{ event 1004 1 "NPC.StepLeftRun" }
//	{ event 1004 10 "NPC.StepRightRun" }
// }

//$sequence run1 "Scientist_XRun2(Scared)" lx loop fps 30 scale 1.00 ACT_RUN_SCARED 1 {
//	{ event 1004 1 "NPC.StepLeftRun" }
//	{ event 1004 10 "NPC.StepRightRun" }
//}


//$sequence run2 "Scientist_XRun3(Look)" lx loop fps 30 scale 1.00 ACT_RUN_SCARED 1 {
//	{ event 1004 1 "NPC.StepLeftRun" }
//	{ event 1004 10 "NPC.StepRightRun" }
// }
//$sequence scientist_fidget2 "Scientist_S-Fidget2" fps 20 scale 1.00
//$sequence idle1 "Scientist_XIdle(Subtle)1" loop fps 18 scale 1.00 ACT_IDLE 50
//$sequence idle3 "SCI2_Fidget1(Look_Up)" fps 15 scale 1.00 ACT_IDLE 1
//$sequence idle4 "SCI2_Fidget1(Look_Down)" fps 15 scale 1.00 ACT_IDLE 2
//$sequence idle5 "SCI2_Fidget1(Look_Right)" fps 15 scale 1.00 ACT_IDLE 1
//$sequence idle6 "SCI2_Fidget1(Look_Left)" fps 15 scale 1.00 ACT_IDLE 1
//$sequence idle7 "Scientist_XFidget5(WShift)" fps 10 scale 1.00 ACT_IDLE 10

// FIXME $sequence crouch_idle3 "Scientist_XFCrouch3" loop fps 18 scale 1.00 ACT_CROUCHIDLE 1

*/