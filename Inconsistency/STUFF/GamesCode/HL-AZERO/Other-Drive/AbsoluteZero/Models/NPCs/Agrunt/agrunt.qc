$modelname MDL\agrunt.mdl
$cd ".\SMD"
$cdtexture ".\Textures"
$scale 1.0
$cliptotextures
$texrendermode "Agrunt_armor.bmp" "chrome" 

$bbox 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000
$cbox 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000
$eyeposition 0.000000 0.000000 75.000000


//reference mesh(es)
$body "body" "AGRUNT_Template_Model(Gun_body)3"

$bodygroup "arm"
{
studio "AGRUNT_Template_Model(Gun_arm)2"
studio "AGRUNT_Template_Model(Bare_arm)4"
}


// 1 attachment(s)
$attachment 0 "Bone39" 0.000000 0.000000 0.000000

// 1 bone controller(s)
$controller 0 "Bip01 Head" XR -45.000000 45.000000


// 21 hit box(es)
$hbox 10 "Bip01 Pelvis" -10.950000 -15.240000 -13.610000 5.010000 7.590000 13.390000
$hbox 6 "Bip01 L Leg" 0.000000 -10.830000 -7.530000 23.549999 6.710000 7.540000
$hbox 6 "Bip01 L Leg1" -1.900000 -2.910000 -10.330000 16.650000 6.380000 4.970000
$hbox 7 "Bip01 R Leg" 0.000000 -10.700000 -7.440000 23.320000 7.190000 6.380000
$hbox 7 "Bip01 R Leg1" -2.350000 -5.530000 -6.510000 15.790000 6.630000 8.580000
$hbox 10 "Bip01 L Foot" -1.200000 -3.200000 -8.390000 11.610000 16.080000 9.900000
$hbox 10 "Bip01 R Foot" 0.000000 -2.890000 -8.380000 11.370000 16.389999 9.900000
$hbox 3 "Bip01 Spine" -0.250000 -6.160000 -11.870000 6.960000 11.400000 11.790000
$hbox 2 "Bip01 Spine1" -0.250000 -10.160000 -11.870000 6.960000 11.400000 11.790000
$hbox 2 "Bip01 Spine2" -0.670000 -0.010000 -12.180000 6.460000 12.320000 12.000000
$hbox 10 "Bip01 Spine3" -6.030000 -15.000000 -12.240000 9.510000 10.020000 12.700000
$hbox 1 "Bip01 Head" -2.020000 6.000000 -8.130000 12.450000 15.210000 7.730000
$hbox 10 "Bip01 Head" -12.020000 -14.160000 -8.130000 12.450000 6.000000 7.730000
$hbox 10 "Bip01 L Arm" 0.000000 -5.020000 -9.560000 13.420000 11.530000 7.330000
$hbox 10 "Bip01 L Arm1" -10.830000 -11.050000 -6.740000 14.000000 10.010000 11.870000
$hbox 4 "Bip01 L Arm1" 14.000000 -11.050000 -6.740000 23.170000 10.010000 11.870000
$hbox 4 "Bip01 L Arm2" 0.000000 -7.730000 -9.440000 28.520000 7.340000 7.400000
$hbox 5 "Bip01 R Arm2" 0.000000 -13.510000 -9.140000 50.770000 7.710000 11.060000
$hbox 10 "Bip01 R Arm" 0.000000 -8.760000 -7.380000 16.840000 11.670000 9.580000
$hbox 10 "Bip01 R Arm1" -10.950000 -11.240000 -11.980000 14.000000 9.930000 6.760000
$hbox 5 "Bip01 R Arm1" 14.000000 -11.240000 -11.980000 26.520000 9.930000 6.760000

