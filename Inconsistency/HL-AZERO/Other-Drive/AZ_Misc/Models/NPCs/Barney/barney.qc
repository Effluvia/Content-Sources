$modelname ".\barney.mdl"
$cd ".\"
$cdtexture ".\"
$cliptotextures
$externaltextures

$scale 1.0

$eyeposition -0.000000 0.000000 63.000000

// 1 attachments
$attachment 0 "Bip01 R Hand" 12.000000 2.300000 5.000000 

// 2 bone controllers
$controller 0 "Bip01 Head" XR -60 60
$controller mouth "Bone05" ZR 0 20


$bodygroup studio
{
studio "TEDDYEATSADICKANDDIES"
}

$bodygroup gun
{
studio "barn_holster_reference"
studio "barn_glock_reference"
blank
}

$bodygroup studio
{
studio "baton"
}

// 81 sequences
$sequence idle1 "idle1" loop fps 15 ACT_IDLE 50 
$sequence idle2 "idle2" fps 15 ACT_IDLE 1 
$sequence idle3 "idle3" fps 15 ACT_IDLE 1 
$sequence idle4_1 "idle4_1" fps 12 ACT_IDLE 1 
$sequence idle4_2 "idle4_2" fps 15 ACT_IDLE 1 
$sequence walk "walk" LX loop fps 30 ACT_WALK 1 { event 1004 0 "common/npc_step1.wav" } { event 1004 12 "common/npc_step3.wav" } 
$sequence run "run" LX loop fps 30 ACT_RUN 1 { event 1004 0 "common/npc_step2.wav" } { event 1004 9 "common/npc_step4.wav" } 
$sequence shootgun "shootgun" fps 6 ACT_RANGE_ATTACK1 1 { event 5001 0 "21" } { event 3 0 } 
$sequence shootgun2 "shootgun2" fps 7 ACT_RANGE_ATTACK1 2 { event 5001 0 "21" } { event 3 0 } 
$sequence draw "draw" fps 15 ACT_ARM 1 { event 2 5 } 
$sequence disarm "disarm" fps 12 ACT_DISARM 1 { event 4 8 } 
$sequence reload "reload" fps 15 ACT_RELOAD 1 
$sequence turnleft "turnleft" fps 15 ACT_TURN_LEFT 1 
$sequence turnright "turnright" fps 15 ACT_TURN_RIGHT 1 
$sequence laflinch "laflinch" fps 16 ACT_FLINCH_LEFTARM 1 
$sequence raflinch "raflinch" fps 16 ACT_FLINCH_RIGHTARM 1 
$sequence llflinch "llflinch" fps 20 ACT_FLINCH_LEFTLEG 1 
$sequence rlflinch "rlflinch" fps 20 ACT_FLINCH_RIGHTLEG 1 
$sequence smlflinch "smlflinch" fps 16 ACT_SMALL_FLINCH 1 
$sequence diesimple "diesimple" X fps 15 ACT_DIE_HEADSHOT 1 { event 2001 7 } 
$sequence dieviolent "dieviolent" X fps 17 ACT_DIE_HEADSHOT 1 { event 2001 11 } 
$sequence diegutshot "diegutshot" fps 15 ACT_DIE_GUTSHOT 1 { event 2001 7 } 
$sequence dieforward "dieforward" X fps 16 ACT_DIEFORWARD 1 { event 2001 14 } 
$sequence diebackward "diebackward" X fps 16 ACT_DIEBACKWARD 1 { event 2001 12 } 
$sequence diecrump "diecrump" X fps 16 ACT_DIESIMPLE 2 { event 2001 31 } 
$sequence barnaclehit "barnaclehit" fps 15 ACT_BARNACLE_HIT 1 
$sequence barnaclepull "barnaclepull" loop fps 45 ACT_BARNACLE_PULL 1 
$sequence barnaclecrunch "barnaclecrunch" fps 5 ACT_BARNACLE_CHOMP 1 
$sequence barnaclechew "barnaclechew" loop fps 5 ACT_BARNACLE_CHEW 1 

