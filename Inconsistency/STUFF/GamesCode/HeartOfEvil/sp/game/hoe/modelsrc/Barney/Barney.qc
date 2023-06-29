$modelname "Barney\barney.mdl"
$cdmaterials "models\Barney"
$cliptotextures

$surfaceprop "flesh"

$root "Bip01"

$scale 1.0

$eyeposition 0.000000 0.000000 63.000000

// Get rid of this bogus bone
$alwayscollapse Scene_Root

// x attachments
//$attachment 0 "Bip01 R Hand" 13.000000 5.500000 8.50000
$attachment "head" "Bip01 Head" 0.00 0.00 0.00 rotate 0 90 -180
$attachment "neck" "Bip01 Neck" 0.500000 1.000000 0.00000

// These are needed for head-turning
$attachment "eyes" "Bip01 Head" 4.70 6.00 -0.00 rotate -0 81 90
$attachment "forward" "Bip01 Neck" 0.00 0.00 0.00 rotate 0 81 90

// Left-hand attachment for tossing grenades
$attachment "lefthand" "Bip01 L Hand" 3.00 0.00 0.00 rotate 0.00 0.00 0.00

// Material footstep sounds
$attachment "LeftFoot" "Bip01 L Foot" 0.00 0.00 0.00 rotate 0.00 0.00 0.00
$attachment "RightFoot" "Bip01 R Foot" 0.00 0.00 0.00 rotate 0.00 0.00 0.00

// fake attachment so Weapon_Bone isn't removed
$attachment "WeaponBoneAttachment" "Weapon_Bone" 0.00 0.00 0.00 rotate 0.00 0.00 0.00

// Colt attachment when holstered
$attachment "Colt_Holster" "Bone03" 0.00 -3.00 0.00 rotate 0 -95 0

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
$bonemerge "Weapon_Bone"

//$ikchain rfoot "Bip01 R Foot" Z 0
//$ikchain lfoot "Bip01 L Foot" Z 0
//$ikautoplaylock rfoot 1.0 0.1
//$ikautoplaylock lfoot 1.0 0.1

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
studio "ref_Neck.smd"
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
$sequence idle "seq_idle" loop fps 20 {
	activity ACT_IDLE 1
//	ikrule lfoot footstep
//	ikrule rfoot footstep
	node standing
}
$sequence crouch_idle "seq_crouch_idle" loop fps 20 {
    activity ACT_CROUCHIDLE 1
    node crouching
}
$sequence sprint "sprint.smd" LX loop fps 50 {
}

//$sequence combatidle "seq_combatidle" loop fps 10 activity ACT_IDLE_ANGRY_PISTOL 1
//$sequence combatidle "seq_combatidle_up" "seq_combatidle_down" loop fps 10 {
//	blend aim_pitch -40 40
//    activity ACT_IDLE_ANGRY 1
//    node shooting
//}
//$sequence crouch_combatidle "seq_crouch_combatidle" loop fps 20 {
////	activity ACT_IDLE_ANGRY_PISTOL_LOW 1
////	activity ACT_IDLE_ANGRY_LOW 1
//	node shootingCrouched
//}

$sequence crouch "seq_crouch" fps 20 {
	activity ACT_TRANSITION 1
	transition standing crouching
}
$sequence uncrouch "seq_uncrouch" fps 20 {
	activity ACT_TRANSITION 1
	transition crouching standing
}

$sequence cower "seq_cower" fps 28 ACT_COWER 1
$sequence crouch_cower "seq_crouch_cower" fps 28 activity ACT_COVER_PISTOL_LOW 1

