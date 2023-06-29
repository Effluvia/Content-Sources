$modelname "barneyzombie\barneyzombie.mdl"
$cdmaterials "models\barneyzombie"
$cliptotextures

// TNB
$surfaceprop "flesh"

$scale 1.0

$eyeposition 0.000000 0.000000 63.000000

// 1 attachment
//$attachment 0 "Bip01 R Hand" 13.000000 5.500000 8.50000
$attachment 1 "Bip01 Head" 0.000000 0.000000 0.00000
$attachment 2 "Bip01 Neck" 0.500000 1.000000 0.00000

// These are needed for head-turning
$attachment "eyes" "Bip01 Head" 0.00 -0.00 -0.00 rotate -0.00 81.00 90.00
$attachment "forward" "Bip01 Spine" 0.00 -0.00 0.00 rotate -0.00 81.00 90.00

// fake attachment so Weapon_Bone isn't removed
$attachment "WeaponBoneAttachment" "Weapon_Bone" 0.00 0.00 0.00 rotate 0.00 0.00 0.00

// 2 bone controllers
//$controller 0 "Bip01 Head" XR -30 30
//$controller mouth "Bone05" ZR 0 15

$poseparameter "aim_pitch" -90 90 loop 360
$poseparameter "aim_yaw" -90 90 loop 360

// Serves as a hint to the game code that the named bone will be used for
// bone merges during the game. If this flag is not present, the bone can
// still be used but you'll get performance warnings. Weapons have a bone
// with the same name.
$bonemerge "Bip01 R Hand"
$bonemerge "Weapon_Bone"

// 21 hit boxes
$hbox 3 "Bip01 Pelvis" -9.890000 -5.660000 -8.600000  2.940000 6.520000 6.790000
$hbox 6 "Bip01 L Leg" 0.000000 -5.920000 -4.000000  19.370001 3.780000 3.730000
$hbox 6 "Bip01 L Leg1" 0.000000 -5.690000 -3.620000  18.830000 3.070000 2.940000
$hbox 6 "Bip01 L Foot" 0.000000 -2.990000 -4.040000  6.420000 8.680000 1.610000
$hbox 7 "Bip01 R Leg" 0.000000 -5.930000 -3.870000  19.270000 3.750000 3.790000
$hbox 7 "Bip01 R Leg1" -0.330000 -5.730000 -2.980000  18.950001 2.960000 3.190000
$hbox 7 "Bip01 R Foot" 0.000000 -3.240000 -2.320000  6.210000 8.440000 3.280000
$hbox 3 "Bip01 Spine" 0.000000 -4.890000 -6.970000  8.370000 8.000000 6.730000
$hbox 2 "Bip01 Spine2" 0.000000 -3.460000 -8.320000  6.110000 8.000000 7.530000
$hbox 2 "Bip01 Spine3" -1.000000 -5.990000 -8.220000  5.960000 8.000000 8.480000
$hbox 4 "Bip01 L Arm" -0.400000 -4.180000 0.000000  7.200000 4.750000 4.200000
$hbox 4 "Bip01 L Arm1" -2.520000 -4.570000 -3.920000  9.860000 2.280000 3.430000
$hbox 4 "Bip01 L Arm2" 0.000000 -2.850000 -2.080000  11.900000 2.850000 2.750000
$hbox 4 "Bip01 L Hand" 0.000000 -1.320000 -2.290000  4.660000 2.600000 1.650000
$hbox 5 "Bip01 R Arm" 0.000000 -4.150000 -4.190000  7.530000 4.120000 4.240000
$hbox 5 "Bip01 R Arm1" -2.690000 -4.650000 -3.420000  9.470000 2.280000 3.760000
$hbox 5 "Bip01 R Arm2" 0.000000 -2.730000 -2.560000  11.000000 3.140000 2.190000
$hbox 5 "Bip01 R Hand" 0.000000 -1.320000 -2.910000  10.380000 3.230000 4.000000
$hbox 1 "Bip01 Head" -0.660000 2.250000 -4.240000  6.000000 7.970000 3.630000
$hbox 1 "Bip01 Head" -0.660000 -4.250000 -5.240000  6.000000 2.250000 4.630000
$hbox 1 "Bip01 Head" 6.000000 -4.250000 -5.240000  11.190000 7.970000 4.630000

$bodygroup head
{
studio "ref_Head.smd"
blank
}

$bodygroup body
{
studio "ref_Body.smd"
}

//$bodygroup weapons
//{
//studio "ref_Weapon_Colt1911A1.smd"
//blank
//}

// Sequences

// Idle
$sequence idle "seq_idle" loop fps 20 ACT_IDLE 1
$sequence combatidle "seq_idle" loop fps 20 ACT_IDLE_ANGRY_PISTOL 1

