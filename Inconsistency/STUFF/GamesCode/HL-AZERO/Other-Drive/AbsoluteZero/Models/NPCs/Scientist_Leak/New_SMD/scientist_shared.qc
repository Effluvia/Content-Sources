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
$sequence walk "Scientist_XWalk1" lx loop fps 20 origin 0 -7 0 scale 1.00 ACT_WALK 1 {
	{ event 1004 0 "NPC.StepLeftWalk" }
	{ event 1004 8 "NPC.StepRightWalk" }
}
$sequence walk_scared "Scientist_XScared-Walk1" lx loop fps 20 origin 0 -7 0 scale 1.00 ACT_WALK_SCARED 1 {
	{ event 1004 0 "NPC.StepLeftWalk" }
	{ event 1004 8 "NPC.StepRightWalk" }
}
$sequence run "Scientist_XXRun1" lx loop fps 25 scale 1.00 ACT_RUN 1 {
	{ event 1004 1 "NPC.StepLeftRun" }
	{ event 1004 10 "NPC.StepRightRun" }
}
$sequence run1 "Scientist_XRun2(Scared)" lx loop fps 30 scale 1.00 ACT_RUN_SCARED 1 {
	{ event 1004 1 "NPC.StepLeftRun" }
	{ event 1004 10 "NPC.StepRightRun" }
}
$sequence run2 "Scientist_XRun3(Look)" lx loop fps 30 scale 1.00 ACT_RUN_SCARED 1 {
	{ event 1004 1 "NPC.StepLeftRun" }
	{ event 1004 10 "NPC.StepRightRun" }
}
$sequence 180_Left "scientist_180-Left1" lx loop fps 30 scale 1.00 ACT_TURN_LEFT 1
$sequence 180_Right "scientist_180-Right1" lx loop fps 15 scale 1.00 ACT_TURN_RIGHT 1
$sequence flinch "scientist_flinch(the_duck)1" fps 30 scale 1.08 ACT_BIG_FLINCH 1
$sequence flinch1 "scientist_smallflinch1" fps 20 scale 1.0 ACT_SMALL_FLINCH 1
$sequence laflinch "sci2_sfch_larm1" fps 15 scale 1.08 ACT_FLINCH_LEFTARM 1
$sequence raflinch "sci2_sfch_rarm1" fps 15 scale 1.08 ACT_FLINCH_RIGHTARM 1
$sequence llflinch "sci2_sfch_lleg1" fps 15 scale 1.08 ACT_FLINCH_LEFTLEG 1
$sequence rlflinch "sci2_sfch_rleg1" fps 15 scale 1.08 ACT_FLINCH_RIGHTLEG 1
$sequence idle1 "Scientist_XIdle(Subtle)1" loop fps 18 scale 1.00 ACT_IDLE 50
$sequence idle3 "SCI2_Fidget1(Look_Up)" fps 15 scale 1.00 ACT_IDLE 1
$sequence idle4 "SCI2_Fidget1(Look_Down)" fps 15 scale 1.00 ACT_IDLE 2
$sequence idle5 "SCI2_Fidget1(Look_Right)" fps 15 scale 1.00 ACT_IDLE 1
$sequence idle6 "SCI2_Fidget1(Look_Left)" fps 15 scale 1.00 ACT_IDLE 1
$sequence idle7 "Scientist_XFidget5(WShift)" fps 10 scale 1.00 ACT_IDLE 10
$sequence crouchstand "SCI2_FearCrouch(up)1" fps 18 scale 1.08 ACT_STAND 1
$sequence crouch_idle "SCI2_FearCrouch1(Idle)" loop fps 18 scale 1.08 ACT_CROUCHIDLE 1
$sequence crouch_idle2 "Scientist_XFCrouch1" loop fps 18 scale 1.00 ACT_CROUCH 1
$sequence crouch_idle3 "Scientist_XFCrouch2" loop fps 18 scale 1.00 ACT_CROUCH 1
// FIXME $sequence crouch_idle3 "Scientist_XFCrouch3" loop fps 18 scale 1.00 ACT_CROUCHIDLE 1
$sequence panic "Scientist_XPanic1" fps 18 scale 1.00 ACT_EXCITED 1
$sequence fear1 "SCI2_Reaction1" fps 18 scale 1.08 ACT_FEAR_DISPLAY 1
$sequence fear2 "SCI2_Reaction(Terror)1" fps 18 scale 1.08 ACT_FEAR_DISPLAY 1
$sequence eye_wipe "SCI2_Reaction(Eye_Wipe)1" fps 18 scale 1.08 ACT_FEAR_DISPLAY 1
$sequence pull_needle "SCI2_Pull_Needle1" fps 15 scale 1.08 ACT_ARM 1 { event 2 25 }
$sequence return_needle "SCI2_Return_Needle1" fps 15 scale 1.08 ACT_DISARM 1 { event 3 11 }
$sequence give_shot "SCI2_Give_Shot1" fps 15 scale 1.08 ACT_MELEE_ATTACK1 1 { event 1 17 }
/*
==============================================================================
DEATH ANIMATIONS
==============================================================================
*/
$sequence diesimple "die_simple" fps 24 scale 1.0 ACT_DIESIMPLE 1 { event 2001 8 }
$sequence dieforward "Scientist_XDie-Forward1" X fps 20 scale 1.00 ACT_DIE_HEADSHOT 1 { event 2001 26 }
$sequence dieforward1 "Scientist_XDieFast1" X fps 16 scale 1.0 ACT_DIEFORWARD 1 { event 2001 11 }
$sequence diebackward "Scientist_XDie-Backward1" X fps 15 scale 1.00 ACT_DIEBACKWARD 1 { event 2001 8 }
$sequence headshot "Sci2_sdth_headsht1" X fps 15 scale 1.08 ACT_DIE_HEADSHOT 20 { event 2001 18 }
$sequence gutshot "Sci2_sdth_gutshot1" X fps 15 scale 1.08 ACT_DIE_GUTSHOT 1 { event 2001 16 }
/*
==============================================================================
DEAD POSES
==============================================================================
*/
$sequence lying_on_back "dead_pose_onback" scale 1.08
$sequence lying_on_stomach "dead_pose_onstomach" scale 1.08
$sequence dead_sitting "dead_pose_sitting" scale 1.08
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
	{ event 1008 1 "NPC_Scientist.CheckTie" }
}
$sequence dryhands "dry_hands" fps 12 scale 1.08
$sequence tieshoe "tie_shoe" fps 12 scale 1.08
$sequence writeboard "scientist_write_board" fps 14 scale 1.08
$sequence studycart "study_pushcart" fps 12 scale 1.08
$sequence lean "lean_computer" fps 12 scale 1.0
$sequence pondering "scientist_pondering" fps 12 scale 1.00
$sequence pondering2 "scientist_pondering2" fps 15 scale 1.0
$sequence pondering3 "scientist_pondering3" fps 12 scale 1.08
$sequence buysoda "scientist_buysoda" fps 15 scale 1.08 {
	{ event 1004 50 "NPC.ButtonBlip1" }
	{ event 1004 81 "Weapons.BulletHit1" }
	{ event 1004 88 "Weapons.BulletHit2" }
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
$sequence deskidle "c1a0_desk_idle" loop fps 20 scale 1.0 {
// Now implemented by level designer	{ event 1003 20 "intro" }
}
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
SCRIPTED SEQUENCES - THEY ARE ARRANGED BY ORDER OF APPEARANCE
==============================================================================
*/
$sequence cprscientist "..\cinematics\cpr\cpr_scientist" loop fps 20 scale 1.0 origin 20 6 0
$sequence cprscientistrevive "..\cinematics\cpr\cpr_scientist(revive)" fps 20 scale 1.0 origin 20 6 0
$sequence cowering_in_corner "scientist_cowering" loop fps 18 scale 1.08 {
// Now implemented by level designer	{ event 1003 3 "gameover" }
}
$sequence sstruggleidle "headcrab_struggle(idle)" loop fps 18
$sequence sstruggle "headcrab_struggle" fps 28 {
	{ event 1000 1 }
}
$sequence headcrabbed "..\cinematics\scientist_headcrabbed\headcrab_scientist" {
	origin 0 0 0 
	fps 20 
	{ event 1000 165 } 
	scale 1.08
}
$sequence c1a0_catwalkidle "c1a0_fallcatwalk(idle)" loop fps 12 scale 1.0
$sequence c1a0_catwalk "c1a0_fallcatwalk" fps 16 scale 1.0 {
	{ event 1000 1 }
	{ event 1008 1 "NPC_Scientist.StayBack" }
	{ event 1008 43 "NPC_Scientist.CatScream" }
	
}
/*
//scale 1.08
*/
$sequence ceiling_dangle "ceiling_dangle" scale 1.0 { 	
	origin 1032 -312 -472
	fps 15
	{ event 1000 1 }
	{ event 1001 1 }
	{ event 1004 5 "NPC_Scientist.DragOff" }
}
$sequence ventpull1 "high_ventpull(sci1)" scale 1.0 fps 15 {
	{ event 1000 1 }
	{ event 1001 1 }
	{ event 1004 8 "NPC_Scientist.VentPull1" }
	{ event 1004 16 "NPC_Scientist.VentPull2" }
	{ event 1004 28 "NPC_Scientist.VentPull3" }
	{ event 1004 31 "NPC_Scientist.VentPull1" }
}
$sequence ventpull2 "high_ventpull(sci2)" scale 1.0 fps 15 {
	origin 0 -28 77
	{ event 1000 1 }
	{ event 1001 1 }
}
$sequence ventpullidle1 "high_ventpull(idle1)" loop fps 15 scale 1.0
$sequence ventpullidle2 "high_ventpull(idle2)" loop fps 15 scale 1.0 origin 0 -28 77
$sequence sitidle "c1a1b_sit(idle)" loop fps 14 scale 1.0 origin 0 -16 0
$sequence sitstand "c1a1b_sit(stand)" fps 18 scale 1.0 origin 0 -16 0
$sequence keypad "keypad" fps 18 scale 1.0 {
	{ event 1008 15 "NPC.ButtonBlip1" }
	{ event 1008 24 "NPC.ButtonBlip1" }
	{ event 1008 34 "NPC.ButtonBlip1" }
	{ event 1008 44 "NPC.ButtonBlip1" }
}
$sequence panic1 "Scientist_XPanic1" fps 18 scale 1.00
$sequence lookwindow "scientist_lookwindow" fps 15 scale 1.0
$sequence wave "scientist_wave" fps 22 scale 1.0
$sequence pulldoor "scientist_pulldoor" fps 18 scale 1.08 {
	{ event 1004 11 "NPC.ButtonLatchLocked2" }
	{ event 1004 26 "NPC.ButtonLatchLocked1" }
}
$sequence beatdoor "scientist_beatdoor" fps 16 scale 1.0 {
	{ event 1004 9 "NPC.ButtonLatchUnlocked1" }
	{ event 1004 20 "NPC.ButtonLatchUnlocked2" }
}
$sequence fallingloop "scientist_fallingloop" fps 35 scale 1.08
$sequence crawlwindow "scientist_crawlwindow2" fps 20 scale 1.08
$sequence divewindow "scientist_divewindow" fps 30 scale 1.08 { event 1001 1 }
$sequence locked_door "scientist_berserk_door" fps 16 scale 1.0 {
	{ event 1004 11 "NPC.ButtonLatchLocked1" }
	{ event 1004 21 "NPC.ButtonLatchLocked2" }
	{ event 1004 31 "NPC.ButtonLatchLocked1" }
}
$sequence push_button2 "scientist_push_button" fps 16 scale 1.0 {
	{ event 1008 1 "NPC_Scientist.DarkRoom" }
}
$sequence unlock_door "unlock_door_c1a2"  fps 15 scale 1.0 {
	{ event 1004 56 "NPC.ButtonLatchLocked1" }
}
$sequence quicklook "scientist_quicklook" fps 16 scale 1.0
$sequence handrailidle "lean_handrail(idle)" loop fps 14 scale 1.0
$sequence handrail "lean_handrail" fps 22 scale 1.0
$sequence hanging_idle "..\cinematics\scientist_bridge_c1a1b\hang_idle" loop fps 20 scale 1.0
$sequence fall "..\cinematics\scientist_bridge_c1a1b\fall"  fps 20 scale 1.0 {
        { event 1000 1 } // die
	{ event 1008 20 "NPC_Scientist.Scream3" }
}
$sequence scientist_get_pulled "..\cinematics\zombie_pull_scientist_wall\scientist_getspulled" fps 17 scale 1.0 {
	origin 0 -36 0
	{ event 1000 1 } // die
}
$sequence hanging_idle2 "..\cinematics\scientist_bridge_c1a1b\hang_idle" loop fps 20 scale 1.0 {
	{ event 1008 1 "NPC_Scientist.Dangling" }
}
$sequence fall_elevator "..\cinematics\scientist_bridge_c1a1b\fall_elevator" fps 20 scale 1.0 {
	{ event 1000 1 }
	{ event 1008 35 "NPC_Scientist.Scream3" }
}
$sequence scientist_idlewall "..\cinematics\zombie_pull_scientist_wall\scientist_idlewall" loop fps 14 {
	scale 1.0
	origin 0 -36 0
}
$sequence ickyjump_sci "..\cinematics\icky_chomp_scientist\new_icky_scientist" fps 18 scale 1.0 {
	{ event 1000 1 }
	{ event 1001 1 }
// Now implemented by level designer	{ event 1003 8 "scream" }
// Now implemented by level designer	{ event 1003 30 "gib" }
}
$sequence haulscientist "..\cinematics\zombie_haul_bodies\haul_scientist" fps 18 scale 1.08 {
	origin 15 0 0
	{ event 1000 1 }
	{ event 1001 1 }
}
$sequence c1a4_wounded_idle "c1a4_wounded_idle" fps 12 loop scale 1.0
$sequence c1a4_dying_speech "c1a4_dying_speech" fps 15 scale 1.0 {
	{ event 1000 1 }
	{ event 1008 10 "NPC_Scientist.Rocket" }
	{ event 1008 105 "NPC_Scientist.Pain3" }
} 

$sequence tentacle_grab "..\cinematics\c1a4i_tentacle_grab\scientist" {
	origin -406.115570 340.254669 454
	rotate -15
	fps 15 
	scale 1.08
	LX LY
	{ event 1000 1 }
	{ event 1004 2 "NPC_Scientist.Scream02" }
// Now implemented by level designer	{ event 1003 18 "smash" }
	{ event 1004 65 "NPC_Scientist.Tentacle" }
}
$sequence helicack "helicack" fps 18 {
// Now implemented by level designer	{ event 1003 30 "gunmedown" }
	{ event 1000 30 }
	{ event 1008 25 "NPC_Scientist.Pain4" }
}
$sequence windive "windive" fps 30 scale 1.0 {
	{ event 1000 1 }
	{ event 1001 1 }
}
$sequence scicrashidle "../cinematics/headcrabbed/scientist(idle)" loop fps 34
$sequence scicrash "../cinematics/headcrabbed/scientist(crash)" fps 20 {
	{ event 1000 1 }
}
$sequence onguard "../cinematics/c3a2f/gunsci_sciidle" fps 12 loop scale 1.08
$sequence seeya "../cinematics/c3a2f/gunsci_sci" fps 33 scale 1.08 {
// Now implemented by level designer	{ event 1003 163 "putdown" }
}
$sequence rocketcrawl "rocket_crawl" loop fps 20
$sequence portal "portal_c3a2d" loop fps 20 {
	{ event 1008 19 "NPC.ButtonBlip1" }
	{ event 1008 22 "NPC.ButtonBlip1" }
	{ event 1008 25 "NPC.ButtonBlip2" }
	{ event 1008 32 "NPC.ButtonBlip2" }
	{ event 1008 47 "NPC.ButtonBlip1" }
	{ event 1008 98 "NPC.ButtonBlip2" }
	{ event 1008 102 "NPC.ButtonBlip1" }
	{ event 1008 110 "NPC.ButtonBlip2" }
	{ event 1008 119 "NPC.ButtonBlip1" }
}
$sequence gluonshow "c3a2_gluon_intro" fps 18 scale 1.08
$sequence crouch "SCI2_FearCrouch(down)1" fps 18 scale 1.08
$sequence kneel "kneel_dead" fps 22 scale 1.00
$sequence punch "scientist_punch" fps 30 scale 1.0