// Walking
// $sequence walk "seq_walk" LX loop fps 30 {
// //	{ event AE_NPC_RIGHTFOOT 3 "NPC_Barney.FootstepRight" }
// //	{ event AE_NPC_LEFTFOOT 18 "NPC_Barney.FootstepLeft" }
// 	{ event 6007 3 } // CL_EVENT_MFOOTSTEP_RIGHT
// 	{ event 6006 18 } // CL_EVENT_MFOOTSTEP_LEFT
// //	ikrule lfoot footstep range  16 17 26 31
// //	ikrule rfoot footstep range 0 2 14 21
// 	activity ACT_WALK 1
// }
$sequence limp_walk "seq_limp_walk" LX loop fps 25 {
//	{ event AE_NPC_RIGHTFOOT 3 "NPC_Barney.FootstepRight" }
//	{ event AE_NPC_LEFTFOOT 18 "NPC_Barney.FootstepLeft" }
	{ event 6007 3 } // CL_EVENT_MFOOTSTEP_RIGHT
	{ event 6006 18 } // CL_EVENT_MFOOTSTEP_LEFT
	activity ACT_WALK_HURT 1
}
$sequence crouch_walk "seq_crouch_walk" LX loop fps 30 {
//	{ event AE_NPC_RIGHTFOOT 3 "NPC_Barney.FootstepRight" }
//	{ event AE_NPC_LEFTFOOT 18 "NPC_Barney.FootstepLeft" }
	{ event 6007 3 } // CL_EVENT_MFOOTSTEP_RIGHT
	{ event 6006 18 } // CL_EVENT_MFOOTSTEP_LEFT
	activity ACT_WALK_CROUCH 1
}
$sequence crouch_limp_walk "seq_crouch_limp_walk" LX loop fps 25 {
//	{ event AE_NPC_RIGHTFOOT 3 "NPC_Barney.FootstepRight" }
//	{ event AE_NPC_LEFTFOOT 18 "NPC_Barney.FootstepLeft" }
	{ event 6007 3 } // CL_EVENT_MFOOTSTEP_RIGHT
	{ event 6006 18 } // CL_EVENT_MFOOTSTEP_LEFT
	activity ACT_WALK_CROUCH_HURT 1
} 

// Running
//$sequence run "seq_run" LX loop fps 25 {
////	{ event AE_NPC_LEFTFOOT 5 "NPC_Barney.RunFootstepLeft" }
////	{ event AE_NPC_RIGHTFOOT 13 "NPC_Barney.RunFootstepRight" }
//	{ event 6008 5 } // CL_EVENT_MFOOTSTEP_LEFT_LOUD
//	{ event 6009 13 } // CL_EVENT_MFOOTSTEP_RIGHT_LOUD
////	ikrule lfoot footstep range 4 5 6 6
////	ikrule rfoot footstep range 12 13 14 14
//	activity ACT_RUN 1
//}
$sequence limp_run "seq_limp_run" LX loop fps 45 {
//	{ event AE_NPC_LEFTFOOT 14 "NPC_Barney.RunFootstepLeft" }
//	{ event AE_NPC_RIGHTFOOT 31 "NPC_Barney.RunFootstepRight" }
	{ event 6008 14 } // CL_EVENT_MFOOTSTEP_LEFT_LOUD
	{ event 6009 31 } // CL_EVENT_MFOOTSTEP_RIGHT_LOUD
	activity ACT_RUN_HURT 1
}
$sequence crouch_run "seq_crouch_run" LX loop fps 50 {
//	{ event AE_NPC_LEFTFOOT 14 "NPC_Barney.RunFootstepLeft" }
//	{ event AE_NPC_RIGHTFOOT 31 "NPC_Barney.RunFootstepRight" }
	{ event 6008 14 } // CL_EVENT_MFOOTSTEP_LEFT_LOUD
	{ event 6009 31 } // CL_EVENT_MFOOTSTEP_RIGHT_LOUD
	activity ACT_RUN_CROUCH 1
}
$sequence crouch_limp_run "seq_crouch_limp_run" LX loop fps 45 {
//	{ event AE_NPC_LEFTFOOT 14 "NPC_Barney.RunFootstepLeft" }
//	{ event AE_NPC_RIGHTFOOT 31 "NPC_Barney.RunFootstepRight" }
	{ event 6008 14 } // CL_EVENT_MFOOTSTEP_LEFT_LOUD
	{ event 6009 31 } // CL_EVENT_MFOOTSTEP_RIGHT_LOUD
	activity ACT_RUN_CROUCH_HURT 1
} 

$include movement.qci