$sequencegroup seqgroup1
$sequence cower_stand "cower_stand" fps 14 
$sequence locked_door "locked_door" loop fps 28 
$sequence fall_loop "fall_loop" loop fps 28 
$sequence barn_wave "barn_wave" fps 22 
$sequence beat_grunt "beat_grunt" fps 14 
$sequence beat_gruntidle "beat_gruntidle" loop fps 14 
$sequence flashlight "flashlight" loop fps 14 { event 1004 5 "*buttons/latchunlocked2.wav" } 
$sequence lying_on_back "lying_on_back" fps 1 
$sequence lying_on_side "lying_on_side" fps 1 
$sequence lying_on_stomach "lying_on_stomach" fps 1 
$sequence stuffed_in_vent "stuffed_in_vent" fps 1 
$sequence standing_idle "standing_idle" loop fps 15 
$sequence cprbarney "cprbarney" loop fps 20 
$sequence cprbarneyrevive "cprbarneyrevive" fps 20 
$sequence barney_dragvent "barney_dragvent" fps 15 { event 1000 1 } 
$sequence dying_barney "dying_barney" fps 18 { event 1000 1 } 
$sequence dying_barneyidle "dying_barneyidle" loop fps 12 
$sequence dying_friend "dying_friend" fps 18 

$sequencegroup seqgroup2
$sequence dying_friendidle "dying_friendidle" loop fps 12 
$sequence c1a3_bidle "c1a3_bidle" AX AY AZR fps 3 
$sequence c1a3_ventb "c1a3_ventb" AX AY AZR fps 18 { event 1000 1 } 
$sequence c1a3_emergeidle "c1a3_emergeidle" AX AY AZR loop fps 16 
$sequence c1a3_emerge "c1a3_emerge" AX AY AZR fps 16 { event 1000 1 } 
$sequence haulbarney "haulbarney" AX AY AZR fps 18 { event 1000 1 } 
$sequence intropush "intropush" fps 16 { event 1004 15 "*buttons/blip1.wav" } { event 1004 24 "*buttons/blip1.wav" } { event 1004 34 "*buttons/blip1.wav" } { event 1004 44 "*buttons/blip1.wav" } 
$sequence fence "fence" AX AY AZR fps 24 { event 1000 70 } { event 1003 70 "lightning" } { event 1001 70 } { event 1004 72 "weapons/electro4.wav" } { event 1008 100 "barney/ba_pain2.wav" } 
$sequence sit1 "sit1" loop fps 20 { event 1003 50 "introchair" } { event 1003 102 "introchair" } origin 0 -40 36
$sequence almostidle "almostidle" loop fps 16 
$sequence almost "almost" fps 26 { event 1000 1 } 
$sequence laseridle "laseridle" fps 6 
$sequence laser_top "laser_top" fps 6 { event 1000 0 } 
$sequence laser_bottom "laser_bottom" fps 6 { event 1000 0 } 

$sequencegroup seqgroup3
$sequence barneyfallidle "barneyfallidle" loop fps 33 
$sequence barneyfall "barneyfall" fps 32 { event 1000 1 } { event 1008 2 "barney/ba_pain1.wav" } { event 1008 24 "barney/ba_pain2.wav" } { event 1003 55 "bsplat" } { event 1008 55 "debris/bustflesh1.wav" } 
$sequence c3a2_draw "c3a2_draw" fps 16 { event 2 13 } 
$sequence unlatch "unlatch" fps 12 { event 1003 7 "barney_door_lock" } 
$sequence retina "retina" fps 20 
$sequence relaxstand "relaxstand" loop fps 10 
$sequence assassinated "assassinated" X fps 18 { event 1000 1 } { event 1003 1 "blood1" } { event 1003 2 "blood2" } 
$sequence trackswitch "trackswitch" fps 22 { event 1003 14 "trainarmswitch" } 
$sequence pepsiswing "pepsiswing" fps 25 { event 1003 1 "die" } { event 1003 8 "swing" } 
$sequence pepsipush "pepsipush" fps 18 { event 1003 51 "topple" } { event 1003 90 "horror" } 
$sequence buttonpush "buttonpush" fps 20 { event 1003 7 "button" } 
$sequence c1a0_lean_idle "c1a0_lean_idle" loop fps 15 
$sequence c1a0_lean "c1a0_lean" fps 15 

$sequencegroup seqgroup4
$sequence barney2_c1a4_traintrack_assist "barney2_c1a4_traintrack_assist" fps 15 
$sequence barney2_c1a4_traintrack_escape "barney2_c1a4_traintrack_escape" fps 15 
$sequence barney2_inspect_floor "barney2_inspect_floor" loop fps 15 
$sequence barney2_squatting_kibbitz "barney2_squatting_kibbitz" fps 15 
$sequence barney_zombie_fight_B "barney_zombie_fight_B" fps 17 

$sequencegroup seqgroup5
$sequence barney_c2a4_sterilizer "barney_c2a4_sterilizer" fps 15 
$sequence barney_c2a4_pre-idle "barney_c2a4_pre-idle" loop fps 15 
