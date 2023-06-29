$modelname "SOG\SOG.mdl"
$cdmaterials "models\SOG"
$cliptotextures

$surfaceprop "flesh"

$scale 1.0

$eyeposition 0.000000 0.000000 63.000000

// 1 attachment
//$attachment 0 "Bip01 R Hand" 30.000000 7.5000000 10.00000
$attachment "head" "Bip01 Head" 0.00 0.00 0.00 rotate 0 90 -180
$attachment "neck" "Bip01 Neck" 0.500000 1.000000 0.00000

// These are needed for head-turning
$attachment "eyes" "Bip01 Head" 0.00 0.00 0.00 rotate 0 90 -180
$attachment "forward" "Bip01 Neck" 0.00 0.00 0.00 rotate 0 80 90

// Left-hand attachment for tossing grenades
$attachment "lefthand" "Bip01 L Hand" 6.00 0.00 0.00 rotate 0.00 0.00 0.00

// Attachment for hoe_corpse_marker
$attachment "CorpseMarker" "Bip01 Pelvis" 0.00 0.00 0.00 rotate 0.00 0.00 0.00

// Water splash attachments
$attachment "left_foot" "Bip01 L Foot" 5.80 2.00 -2.00 rotate -0 0 0
$attachment "right_foot" "Bip01 R Foot" 5.00 2.00 2.00 rotate 0 -0 -0

// 2 bone controllers
//$controller 0 "Bip01 Head" XR -30 30
//$controller mouth "Bone05" ZR 0 25

$poseparameter "aim_pitch" -90 90 loop 360
$poseparameter "aim_yaw" -90 90 loop 360

// Serves as a hint to the game code that the named bone will be used for
// bone merges during the game. If this flag is not present, the bone can
// still be used but you'll get performance warnings. Weapons have a bone
// with the same name.
$bonemerge "Bip01 R Hand"

// 20 hit boxes
$hbox 3 "Bip01 Pelvis" -4.690000 -6.440000 -6.750000  5.000000 5.560000 6.750000
$hbox 6 "Bip01 L Leg" 2.660000 -3.690000 -3.090000  18.160000 4.880000 3.310000
$hbox 6 "Bip01 L Leg1" 0.380000 -3.970000 -2.840000  17.600000 4.000000 2.940000
$hbox 6 "Bip01 L Foot" -0.590000 -2.340000 -2.630000  3.790000 8.000000 2.190000
$hbox 7 "Bip01 R Leg" 2.470000 -3.690000 -3.160000  18.129999 4.880000 3.380000
$hbox 7 "Bip01 R Leg1" 0.310000 -3.970000 -2.840000  17.600000 3.940000 2.970000
$hbox 7 "Bip01 R Foot" -0.560000 -2.340000 -2.190000  3.810000 8.000000 2.660000
$hbox 3 "Bip01 Spine1" -1.250000 -8.500000 -5.500000  5.830000 5.500000 5.500000
$hbox 2 "Bip01 Spine2" -0.060000 -8.530000 -7.0000  7.000000 7.000000 7.0000
$hbox 2 "Bip01 Spine3" -2.250000 -4.00000 -6.310000  5.500000 5.090000 6.310000
$hbox 1 "Bip01 Neck" -3.110000 -1.500000 -3.000000  2.050000 3.500000 3.000000
$hbox 1 "Bip01 Head" -1.0000 -1.5000 -3.000000  9.0000 7.000000 3.000000
$hbox 4 "Bip01 L Arm" 0.940000 -4.880000 -4.130000  5.440000 2.280000 3.500000
$hbox 4 "Bip01 L Arm1" -2.160000 -4.340000 -2.560000  11.560000 1.410000 2.380000
$hbox 4 "Bip01 L Arm2" 0.590000 -1.810000 -2.190000  12.750000 2.840000 2.410000
$hbox 4 "Bip01 L Hand" 3.000000 -1.000000 -2.000000  6.000000 1.500000 3.500000
$hbox 5 "Bip01 R Arm" 1.380000 -4.840000 -3.560000  5.690000 2.310000 4.310000
$hbox 5 "Bip01 R Arm1" -1.880000 -4.470000 -2.160000  11.810000 1.280000 2.810000
$hbox 5 "Bip01 R Arm2" 0.440000 -1.970000 -2.340000  12.590000 2.720000 2.220000
$hbox 5 "Bip01 R Hand" 3.000000 -1.000000 -2.000000  6.000000 1.500000 3.500000