// Attacking
$sequence shootcolt1911a1 "seq_fire_up" "seq_fire_down" fps 20 {
	blend aim_pitch -50 50
//	activity ACT_RANGE_ATTACK_PISTOL 1
	node shooting
}
$sequence crouch_shootcolt1911a1 "seq_crouch_fire_up" "seq_crouch_fire_down" fps 20 {
	blend aim_pitch -40 40
//	activity ACT_RANGE_ATTACK_PISTOL_LOW 1
}

// idle-stand -> shoot
$sequence idle_to_fire "seq_idle_to_fire_up" "seq_idle_to_fire_down" fps 40.0 {
	blend aim_pitch -40 40
	activity ACT_TRANSITION 1
	transition standing shooting
}

// shoot -> idle-stand
$sequence fire_to_idle "seq_fire_to_idle_up" "seq_fire_to_idle_down" fps 40.0 {
	blend aim_pitch -40 40
	activity ACT_TRANSITION 1
	transition shooting standing
}

$sequence throwgrenade "seq_throwgrenade" fps 30 { event AE_HUMAN_GRENADE_TOSS 38 } activity ACT_RANGE_ATTACK2 1
$sequence crouch_throwgrenade "seq_crouch_throwgrenade" fps 30 { event AE_HUMAN_GRENADE_TOSS 52 } activity ACT_RANGE_ATTACK2_LOW 1

$sequence dropgrenade "seq_dropgrenade" fps 30 {
	{ event AE_HUMAN_GRENADE_DROP 40 }
	activity ACT_SPECIAL_ATTACK1 1
}
$sequence crouch_dropgrenade "seq_crouch_dropgrenade" fps 30 {
	{ event AE_HUMAN_GRENADE_DROP 48 }
	activity ACT_SPECIAL_ATTACK1_LOW 1
}

$sequence frontkick "seq_frontkick" fps 40 { event AE_NPC_KICK 12 } { event 2010 12 } activity ACT_MELEE_ATTACK1 1

$sequence reload "seq_reload_idle" fps 30 {
	{ event 3015 36 }
	activity ACT_RELOAD_PISTOL 1
	node standing
}
$sequence crouch_reload "seq_crouch_reload" fps 40 {
	{ event 3015 48 }
	activity ACT_RELOAD_PISTOL_LOW 1
	node crouching
}

//Signalling
$sequence signal_advance "seq_signal_advance" fps 30 {
    activity ACT_SIGNAL1 1
}
$sequence crouch_signal_advance "seq_crouch_signal_advance" fps 30 {
    activity ACT_SIGNAL1_LOW 1
}

$sequence signal_flank "seq_signal_flank" fps 40 {
    activity ACT_SIGNAL2 1
}
$sequence crouch_signal_flank "seq_crouch_signal_flank" fps 40 {
    activity ACT_SIGNAL2_LOW 1
}

$sequence signal_retreat "seq_signal_retreat" fps 40 {
    activity ACT_SIGNAL3 1
}
$sequence crouch_signal_retreat "seq_crouch_signal_retreat" fps 40 {
    activity ACT_SIGNAL3_LOW 1
}

// Turning
$sequence turnleft "seq_turnleft" fps 15 {
//	activity ACT_TURN_LEFT 1
	node standing
}
$sequence turnright "seq_turnright" fps 15 {
//	activity ACT_TURN_RIGHT 1
	node standing
}

$sequence crouch_turnleft "seq_crouch_turnleft" fps 15 {
//    activity ACT_TURN_LEFT_LOW 1
    node crouching
}
$sequence crouch_turnright "seq_crouch_turnright" fps 15 {
//    activity ACT_TURN_RIGHT_LOW 1
    node crouching
}

// Flinching
$sequence laflinch "seq_laflinch" fps 16 activity ACT_FLINCH_LEFTARM 1
$sequence raflinch "seq_raflinch" fps 16 activity ACT_FLINCH_RIGHTARM 1
$sequence llflinch "seq_llflinch" fps 20 activity ACT_FLINCH_LEFTLEG 1
$sequence rlflinch "seq_rlflinch" fps 20 activity ACT_FLINCH_RIGHTLEG 1
$sequence smlflinch "seq_smflinch" fps 30 activity ACT_SMALL_FLINCH 5

