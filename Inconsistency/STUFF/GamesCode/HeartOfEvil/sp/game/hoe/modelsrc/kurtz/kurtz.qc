$modelname "kurtz\kurtz.mdl"
$cdmaterials "models\kurtz"
$cliptotextures

$surfaceprop "flesh"

$scale 1.0

$eyeposition 0.000000 0.000000 63.000000

// right hand is used for throwing Barney's head
$attachment "righthand" "Bip01 R Hand" 5.00 3.00 2.00 rotate 0 0 0

// 3 attachments
$attachment "head" "Bip01 Head" 0.00 0.00 0.00 rotate 0 90 90
$attachment "neck" "Bip01 Neck" 0.500000 1.000000 0.000000 

// These are needed for head-turning
$attachment "eyes" "Bip01 Head" 0.00 -0.00 -0.00 rotate -0.00 81.00 90.00
$attachment "forward" "Bip01 Spine" 0.00 -0.00 0.00 rotate -0.00 81.00 90.00

// Material footstep sounds
$attachment "LeftFoot" "Bip01 L Foot" 0.00 0.00 0.00 rotate 0.00 0.00 0.00
$attachment "RightFoot" "Bip01 R Foot" 0.00 0.00 0.00 rotate 0.00 0.00 0.00

// Attachment for hoe_corpse_marker
$attachment "CorpseMarker" "Bip01 Pelvis" 0.00 0.00 0.00 rotate 0.00 0.00 0.00

// 2 bone controllers
//$controller 0 "Bip01 Head" XR -30 30
//$controller mouth "Bone05" ZR 0 15

// 20 hit boxes
$hbox 3 "Bip01 Pelvis" -4.690000 -6.440000 -6.750000  5.000000 5.560000 6.750000
$hbox 6 "Bip01 L Leg" 2.660000 -3.690000 -3.090000  18.160000 4.880000 3.310000
$hbox 6 "Bip01 L Leg1" 0.380000 -3.970000 -2.840000  17.600000 4.000000 2.940000
$hbox 6 "Bip01 L Foot" -0.590000 -2.340000 -2.630000  3.790000 8.000000 2.190000
$hbox 7 "Bip01 R Leg" 2.470000 -3.690000 -3.160000  18.129999 4.880000 3.380000
$hbox 7 "Bip01 R Leg1" 0.310000 -3.970000 -2.840000  17.600000 3.940000 2.970000
$hbox 7 "Bip01 R Foot" -0.560000 -2.340000 -2.190000  3.810000 8.000000 2.660000
$hbox 3 "Bip01 Spine1" -1.250000 -8.500000 -5.500000  5.830000 5.500000 5.500000
$hbox 2 "Bip01 Spine2" -0.060000 -8.530000 -7.000000  7.000000 7.000000 7.000000
$hbox 2 "Bip01 Spine3" -2.250000 -4.000000 -6.310000  5.500000 5.090000 6.310000
$hbox 1 "Bip01 Neck" -3.110000 -1.500000 -3.000000  2.050000 3.500000 3.000000
$hbox 1 "Bip01 Head" -1.000000 -1.500000 -3.000000  9.000000 7.000000 3.000000
$hbox 4 "Bip01 L Arm" 0.940000 -4.880000 -4.130000  5.440000 2.280000 3.500000
$hbox 4 "Bip01 L Arm1" -2.160000 -4.340000 -2.560000  11.560000 1.410000 2.380000
$hbox 4 "Bip01 L Arm2" 0.590000 -1.810000 -2.190000  12.750000 2.840000 2.410000
$hbox 4 "Bip01 L Hand" 3.000000 -1.000000 -2.000000  6.000000 1.500000 3.500000
$hbox 5 "Bip01 R Arm" 1.380000 -4.840000 -3.560000  5.690000 2.310000 4.310000
$hbox 5 "Bip01 R Arm1" -1.880000 -4.470000 -2.160000  11.810000 1.280000 2.810000
$hbox 5 "Bip01 R Arm2" 0.440000 -1.970000 -2.340000  12.590000 2.720000 2.220000
$hbox 5 "Bip01 R Hand" 3.000000 -1.000000 -2.000000  6.000000 1.500000 3.500000

$bodygroup head
{
studio "ref_head.smd"
studio "ref_head_wberet.smd"
blank
}

$bodygroup body
{
studio "ref_body_uniform.smd"
studio "ref_body_blackpjs.smd"
}

$bodygroup weapon
{
blank
}