$bodygroup face
{
studio "SOG_Face_Blond_Hair.smd"
studio "SOG_Face_Brown_Hair.smd"
studio "SOG_Face_Grey_Hair.smd"
studio "SOG_Face_Stealth.smd"
studio "SOG_Face_Zombie.smd"
studio "ref_Neck.smd"

studio "SOG_Face_Blond_Hair_Off.smd"
studio "SOG_Face_Brown_Hair_Off.smd"
studio "SOG_Face_Grey_Hair_Off.smd"
studio "SOG_Face_Stealth_Off.smd"
studio "SOG_Face_Zombie_Off.smd"
}

$bodygroup body
{
studio "Body_SOG_noface.smd"
}

//$bodygroup weapons
//{
//studio "ref_Weapon_M21.smd"
//blank
//}

// Sequences

// Idle
$sequence idle "seq_idle" loop fps 20 {
	activity ACT_IDLE 1
	node standing
}
$sequence crouch_idle "seq_crouch_idle" loop fps 20 {
	activity ACT_CROUCHIDLE 1
	node crouching
}

// $sequence combatidle "seq_combatidle" loop fps 10 {
//     activity ACT_IDLE_ANGRY 1
// }
// $sequence crouch_combatidle "seq_crouch_combatidle" loop fps 20 {
//     activity ACT_COVER_LOW 1
// }

$sequence crouch "seq_crouch" fps 20 {
	activity ACT_TRANSITION 1
	transition "standing" "crouching"
}
$sequence uncrouch "seq_uncrouch" fps 20 {
	activity ACT_TRANSITION 1
	transition "crouching" "standing"
}

$sequence cower "seq_cower" fps 28 {
    activity ACT_COWER 1
}
$sequence crouch_cower "seq_crouch_cower" fps 28 {
    activity ACT_COWER_LOW 1
}

// Walking
// $sequence walk "seq_walk" LX loop fps 30 {
//     { event AE_NPC_RIGHTFOOT 3 "NPC_SOG.FootstepRight" }
//     { event AE_NPC_LEFTFOOT 18 "NPC_SOG.FootstepLeft" }
//     activity ACT_WALK 1
// } 
$sequence limp_walk "seq_limp_walk" LX loop fps 25 {
    { event AE_NPC_RIGHTFOOT 3 "NPC_SOG.FootstepRight" }
    { event AE_NPC_LEFTFOOT 18 "NPC_SOG.FootstepLeft" }
    activity ACT_WALK_HURT 1
    node walking
} 
$sequence crouch_walk "seq_crouch_walk" LX loop fps 30 {
    { event AE_NPC_RIGHTFOOT 3 "NPC_SOG.FootstepRight" }
    { event AE_NPC_LEFTFOOT 18 "NPC_SOG.FootstepLeft" }
    activity ACT_WALK_CROUCH 1
    node walking
} 
$sequence crouch_limp_walk "seq_crouch_limp_walk" LX loop fps 25 {
    { event AE_NPC_RIGHTFOOT 3 "NPC_SOG.FootstepRight" }
    { event AE_NPC_LEFTFOOT 18 "NPC_SOG.FootstepLeft" }
    activity ACT_WALK_CROUCH_HURT 1
    node walking
} 

// Running
// $sequence run "seq_run" LX loop fps 50 {
//     { event AE_NPC_RIGHTFOOT 14 "NPC_SOG.RunFootstepRight" }
//     { event AE_NPC_LEFTFOOT 31 "NPC_SOG.RunFootstepLeft" }
//     activity ACT_RUN 1
// } 
$sequence limp_run "seq_limp_run" LX loop fps 45 {
    { event AE_NPC_RIGHTFOOT 14 "NPC_SOG.RunFootstepRight" }
    { event AE_NPC_LEFTFOOT 31 "NPC_SOG.RunFootstepLeft" }
    activity ACT_RUN_HURT 1
    node running
} 
$sequence crouch_run "seq_crouch_run" LX loop fps 50 {
    { event AE_NPC_RIGHTFOOT 14 "NPC_SOG.RunFootstepRight" }
    { event AE_NPC_LEFTFOOT 31 "NPC_SOG.RunFootstepLeft" }
    activity ACT_RUN_CROUCH 1
    node running
} 
$sequence crouch_limp_run "seq_crouch_limp_run" LX loop fps 45 {
    { event AE_NPC_RIGHTFOOT 14 "NPC_SOG.RunFootstepRight" }
    { event AE_NPC_LEFTFOOT 31 "NPC_SOG.RunFootstepLeft" }
    activity ACT_RUN_CROUCH_HURT 1
    node running
} 