$sequence crouch_laflinch "seq_crouch_laflinch" fps 16
$sequence crouch_raflinch "seq_crouch_raflinch" fps 16
$sequence crouch_llflinch "seq_crouch_llflinch" fps 20
$sequence crouch_rlflinch "seq_crouch_rlflinch" fps 20
$sequence crouch_smlflinch "seq_crouch_smflinch" fps 30 ACT_SMALL_FLINCH_LOW 1

// These are used when retrieving a weapon
$sequence pickup_ground "seq_pickupkey" fps 20 {
	{ event 2040 23 }
	activity ACT_PICKUP_GROUND 1
}
$sequence pickup_rack "buttonpush" fps 30 {
	{ event 2040 8 }
	activity ACT_PICKUP_RACK 1
}

// Dying (on the spot)
$sequence die_headshot "seq_die_headshot" X fps 20 { event 8 7 } { event 2001 10 } activity ACT_DIE_HEADSHOT 1
$sequence die_chestshot "seq_die_chestshot" X fps 15 { event 8 2 } { event 2001 5 } activity ACT_DIE_CHESTSHOT 1
$sequence die_gutshot "seq_die_gutshot" fps 15 { event 8 17 } { event 2001 33 } activity ACT_DIE_GUTSHOT 1
$sequence die_forward "seq_die_forward" X fps 18 { event 8 2 } { event 2001 8 } activity ACT_DIEFORWARD 1
$sequence die_backward "seq_die_backward" X fps 15 { event 8 4 } { event 2001 19 } activity ACT_DIEBACKWARD 1
$sequence die_simple "seq_die_simple" X fps 18 { event 8 5 } { event 2001 13 } activity ACT_DIESIMPLE 1

$sequence crouch_die_headshot "seq_crouch_die_headshot" X fps 30 { event 8 15 } { event 2001 30 } 
$sequence crouch_die_chestshot "seq_crouch_die_chestshot" X fps 30 { event 8 12 } { event 2001 30 } 
$sequence crouch_die_gutshot "seq_crouch_die_gutshot" fps 30 { event 8 8 } { event 2001 38 } 
$sequence crouch_die_forward "seq_crouch_die_forward" X fps 30 { event 8 8 } { event 2001 30 } 
$sequence crouch_die_backward "seq_crouch_die_backward" X fps 30 { event 8 8 } { event 2001 30 } 
$sequence crouch_die_simple "seq_crouch_die_simple" X fps 30 { event 8 2 } { event 2001 14 } 

// Dying (thrown forcefully into air by explosion)
$sequence explosion_hit_forward "seq_explosion_hit_forward" LX LY fps 30 { event 8 4 }
$sequence explosion_fly_forward "seq_explosion_fly_forward" loop fps 30
$sequence explosion_land_forward "seq_explosion_land_forward" fps 30 { event 2001 9 }

$sequence explosion_hit_backward "seq_explosion_hit_backwards" LX LY fps 30 { event 8 4 }
$sequence explosion_fly_backward "seq_explosion_fly_backwards" loop fps 30
$sequence explosion_land_backward "seq_explosion_land_backwards" fps 30 { event 2001 9 }

$sequence explosion_hit_left "seq_explosion_hit_left" LX LY fps 30 { event 8 4 }
$sequence explosion_fly_left "seq_explosion_fly_left" loop fps 30
$sequence explosion_land_left "seq_explosion_land_left" fps 30 { event 2001 8 }

$sequence explosion_hit_right "seq_explosion_hit_right" LX LY fps 30 { event 8 4 }
$sequence explosion_fly_right "seq_explosion_fly_right" loop fps 30
$sequence explosion_land_right "seq_explosion_land_right" fps 30 { event 2001 9 }

// Barnacled
$sequence barnaclehit "seq_barnaclehit" fps 15 ACT_BARNACLE_HIT 1 
$sequence barnaclepull "seq_barnaclepull" loop fps 45 ACT_BARNACLE_PULL 1 
$sequence barnaclecrunch "seq_barnaclecrunch" fps 5 ACT_BARNACLE_CHOMP 1 
$sequence barnaclechew "seq_barnaclechew" loop fps 5 ACT_BARNACLE_CHEW 1 
$sequence barnaclereleased "seq_explosion_fly_forward" loop fps 30 ACT_BARNACLE_RELEASED 1