// 34 animation sequence(s)
$sequence "idle1" "idle1" fps 15 loop ACT_IDLE 50 
$sequence "threat" "threat" fps 15 ACT_THREAT_DISPLAY 1 
$sequence "walk" "walk" LX fps 15 loop ACT_WALK 1 { event 10 9 } { event 11 19 } 
$sequence "run" "run" LX fps 35 loop ACT_RUN 1 { event 10 11 } { event 11 23 } 
$sequence "turnl" "turnl" fps 15 ACT_TURN_LEFT 1 
$sequence "turnr" "turnr" fps 15 ACT_TURN_RIGHT 1 
$sequence "smallflinch" "smallflinch" fps 15 ACT_SMALL_FLINCH 1 
$sequence "bigflinch" "bigflinch" fps 15 ACT_BIG_FLINCH 1  { event 1004 1 "agrunt/ag_pain1.wav" } { event 1004 34 "agrunt/bflinch.wav" }
$sequence "mattack2" "mattack2" fps 15 ACT_MELEE_ATTACK1 1 { event 13 8 } 
$sequence "mattack3" "mattack3" fps 15 ACT_MELEE_ATTACK1 1 { event 12 7 } 
$sequence "victorysquat_1" "victorysquat_1" fps 5 ACT_CROUCH 1 
$sequence "victoryeat" "victoryeat" fps 8 loop ACT_VICTORY_DANCE 1 
$sequence "victorysquat_2" "victorysquat_2" fps 5 ACT_STAND 1 
$sequence "larmflinch" "larmflinch" fps 20 ACT_FLINCH_LEFTARM 1 
$sequence "llegflinch" "llegflinch" fps 20 ACT_FLINCH_LEFTLEG 1 
$sequence "rarmflinch" "rarmflinch" fps 20 ACT_FLINCH_RIGHTARM 1 
$sequence "rlegflinch" "rlegflinch" fps 20 ACT_FLINCH_RIGHTLEG 1 
$sequence "hornetattack" "attack1_down" "attack1_up" blend XR -30 45 fps 15 ACT_RANGE_ATTACK1 1 {
  { event 1 17 } 
  { event 2 19 } 
  { event 3 23 } 
  { event 4 25 } 
  { event 5 26 } 
  { event 5 26 }
}
$sequence "quickshoot" "attack2_down" "attack2_up" blend XR -30 45 fps 20 ACT_RANGE_ATTACK1 1 {
{ event 1 17 } 
{ event 2 20 } 
{ event 3 22 } 
{ event 3 22 } 
{ event 3 22 } 
}
$sequence "longshoot" "attack3_down" "attack3_up" blend XR -30 45 fps 15 ACT_RANGE_ATTACK1 1 {
  { event 1 18 } 
  { event 2 20 } 
  { event 3 22 } 
  { event 4 24 } 
  { event 5 26 } 
  { event 5 29 } 
  { event 5 29 } 
  { event 5 29 } 
 }
$sequence "diehead" "diehead" fps 20 ACT_DIE_HEADSHOT 1 { event 2002 15 } 
$sequence "diegut" "diegut" fps 15 ACT_DIE_GUTSHOT 1 { event 2002 14 } 
$sequence "dieforward" "dieforward" fps 16 ACT_DIEFORWARD 1 { event 2002 10 } 
$sequence "diesimple" "diesimple" fps 10 ACT_DIESIMPLE 1 { event 2002 6 } 
$sequence "diebackward" "diebackward" fps 15 ACT_DIEBACKWARD 1 { event 2002 25 } 
$sequence "float" "float" fps 15 loop
$sequence "scare" "scare" fps 20 {
  { event 1004 1 "weapons/bullet_hit1.wav" } 
  { event 1004 24 "weapons/bullet_hit2.wav" } 
  { event 1004 44 "agrunt/ag_alert3.wav" } 
  { event 1004 62 "weapons/bullet_hit1.wav" } 
 }
$sequence "bigopen" "bigopen" fps 20 { event 1003 2 "door" } { event 1008 5 "agrunt/ag_alert1.wav" } 
$sequence "smashrail" "smashrail" fps 20 
$sequence "landhard" "landhard" fps 20 
$sequence "die_beta" "die_beta" fps 15 ACT_DIEBACKWARD 1 { event 2002 54 } 
$sequence "die_beta2" "die_beta2" fps 15 ACT_DIEBACKWARD 1 { event 2002 30 } 
$sequence "idle2" "idle2" fps 12 ACT_IDLE 50 
$sequence "bigflinch_a1a0b" "bigflinch" fps 15
$sequence stomp_it "Agrunt_stomp" AX AY AZR fps 24

// End of QC script.
