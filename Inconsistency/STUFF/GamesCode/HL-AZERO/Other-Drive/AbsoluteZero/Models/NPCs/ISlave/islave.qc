$modelname "islave.mdl"
$scale 1.0

$cd ./SMD
$cdtexture ./maps_8bit

$controller 0 "Bip01 Neck1" YR 60 -60

$attachment 0 "Bip01 R Hand" 0 0 0
$attachment 1 "Bip01 L Hand" 0 0 0

// whole body
$body studio "ISLAVE_Template_Model-1"

$eyeposition 0 0 43
//$eyeposition 0 38 43

$texturegroup skinfamilies 
{
{ "chrome1.bmp" }
{ "chrome1a.bmp" }
{ "chrome1b.bmp" }
{ "chrome1c.bmp" }
{ "chrome1d.bmp" }
}

// hitboxes.
$hbox 6 "Bip01 L Leg" -3.96 -6.55 -3.51  18.51 5.56 4.46
$hbox 6 "Bip01 L Leg1" -0.64 -2.27 -2.09  13.41 3.03 1.95
$hbox 6 "Bip01 L Foot" -0.55 -2.82 -6.41  8.70 8.88 4.51

//$hbox 0 "Bone13" -4.04 0.00 -3.04  12.69 4.74 0.00

$hbox 7 "Bip01 R Leg" -4.07 -6.65 -4.52  18.51 5.49 3.46
$hbox 7 "Bip01 R Leg1" -0.54 -2.34 -2.00  13.41 1.72 2.03
$hbox 7 "Bip01 R Foot" -0.54 -3.17 -4.46  8.66 9.01 6.46

//$hbox 0 "Bone19" 0.00 0.00 -3.04  3.41 4.65 0.00
//$hbox 0 "Bone16" -3.82 -15.69 -3.17  4.10 4.64 0.00

$hbox 3 "Bip01 Pelvis" -6.31 -8.74 -7.26  6.33 5.23 7.64
$hbox 3 "Bip01 Spine" 0.00 -6.02 -4.51  6.15 5.10 5.35
$hbox 2 "Bip01 Spine1" 0.00 -1.49 -5.48  6.18 0.00 6.13
$hbox 2 "Bip01 Spine2" 0.00 -10.04 -13.03  8.82 8.40 13.29
$hbox 2 "Bip01 Spine3" 0.00 -13.07 -13.18  10.64 11.36 13.76

$hbox 4 "Bip01 L Arm" 0.00 -0.00 -6.89  8.36 1.88 0.00
$hbox 4 "Bip01 L Arm1" 0.00 -5.12 -4.23  22.31 2.61 2.65
$hbox 4 "Bip01 L Arm2" 0.00 -4.05 -4.42  14.97 2.97 4.59
$hbox 4 "Bip01 L Hand" 0.00 -1.87 -2.40  7.80 2.52 2.61
//$hbox 0 "Bip01 L Finger01" 0.00 -0.20 -0.80  3.75 1.40 1.08
//$hbox 0 "Bip01 L Finger11" 0.00 -0.54 -0.32  3.61 1.58 1.52

$hbox 5 "Bip01 R Arm" 0.00 0.00 0.00  8.42 1.87 7.10
$hbox 5 "Bip01 R Arm1" 0.00 -4.98 -2.42  22.31 2.82 4.49
$hbox 5 "Bip01 R Arm2" -0.04 -3.65 -4.19  15.23 3.34 4.69
$hbox 5 "Bip01 R Hand" 0.00 -1.84 -2.38  7.54 2.58 2.63
//$hbox 0 "Bip01 R Finger01" 0.00 -0.25 -0.99  3.53 1.35 0.92
//$hbox 0 "Bip01 R Finger11" 0.00 -0.55 -1.20  3.61 1.58 0.62

//$hbox 0 "Bip01 Neck" -5.02 -4.69 -3.62  3.41 6.57 4.04
//$hbox 0 "Bip01 Neck1" 0.00 -4.61 -5.52  2.85 3.54 6.01

$hbox 1 "Bip01 Head" 0.00 -5.54 -7.74  11.38 9.60 8.16

//$hbox 0 "Bone05" -2.57 -6.59 -3.60  2.94 0.00 3.17
//$hbox 0 "Bone06" -3.51 -6.97 -1.24  1.59 0.13 1.12
//$hbox 0 "Bone07" -3.24 -2.27 -1.64  0.00 0.00 1.44
//$hbox 0 "Bone08" -3.62 -1.87 -2.92  0.42 0.00 2.63
//$hbox 0 "Bone22" -1.56 -2.92 0.00  0.00 0.00 3.55
//$hbox 0 "Bone26" 0.00 -2.93 0.00  1.76 0.00 3.57
//$hbox 0 "Bone30" -0.89 -2.93 -1.05  0.88 0.02 0.43