// Repel
$sequence repel_jump "seq_repel_jump" fps 22 ACT_HOVER 1 
$sequence repel_repel "seq_repel_repel" loop fps 18 ACT_GLIDE 1 
$sequence repel_shoot "seq_repel_fire_up" "seq_repel_fire_down" loop fps 18 ACT_FLY 1 blend aim_pitch -40 40 { event 5001 1 "50" } { event 5001 3 "50" } { event 5001 5 "50" } { event 2 1 } { event 3 3 } { event 4 5 } 
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
$sequence flashlight "flashlight" loop fps 14 { event 1004 5 "Buttons.latchlocked2" } 
$sequence unlatch "unlatch" fps 12 { event 1003 7 "barney_door_lock" } 
$sequence buttonpush "buttonpush" fps 20
$sequence headbutt "headbutt" fps 32 { event 1004 38 "Debris.metal1" } 
$sequence sog_alert "sog_alert" fps 15

$animation a_sog_grab "sog_grabpull" frame 0 17
$sequence sog_grab a_sog_grab fps 40 {
	{ event AE_BARNEY_LEAFGIB 0 }
	{ event 1004 0 "Ambience.bush1" }
	{ event 5004 2 "NPC_Barney.pain1" }
}

$animation a_sog_pull "sog_grabpull" frame 18 255
$sequence sog_pull a_sog_pull fps 40 LX loop {
	{ event 5004 0 "NPC_Barney.pain2" }
	{ event 1004 6 "Ambience.bush2" }
	{ event AE_BARNEY_LEAFGIB 5 }
	{ event AE_NPC_WEAPON_FIRE 13 "0" }
	{ event AE_NPC_WEAPON_FIRE 25 "0" }
	{ event AE_BARNEY_LEAFGIB 30 }
	{ event AE_NPC_WEAPON_FIRE 41 "0" }
	{ event 5004 50 "NPC_Barney.pain2" }
//	{ event AE_BARNEY_LEAFGIB 74 }
//	{ event AE_BARNEY_LEAFGIB 100 }
//	{ event AE_BARNEY_LEAFGIB 125 }
//	{ event AE_BARNEY_LEAFGIB 150 }
//	{ event AE_BARNEY_LEAFGIB 175 }
//	{ event AE_BARNEY_LEAFGIB 200 }
//	{ event AE_BARNEY_LEAFGIB 225 }
//	{ event AE_BARNEY_LEAFGIB 250 }

	// Without this the engine tries to blend in seq_idle at the start of dragging.
	// See CAI_BlendedMotor::GetInteriorSequence and CAI_BlendedMotor::SetMoveScriptAnim
	keyvalues {
		"interior" "sog_pull"
	}
}

// 1 event is to shoot the bullet, the other for muzzleflash/brass ejection
$sequence sog_grabpull "sog_grabpull" fps 40 {
	{ event AE_NPC_WEAPON_FIRE 33 "0" } { event AE_NPC_WEAPON_SET_ACTIVITY 33 "ACT_USE" }
	{ event AE_NPC_WEAPON_FIRE 45 "0" } { event AE_NPC_WEAPON_SET_ACTIVITY 45 "ACT_USE" }
	{ event AE_NPC_WEAPON_FIRE 61 "0" } { event AE_NPC_WEAPON_SET_ACTIVITY 61 "ACT_USE" }
	{ event AE_SV_DUSTTRAIL 30 "LeftFoot 10.0 18" }
}
$animation a_sog_loop "sog_grabpull" frames 90 90
$sequence sog_loop a_sog_loop fps 1 loop {
}

$sequence liftdoor "liftdoor" fps 32 { event 1003 16 "1" }
$sequence idle1 "seq_idle" loop fps 20
$sequence pickupkey "seq_pickupkey" fps 20

