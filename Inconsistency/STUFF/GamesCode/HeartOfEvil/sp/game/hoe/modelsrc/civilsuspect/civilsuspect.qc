$modelname "civilsuspect.mdl"
$cdmaterials "models\civilsuspect"
$cliptotextures

$surfaceprop "flesh"

$scale 1.0

// 3 attachments
//$attachment 0 "Bip01 R Hand" 20.000000 2.000000 5.000000 
//$attachment 1 "Bip01 R Hand" 15.000000 1.500000 3.750000 
//$attachment 2 "Bip01 R Hand" 30.000000 3.000000 7.500000 

// 2 bone controllers
//$controller 0 "Bip01 Head" XR -60 60
//$controller mouth "Bone05" ZR 0 45

// These are needed for head-turning
$attachment "eyes" "Bip01 Head" 0.00 -0.00 -0.00 rotate -0.00 81.00 90.00
$attachment "forward" "Bip01 Spine" 0.00 -0.00 0.00 rotate -0.00 81.00 90.00

// Material footstep sounds
$attachment "LeftFoot" "Bip01 L Foot" 0.00 0.00 0.00 rotate 0.00 0.00 0.00
$attachment "RightFoot" "Bip01 R Foot" 0.00 0.00 0.00 rotate 0.00 0.00 0.00

// 20 hit boxes
$hbox 3 "Bip01 Pelvis" -4.690000 -4.440000 -6.750000  4.000000 5.560000 6.750000
$hbox 6 "Bip01 L Leg" 2.660000 -3.690000 -3.090000  18.160000 4.880000 3.310000
$hbox 6 "Bip01 L Leg1" 0.380000 -3.970000 -2.840000  17.600000 4.000000 2.940000
$hbox 6 "Bip01 L Foot" -0.590000 -2.340000 -2.630000  3.790000 8.000000 2.190000
$hbox 7 "Bip01 R Leg" 2.470000 -3.690000 -3.160000  18.129999 4.880000 3.380000
$hbox 7 "Bip01 R Leg1" 0.310000 -3.970000 -2.840000  17.600000 3.940000 2.970000
$hbox 7 "Bip01 R Foot" -0.560000 -2.340000 -2.190000  3.810000 8.000000 2.660000
$hbox 3 "Bip01 Spine1" -3.250000 -5.500000 -5.500000  4.830000 5.500000 5.500000
$hbox 2 "Bip01 Spine2" -0.060000 -5.530000 -7.590000  8.000000 7.000000 7.590000
$hbox 2 "Bip01 Spine3" -2.250000 -6.810000 -6.310000  6.500000 5.090000 6.310000
$hbox 2 "Bip01 Neck" -3.110000 -1.500000 -3.000000  2.050000 3.500000 3.000000
$hbox 1 "Bip01 Head" 0.090000 -3.660000 -3.000000  8.410000 5.090000 3.000000
$hbox 4 "Bip01 L Arm" 0.940000 -2.880000 -4.130000  5.440000 4.280000 3.500000
$hbox 4 "Bip01 L Arm1" -2.160000 -2.340000 -2.560000  11.560000 3.410000 2.380000
$hbox 4 "Bip01 L Arm2" 0.590000 -1.810000 -2.190000  10.750000 2.840000 2.410000
$hbox 4 "Bip01 L Hand" 0.000000 -1.000000 -2.000000  3.000000 1.500000 3.500000
$hbox 5 "Bip01 R Arm" 1.380000 -2.840000 -3.560000  5.690000 4.310000 4.310000
$hbox 5 "Bip01 R Arm1" -1.880000 -2.470000 -2.160000  11.810000 3.280000 2.810000
$hbox 5 "Bip01 R Arm2" 0.440000 -1.970000 -2.340000  10.590000 2.720000 2.220000
$hbox 5 "Bip01 R Hand" 0.000000 -1.000000 -2.000000  3.000000 1.500000 3.500000

$bodygroup body
{
studio "body_civilsuspect.smd"
}

//$bodygroup weapon
//{
//blank
//studio "ref_p_ak47.smd"
//}


