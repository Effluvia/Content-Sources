$modelname "superzombie\superzombie.mdl"
$cdmaterials "models\superzombie"
$cliptotextures

$scale 1.0

$surfaceprop "zombieflesh"

$eyeposition 0.000000 0.000000 63.000000

// 1 attachment
//$attachment 0 "Bip01 R Hand" 30.000000 7.5000000 10.00000
$attachment "head" "Bip01 Head" 0.000000 0.000000 0.00000
$attachment "neck" "Bip01 Neck" 0.500000 1.000000 0.00000

// These are needed for head-turning
$attachment "eyes" "Bip01 Head" 0.00 -0.00 -0.00 rotate -0.00 81.00 90.00
$attachment "forward" "Bip01 Spine" 0.00 -0.00 0.00 rotate -0.00 81.00 90.00

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

$bodygroup head
{
studio "ref_Head_Knifed.smd"
studio "ref_Head_SwissCheese.smd"
blank
}

$bodygroup body
{
studio "ref_Body_Knifed.smd"
studio "ref_Body_SwissCheese.smd"
}

//$bodygroup weapons
//{
//studio "ref_Weapon_M16.smd"
//studio "ref_Weapon_870.smd"
//studio "ref_Weapon_M60.smd"
//studio "ref_Weapon_RPG7.smd"
//blank
//}

$bodygroup ammo
{
blank
studio "ref_ammo.smd"
}


// sequences
$sequence idle1 "idle1" loop fps 15 ACT_IDLE 1 {
	node standing
}
$sequence combatidle "seq_combatidle_up" "seq_combatidle_down" loop fps 10 {
	blend aim_pitch -40 40
    activity ACT_IDLE_ANGRY_RPG 1
    node "standingAngry"
}
$sequence combatidle_hip "seq_combatidle_hip" loop fps 10 {
	addlayer layer_Aim_hip
    activity ACT_IDLE_ANGRY 1
    node "standingAngryHip"
}


$sequence walk "walk" LX loop fps 30 {
	activity ACT_WALK 1
	{ event AE_NPC_RIGHTFOOT 13 "NPC_Spx.FootstepRight" }
	{ event AE_NPC_LEFTFOOT 29 "NPC_Spx.FootstepLeft" }
}
$sequence lurch "lurch" LX loop fps 30 {
	activity ACT_WALK_HURT 1
	{ event AE_NPC_RIGHTFOOT 9 "NPC_Spx.FootstepRight" }
	{ event AE_NPC_LEFTFOOT 29 "NPC_Spx.FootstepLeft" }
}

$animation a_aim_hip_neutral seq_fire_m16 frame 0 0
$animation a_aim_hip_up seq_fire_hip_up frame 0 0 subtract a_aim_hip_neutral 0
$animation a_aim_hip_down seq_fire_hip_down frame 0 0 subtract a_aim_hip_neutral 0

$sequence layer_Aim_hip {
	a_aim_hip_up
	a_aim_hip_down
	blend aim_pitch -45 45
	delta
	hidden
}

$sequence shootm16 "seq_fire_m16" fps 10 {
	activity ACT_RANGE_ATTACK_SMG1 1
	addlayer layer_Aim_hip
	node "shootingHip"
	snap
}

$sequence shootshotgun "seq_fire_hip_up" "seq_fire_hip_down" {
	activity ACT_RANGE_ATTACK_SHOTGUN 1
	blend aim_pitch -45 45
//	{ event 5001 0 "51" }
//	{ event 2 0 }
//	{ event 4 6 }
	node "shootingHip"
}

$sequence shootm60 "seq_fire_hip_up" "seq_fire_hip_down" fps 60 {
	activity ACT_RANGE_ATTACK_AR2 1
	blend aim_pitch -45 45
//	{ event 5001 0 "50" } { event 2 0 }
	node "shootingHip"
}

$sequence shootrpg7 "seq_fire_up" "seq_fire_down" fps 20 {
	activity ACT_RANGE_ATTACK_RPG 1
	blend aim_pitch -40 40
//	{ event 2 0 }
	node shooting
}

// idle-stand -> shoot (hip)
$sequence idle_to_fire_hip "seq_idle_to_fire_up_hip" "seq_idle_to_fire_down_hip" fps 60.0 {
	blend aim_pitch -40 40
	activity ACT_TRANSITION 1
	transition standing shootingHip
}

// idle-stand -> shoot
$sequence idle_to_fire "seq_idle_to_fire_up" "seq_idle_to_fire_down" fps 60.0 {
	blend aim_pitch -40 40
	activity ACT_TRANSITION 1
	transition standing shooting
}


