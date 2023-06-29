$cdmaterials "models\spx_crossbreed"
$cliptotextures

$scale 1.0

$eyeposition 0.000000 0.000000 55.000000

$surfaceprop "zombieflesh"

// attachments
$attachment "blood_left" "Bip01 L Hand" 0.00 0.00 0.00 rotate 0 0 0
$attachment "blood_right" "Bip01 R Hand" 0.00 0.00 0.00 rotate 0 0 0

// 0 bone controllers

// 20 hit boxes
$hbox 6 "Bip01 L Leg" 0.000000 -7.370000 -3.070000  18.330000 3.790000 3.920000
$hbox 6 "Bip01 L Leg1" 0.000000 -5.410000 -3.290000  18.330000 1.660000 2.830000
$hbox 6 "Bip01 L Foot" -1.240000 -2.870000 -2.870000  2.170000 8.130000 1.150000
$hbox 7 "Bip01 R Leg" 0.000000 -7.610000 -4.480000  18.330000 3.830000 2.850000
$hbox 7 "Bip01 R Leg1" 0.000000 -5.710000 -3.290000  18.330000 1.670000 2.740000
$hbox 7 "Bip01 R Foot" -1.250000 -2.870000 -2.070000  2.160000 8.160000 1.750000
$hbox 3 "Bip01 Pelvis" -8.580000 -4.740000 -7.000000  1.560000 5.330000 6.820000
$hbox 3 "Bip01 Spine" -0.420000 -3.390000 -6.360000  4.960000 7.360000 6.420000
$hbox 3 "Bip01 Spine1" 0.000000 -3.410000 -8.130000  4.770000 4.990000 8.370000
$hbox 2 "Bip01 Spine2" 0.000000 -1.150000 -9.320000  6.380000 7.040000 6.300000
$hbox 2 "Bip01 Spine3" -1.830000 -7.110000 -9.670000  11.870000 7.400000 8.880000
$hbox 1 "Bip01 Head" -3.240000 -6.890000 -5.100000  8.770000 8.960000 7.940000
$hbox 4 "Bip01 L Arm" 0.000000 -4.150000 -0.950000  5.770000 4.010000 2.810000
$hbox 4 "Bip01 L Arm1" -3.040000 -3.420000 -2.510000  18.570000 2.910000 2.960000
$hbox 4 "Bip01 L Arm2" 0.000000 -2.900000 -2.330000  14.800000 1.950000 2.060000
$hbox 4 "Bip01 L Hand" -0.590000 -1.470000 -1.680000  5.320000 0.570000 2.810000
$hbox 5 "Bip01 R Arm" 0.000000 -4.640000 -5.640000  5.860000 1.560000 0.000000
$hbox 5 "Bip01 R Arm1" -3.880000 -4.380000 -3.370000  18.780001 2.620000 2.200000
$hbox 5 "Bip01 R Arm2" 0.000000 -1.400000 -1.610000  15.650000 1.530000 2.760000
$hbox 5 "Bip01 R Hand" -0.590000 -1.470000 -1.680000  5.320000 0.570000 2.810000