/*
==============================================================================
COMMON ANIMATIONS
==============================================================================
*/
$sequence idle1 "ISLAVE_Idle1(Head_Scratch)" fps 15 loop ACT_IDLE 1
$sequence idle2 "ISLAVE_Idle2(Head_Tilt)" fps 15 loop ACT_IDLE 2
$sequence idle3 "ISLAVE_Idle3(Subtle)2" fps 15 loop ACT_IDLE 8
$sequence crouch "ISLAVE_Crouch1" fps 15 ACT_CROUCH 1
$sequence walk1 "ISLAVE_Walk1" LX fps 15 loop ACT_WALK 1
$sequence walk2 "ISLAVE_Walk2" LX fps 15 loop ACT_WALK 1
$sequence run1 "ISLAVE_Run1" LX fps 25 loop ACT_RUN 1
$sequence right "ISLAVE_180_right2" fps 15 ACT_TURN_LEFT 1
$sequence left "ISLAVE_180(TurnAround)2" fps 15 ACT_TURN_RIGHT 1
$sequence jump "ISLAVE_Jump1" X fps 15 
$sequence stairsup "ISLAVE_Stairs_Up1" LX LZ fps 15 loop
$sequence attack1 "ISLAVE_Attack(Rapid_Fist)1" fps 22 ACT_MELEE_ATTACK1 4 { event 1 8 } { event 1 14 } { event 1 22 }
$sequence zapattack1 "ISLAVE_Attack(Two-Fist)2" fps 15 ACT_RANGE_ATTACK1 1 { event 3 1 } { event 3 5 }  { event 3 11 }  { event 3 16 } { event 4 25 } { event 5 30 }
$sequence flinch2 "ISLAVE_Flinch(Small)2" fps 15 ACT_SMALL_FLINCH 1
$sequence laflinch "ISLAVE_SFCH_larm1" fps 15 ACT_FLINCH_LEFTARM 1
$sequence raflinch "ISLAVE_SFCH_rarm1" fps 15 ACT_FLINCH_RIGHTARM 1 
$sequence llflinch "ISLAVE_SFCH_lleg1" fps 15 ACT_FLINCH_LEFTLEG 1
$sequence rlflinch "ISLAVE_SFCH_Rleg1" fps 15 ACT_FLINCH_RIGHTLEG 1
$sequence bigflinch "ISLAVE_Flinch(Big)1" fps 15 ACT_BIG_FLINCH 1 
/*
==============================================================================
BARNACLED
==============================================================================
*/
$sequence "barnacle1" "barnacled_1" fps 12 ACT_BARNACLE_HIT 1 
$sequence "barnacle2" "barnacled_2" fps 22 loop ACT_BARNACLE_PULL 1 
$sequence "barnacle3" "barnacled_3" fps 8 ACT_BARNACLE_CHOMP 1 
$sequence "barnacle4" "barnacled_4" fps 10 loop ACT_BARNACLE_CHEW 1 
/*
==============================================================================
DEATH ANIMATIONS
==============================================================================
*/
$sequence dieheadshot "ISLAVE_XDeath(Headshot)1" X Y fps 20 ACT_DIE_HEADSHOT 1
$sequence diesimple "ISLAVE_Die(Simple)1" X Y fps 20 ACT_DIESIMPLE 1
$sequence dieforward "ISLAVE_XDeath(Forward)1" X Y fps 20 ACT_DIEFORWARD 1
$sequence diebackward "ISLAVE_Die(Backward)1" X Y fps 20 ACT_DIEBACKWARD 1
$sequence dieforward "ISLAVE_Die(Violent)1" X Y fps 20 ACT_DIEVIOLENT 1
/*
==============================================================================
SCRIPTED SEQUENCES
==============================================================================
*/
$sequence collar1 "collar_pain" fps 18 ACT_RELOAD 1
$sequence collar2 "collar_pain2" fps 18 ACT_RELOAD 1
$sequence pushup "operate_machine1" fps 20
$sequence grab "operate_machine2" fps 20
$sequence updown "operate_machine3" fps 20
$sequence downup "operate_machine4" fps 20
$sequence jibber "jibber" fps 30 loop {
	{ event 1008 20 "NPC_Vortigaunt.Word5" }
	{ event 1008 53 "NPC_Vortigaunt.Word7" }
}
$sequence jabber "jabber" fps 30 loop {
	{ event 1008 10 "NPC_Vortigaunt.Word3" }
	{ event 1008 40 "NPC_Vortigaunt.Word4" }
}
$sequence pepsihit "pepsi_swing" fps 25 {
	{ event 1000 1 }
	{ event 1003 1 "break" }
}
$sequence pepsitopple "pepsi_topple" fps 18 {
	{ event 1000 1 }
	{ event 1003 58 "break2" }
}
$sequence pepsiidle "pepsi_topple(idle)" loop fps 18
$sequence buttonpush "buttonpush" X fps 15 
$sequence valvefry "valvefry" X fps 15 

==============================================================================
RESURRECT ANIMATIONS
==============================================================================
*/
$sequence dieheadshot-RES "ISLAVE_RES-Die(Headshot)1" X Y fps 20 ACT_DIE_HEADSHOT 1
$sequence diesimple-RES "ISLAVE_RES-Die(Simple)1" X Y fps 20 ACT_DIESIMPLE 1
$sequence diebackward-RES "ISLAVE_RES-Die(Bwrd)1" X Y fps 20 ACT_DIEBACKWARD 1
$sequence dieforward-RES "ISLAVE_RES-Die(Forward)1" X Y fps 40 ACT_DIEFORWARD 1
/*