// sequences
$sequence idle1 "idle1" loop fps 15 ACT_IDLE 50 
//$sequence signal1 "idle1" loop fps 40 ACT_SIGNAL1 1
//$sequence signal2 "idle1" loop fps 40 ACT_SIGNAL2 1
//$sequence signal3 "idle1" loop fps 40 ACT_SIGNAL3 1
//$sequence cower "idle1" loop fps 15 ACT_COWER 1
//$sequence combatidle "idle1" loop fps 30 ACT_IDLE_ANGRY 1
// $sequence walk "walk" LX loop fps 30 {
// //	{ event 1004 3 "NPC_Barney.FootstepRight" }
// //	{ event 1004 18 "NPC_Barney.FootstepLeft" }
// 	{ event 6007 3 } // CL_EVENT_MFOOTSTEP_RIGHT
// 	{ event 6006 18 } // CL_EVENT_MFOOTSTEP_LEFT
// 	activity ACT_WALK 1
// } 
// $sequence run "run" LX loop fps 25 {
// //	{ event 1004 5 "NPC_Barney.RunFootstepLeft" }
// //	{ event 1004 13 "NPC_Barney.RunFootstepRight" }
// 	{ event 6007 5 } // CL_EVENT_MFOOTSTEP_RIGHT_LOUD
// 	{ event 6006 13 } // CL_EVENT_MFOOTSTEP_LEFT_LOUD
// 	activity ACT_RUN 1
// } 
//$sequence shootmp5 "shoot_mp5_blend1" "shoot_mp5_blend2" fps 10 ACT_RANGE_ATTACK1 1 blend XR -50 50 { event 3 0 } { event 4 1 } { event 5 2 } { event 5001 0 "50" } { event 5001 1 "50" } { event 5001 2 "50" }
//$sequence shootshotgun "shoot_shotgun_blend1" "shoot_shotgun_blend2" fps 5 ACT_RANGE_ATTACK1 1 blend XR -50 50 { event 3 0 } { event 5001 0 "51" }
//$sequence shootrpg "shoot_rpg_blend1" "shoot_rpg_blend2" fps 10 ACT_RANGE_ATTACK1 1 blend XR -50 50 { event 3 0 } 
//$sequence reload "reload" fps 15 ACT_RELOAD 1 { event 2 8 }
$sequence turnleft "turnleft" fps 15 ACT_TURN_LEFT 1 
$sequence turnright "turnright" fps 15 ACT_TURN_RIGHT 1 
$sequence laflinch "laflinch" fps 16 ACT_FLINCH_LEFTARM 1 
$sequence raflinch "raflinch" fps 16 ACT_FLINCH_RIGHTARM 1 
$sequence llflinch "llflinch" fps 20 ACT_FLINCH_LEFTLEG 1 
$sequence rlflinch "rlflinch" fps 20 ACT_FLINCH_RIGHTLEG 1 
$sequence smlflinch "smlflinch" fps 30 ACT_SMALL_FLINCH 1 
$sequence diesimple "diesimple" X fps 15 ACT_DIE_HEADSHOT 1 { event 7 0 } { event 2001 5 } 
$sequence dieviolent "dieviolent" X fps 20 ACT_DIE_HEADSHOT 1 { event 7 0 } { event 2001 10 } 
$sequence diegutshot "diegutshot" fps 15 ACT_DIE_GUTSHOT 1 { event 7 0 } { event 2001 33 } 
$sequence dieforward "dieforward" X fps 18 ACT_DIEFORWARD 1 { event 7 0 } { event 2001 8 } 
$sequence diebackward "diebackward" X fps 15 ACT_DIEBACKWARD 1 { event 7 0 } { event 2001 19 } 
$sequence diecrump "diecrump" X fps 18 ACT_DIESIMPLE 2 { event 7 0 } { event 2001 13 } 
$sequence barnaclehit "barnaclehit" fps 15 ACT_BARNACLE_HIT 1 
$sequence barnaclepull "barnaclepull" loop fps 45 ACT_BARNACLE_PULL 1 
$sequence barnaclecrunch "barnaclecrunch" fps 5 ACT_BARNACLE_CHOMP 1 
$sequence barnaclechew "barnaclechew" loop fps 5 ACT_BARNACLE_CHEW 1 
$sequence barn_wave "barn_wave" fps 22 
$sequence buttonpush "buttonpush" fps 20