// Attacking
// $sequence shootm21 "seq_fire_up" "seq_fire_down" fps 20 {
// 	blend aim_pitch -40 40
// //	{ event 5001 0 "50" } { event 2 0 }
// 	activity ACT_RANGE_ATTACK_SNIPER_RIFLE 1
// }
// $sequence crouch_shootm21 "seq_crouch_fire_up" "seq_crouch_fire_down" fps 20 {
// 	blend aim_pitch -40 40
// //	{ event 5001 0 "50" } { event 2 0 }
// 	activity ACT_RANGE_ATTACK1_LOW 1
// }

$sequence throwgrenade "seq_throwgrenade" fps 30 { event AE_HUMAN_GRENADE_TOSS 38 } activity ACT_RANGE_ATTACK2 1
$sequence crouch_throwgrenade "seq_crouch_throwgrenade" fps 30 { event AE_HUMAN_GRENADE_TOSS 52 } activity ACT_RANGE_ATTACK2_LOW 1

$sequence dropgrenade "seq_dropgrenade" fps 30 { event AE_HUMAN_GRENADE_DROP 40 } activity ACT_SPECIAL_ATTACK1 1
$sequence crouch_dropgrenade "seq_crouch_dropgrenade" fps 30 { event AE_HUMAN_GRENADE_DROP 48 } activity ACT_SPECIAL_ATTACK1_LOW 1 

$sequence frontkick "seq_frontkick" fps 40 {
	{ event AE_NPC_KICK 12 }
	{ event 2010 12 } 
	activity ACT_MELEE_ATTACK1 1
}

$sequence reload "seq_reload" fps 40 {
    { event 3015 40 }
    activity ACT_RELOAD 1
    node standing
}
$sequence crouch_reload "seq_crouch_reload" fps 40 {
    { event 3015 48 } 
    activity ACT_RELOAD_LOW 1
    node crouching
} 

//Signalling
$sequence signal_advance "seq_signal_advance" fps 30 activity ACT_SIGNAL1 1
$sequence crouch_signal_advance "seq_crouch_signal_advance" fps 30

$sequence signal_flank "seq_signal_flank" fps 40 activity ACT_SIGNAL2 1
$sequence crouch_signal_flank "seq_crouch_signal_flank" fps 40

$sequence signal_retreat "seq_signal_retreat" fps 40 activity ACT_SIGNAL3 1
$sequence crouch_signal_retreat "seq_crouch_signal_retreat" fps 40

// Turning
// $sequence turnleft "seq_turnleft" fps 15 {
//     activity ACT_TURN_LEFT 1
// }
// $sequence turnright "seq_turnright" fps 15 {
//     activity ACT_TURN_RIGHT 1
// }

// $sequence crouch_turnleft "seq_crouch_turnleft" fps 15 {
//     activity ACT_TURN_LEFT_LOW 1
// }
// $sequence crouch_turnright "seq_crouch_turnright" fps 15 {
//     activity ACT_TURN_RIGHT_LOW 1
// }

// Flinching
$sequence laflinch "seq_laflinch" fps 16
$sequence raflinch "seq_raflinch" fps 16
$sequence llflinch "seq_llflinch" fps 20
$sequence rlflinch "seq_rlflinch" fps 20
$sequence smlflinch "seq_smflinch" fps 30 activity ACT_SMALL_FLINCH 1

$sequence crouch_laflinch "seq_crouch_laflinch" fps 16
$sequence crouch_raflinch "seq_crouch_raflinch" fps 16
$sequence crouch_llflinch "seq_crouch_llflinch" fps 20
$sequence crouch_rlflinch "seq_crouch_rlflinch" fps 20
$sequence crouch_smlflinch "seq_crouch_smflinch" fps 30