$weightlist right_arm { "Bip01 R Arm" 1.0 }
$sequence gesture_holster seq_holster fps 20 {
	{ event AE_NPC_HOLSTER 19 }
	activity ACT_DISARM 1
	weightlist right_arm
}
$sequence gesture_unholster seq_unholster fps 20 {
	{ event AE_NPC_DRAW 9 }
	activity ACT_ARM 1
	weightlist right_arm
}
$sequence drive "seq_drive" loop fps 20
$sequence get_in_truck "seq_enter_truck" fps 20 LX LY LZ LZR {
	keyvalues
	{
		// Not sure what this is about, needed to stop some asserts.
		// See CAI_PassengerBehavior::CacheBlendTargets.
		entryexit_blend
		{
		}
	}
	{ event AE_HUMAN_OPEN_VEHICLE 5 }
	{ event AE_HUMAN_CLOSE_VEHICLE 74 }
}
$sequence exit_truck "seq_exit_truck" fps 20 LX LY LZ LZR {
	keyvalues
	{
		// Not sure what this is about, needed to stop some asserts.
		// See CAI_PassengerBehavior::CacheBlendTargets.
		entryexit_blend
		{
		}
	}
	{ event AE_HUMAN_OPEN_VEHICLE 0 }
	{ event AE_HUMAN_CLOSE_VEHICLE 39 }
}
$sequence get_in_huey "seq_get_in_huey" LX LY LZ LZR fps 40 {
	keyvalues
	{
		// Not sure what this is about, needed to stop some asserts.
		// See CAI_PassengerBehavior::CacheBlendTargets.
		entryexit_blend
		{
		}
	}
}
$sequence sit_in_huey "seq_sit_in_huey" loop fps 20 {
	activity ACT_PASSENGER_IDLE 1
}
$sequence g_dive_in_huey "seq_dive_in_huey" fps 15 {
	keyvalues
	{
		faceposer
		{
			type gesture
		}
	}
}
$sequence g_huey_kidnap "seq_huey_kidnap" fps 10 {
	{ event 5004 25 "NPC_Barney.pain2" }
	{ event AE_NPC_WEAPON_FIRE 39 "0" }
	keyvalues
	{
		faceposer
		{
			type gesture
		}
	}
}

// head movement from HL:S
 $animation mdldecompiler_delta.smd "mdldecompiler_delta.smd" fps 30
// $poseparameter head_yaw -90.00 90.00
// $animation neck_right "neck_right" fps 30.000000 subtract mdldecompiler_delta.smd 0
// $animation neck_center "neck_center" fps 30.000000 subtract mdldecompiler_delta.smd 0
// $animation neck_left "neck_left" fps 30.000000 subtract mdldecompiler_delta.smd 0
// $sequence barney_neckturn "neck_right" fps 30.00 {
//   blendwidth 3
//   blend head_yaw -90.000000 90.000000
//   delta
//   autoplay
//  neck_center neck_left
// }

//$poseparameter head_yaw -90.00 90.00
//$poseparameter head_pitch -90.00 90.00
$animation a_pose_neutral "mdldecompiler_delta.smd" fps 30
$animation a_head_rot "neck_center" fps 30.000000 subtract a_pose_neutral 0
$animation a_head_rot_dwn "head_poses" frame 3 3 subtract a_pose_neutral 0
$animation a_head_rot_up "head_poses" frame 2 2 subtract a_pose_neutral 0
$animation a_head_rot_z_rt "head_poses" frame 0 0 subtract a_pose_neutral 0
$animation a_head_rot_z_lt "head_poses" frame 1 1 subtract a_pose_neutral 0
$sequence head_rot_z { a_head_rot_z_lt a_head_rot a_head_rot_z_rt calcblend head_yaw "eyes" ZR } delta autoplay hidden
$sequence head_rot_y { a_head_rot_dwn a_head_rot a_head_rot_up calcblend head_pitch "eyes" YR } delta autoplay hidden

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

$sequence intropush_pre seq_intropush_pre fps 20
$sequence intropush_loop "seq_intropush_loop" loop fps 20
$sequence intropush_post seq_intropush_post fps 20

$sequence barnacleground "seq_barnacle_ground" fps 30 ACT_BARNACLE_HIT_GROUND 1