// 44 sequences
$sequence idle1 "idle1" loop fps 15 ACT_IDLE 4 
$sequence turnleft "turnleft" fps 20 ACT_TURN_LEFT 1 
$sequence turnright "turnright" fps 20 ACT_TURN_RIGHT 1 
$sequence flinchsmall "flinchsmall" fps 22 ACT_SMALL_FLINCH 5 
$sequence flinch "flinch" fps 22 ACT_SMALL_FLINCH 1 
$sequence bigflinch "bigflinch" fps 20 ACT_BIG_FLINCH 1 
$sequence falling "falling" loop fps 26 ACT_FALL 1 
$sequence attack1 "attack1" fps 15 {
    { event AE_SPX_ATTACK_RIGHT 10 }
    { event AE_SPX_ATTACK_LEFT 19 }
    activity ACT_MELEE_ATTACK1 5
}
$sequence attack2 "attack2" fps 15 {
    { event AE_SPX_ATTACK_BOTH 6 }
    activity ACT_MELEE_ATTACK1 1
}
$sequence walk "walk" LX loop fps 22 ACT_WALK 1 {
    { event AE_NPC_LEFTFOOT 2 "NPC_Spx.FootstepLeft" }
    { event AE_NPC_RIGHTFOOT 12 "NPC_Spx.FootstepRight" }
    { event AE_NPC_LEFTFOOT 22 "NPC_Spx.FootstepLeft" }
    { event AE_NPC_RIGHTFOOT 31 "NPC_Spx.FootstepRight" }
    { event AE_NPC_LEFTFOOT 41 "NPC_Spx.FootstepLeft" }
    { event AE_NPC_RIGHTFOOT 51 "NPC_Spx.FootstepRight" }
}
$sequence laflinch "laflinch" fps 15 ACT_FLINCH_LEFTARM 1 
$sequence raflinch "raflinch" fps 15 ACT_FLINCH_RIGHTARM 1 
$sequence llflinch "llflinch" fps 15 ACT_FLINCH_LEFTLEG 1 
$sequence rlflinch "rlflinch" fps 15 ACT_FLINCH_RIGHTLEG 1 
$sequence dieheadshot "dieheadshot" X fps 20 ACT_DIE_HEADSHOT 1 { event 2001 19 } 
$sequence dieheadshot2 "dieheadshot2" fps 22 ACT_DIE_GUTSHOT 1 { event 2001 18 } 
$sequence diesimple "diesimple" X fps 20 ACT_DIESIMPLE 1 { event 2001 13 } 
$sequence diebackward "diebackward" fps 18 ACT_DIEBACKWARD 1 { event 2001 10 } 
$sequence dieforward "dieforward" X fps 18 ACT_DIEFORWARD 1 { event 2001 21 } 
$sequence eatbody "eatbody" loop fps 15 {
	activity ACT_SPX_EAT 1
	{ event AE_SPX_EAT 11 }
}
$sequence eatbodycrouch "eatbodycrouch" fps 30 ACT_CROUCH 1
$sequence eatbodystand "eatbodystand" fps 30 ACT_STAND 1

$sequence getup "getup" fps 15 
$sequence pause "pause" fps 15 
$sequence busting_through_wall "busting_through_wall" fps 20 
$sequence kick_punch_wall "kick_punch_wall" fps 14 
$sequence bust_window "bust_window" fps 26 
$sequence soda "soda" fps 26 { event 1004 63 "*buttons/latchunlocked1.wav" } 
$sequence slideidle "slideidle" loop fps 16 
$sequence slidewall "slidewall" fps 18 { event 1000 1 } 
$sequence ventclimbidle "ventclimbidle" loop fps 15 
$sequence ventclimb "ventclimb" fps 20 
$sequence deadidle "deadidle" loop fps 14 
$sequence deadwall "deadwall" fps 16 
$sequence freakdie "freakdie" fps 12 { event 1000 0 } 
$sequence freak "freak" loop fps 16 { event 1003 2 "freak" } 

$sequence eatbodytable "eatbodytable" loop fps 20 
$sequence ripdoor "ripdoor" fps 20 { event 1008 1 "*debris/bustcrate1.wav" } 
$sequence zombie_pull_scientist "zombie_pull_scientist" fps 17 
$sequence zombie_eating "zombie_eating" loop fps 15 
$sequence eat_to_stand "eat_to_stand"  fps 15 
$sequence vent_zidle "vent_zidle" fps 18 
$sequence vent_c1a3 "vent_c1a3"  fps 18 { event 1000 1 } 
$sequence haulzombie "haulzombie"  fps 18 

$sequence c2a3_snack_getup "c2a3_snack_getup"  fps 12 
//$sequence gate "seq_gate" loop fps 20 { event 1008 10 "*debris/bustmetal1.wav" } 
$sequence gate "seq_gate" loop fps 20 { event 1003 10 "1" } 
$sequence breakgate "seq_breakgate" fps 20

$animation a_chainsawed dieheadshot frame 3 10 fixuploop 0 1
$sequence chainsawed a_chainsawed fps 20 loop activity ACT_SPX_CHAINSAWED 1

$animation a_flinch1 laflinch subtract a_flinch1 0
$sequence g_flinch1 a_flinch1 fps 15 activity ACT_GESTURE_SMALL_FLINCH 1 delta
$animation a_flinch2 raflinch subtract a_flinch2 0
$sequence g_flinch2 a_flinch2 fps 15 activity ACT_GESTURE_SMALL_FLINCH 1 delta
$animation a_flinch3 flinchsmall1 subtract a_flinch3 0
$sequence g_flinch3 a_flinch3 fps 16 activity ACT_GESTURE_SMALL_FLINCH 1 delta
$animation a_flinch4 flinchsmall subtract a_flinch4 0
$sequence g_flinch4 a_flinch4 fps 20 activity ACT_GESTURE_FLINCH_HEAD 1 delta
$animation a_flinch5 llflinch subtract a_flinch5 0
$sequence g_flinch5 a_flinch5 fps 15 activity ACT_GESTURE_BIG_FLINCH 1 delta
$animation a_flinch6 rlflinch subtract a_flinch6 0
$sequence g_flinch6 a_flinch6 fps 15 activity ACT_GESTURE_BIG_FLINCH 1 delta