$sequence g_flinch1 seq_smflinch subtract g_flinch1 0 fps 30 activity ACT_GESTURE_SMALL_FLINCH 1 delta

// These are used when retrieving a weapon
$sequence pickup_ground "seq_crouch" fps 20 {
	{ event 2040 8 }
	activity ACT_PICKUP_GROUND 1
}
$sequence pickup_rack "seq_pickup" fps 30 {
	{ event 2040 11 }
	activity ACT_PICKUP_RACK 1
}

// Dying (on the spot)
$sequence die_headshot "seq_die_headshot" X fps 20 { event 8 7 } { event 2001 10 } 
$sequence die_chestshot "seq_die_chestshot" X fps 15 { event 8 2 } { event 2001 5 } 
$sequence die_gutshot "seq_die_gutshot" fps 15 { event 8 17 } { event 2001 33 } 
$sequence die_forward "seq_die_forward" X fps 18 { event 8 2 } { event 2001 8 } 
$sequence die_backward "seq_die_backward" X fps 15 { event 8 4 } { event 2001 19 } 
$sequence die_simple "seq_die_simple" X fps 18 { event 8 5 } { event 2001 13 } 

$sequence crouch_die_headshot "seq_crouch_die_headshot" X fps 30 { event 8 15 } { event 2001 30 } 
$sequence crouch_die_chestshot "seq_crouch_die_chestshot" X fps 30 { event 8 12 } { event 2001 30 } 
$sequence crouch_die_gutshot "seq_crouch_die_gutshot" fps 30 { event 8 8 } { event 2001 38 } 
$sequence crouch_die_forward "seq_crouch_die_forward" X fps 30 { event 8 8 } { event 2001 30 } 
$sequence crouch_die_backward "seq_crouch_die_backward" X fps 30 { event 8 8 } { event 2001 30 } 
$sequence crouch_die_simple "seq_crouch_die_simple" X fps 30 { event 8 2 } { event 2001 14 } 

// Dying (thrown forcefully into air by explosion)
$sequence explosion_hit_forward "seq_explosion_hit_forward" LX LY fps 30 { event 8 4 }
$sequence explosion_fly_forward "seq_explosion_fly_forward" loop fps 30
$sequence explosion_land_forward "seq_explosion_land_forward" LX LY fps 30 { event 2001 9 }

$sequence explosion_hit_backward "seq_explosion_hit_backwards" LX LY fps 30 { event 8 4 }
$sequence explosion_fly_backward "seq_explosion_fly_backwards" loop fps 30
$sequence explosion_land_backward "seq_explosion_land_backwards" LX LY fps 30 { event 2001 9 }

$sequence explosion_hit_left "seq_explosion_hit_left" LX LY fps 30 { event 8 4 }
$sequence explosion_fly_left "seq_explosion_fly_left" loop fps 30
$sequence explosion_land_left "seq_explosion_land_left" LX LY fps 30 { event 2001 8 }

$sequence explosion_hit_right "seq_explosion_hit_right" LX LY fps 30 { event 8 4 }
$sequence explosion_fly_right "seq_explosion_fly_right" loop fps 30
$sequence explosion_land_right "seq_explosion_land_right" LX LY fps 30 { event 2001 9 }

// Barnacled
$sequence barnaclehit "seq_barnaclehit" fps 15 ACT_BARNACLE_HIT 1 
$sequence barnaclepull "seq_barnaclepull" loop fps 45 ACT_BARNACLE_PULL 1 
$sequence barnaclecrunch "seq_barnaclecrunch" fps 5 ACT_BARNACLE_CHOMP 1 
$sequence barnaclechew "seq_barnaclechew" loop fps 5 ACT_BARNACLE_CHEW 1 
$sequence barnaclereleased "seq_explosion_fly_forward" loop fps 30 ACT_BARNACLE_RELEASED 1
$sequence barnacleground "seq_barnacle_ground" loop fps 30 ACT_BARNACLE_HIT_GROUND 1

