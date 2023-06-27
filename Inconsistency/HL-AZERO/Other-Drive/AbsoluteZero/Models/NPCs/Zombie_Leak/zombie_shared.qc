// hitboxes
$hbox 6 "Bip01 L Leg" 0.00 -7.37 -3.07  18.33 3.79 3.92
$hbox 6 "Bip01 L Leg1" 0.00 -5.41 -3.29  18.33 1.66 2.83
$hbox 6 "Bip01 L Foot" -1.24 -2.87 -2.87  2.17 8.13 1.15

$hbox 7 "Bip01 R Leg" 0.00 -7.61 -4.48  18.33 3.83 2.85
$hbox 7 "Bip01 R Leg1" 0.00 -5.71 -3.29  18.33 1.67 2.74
$hbox 7 "Bip01 R Foot" -1.25 -2.87 -2.07  2.16 8.16 1.75

$hbox 3 "Bip01 Pelvis" -8.58 -4.74 -7.00  1.56 5.33 6.82
$hbox 3 "Bip01 Spine" -0.42 -3.39 -6.36  4.96 7.36 6.42
$hbox 3 "Bip01 Spine1" 0.00 -3.41 -8.13  4.77 4.99 8.37
$hbox 2 "Bip01 Spine2" 0.00 -1.15 -9.32  6.38 7.04 6.3
$hbox 2 "Bip01 Spine3" -1.83 -7.11 -9.67  11.87 7.40 8.88

$hbox 1 "Bip01 Head" -3.24 -6.89 -5.10  8.77 8.96 7.94

$hbox 4 "Bip01 L Arm" 0.00 -4.15 -0.95  5.77 4.01 2.81
$hbox 4 "Bip01 L Arm1" -3.04 -3.42 -2.51  18.57 2.91 2.96
$hbox 4 "Bip01 L Arm2" 0.00 -2.90 -2.33  14.80 1.95 2.06
$hbox 4 "Bip01 L Hand" -0.59 -1.47 -1.68  5.32 0.57 2.81

$hbox 5 "Bip01 R Arm" 0.00 -4.64 -5.64  5.86 1.56 0.00
$hbox 5 "Bip01 R Arm1" -3.88 -4.38 -3.37  18.78 2.62 2.20
$hbox 5 "Bip01 R Arm2" 0.00 -1.40 -1.61  15.65 1.53 2.76
$hbox 5 "Bip01 R Hand" -0.59 -1.47 -1.68  5.32 0.57 2.81

/*
==============================================================================
COMMON ANIMATIONS
==============================================================================
*/

$sequence idle1 "zombie_idle" fps 15 LOOP scale 1.0 ACT_IDLE 4
$sequence idle2 "zombie_idle2" fps 15 LOOP scale 1.0 ACT_IDLE 1
$sequence turnleft "zombie_turnleft" fps 20 scale 1.0 ACT_TURN_LEFT 1
$sequence turnright "zombie_turnright" fps 20 scale 1.0 ACT_TURN_RIGHT 1
$sequence flinchsmall "zombie_smallflinch" fps 22 scale 1.0 ACT_SMALL_FLINCH 5
$sequence flinch "zombie_flinch" fps 22 scale 1.0 ACT_SMALL_FLINCH 1
$sequence bigflinch "zombie_bigflinch" fps 20 scale 1.0 ACT_BIG_FLINCH 1
$sequence getup "zombie_getup" fps 15 scale 1.0
$sequence falling "zombie_falling" loop fps 26 scale 1.0 ACT_FALL 1
$sequence attack1 "zombie_attack" fps 15 scale 1.0 ACT_MELEE_ATTACK1 5 { event 1 10 } { event 2 19 }
$sequence attack2 "zombie_attack2" fps 15 scale 0.895 ACT_MELEE_ATTACK1 1 { event 3 6 }
$sequence walk "zombie_walk1" lx loop fps 22 scale 0.895 ACT_WALK 1 {
  { event 1004 2 "common/npc_step1.wav" } 
  { event 1004 12 "common/npc_step3.wav" } 
  { event 1004 22 "common/npc_step2.wav" } 
  { event 1004 31 "common/npc_step4.wav" } 
  { event 1004 41 "common/npc_step1.wav" } 
  { event 1004 51 "common/npc_step3.wav" } 
 }

/*
==============================================================================
DEATH ANIMATIONS
==============================================================================
*/

$sequence dieheadshot "zombie_death" fps 22 scale 1.0 ACT_DIE_HEADSHOT 1 { event 2001 18 }
$sequence diesimple "zombie_death1" x fps 20 scale 1.0 ACT_DIESIMPLE 1 { event 2001 13 }
$sequence diebackward "zombie_death2" fps 18 scale 1.0 ACT_DIEBACKWARD 1 { event 2001 10 }