$sequence bite1 "seq_bite1" fps 30 ACT_MELEE_ATTACK1 1 { event AE_ZOMBIE_BITE 12 }
$sequence bite2 "seq_bite2" fps 30 ACT_MELEE_ATTACK1 1 { event AE_ZOMBIE_BITE 12 }
$sequence frontkick "seq_frontkick_hip" fps 30 ACT_MELEE_ATTACK2 1 { event AE_ZOMBIE_KICK 17 }
$sequence reload "reload" fps 15 ACT_RELOAD 1 { event 3015 8 }
$sequence eat "seq_eat" fps 20 ACT_ZOMBIE_EAT 1
$sequence turnleft "turnleft" fps 15 ACT_TURN_LEFT 1 
$sequence turnright "turnright" fps 15 ACT_TURN_RIGHT 1 
$sequence laflinch "laflinch" fps 16 ACT_FLINCH_LEFTARM 1 
$sequence raflinch "raflinch" fps 16 ACT_FLINCH_RIGHTARM 1 
$sequence llflinch "llflinch" fps 20 ACT_FLINCH_LEFTLEG 1 
$sequence rlflinch "rlflinch" fps 20 ACT_FLINCH_RIGHTLEG 1 
$sequence smlflinch "smlflinch" fps 30 ACT_SMALL_FLINCH 1 
$sequence diesimple "diesimple" X fps 15 ACT_DIE_HEADSHOT 1 { event 2001 5 } 
$sequence dieviolent "dieviolent" X fps 20 ACT_DIE_HEADSHOT 1 { event 2001 10 } 
$sequence diegutshot "diegutshot" fps 15 ACT_DIE_GUTSHOT 1 { event 2001 33 } 
$sequence dieforward "dieforward" X fps 18 ACT_DIEFORWARD 1 { event 2001 8 } 
$sequence diebackward "diebackward" X fps 15 ACT_DIEBACKWARD 1 { event 2001 19 } 
$sequence diecrump "diecrump" X fps 18 ACT_DIESIMPLE 2 { event 2001 13 } 
$sequence barnaclehit "barnaclehit" fps 15 ACT_BARNACLE_HIT 1 
$sequence barnaclepull "barnaclepull" loop fps 45 ACT_BARNACLE_PULL 1 
$sequence barnaclecrunch "barnaclecrunch" fps 5 ACT_BARNACLE_CHOMP 1 
$sequence barnaclechew "barnaclechew" loop fps 5 ACT_BARNACLE_CHEW 1 

// Cheap being-chainsawed animation
$animation a_chainsawed diebackward frame 2 6 fixuploop 0 1
$sequence chainsawed a_chainsawed fps 20 loop activity ACT_ZOMBIE_CHAINSAWED 1

$sequence namd3_slump seq_namd3_slump loop fps 1
//$animation a_namd3_rise seq_namd3_rise alignto idle1
$sequence namd3_rise seq_namd3_rise fps 40 fadeout 0
$animation a_namd3_grab seq_namd3_grab alignto idle1
$sequence namd3_grab a_namd3_grab fadein 0 fadeout 0
$animation a_namd3_strangle seq_namd3_strangle alignto idle1

// 1 event is to shoot the bullet, the other for muzzleflash/brass ejection
$sequence namd3_strangle a_namd3_strangle fadein 0 loop {
	{ event AE_NPC_WEAPON_FIRE 0 "0" }  { event AE_NPC_WEAPON_SET_ACTIVITY 0 "ACT_USE" }
	{ event AE_NPC_WEAPON_FIRE 5 "0" }  { event AE_NPC_WEAPON_SET_ACTIVITY 5 "ACT_USE" }
	{ event AE_NPC_WEAPON_FIRE 10 "0" } { event AE_NPC_WEAPON_SET_ACTIVITY 10 "ACT_USE" }
	{ event AE_NPC_WEAPON_FIRE 15 "0" } { event AE_NPC_WEAPON_SET_ACTIVITY 15 "ACT_USE" }
	{ event AE_NPC_WEAPON_FIRE 20 "0" } { event AE_NPC_WEAPON_SET_ACTIVITY 20 "ACT_USE" }
}
$animation a_namd3_toss seq_namd3_toss alignto idle1
$sequence namd3_toss a_namd3_toss fps 20 fadein 0 fadeout 0

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
	$jointconstrain "bip01 r arm1" y limit -65.00 64.00 0.20
	$jointconstrain "bip01 r arm1" z limit -73.00 12.00 0.20

	$jointconstrain "bip01 r arm2" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 r arm2" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 r arm2" z limit -120.00 4.00 0.20

	$jointconstrain "bip01 l arm1" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 l arm1" y limit -67.00 65.00 0.20
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