$sequence namd1_grab seq_namd1_grab fps 30 fadeout 0
$sequence namd1_hold seq_namd1_hold fps 15 loop
$sequence namd1_shove seq_namd1_shove fps 30 fadeout 0

$include movement.qci

// head movement from HL:S
$poseparameter head_yaw -90.00 90.00
$animation neck_subtract "neck_center" fps 30
$animation neck_right "neck_right" fps 30.000000 subtract neck_subtract 0
$animation neck_center "neck_center" fps 30.000000 subtract neck_subtract 0
$animation neck_left "neck_left" fps 30.000000 subtract neck_subtract 0
$sequence barney_neckturn "neck_right" fps 30.00 {
  blendwidth 3
  blend head_yaw -90.000000 90.000000
  delta
  autoplay
 neck_center neck_left
}

// mouth stuff taken from HL:S
$poseparameter mouth 0.00 90.00
$animation pose_subtract "mouth_close" fps 30.000000
$animation mouth_close "mouth_close" fps 30.000000 subtract pose_subtract 0
$animation mouth_open "mouth_open" fps 30.000000 subtract pose_subtract 0
$sequence barney_mouth "mouth_close" fps 30.00 {
  blendwidth 2
  blend mouth 0.000000 90.000000
  delta
  autoplay
 mouth_open
}

// collision_model.smd and $collisionjoints taken from hl1 barney model
$sequence ragdoll "ragdoll_pose" ACT_DIERAGDOLL 1 fps 30.00
$collisionjoints "collision_model.smd" {

	$mass 115.0
	$inertia 10.00
	$damping 0.01
	$rotdamping 1.50
	$rootbone "bip01"

	$jointconstrain "bip01 spine2" x limit -60.00 60.00 0.20
	$jointconstrain "bip01 spine2" y limit -25.00 25.00 0.20
	$jointconstrain "bip01 spine2" z limit -25.00 30.00 0.20

	$jointconstrain "bip01 r arm1" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 r arm1" y limit -45.00 64.00 0.20
	$jointconstrain "bip01 r arm1" z limit -73.00 12.00 0.20

	$jointconstrain "bip01 r arm2" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 r arm2" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 r arm2" z limit -120.00 4.00 0.20

	$jointconstrain "bip01 l arm1" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 l arm1" y limit -67.00 30.00 0.20
	$jointconstrain "bip01 l arm1" z limit -73.00 12.00 0.20

	$jointconstrain "bip01 l arm2" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 l arm2" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 l arm2" z limit -120.00 4.00 0.20

	$jointconstrain "bip01 l hand" x limit -60.00 60.00 0.20
	$jointconstrain "bip01 l hand" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 l hand" z limit -57.00 70.00 0.20

	$jointconstrain "bip01 r hand" x limit -60.00 60.00 0.20
	$jointconstrain "bip01 r hand" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 r hand" z limit -57.00 70.00 0.20

	$jointconstrain "bip01 r leg" x limit -25.00 20.00 0.20
	$jointconstrain "bip01 r leg" y limit -40.00 20.00 0.20
	$jointconstrain "bip01 r leg" z limit -37.00 63.00 0.20

	$jointconstrain "bip01 r leg1" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 r leg1" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 r leg1" z limit -95.00 4.00 0.20

	$jointrotdamping "bip01 head" 5.00
	$jointconstrain "bip01 head" x limit -20.00 20.00 0.20
	$jointconstrain "bip01 head" y limit -25.00 25.00 0.20
	$jointconstrain "bip01 head" z limit -13.00 30.00 0.20

	$jointconstrain "bip01 l leg" x limit -25.00 20.00 0.20
	$jointconstrain "bip01 l leg" y limit -20.00 40.00 0.20
	$jointconstrain "bip01 l leg" z limit -37.00 63.00 0.20

	$jointconstrain "bip01 l leg1" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 l leg1" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 l leg1" z limit -95.00 4.00 0.20

	$jointconstrain "bip01 l foot" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 l foot" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 l foot" z limit -15.00 35.00 0.20

	$jointconstrain "bip01 r foot" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 r foot" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 r foot" z limit -15.00 35.00 0.20
}