/*
==============================================================================
SCRIPTED SEQUENCES
==============================================================================
*/

$sequence pause "zombie_pause" fps 15 scale 0.895
$sequence busting_through_wall "zombie_bust_through" fps 20 scale 1.0
$sequence kick_punch_wall "kick_punch_wall" fps 14 scale 0.895
$sequence bust_window "jump_through_window_c1a2" fps 26 scale 0.895
$sequence soda "zombie_soda" fps 26 scale 0.895 {
	{ event 1004 63 "*buttons/latchunlocked1.wav" }
}
$sequence slideidle "hitwall_slidedown(idle)" loop fps 16 scale 0.895
$sequence slidewall "hitwall_slidedown" fps 18 scale 0.895 {
	{ event 1000 1 }
}
$sequence ventclimbidle "zombie_pause"  loop fps 15 scale 0.895 origin 0 0 80
$sequence ventclimb "climb_out_vent"  fps 20 scale 0.895 origin 0 0 80 
$sequence deadidle "slumped_c1a1" loop fps 14 scale 0.895
$sequence deadwall "slumped_c1a1(die)" fps 16 { event 1000 1 } scale 0.895
$sequence freakdie "zombie_freakout(die)" fps 12 scale 0.895 {
	{ event 1000 4 }
}
$sequence freak "zombie_freakout" loop fps 16 scale 0.895 {
// Now implemented by level designer { event 1003 6 "freak" }
}
$sequence eatbodytable "zombie_eatbody_table" loop fps 20 scale 0.895
$sequence eatbody "zombie_eatbody" loop fps 15 scale 0.895
$sequence eatbodystand "zombie_eatbody(stand)" fps 30 scale 0.895
$sequence ripdoor "zombie_ripdoor" fps 20 scale 0.895 {
	{ event 1008 1 "*debris/bustcrate1.wav" }
}
$sequence zombie_pull_scientist "zombie_pull_scientist" fps 17 scale 0.895

$sequence "zombie_fight" "barney_zombie_fight_Z" AX AY AZR fps 17 {
	{ event 1003 236 "killzombie" }
	{ event 1004 45 "Zombie/zo_alert10.wav" }
    	{ event 1004 100 "Zombie/zo_idle1.wav" } 
    	{ event 1004 115 "Zombie/zo_attack2.wav" } 
    	{ event 1004 132 "Zombie/zo_pain1.wav" } 
    	{ event 1004 146 "Zombie/zo_pain2.wav" }  
	{ event 1004 170 "Zombie/zo_attack1.wav" }  
}

$sequence zombie_eating "zombie_eating" loop fps 15 scale 0.895 {

}
$sequence eat_to_stand "eat_to_stand" fps 15 scale 0.895
$sequence vent_zidle "vent_zidle" fps 18 scale 0.895 
$sequence vent_c1a3 "vent_c1a3" fps 18 scale 0.895 {
	{ event 1000 1 }

}
$sequence haulzombie "haulzombie" fps 18 scale 0.895 {
}
$sequence c2a3_snack_getup "c2a3_snack_getup" fps 12 scale 0.895 {
	rotate 90
	origin 32 -147 0
}

$sequence c2a3_snack_idle "zombie_eatbody" loop fps 15 rotate 45 scale 0.895

$sequence "compthrow_idle" "compthrow_idle" AX AY AZR fps 30 loop rotate 180 { event 1001 1 } scale 0.895
$sequence "compthrow" "compthrow" AX AY AZR fps 30 frame 0 52 rotate 180 { event 1001 1 } scale 0.895
$sequence "iceslip" "iceslip" AX AY AZR fps 16 rotate 180 scale 0.895
$sequence "get_run_over" "get_run_over" AX AY AZR fps 30 { event 1000 1 } scale 0.895

$sequence "ventcrush" "zombie_ventcrush" fps 20 scale 0.895
$sequence "ventcrush_idle" "zombie_ventcrush(idle)" fps 20 scale 0.895
$sequence "ventcrush_idle2" "zombie_ventcrush(idle2)" fps 15 scale 0.895
$sequence "ventcrushdead" "zombie_ventcrushdead" fps 30 scale 0.895
$sequence getup&bust "zombie_bustgetup" fps 15 scale 1.0
//$sequence "getup_idle" "getup_idle" fps 30 loop scale 0.895

$sequence "standtowalk" "zombie_standtowalk" fps 20 loop