// From HL:S
$sequence ragdoll "ragdoll_fixed" ACT_DIERAGDOLL 1 fps 30.00
$definemacro spxphys smd \\
$collisionjoints $smd$ { \\
 \\
	$mass 115.0 \\
	$inertia 10.00 \\
	$damping 0.01 \\
	$rotdamping 1.50 \\
	$rootbone "bip01" \\
 \\
	$jointconstrain "bip01 spine2" x limit -60.00 60.00 0.20 \\
	$jointconstrain "bip01 spine2" y limit -25.00 25.00 0.20 \\
	$jointconstrain "bip01 spine2" z limit -25.00 40.00 0.20 \\
 \\
	$jointconstrain "bip01 r arm1" x limit -15.00 30.00 0.20 \\
	$jointconstrain "bip01 r arm1" y limit -45.00 64.00 0.20 \\
	$jointconstrain "bip01 r arm1" z limit -73.00 12.00 0.20 \\
 \\
	$jointconstrain "bip01 l arm1" x limit -30.00 15.00 0.20 \\
	$jointconstrain "bip01 l arm1" y limit -67.00 30.00 0.20 \\
	$jointconstrain "bip01 l arm1" z limit -73.00 12.00 0.20 \\
 \\
	$jointconstrain "bip01 l arm2" x limit 0.00 0.00 0.00 \\
	$jointconstrain "bip01 l arm2" y limit -10.00 40.00 0.20 \\
	$jointconstrain "bip01 l arm2" z limit -100.00 20.00 0.20 \\
 \\
	$jointconstrain "bip01 l hand" x limit -60.00 60.00 0.20 \\
	$jointconstrain "bip01 l hand" y limit -30.00 30.00 0.20 \\
	$jointconstrain "bip01 l hand" z limit -90.00 70.00 0.20 \\
 \\
	$jointconstrain "bip01 r arm2" x limit 0.00 0.00 0.00 \\
	$jointconstrain "bip01 r arm2" y limit -40.00 10.00 0.20 \\
	$jointconstrain "bip01 r arm2" z limit -100.00 20.00 0.20 \\
 \\
	$jointconstrain "bip01 r hand" x limit -60.00 60.00 0.20 \\
	$jointconstrain "bip01 r hand" y limit -30.00 30.00 0.20 \\
	$jointconstrain "bip01 r hand" z limit -90.00 70.00 0.20 \\
 \\
	$jointconstrain "bip01 r leg" x limit -25.00 20.00 0.20 \\
	$jointconstrain "bip01 r leg" y limit -40.00 20.00 0.20 \\
	$jointconstrain "bip01 r leg" z limit -37.00 63.00 0.20 \\
 \\
	$jointconstrain "bip01 r leg1" x limit 0.00 0.00 0.00 \\
	$jointconstrain "bip01 r leg1" y limit 0.00 0.00 0.00 \\
	$jointconstrain "bip01 r leg1" z limit -95.00 4.00 0.20 \\
 \\
	$jointrotdamping "bip01 head" 5.00 \\
	$jointconstrain "bip01 head" x limit -20.00 20.00 0.20 \\
	$jointconstrain "bip01 head" y limit -25.00 25.00 0.20 \\
	$jointconstrain "bip01 head" z limit -13.00 30.00 0.20 \\
 \\
	$jointconstrain "bip01 l leg" x limit -25.00 20.00 0.20 \\
	$jointconstrain "bip01 l leg" y limit -20.00 40.00 0.20 \\
	$jointconstrain "bip01 l leg" z limit -37.00 63.00 0.20 \\
 \\
	$jointconstrain "bip01 l leg1" x limit 0.00 0.00 0.00 \\
	$jointconstrain "bip01 l leg1" y limit 0.00 0.00 0.00 \\
	$jointconstrain "bip01 l leg1" z limit -95.00 4.00 0.20 \\
 \\
	$jointconstrain "bip01 l foot" x limit -30.00 30.00 0.20 \\
	$jointconstrain "bip01 l foot" y limit -30.00 30.00 0.20 \\
	$jointconstrain "bip01 l foot" z limit -15.00 35.00 0.20 \\
 \\
	$jointconstrain "bip01 r foot" x limit -30.00 30.00 0.20 \\
	$jointconstrain "bip01 r foot" y limit -30.00 30.00 0.20 \\
	$jointconstrain "bip01 r foot" z limit -15.00 35.00 0.20 \\
}