// Repel
$sequence repel_jump "seq_repel_jump" fps 22 ACT_HOVER 1 
$sequence rappel_jump0 "seq_rappel_jump0" fps 22 LX LY LZ activity ACT_RAPPEL_JUMP0 1
$sequence rappel_jump2 "seq_rappel_jump2" fps 22 LX LY LZ activity ACT_RAPPEL_JUMP2 1
$sequence repel_repel "seq_repel_repel" loop fps 18 {
	activity ACT_RAPPEL_LOOP 1
}
$sequence repel_shoot "seq_repel_fire_down" "seq_repel_fire_up" loop fps 18 ACT_FLY 1 blend aim_pitch -40 40 { event 5001 1 "50" } { event 5001 3 "50" } { event 5001 5 "50" } { event 2 1 } { event 3 3 } { event 4 5 } 
$sequence repel_land "seq_repel_land" fps 20 ACT_LAND 1 


// Corpse Sequences


$sequence lying_on_back "seq_lying_on_back" fps 1 
$sequence lying_on_side "seq_lying_on_side" fps 1 
$sequence lying_on_stomach "seq_lying_on_stomach" fps 1 
$sequence hanging_byfeet "seq_hanging_byfeet" loop fps 10
$sequence hanging_byarms "seq_hanging_byarms" loop fps 10
$sequence hanging_byneck "seq_hanging_byneck" fps 10
$sequence deadsitting "seq_deadsitting" fps 1
$sequence deadseated "seq_deadseated" fps 1

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

