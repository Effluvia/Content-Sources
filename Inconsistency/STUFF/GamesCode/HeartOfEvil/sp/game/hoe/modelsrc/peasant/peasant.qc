$cdmaterials "models\peasant"
$cliptotextures

$scale 1.0

$surfaceprop "flesh"

$eyeposition 0.000000 0.000000 60.000000

$bbox -16.000000 -16.000000 0.000000 16.000000 16.000000 72.000000

// 3 attachments
//$attachment 0 "Bip02 R Hand" 30.000000 7.5000000 10.00000
$attachment "head" "Bip02 Head" 0.00 0.00 0.00 rotate 0 90 90
$attachment "neck" "Bip02 Neck" 0.500000 1.000000 0.00000

// Needed for head-turning
$attachment "eyes" "Bip02 Head" 5.05 1.82 0.21 rotate -0.00 80.01 90.00
$attachment "forward" "Bip02 Spine" 0.00 -0.00 0.00 rotate -0.00 90.00 90.00

// Left-hand attachment for dropping grenade
$attachment "lefthand" "Bip02 L Hand" 3.00 0.00 0.00 rotate 0.00 0.00 0.00

// Attachment for hoe_corpse_marker
$attachment "CorpseMarker" "Bip02 Pelvis" 0.00 0.00 0.00 rotate 0.00 0.00 0.00

// Corpse rope attachments
$attachment "rope_lefthand" "Bip02 L Hand" -0.00 -0.00 2.00 rotate -0 -0 0
$attachment "rope_righthand" "Bip02 R Hand" -0.00 0.00 -2.00 rotate 0 0 0

$attachment "shackle_left" "Bip02 L Arm2" 10.00 -0.50 0.00 rotate 90 -0 0
$attachment "shackle_right" "Bip02 R Arm2" 10.00 -0.50 -0.00 rotate 90 -0 0

// 2 bone controllers
//$controller 0 "Bip02 Head" XR -60 60
//$controller mouth "Bone01" ZR 0 20

// 19 hit boxes
$hbox 6 "Bip02 L Leg" 0.000000 -5.690000 -3.550000  18.709999 3.610000 3.430000
$hbox 6 "Bip02 L Leg1" 0.000000 -3.770000 -2.970000  16.959999 2.210000 2.410000
$hbox 6 "Bip02 L Foot" -0.070000 -2.290000 -3.140000  3.620000 8.170000 1.450000
$hbox 7 "Bip02 R Leg" 0.000000 -5.960000 -3.770000  18.639999 3.580000 3.390000
$hbox 7 "Bip02 R Leg1" 0.000000 -3.850000 -2.580000  16.969999 2.160000 2.980000
$hbox 7 "Bip02 R Foot" -0.110000 -2.290000 -1.380000  3.610000 8.170000 2.990000
$hbox 3 "Bip02 Pelvis" -6.670000 -4.400000 -6.460000  5.120000 5.170000 6.830000
$hbox 3 "Bip02 Spine" 0.000000 -3.290000 -6.830000  5.320000 5.000000 6.910000
$hbox 2 "Bip02 Spine1" 0.000000 -3.900000 -6.200000  5.700000 5.670000 5.910000
$hbox 2 "Bip02 Spine2" 0.000000 -3.650000 -5.970000  8.990000 4.930000 5.440000
$hbox 4 "Bip02 L Arm" 0.000000 -3.170000 -1.870000  5.470000 2.950000 1.580000
$hbox 4 "Bip02 L Arm1" 0.000000 -3.400000 -1.950000  12.950000 2.060000 2.460000
$hbox 4 "Bip02 L Arm2" 0.000000 -2.880000 -2.280000  11.550000 2.100000 1.700000
$hbox 4 "Bip02 L Hand" 0.000000 -1.640000 -1.960000  4.120000 1.790000 1.770000
$hbox 5 "Bip02 R Arm" 0.000000 -3.080000 -1.440000  5.890000 2.970000 2.450000
$hbox 5 "Bip02 R Arm1" 0.000000 -3.350000 -2.710000  12.920000 2.400000 2.110000
$hbox 5 "Bip02 R Arm2" 0.000000 -2.950000 -1.970000  11.250000 2.220000 2.450000
$hbox 5 "Bip02 R Hand" 0.000000 -1.850000 -1.770000  3.930000 1.590000 2.080000
$hbox 1 "Bip02 Head" 0.000000 -3.960000 -4.860000  10.020000 6.230000 5.070000