// 36 sequences
$sequence idle "idle" loop fps 10 {
	activity ACT_IDLE 1
}
$sequence walk "walk" LX loop fps 30 {
//	{ event AE_NPC_RIGHTFOOT 3 "NPC_Barney.FootstepRight" }
//	{ event AE_NPC_LEFTFOOT 18 "NPC_Barney.FootstepLeft" }
	{ event 6007 3 } // CL_EVENT_MFOOTSTEP_RIGHT
	{ event 6006 18 } // CL_EVENT_MFOOTSTEP_LEFT
	activity ACT_WALK 1
} 
$sequence limp_walk "limp_walk" LX loop fps 25 { event 1004 3 "common/npc_step1.wav" } { event 1004 18 "common/npc_step3.wav" } 
$sequence run "run" LX loop fps 25 { event 1004 5 "common/npc_step2.wav" } { event 1004 13 "common/npc_step4.wav" } 
$sequence limp_run "limp_run" LX loop fps 45 { event 1004 14 "common/npc_step2.wav" } { event 1004 31 "common/npc_step4.wav" } 
$sequence turnleft "turnleft" fps 15 
$sequence turnright "turnright" fps 15 
$sequence laflinch "laflinch" fps 16 
$sequence raflinch "raflinch" fps 16 
$sequence llflinch "llflinch" fps 20 
$sequence rlflinch "rlflinch" fps 20 
$sequence smlflinch "smlflinch" fps 30 
$sequence die_headshot "die_headshot" X fps 20 { event 8 7 } { event 2001 10 } 
$sequence die_chestshot "die_chestshot" X fps 15 { event 8 2 } { event 2001 5 } 
$sequence die_gutshot "die_gutshot" fps 15 { event 8 17 } { event 2001 33 } 
$sequence die_forward "die_forward" X fps 18 { event 8 2 } { event 2001 8 } 
$sequence die_backward "die_backward" X fps 15 { event 8 4 } { event 2001 19 } 
$sequence die_simple "die_simple" X fps 18 { event 8 5 } { event 2001 13 } 
$sequence explosion_hit_forward "explosion_hit_forward" LX LY fps 30 { event 8 4 } 
$sequence explosion_fly_forward "explosion_fly_forward" loop fps 30 
$sequence explosion_land_forward "explosion_land_forward" fps 30 { event 2001 9 } 
$sequence explosion_hit_backward "explosion_hit_backward" LX LY fps 30 { event 8 4 } 
$sequence explosion_fly_backward "explosion_fly_backward" loop fps 30 
$sequence explosion_land_backward "explosion_land_backward" fps 30 { event 2001 9 } 
$sequence explosion_hit_left "explosion_hit_left" LX LY fps 30 { event 8 4 } 
$sequence explosion_fly_left "explosion_fly_left" loop fps 30 
$sequence explosion_land_left "explosion_land_left" fps 30 { event 2001 8 } 
$sequence explosion_hit_right "explosion_hit_right" LX LY fps 30 { event 8 4 } 
$sequence explosion_fly_right "explosion_fly_right" loop fps 30 
$sequence explosion_land_right "explosion_land_right" fps 30 { event 2001 9 } 
$sequence barnaclehit "barnaclehit" fps 15 ACT_BARNACLE_HIT 1 
$sequence barnaclepull "barnaclepull" loop fps 45 ACT_BARNACLE_PULL 1 
$sequence barnaclecrunch "barnaclecrunch" fps 5 ACT_BARNACLE_CHOMP 1 
$sequence barnaclechew "barnaclechew" loop fps 5 ACT_BARNACLE_CHEW 1 
$sequence still "still" fps 1 
$sequence throwhead "throwhead" fps 20 { event AE_KURTZ_THROWHEAD 34 } 

$animation mdldecompiler_delta.smd "mdldecompiler_delta.smd" fps 30

// head movement from HL:S
$poseparameter head_yaw -90.00 90.00
$animation neck_right "neck_right" fps 30.000000 subtract mdldecompiler_delta.smd 0
$animation neck_center "neck_center" fps 30.000000 subtract mdldecompiler_delta.smd 0
$animation neck_left "neck_left" fps 30.000000 subtract mdldecompiler_delta.smd 0
$sequence kurtz_neckturn "neck_right" fps 30.00 {
  blendwidth 3
  blend head_yaw -90.000000 90.000000
  delta
  autoplay
 neck_center neck_left
}

// mouth stuff taken from HL:S
$poseparameter mouth 0.00 90.00
$animation mouth_close "mouth_close" fps 30.000000 subtract mdldecompiler_delta.smd 0
$animation mouth_open "mouth_open" fps 30.000000 subtract mdldecompiler_delta.smd 0
$sequence kurtz_mouth "mouth_close" fps 30.00 {
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

	$jointconstrain "bip01 r arm1" x limit -40.00 40.00 0.00
	$jointconstrain "bip01 r arm1" y limit -65.00 105.00 0.20
	$jointconstrain "bip01 r arm1" z limit -73.00 12.00 0.20

	$jointconstrain "bip01 r arm2" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 r arm2" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 r arm2" z limit -120.00 4.00 0.20

	$jointconstrain "bip01 l arm1" x limit -40.00 40.00 0.00
	$jointconstrain "bip01 l arm1" y limit -105.00 65.00 0.20
	$jointconstrain "bip01 l arm1" z limit -73.00 12.00 0.20

	$jointconstrain "bip01 l arm2" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 l arm2" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 l arm2" z limit -120.00 4.00 0.20

	$jointconstrain "bip01 l hand" x limit -60.00 60.00 0.20
	$jointconstrain "bip01 l hand" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 l hand" z limit -20.00 20.00 0.20

	$jointconstrain "bip01 r hand" x limit -60.00 60.00 0.20
	$jointconstrain "bip01 r hand" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 r hand" z limit -20.00 20.00 0.20

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