$include movement.qci

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
	$jointconstrain "bip01 r arm1" y limit -65.00 64.00 0.20
	$jointconstrain "bip01 r arm1" z limit -73.00 12.00 0.20

	$jointconstrain "bip01 r arm2" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 r arm2" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 r arm2" z limit -100.00 4.00 0.20

	$jointconstrain "bip01 l arm1" x limit -40.00 40.00 0.00
	$jointconstrain "bip01 l arm1" y limit -67.00 65.00 0.20
	$jointconstrain "bip01 l arm1" z limit -73.00 12.00 0.20

	$jointconstrain "bip01 l arm2" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 l arm2" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 l arm2" z limit -100.00 4.00 0.20

	$jointconstrain "bip01 l hand" x limit -60.00 60.00 0.20
	$jointconstrain "bip01 l hand" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 l hand" z limit -30.00 20.00 0.20

	$jointconstrain "bip01 r hand" x limit -60.00 60.00 0.20
	$jointconstrain "bip01 r hand" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 r hand" z limit -30.00 20.00 0.20

	$jointconstrain "bip01 r leg" x limit -25.00 20.00 0.20
	$jointconstrain "bip01 r leg" y limit -40.00 20.00 0.20
	$jointconstrain "bip01 r leg" z limit -37.00 83.00 0.20

	$jointconstrain "bip01 r leg1" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 r leg1" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 r leg1" z limit -125.00 4.00 0.20

	$jointrotdamping "bip01 head" 5.00
	$jointconstrain "bip01 head" x limit -20.00 20.00 0.20
	$jointconstrain "bip01 head" y limit -25.00 25.00 0.20
	$jointconstrain "bip01 head" z limit -13.00 30.00 0.20

	$jointconstrain "bip01 l leg" x limit -25.00 20.00 0.20
	$jointconstrain "bip01 l leg" y limit -20.00 40.00 0.20
	$jointconstrain "bip01 l leg" z limit -37.00 83.00 0.20

	$jointconstrain "bip01 l leg1" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 l leg1" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 l leg1" z limit -125.00 4.00 0.20

	$jointconstrain "bip01 l foot" x limit -20.00 20.00 0.00
	$jointconstrain "bip01 l foot" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 l foot" z limit -15.00 35.00 0.20

	$jointconstrain "bip01 r foot" x limit -20.00 20.00 0.00
	$jointconstrain "bip01 r foot" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 r foot" z limit -15.00 35.00 0.20


	$jointcollide "Bip01 Pelvis" "Bip01 L Arm1"
	$jointcollide "Bip01 Pelvis" "Bip01 L Arm2"
	$jointcollide "Bip01 Pelvis" "Bip01 L Hand"
	$jointcollide "Bip01 Pelvis" "Bip01 R Arm1"
	$jointcollide "Bip01 Pelvis" "Bip01 R Arm2"
	$jointcollide "Bip01 Pelvis" "Bip01 R Hand"

	$jointcollide "Bip01 Spine2" "Bip01 L Arm2"
	$jointcollide "Bip01 Spine2" "Bip01 L Hand"
	$jointcollide "Bip01 Spine2" "Bip01 R Arm2"
	$jointcollide "Bip01 Spine2" "Bip01 R Hand"

	$jointcollide "Bip01 Head" "Bip01 L Arm1"
	$jointcollide "Bip01 Head" "Bip01 L Arm2"
	$jointcollide "Bip01 Head" "Bip01 L Hand"
	$jointcollide "Bip01 Head" "Bip01 R Arm1"
	$jointcollide "Bip01 Head" "Bip01 R Arm2"
	$jointcollide "Bip01 Head" "Bip01 R Hand"

	$jointcollide "Bip01 L Leg" "Bip01 R Leg"
	$jointcollide "Bip01 L Leg" "Bip01 R Leg1"
	$jointcollide "Bip01 L Leg" "Bip01 R Foot"
	$jointcollide "Bip01 L Leg1" "Bip01 R Leg"
	$jointcollide "Bip01 L Leg1" "Bip01 R Leg1"
	$jointcollide "Bip01 L Leg1" "Bip01 R Foot"
	$jointcollide "Bip01 L Foot" "Bip01 R Leg"
	$jointcollide "Bip01 L Foot" "Bip01 R Leg1"
	$jointcollide "Bip01 L Foot" "Bip01 R Foot"

	$jointcollide "Bip01 L Leg" "Bip01 L Arm1"
	$jointcollide "Bip01 L Leg" "Bip01 L Arm2"
	$jointcollide "Bip01 L Leg" "Bip01 L Hand"
	$jointcollide "Bip01 L Leg1" "Bip01 L Arm1"
	$jointcollide "Bip01 L Leg1" "Bip01 L Arm2"
	$jointcollide "Bip01 L Leg1" "Bip01 L Hand"
	$jointcollide "Bip01 L Foot" "Bip01 L Arm1"
	$jointcollide "Bip01 L Foot" "Bip01 L Arm2"
	$jointcollide "Bip01 L Foot" "Bip01 L Hand"

	$jointcollide "Bip01 L Leg" "Bip01 R Arm1"
	$jointcollide "Bip01 L Leg" "Bip01 R Arm2"
	$jointcollide "Bip01 L Leg" "Bip01 R Hand"
	$jointcollide "Bip01 L Leg1" "Bip01 R Arm1"
	$jointcollide "Bip01 L Leg1" "Bip01 R Arm2"
	$jointcollide "Bip01 L Leg1" "Bip01 R Hand"
	$jointcollide "Bip01 L Foot" "Bip01 R Arm1"
	$jointcollide "Bip01 L Foot" "Bip01 R Arm2"
	$jointcollide "Bip01 L Foot" "Bip01 R Hand"

	$jointcollide "Bip01 R Leg" "Bip01 L Arm1"
	$jointcollide "Bip01 R Leg" "Bip01 L Arm2"
	$jointcollide "Bip01 R Leg" "Bip01 L Hand"
	$jointcollide "Bip01 R Leg1" "Bip01 L Arm1"
	$jointcollide "Bip01 R Leg1" "Bip01 L Arm2"
	$jointcollide "Bip01 R Leg1" "Bip01 L Hand"
	$jointcollide "Bip01 R Foot" "Bip01 L Arm1"
	$jointcollide "Bip01 R Foot" "Bip01 L Arm2"
	$jointcollide "Bip01 R Foot" "Bip01 L Hand"

	$jointcollide "Bip01 R Leg" "Bip01 R Arm1"
	$jointcollide "Bip01 R Leg" "Bip01 R Arm2"
	$jointcollide "Bip01 R Leg" "Bip01 R Hand"
	$jointcollide "Bip01 R Leg1" "Bip01 R Arm1"
	$jointcollide "Bip01 R Leg1" "Bip01 R Arm2"
	$jointcollide "Bip01 R Leg1" "Bip01 R Hand"
	$jointcollide "Bip01 R Foot" "Bip01 R Arm1"
	$jointcollide "Bip01 R Foot" "Bip01 R Arm2"
	$jointcollide "Bip01 R Foot" "Bip01 R Hand"
}