//$bodygroup weapon
//{
//blank
//}

// 128 sequences
$sequence walk "walk" LX loop fps 20  {
	{ event AE_NPC_RIGHTFOOT 0 "NPC_Barney.FootstepRight" }
	{ event AE_NPC_LEFTFOOT 8 "NPC_Barney.FootstepLeft" }
	activity ACT_WALK 1
} 
$sequence walk_scared "walk_scared" LX loop fps 20  {
	{ event AE_NPC_RIGHTFOOT 0 "NPC_Barney.FootstepRight" }
	{ event AE_NPC_LEFTFOOT 8 "NPC_Barney.FootstepLeft" }
	activity ACT_WALK_SCARED 1
} 
$sequence run "run" LX loop fps 25 {
	{ event AE_NPC_LEFTFOOT 0 "NPC_Barney.RunFootstepLeft" }
	{ event AE_NPC_RIGHTFOOT 9 "NPC_Barney.RunFootstepRight" }
	activity ACT_RUN 1
} 
$sequence run1 "run1" LX loop fps 30 ACT_RUN_SCARED 1 {
	{ event AE_NPC_LEFTFOOT 0 "NPC_Barney.RunFootstepLeft" }
	{ event AE_NPC_RIGHTFOOT 9 "NPC_Barney.RunFootstepRight" }
	activity ACT_RUN_SCARED 1
}  
$sequence run2 "run2" LX loop fps 30 {
	{ event AE_NPC_LEFTFOOT 0 "NPC_Barney.RunFootstepLeft" }
	{ event AE_NPC_RIGHTFOOT 9 "NPC_Barney.RunFootstepRight" }
	activity ACT_RUN_SCARED 1
}
//$sequence 180_Left "180_Left" LX loop fps 30 ACT_TURN_LEFT 1 
//$sequence 180_Right "180_Right" LX loop fps 15 ACT_TURN_RIGHT 1 
$sequence 180_Left "180_Left" LX fps 30 ACT_TURN_LEFT 1 
$sequence 180_Right "180_Right" LX fps 15 ACT_TURN_RIGHT 1 
$sequence flinch "flinch" fps 30 ACT_BIG_FLINCH 1 
$sequence flinch1 "flinch1" fps 20 ACT_SMALL_FLINCH 1 
$sequence laflinch "laflinch" fps 15 ACT_FLINCH_LEFTARM 1 
$sequence raflinch "raflinch" fps 15 ACT_FLINCH_RIGHTARM 1 
$sequence llflinch "llflinch" fps 15 ACT_FLINCH_LEFTLEG 1 
$sequence rlflinch "rlflinch" fps 15 ACT_FLINCH_RIGHTLEG 1 
$sequence idle1 "idle1" loop fps 18 ACT_IDLE 50 
$sequence idle3 "idle3" fps 15 ACT_IDLE 1 
$sequence idle4 "idle4" fps 15 ACT_IDLE 2 
$sequence idle5 "idle5" fps 15 ACT_IDLE 1 
$sequence idle6 "idle6" fps 15 ACT_IDLE 1 
$sequence idle7 "idle7" fps 10 ACT_IDLE 10 
$sequence crouchstand "crouchstand" fps 18 ACT_STAND 1 
$sequence crouch_idle "crouch_idle" loop fps 18 ACT_CROUCHIDLE 1 
$sequence crouch_idle2 "crouch_idle2" loop fps 18 ACT_CROUCHIDLE 1 
$sequence crouch_idle3 "crouch_idle3" loop fps 18 ACT_CROUCHIDLE 1 
$sequence panic "panic" fps 18 ACT_EXCITED 1 
$sequence fear1 "fear1" fps 18 ACT_FEAR_DISPLAY 1 
$sequence fear2 "fear2" fps 18 ACT_FEAR_DISPLAY 1 
$sequence eye_wipe "eye_wipe" fps 18 ACT_FEAR_DISPLAY 1 
$sequence diesimple "diesimple" fps 24 ACT_DIESIMPLE 1 {
	{ event 2001 8 }
	{ event AE_RAGDOLL 19 }
} 
$sequence dieforward "dieforward" X fps 20 ACT_DIE_CHESTSHOT 1 {
	{ event 2001 26 } 
	{ event AE_RAGDOLL 30 }
}
$sequence dieforward1 "dieforward1" X fps 16 ACT_DIEFORWARD 1 {
	{ event 2001 11 } 
	{ event AE_RAGDOLL 24 }
}
$sequence diebackward "diebackward" X fps 15 ACT_DIEBACKWARD 1 {
	{ event 2001 8 }
	{ event AE_RAGDOLL 15 }
}
$sequence headshot "headshot" X fps 15 ACT_DIE_HEADSHOT 20 {
	{ event 2001 18 } 
	{ event AE_RAGDOLL 21 }
}
$sequence gutshot "gutshot" X fps 15 ACT_DIE_GUTSHOT 1 {
	{ event 2001 16 }
	{ event AE_RAGDOLL 20 }
}
$sequence barnacled1 "barnacled1" fps 18 ACT_BARNACLE_HIT 1 
$sequence barnacled2 "barnacled2" loop fps 28 ACT_BARNACLE_PULL 1 
$sequence barnacled3 "barnacled3" fps 18 ACT_BARNACLE_CHOMP 1 
$sequence barnacled4 "barnacled4" loop fps 15 ACT_BARNACLE_CHEW 1 

