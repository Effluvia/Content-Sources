/*
==============================================================================

shared file for the heavy weapons character

==============================================================================
*/


$sequence idle "idle1" loop ACT_IDLE 1
$sequence idle1 "idle2" loop ACT_IDLE 1
$sequence creeping_walk "walk1" LX loop {
	{ event 1004 0 "common/npc_step2.wav" }
	{ event 1004 20 "common/npc_step4.wav" }
} ACT_WALK 1
/* $sequence power_walk "run" LX loop fps 25 {
	{ event 1004 0 "common/npc_step2.wav" }
	{ event 1004 8 "common/npc_step4.wav" }
} ACT_RUN 1 */
$sequence turn_left "180_L" ACT_TURN_LEFT 1
$sequence turn_right "180_R" ACT_TURN_RIGHT 1
$sequence spinup "spinup_up" "spinup_down" fps 15 blend XR -45 45
$sequence spindown "spindown_up" "spindown_down" fps 15 blend XR -45 45

$sequence attack "shoot1_down" "shoot1_up" LOOP fps 30 blend XR -45 45 {
{ event 5001 1 "50" } 
{ event 5001 8 "50" }
{ event 5001 15 "50" }
{ event 5001 22 "50" } 
{ event 5001 29 "50" } 
{ event 5001 36 "50" }
{ event 5001 43 "50" }
{ event 5001 50 "50" }
{ event 5001 57 "50" }
{ event 1 1 }
{ event 2 8 }
{ event 3 15 }
{ event 4 22 }
{ event 5 29 }
{ event 6 36 }
{ event 7 43 }
{ event 8 50 }
{ event 9 57 }
} ACT_RANGE_ATTACK1 1

//$sequence attack "shoot1" loop { 
//{ event 5001 1 "50" } 
//{ event 5001 8 "50" }
//{ event 5001 15 "50" }
//{ event 5001 22 "50" } 
//{ event 5001 29 "50" } 
//{ event 5001 36 "50" }
//{ event 5001 43 "50" }
//{ event 5001 50 "50" }
//{ event 5001 57 "50" }
//{ event 1 1 }
//{ event 2 8 }
//{ event 3 15 }
//{ event 4 22 }
//{ event 5 29 }
//{ event 6 36 }
//{ event 7 43 }
//{ event 8 50 }
//{ event 9 57 }
//} ACT_RANGE_ATTACK1 1

$sequence melee "melee_gunkata"
$sequence melee1 "melee_karate" {
{ event 2010 5 } 
{ event 2010 12 } 
{ event 2010 36 } 
{ event 2010 55 } 
}
$sequence small_pain "flinch_sm" X Y ACT_SMALL_FLINCH 1
$sequence small_pain2 "flinch_sm2" X Y ACT_SMALL_FLINCH 2
$sequence die_backwards "dieback" X Y ACT_DIEBACKWARD 1 { event 2001 10 } 
$sequence die_crumple "diecrumple" X Y ACT_DIESIMPLE 1 { event 2001 4 } 
$sequence die_violent "dieviolent" X Y ACT_DIE_HEADSHOT 1 { event 2001 12 } 

$sequence c2a3_ambush4 "ambush_heavy" {
	origin -98 192 184
	scale 1.0
	AX AY AZR
	fps 20
}

$sequence barnacled1 "barnacled1" fps 15 ACT_BARNACLE_HIT 1
$sequence barnacled2 "barnacled2" loop fps 30 ACT_BARNACLE_PULL 1
$sequence barnacled3 "barnacled3" fps 5 ACT_BARNACLE_CHOMP 1
$sequence barnacled4 "barnacled4" loop fps 5 ACT_BARNACLE_CHEW 1
$sequence throwgrenade "Throw_grenade" fps 30 { event 7 46 } ACT_RANGE_ATTACK2 1
$sequence retreat_signal "Signal_retreat" fps 35 ACT_SIGNAL3 1
//$sequence retreat_signal_30fps "Signal_retreat" fps 30 ACT_SIGNAL3 2
$sequence advance_signal "Signal_advance" fps 30 ACT_SIGNAL1 1 { event 10 11 }
$sequence flank_signal "Signal_flank" fps 30 ACT_SIGNAL2 1 { event 10 11 }