// Walking
$sequence walk "seq_walk" LX loop fps 30 {
	{ event AE_NPC_RIGHTFOOT 13 "NPC_Spx.FootstepRight" }
	{ event AE_NPC_LEFTFOOT 29 "NPC_Spx.FootstepLeft" }
//	{ event 6007 13 } // CL_EVENT_MFOOTSTEP_RIGHT
//	{ event 6006 29 } // CL_EVENT_MFOOTSTEP_LEFT
	activity ACT_WALK 1
}
$sequence lurch "seq_limp_walk" LX loop fps 30  {
	{ event AE_NPC_RIGHTFOOT 8 "NPC_Spx.FootstepRight" }
	{ event AE_NPC_LEFTFOOT 29 "NPC_Spx.FootstepLeft" }
//	{ event 6007 8 } // CL_EVENT_MFOOTSTEP_RIGHT
//	{ event 6006 29 } // CL_EVENT_MFOOTSTEP_LEFT
	activity ACT_WALK_HURT 1
} 

// Attacking
$sequence shootcolt1911a1 "seq_fire_up" "seq_fire_down" fps 20 {
	blend aim_pitch -50 50 
	activity ACT_RANGE_ATTACK_PISTOL 1
}
$sequence bite1 "seq_bite1" fps 30 ACT_MELEE_ATTACK1 1 { event AE_ZOMBIE_BITE 12 }
$sequence bite2 "seq_bite2" fps 30 ACT_MELEE_ATTACK1 1 { event AE_ZOMBIE_BITE 12 }
$sequence frontkick "seq_frontkick_hip" fps 30 ACT_MELEE_ATTACK2 1 { event AE_ZOMBIE_KICK 17 }

$sequence reload "reload" fps 15 { event 3015 8 } activity ACT_RELOAD_PISTOL 1
$sequence eat "seq_eat" fps 20 ACT_ZOMBIE_EAT 1

// Turning
$sequence turnleft "seq_turnleft" fps 15 ACT_TURN_LEFT 1
$sequence turnright "seq_turnright" fps 15 ACT_TURN_RIGHT 1

// Flinching
$sequence laflinch "seq_laflinch" fps 16 ACT_FLINCH_LEFTARM 1
$sequence raflinch "seq_raflinch" fps 16 ACT_FLINCH_RIGHTARM 1
$sequence llflinch "seq_llflinch" fps 20 ACT_FLINCH_LEFTLEG 1
$sequence rlflinch "seq_rlflinch" fps 20 ACT_FLINCH_RIGHTLEG 1
$sequence smlflinch "seq_smflinch" fps 30 ACT_SMALL_FLINCH 1

// Dying (on the spot)
$sequence die_headshot "seq_die_headshot" X fps 20 ACT_DIE_HEADSHOT 1 { event 8 7 } { event 2001 10 } 
$sequence die_chestshot "seq_die_chestshot" X fps 15 ACT_DIE_CHESTSHOT 1 { event 8 2 } { event 2001 5 } 
$sequence die_gutshot "seq_die_gutshot" fps 15 ACT_DIE_GUTSHOT 1 { event 8 17 } { event 2001 33 } 
$sequence die_forward "seq_die_forward" X fps 18 ACT_DIEFORWARD 1 { event 8 2 } { event 2001 8 } 
$sequence die_backward "seq_die_backward" X fps 15 ACT_DIEBACKWARD 1 { event 8 4 } { event 2001 19 } 
$sequence die_simple "seq_die_simple" X fps 18 ACT_DIESIMPLE 1 { event 8 5 } { event 2001 13 } 

// Barnacled
$sequence barnaclehit "seq_barnaclehit" fps 15 ACT_BARNACLE_HIT 1 
$sequence barnaclepull "seq_barnaclepull" loop fps 45 ACT_BARNACLE_PULL 1 
$sequence barnaclecrunch "seq_barnaclecrunch" fps 5 ACT_BARNACLE_CHOMP 1 
$sequence barnaclechew "seq_barnaclechew" loop fps 5 ACT_BARNACLE_CHEW 1 


// Corpse Sequences


$sequence lying_on_back "seq_lying_on_back" fps 1 
$sequence lying_on_side "seq_lying_on_side" fps 1 
$sequence lying_on_stomach "seq_lying_on_stomach" fps 1 
$sequence hanging_byfeet "seq_hanging_byfeet" fps 10
$sequence hanging_byarms "seq_hanging_byarms" fps 10
$sequence hanging_byneck "seq_hanging_byneck" fps 10
$sequence deadsitting "seq_deadsitting" fps 1

$sequence getup "seq_getup" fps 20 fadeout 0
$sequence bashdoor "seq_bashdoor" fps 20 { event 1003 20 "1" } { event 1004 20 "Debris.metal1" }

$animation a_chainsawed seq_die_backward frame 3 6 fixuploop 0 1
$sequence chainsawed a_chainsawed fps 20 loop activity ACT_ZOMBIE_CHAINSAWED 1

$animation mdldecompiler_delta.smd "mdldecompiler_delta.smd" fps 30

// head movement from HL:S
$poseparameter head_yaw -90.00 90.00
$animation neck_right "neck_right" fps 30.000000 subtract mdldecompiler_delta.smd 0
$animation neck_center "neck_center" fps 30.000000 subtract mdldecompiler_delta.smd 0
$animation neck_left "neck_left" fps 30.000000 subtract mdldecompiler_delta.smd 0
$sequence barney_neckturn "neck_right" fps 30.00 {
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