$sequence lying_on_back "lying_on_back_ragdoll" fps 30 
$sequence lying_on_stomach "lying_on_stomach" fps 30 
$sequence dead_sitting "dead_sitting" fps 30 
$sequence dead_table1 "dead_table1" fps 30 
$sequence dead_table2 "dead_table2" fps 30 
$sequence dead_table3 "dead_table3" fps 30 
$sequence lookwindow "lookwindow" loop fps 15
$sequence dropgrenade "dropgrenade" { event AE_HUMAN_GRENADE_DROP 31 "8.0"}
$sequence hanging_idle "hanging_idle" loop fps 20 
$sequence lean "lean" fps 12 
$sequence wave "wave" fps 22 
$sequence sitidle "sitidle" loop fps 14
$sequence locked_door "locked_door" fps 16 {
	{ event 1004 11 "Buttons.latchlocked1" }
	{ event 1004 21 "Buttons.latchlocked2" }
	{ event 1004 31 "Buttons.latchlocked1" }
}
$sequence rocketcrawl "rocketcrawl" loop fps 20 
$sequence pondering "pondering" fps 12 
$sequence c1a4_wounded_idle "c1a4_wounded_idle" loop fps 12 
$sequence cowering_in_corner "cowering_in_corner" loop fps 18

$animation mdldecompiler_delta.smd "mdldecompiler_delta.smd" fps 30

// head movement from HL:S
$poseparameter head_yaw -90.00 90.00
$animation neck_right "neck_right" fps 30.000000 subtract mdldecompiler_delta.smd 0
$animation neck_center "neck_center" fps 30.000000 subtract mdldecompiler_delta.smd 0
$animation neck_left "neck_left" fps 30.000000 subtract mdldecompiler_delta.smd 0
$sequence peasant_neckturn "neck_right" fps 30.00 {
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
$sequence peasant_mouth "mouth_close" fps 30.00 {
  blendwidth 2
  blend mouth 0.000000 90.000000
  delta
  autoplay
 mouth_open
}

// collision_model.smd and $collisionjoints taken from hl1 scientist model
// FIXME: the scale is wrong
$sequence ragdoll "ragdoll_pose" ACT_DIERAGDOLL 1 fps 30.00
$collisionjoints "collision_model.smd" {
	$mass 115.0
	$inertia 10.00
	$damping 0.01
	$rotdamping 1.50
	$rootbone "bip02"

	$jointconstrain "bip02 spine1" x limit -60.00 60.00 0.20
	$jointconstrain "bip02 spine1" y limit -25.00 25.00 0.20
	$jointconstrain "bip02 spine1" z limit -25.00 30.00 0.20

	$jointconstrain "bip02 r arm1" x limit -40.00 40.00 0.00
	$jointconstrain "bip02 r arm1" y limit -5.00 115.00 0.20
	$jointconstrain "bip02 r arm1" z limit -160.00 12.00 0.20

	$jointconstrain "bip02 l arm1" x limit -40.00 40.00 0.00
	$jointconstrain "bip02 l arm1" y limit -115.00 5.00 0.20
	$jointconstrain "bip02 l arm1" z limit -160.00 12.00 0.20

	$jointconstrain "bip02 l arm2" x limit 0.00 0.00 0.00
	$jointconstrain "bip02 l arm2" y limit 0.00 0.00 0.00
	$jointconstrain "bip02 l arm2" z limit -120.00 4.00 0.20

	$jointconstrain "bip02 l hand" x limit -60.00 60.00 0.20
	$jointconstrain "bip02 l hand" y limit 0.00 0.00 0.00
	$jointconstrain "bip02 l hand" z limit -57.00 70.00 0.20

	$jointconstrain "bip02 r arm2" x limit 0.00 0.00 0.00
	$jointconstrain "bip02 r arm2" y limit 0.00 0.00 0.00
	$jointconstrain "bip02 r arm2" z limit -120.00 4.00 0.20

	$jointconstrain "bip02 r hand" x limit -60.00 60.00 0.20
	$jointconstrain "bip02 r hand" y limit 0.00 0.00 0.00
	$jointconstrain "bip02 r hand" z limit -57.00 70.00 0.20

	$jointconstrain "bip02 r leg" x limit -25.00 20.00 0.20
	$jointconstrain "bip02 r leg" y limit -40.00 20.00 0.20
	$jointconstrain "bip02 r leg" z limit -37.00 63.00 0.20

	$jointconstrain "bip02 r leg1" x limit 0.00 0.00 0.00
	$jointconstrain "bip02 r leg1" y limit 0.00 0.00 0.00
	$jointconstrain "bip02 r leg1" z limit -95.00 4.00 0.20

	$jointrotdamping "bip02 head" 5.00
	$jointconstrain "bip02 head" x limit -20.00 20.00 0.20
	$jointconstrain "bip02 head" y limit -25.00 25.00 0.20
	$jointconstrain "bip02 head" z limit -13.00 30.00 0.20

	$jointconstrain "bip02 l leg" x limit -25.00 20.00 0.20
	$jointconstrain "bip02 l leg" y limit -20.00 40.00 0.20
	$jointconstrain "bip02 l leg" z limit -37.00 63.00 0.20

	$jointconstrain "bip02 l leg1" x limit 0.00 0.00 0.00
	$jointconstrain "bip02 l leg1" y limit 0.00 0.00 0.00
	$jointconstrain "bip02 l leg1" z limit -95.00 4.00 0.20

	$jointconstrain "bip02 l foot" x limit 0.00 0.00 0.00
	$jointconstrain "bip02 l foot" y limit 0.00 0.00 0.00
	$jointconstrain "bip02 l foot" z limit -15.00 35.00 0.20

	$jointconstrain "bip02 r foot" x limit 0.00 0.00 0.00
	$jointconstrain "bip02 r foot" y limit 0.00 0.00 0.00
	$jointconstrain "bip02 r foot" z limit -15.00 35.00 0.20
}
