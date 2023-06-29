// Created by Crowbar 0.68

$modelname "Atlas.mdl"
$cd "."
$cdtexture "."
$cliptotextures
$scale 1.0

$bodygroup "studio"
{
	studio "atlas_ab"
}
$bodygroup "body"
{
	studio "atlas_ball"
}
$bodygroup "studio"
{
	studio "atlas_ar"
}

$flags 0

$texrendermode "caja.bmp" additive

$attachment 0 "Bip01 R Hand" 20 2 5
$attachment 1 "Bip01 R Hand" 15 1.5 3.75
$attachment 2 "Bip01 R Hand" 30 3 7.5

$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 3 "Bip01 Pelvis" -4.69 -4.44 -6.75 4 5.56 6.75
$hbox 6 "Bip01 L Leg" 2.66 -3.69 -3.09 18.16 4.88 3.31
$hbox 6 "Bip01 L Leg1" 0.38 -3.97 -2.84 17.6 4 2.94
$hbox 6 "Bip01 L Foot" -0.59 -2.34 -2.63 3.79 8 2.19
$hbox 7 "Bip01 R Leg" 2.47 -3.69 -3.16 18.129999 4.88 3.38
$hbox 7 "Bip01 R Leg1" 0.31 -3.97 -2.84 17.6 3.94 2.97
$hbox 7 "Bip01 R Foot" -0.56 -2.34 -2.19 3.81 8 2.66
$hbox 3 "Bip01 Spine1" -3.25 -5.5 -5.5 4.83 5.5 5.5
$hbox 2 "Bip01 Spine2" -0.06 -5.53 -7.59 8 7 7.59
$hbox 2 "Bip01 Spine3" -2.25 -6.81 -6.31 6.5 5.09 6.31
$hbox 2 "Bip01 Neck" -3.11 -1.5 -3 2.05 3.5 3
$hbox 1 "Bip01 Head" 0.09 -3.66 -3 8.41 5.09 3
$hbox 4 "Bip01 L Arm" 0.94 -2.88 -4.13 5.44 4.28 3.5
$hbox 4 "Bip01 L Arm1" -2.16 -2.34 -2.56 11.56 3.41 2.38
$hbox 4 "Bip01 L Arm2" 0.59 -1.81 -2.19 10.75 2.84 2.41
$hbox 4 "Bip01 L Hand" 0 -1 -2 3 1.5 3.5
$hbox 5 "Bip01 R Arm" 1.38 -2.84 -3.56 5.69 4.31 4.31
$hbox 5 "Bip01 R Arm1" -1.88 -2.47 -2.16 11.81 3.28 2.81
$hbox 5 "Bip01 R Arm2" 0.44 -1.97 -2.34 10.59 2.72 2.22
$hbox 5 "Bip01 R Hand" 0 -1 -2 3 1.5 3.5

$controller 0 "Bip01 Spine" XR -30 30
$controller 1 "Bip01 Spine1" XR -30 30
$controller 2 "Bip01 Spine2" XR -30 30
$controller 3 "Bip01 Spine3" XR -30 30

$sequence "look_idle" {
	"Atlas_anims\look_idle"
	ACT_IDLE 1
	fps 14
	loop
}
$sequence "idle" {
	"Atlas_anims\idle"
	ACT_IDLE 1
	fps 14
	loop
}
$sequence "deep_idle" {
	"Atlas_anims\deep_idle"
	ACT_IDLE 4
	fps 12
	loop
}
$sequence "run2" {
	"Atlas_anims\run2"
	ACT_RUN 1
	fps 40
	loop
	LX
}
$sequence "walk2handed" {
	"Atlas_anims\walk2handed"
	ACT_WALK 1
	fps 20
	loop
	LX
}
$sequence "2handshoot" {
	"Atlas_anims\2handshoot"
	ACT_RANGE_ATTACK1 1
	fps 20
}
$sequence "crawl" {
	"Atlas_anims\crawl"
	ACT_CROUCH 1
	fps 22
	loop
	LX
}
$sequence "crouch_idle" {
	"Atlas_anims\crouch_idle"
	ACT_CROUCHIDLE 1
	fps 4
	loop
}
$sequence "jump" {
	"Atlas_anims\jump"
	ACT_HOP 1
	fps 30
}
$sequence "long_jump" {
	"Atlas_anims\long_jump"
	ACT_LEAP 1
	fps 24
	LX
}
$sequence "swim" {
	"Atlas_anims\swim"
	ACT_SWIM 1
	fps 14
	loop
}
$sequence "treadwater" {
	"Atlas_anims\treadwater"
	ACT_HOVER 1
	fps 14
	loop
}
$sequence "die_simple" {
	"Atlas_anims\die_simple"
	ACT_DIESIMPLE 1
	{ event 2001 9 }
	fps 22
}
$sequence "die_backwards1" {
	"Atlas_anims\die_backwards1"
	ACT_DIEBACKWARD 4
	{ event 2001 13 }
	fps 26
}
$sequence "die_backwards" {
	"Atlas_anims\die_backwards"
	ACT_DIEBACKWARD 1
	{ event 2001 15 }
	fps 30
}
$sequence "die_forwards" {
	"Atlas_anims\die_forwards"
	ACT_DIEFORWARD 1
	{ event 2001 8 }
	fps 26
}
$sequence "headshot" {
	"Atlas_anims\headshot"
	ACT_DIE_HEADSHOT 4
	{ event 2001 22 }
	fps 24
}
$sequence "die_spin" {
	"Atlas_anims\die_spin"
	ACT_DIE_HEADSHOT 1
	{ event 2001 16 }
	fps 30
}
$sequence "gutshot" {
	"Atlas_anims\gutshot"
	ACT_DIE_GUTSHOT 1
	{ event 2001 22 }
	fps 26
}
$sequence "ref_draw_crowbar" {
	"Atlas_anims\ref_draw_crowbar_blend01"
	"Atlas_anims\ref_draw_crowbar_blend02"
	blend XR -70 70
	fps 12
}
$sequence "ref_aim_crowbar" {
	"Atlas_anims\ref_aim_crowbar_blend01"
	"Atlas_anims\ref_aim_crowbar_blend02"
	blend XR -70 70
	fps 12
	loop
}
$sequence "ref_shoot_crowbar" {
	"Atlas_anims\ref_shoot_crowbar_blend01"
	"Atlas_anims\ref_shoot_crowbar_blend02"
	blend XR -70 70
	fps 24
}
$sequence "crouch_draw_crowbar" {
	"Atlas_anims\crouch_draw_crowbar_blend01"
	"Atlas_anims\crouch_draw_crowbar_blend02"
	blend XR -70 70
	fps 12
}
$sequence "crouch_aim_crowbar" {
	"Atlas_anims\crouch_aim_crowbar_blend01"
	"Atlas_anims\crouch_aim_crowbar_blend02"
	blend XR -70 70
	fps 12
	loop
}
$sequence "crouch_shoot_crowbar" {
	"Atlas_anims\crouch_shoot_crowbar_blend01"
	"Atlas_anims\crouch_shoot_crowbar_blend02"
	blend XR -70 70
	fps 24
}
$sequence "ref_cock_wrench" {
	"Atlas_anims\ref_cock_wrench_blend01"
	"Atlas_anims\ref_cock_wrench_blend02"
	blend XR -70 70
	fps 10
}
$sequence "ref_hold_wrench" {
	"Atlas_anims\ref_hold_wrench_blend01"
	"Atlas_anims\ref_hold_wrench_blend02"
	blend XR -70 70
	fps 13
}
$sequence "ref_shoot_wrench" {
	"Atlas_anims\ref_shoot_wrench_blend01"
	"Atlas_anims\ref_shoot_wrench_blend02"
	blend XR -70 70
	fps 13
}
$sequence "crouch_cock_wrench" {
	"Atlas_anims\crouch_cock_wrench_blend01"
	"Atlas_anims\crouch_cock_wrench_blend02"
	blend XR -70 70
	fps 10
}
$sequence "crouch_hold_wrench" {
	"Atlas_anims\crouch_hold_wrench_blend01"
	"Atlas_anims\crouch_hold_wrench_blend02"
	blend XR -70 70
	fps 13
}
$sequence "crouch_shoot_wrench" {
	"Atlas_anims\crouch_shoot_wrench_blend01"
	"Atlas_anims\crouch_shoot_wrench_blend02"
	blend XR -70 70
	fps 13
}
$sequence "ref_draw_gren" {
	"Atlas_anims\ref_draw_gren_blend01"
	"Atlas_anims\ref_draw_gren_blend02"
	blend XR -70 70
	fps 10
}
$sequence "ref_aim_gren" {
	"Atlas_anims\ref_aim_gren_blend01"
	"Atlas_anims\ref_aim_gren_blend02"
	blend XR -70 70
	fps 12
	loop
}
$sequence "ref_cock_gren" {
	"Atlas_anims\ref_cock_gren_blend01"
	"Atlas_anims\ref_cock_gren_blend02"
	blend XR -70 70
	fps 18
}
$sequence "ref_hold_gren" {
	"Atlas_anims\ref_hold_gren_blend01"
	"Atlas_anims\ref_hold_gren_blend02"
	blend XR -70 70
	fps 15
}
$sequence "ref_shoot_gren" {
	"Atlas_anims\ref_shoot_gren_blend01"
	"Atlas_anims\ref_shoot_gren_blend02"
	blend XR -70 70
	fps 15
}
$sequence "crouch_draw_gren" {
	"Atlas_anims\crouch_draw_gren_blend01"
	"Atlas_anims\crouch_draw_gren_blend02"
	blend XR -70 70
	fps 10
}
$sequence "crouch_aim_gren" {
	"Atlas_anims\crouch_aim_gren_blend01"
	"Atlas_anims\crouch_aim_gren_blend02"
	blend XR -70 70
	fps 12
	loop
}
$sequence "crouch_cock_gren" {
	"Atlas_anims\crouch_cock_gren_blend01"
	"Atlas_anims\crouch_cock_gren_blend02"
	blend XR -70 70
	fps 18
}
$sequence "crouch_hold_gren" {
	"Atlas_anims\crouch_hold_gren_blend01"
	"Atlas_anims\crouch_hold_gren_blend02"
	blend XR -70 70
	fps 15
}
$sequence "crouch_shoot_gren" {
	"Atlas_anims\crouch_shoot_gren_blend01"
	"Atlas_anims\crouch_shoot_gren_blend02"
	blend XR -70 70
	fps 15
}
$sequence "ref_draw_trip" {
	"Atlas_anims\ref_draw_trip_blend01"
	"Atlas_anims\ref_draw_trip_blend02"
	blend XR -70 70
	fps 17
}
$sequence "ref_aim_trip" {
	"Atlas_anims\ref_aim_trip_blend01"
	"Atlas_anims\ref_aim_trip_blend02"
	blend XR -70 70
	fps 12
	loop
}
$sequence "ref_shoot_trip" {
	"Atlas_anims\ref_shoot_trip_blend01"
	"Atlas_anims\ref_shoot_trip_blend02"
	blend XR -70 70
	fps 18
}
$sequence "crouch_draw_trip" {
	"Atlas_anims\crouch_draw_trip_blend01"
	"Atlas_anims\crouch_draw_trip_blend02"
	blend XR -70 70
	fps 17
}
$sequence "crouch_aim_trip" {
	"Atlas_anims\crouch_aim_trip_blend01"
	"Atlas_anims\crouch_aim_trip_blend02"
	blend XR -70 70
	fps 12
	loop
}
$sequence "crouch_shoot_trip" {
	"Atlas_anims\crouch_shoot_trip_blend01"
	"Atlas_anims\crouch_shoot_trip_blend02"
	blend XR -70 70
	fps 18
}
$sequence "ref_draw_onehanded" {
	"Atlas_anims\ref_draw_onehanded_blend01"
	"Atlas_anims\ref_draw_onehanded_blend02"
	blend XR -70 70
	fps 15
}
$sequence "ref_aim_onehanded" {
	"Atlas_anims\ref_aim_onehanded_blend01"
	"Atlas_anims\ref_aim_onehanded_blend02"
	blend XR -70 70
	fps 12
	loop
}
$sequence "ref_shoot_onehanded" {
	"Atlas_anims\ref_shoot_onehanded_blend01"
	"Atlas_anims\ref_shoot_onehanded_blend02"
	blend XR -70 70
	{ event 5001 0 "15" }
	fps 25
}
$sequence "ref_reload_onehanded" {
	"Atlas_anims\ref_reload_onehanded_blend01"
	"Atlas_anims\ref_reload_onehanded_blend02"
	blend XR -70 70
	fps 20
}
$sequence "crouch_draw_onehanded" {
	"Atlas_anims\crouch_draw_onehanded_blend01"
	"Atlas_anims\crouch_draw_onehanded_blend02"
	blend XR -70 70
	fps 15
}
$sequence "crouch_aim_onehanded" {
	"Atlas_anims\crouch_aim_onehanded_blend01"
	"Atlas_anims\crouch_aim_onehanded_blend02"
	blend XR -70 70
	fps 12
	loop
}
$sequence "crouch_shoot_onehanded" {
	"Atlas_anims\crouch_shoot_onehanded_blend01"
	"Atlas_anims\crouch_shoot_onehanded_blend02"
	blend XR -70 70
	{ event 5001 0 "15" }
	fps 25
}
$sequence "crouch_reload_onehanded" {
	"Atlas_anims\crouch_reload_onehanded_blend01"
	"Atlas_anims\crouch_reload_onehanded_blend02"
	blend XR -70 70
	fps 20
}
$sequence "ref_draw_python" {
	"Atlas_anims\ref_draw_python_blend01"
	"Atlas_anims\ref_draw_python_blend02"
	blend XR -70 70
	fps 15
}
$sequence "ref_aim_python" {
	"Atlas_anims\ref_aim_python_blend01"
	"Atlas_anims\ref_aim_python_blend02"
	blend XR -70 70
	fps 12
	loop
}
$sequence "ref_shoot_python" {
	"Atlas_anims\ref_shoot_python_blend01"
	"Atlas_anims\ref_shoot_python_blend02"
	blend XR -70 70
	{ event 5001 0 "15" }
	fps 16
}
$sequence "ref_reload_python" {
	"Atlas_anims\ref_reload_python_blend01"
	"Atlas_anims\ref_reload_python_blend02"
	blend XR -70 70
	fps 15
}
$sequence "crouch_draw_python" {
	"Atlas_anims\crouch_draw_python_blend01"
	"Atlas_anims\crouch_draw_python_blend02"
	blend XR -70 70
	fps 15
}
$sequence "crouch_aim_python" {
	"Atlas_anims\crouch_aim_python_blend01"
	"Atlas_anims\crouch_aim_python_blend02"
	blend XR -70 70
	fps 12
	loop
}
$sequence "crouch_shoot_python" {
	"Atlas_anims\crouch_shoot_python_blend01"
	"Atlas_anims\crouch_shoot_python_blend02"
	blend XR -70 70
	{ event 5001 0 "15" }
	fps 16
}
$sequence "crouch_reload_python" {
	"Atlas_anims\crouch_reload_python_blend01"
	"Atlas_anims\crouch_reload_python_blend02"
	blend XR -70 70
	fps 15
}
$sequence "ref_draw_shotgun" {
	"Atlas_anims\ref_draw_shotgun_blend01"
	"Atlas_anims\ref_draw_shotgun_blend02"
	blend XR -70 70
	fps 18
}
$sequence "ref_aim_shotgun" {
	"Atlas_anims\ref_aim_shotgun_blend01"
	"Atlas_anims\ref_aim_shotgun_blend02"
	blend XR -70 70
	fps 12
	loop
}
$sequence "ref_shoot_shotgun" {
	"Atlas_anims\ref_shoot_shotgun_blend01"
	"Atlas_anims\ref_shoot_shotgun_blend02"
	blend XR -70 70
	{ event 5005 0 "muzzle_SG.txt" }
	fps 20
}
$sequence "ref_reload_shotgun" {
	"Atlas_anims\ref_reload_shotgun_blend01"
	"Atlas_anims\ref_reload_shotgun_blend02"
	blend XR -70 70
	fps 22
}
$sequence "crouch_draw_shotgun" {
	"Atlas_anims\crouch_draw_shotgun_blend01"
	"Atlas_anims\crouch_draw_shotgun_blend02"
	blend XR -70 70
	fps 18
}
$sequence "crouch_aim_shotgun" {
	"Atlas_anims\crouch_aim_shotgun_blend01"
	"Atlas_anims\crouch_aim_shotgun_blend02"
	blend XR -70 70
	fps 12
	loop
}
$sequence "crouch_shoot_shotgun" {
	"Atlas_anims\crouch_shoot_shotgun_blend01"
	"Atlas_anims\crouch_shoot_shotgun_blend02"
	blend XR -70 70
	{ event 5005 0 "muzzle_SG.txt" }
	fps 20
}
$sequence "crouch_reload_shotgun" {
	"Atlas_anims\crouch_reload_shotgun_blend01"
	"Atlas_anims\crouch_reload_shotgun_blend02"
	blend XR -70 70
	fps 22
}
$sequence "ref_draw_gauss" {
	"Atlas_anims\ref_draw_gauss_blend01"
	"Atlas_anims\ref_draw_gauss_blend02"
	blend XR -70 70
	fps 18
}
$sequence "ref_aim_gauss" {
	"Atlas_anims\ref_aim_gauss_blend01"
	"Atlas_anims\ref_aim_gauss_blend02"
	blend XR -70 70
	fps 12
	loop
}
$sequence "ref_shoot_gauss" {
	"Atlas_anims\ref_shoot_gauss_blend01"
	"Atlas_anims\ref_shoot_gauss_blend02"
	blend XR -70 70
	fps 20
}
$sequence "crouch_draw_gauss" {
	"Atlas_anims\crouch_draw_gauss_blend01"
	"Atlas_anims\crouch_draw_gauss_blend02"
	blend XR -70 70
	fps 18
}
$sequence "crouch_aim_gauss" {
	"Atlas_anims\crouch_aim_gauss_blend01"
	"Atlas_anims\crouch_aim_gauss_blend02"
	blend XR -70 70
	fps 12
	loop
}
$sequence "crouch_shoot_gauss" {
	"Atlas_anims\crouch_shoot_gauss_blend01"
	"Atlas_anims\crouch_shoot_gauss_blend02"
	blend XR -70 70
	fps 20
}
$sequence "ref_draw_mp5" {
	"Atlas_anims\ref_draw_mp5_blend01"
	"Atlas_anims\ref_draw_mp5_blend02"
	blend XR -70 70
	fps 17
}
$sequence "ref_aim_mp5" {
	"Atlas_anims\ref_aim_mp5_blend01"
	"Atlas_anims\ref_aim_mp5_blend02"
	blend XR -70 70
	fps 12
	loop
}
$sequence "ref_shoot_mp5" {
	"Atlas_anims\ref_shoot_mp5_blend01"
	"Atlas_anims\ref_shoot_mp5_blend02"
	blend XR -70 70
	{ event 5005 0 "muzzle_MP5.txt" }
	fps 30
}
$sequence "ref_reload_mp5" {
	"Atlas_anims\ref_reload_mp5_blend01"
	"Atlas_anims\ref_reload_mp5_blend02"
	blend XR -70 70
	fps 16
}
$sequence "crouch_draw_mp5" {
	"Atlas_anims\crouch_draw_mp5_blend01"
	"Atlas_anims\crouch_draw_mp5_blend02"
	blend XR -70 70
	fps 17
}
$sequence "crouch_aim_mp5" {
	"Atlas_anims\crouch_aim_mp5_blend01"
	"Atlas_anims\crouch_aim_mp5_blend02"
	blend XR -70 70
	fps 12
	loop
}
$sequence "crouch_shoot_mp5" {
	"Atlas_anims\crouch_shoot_mp5_blend01"
	"Atlas_anims\crouch_shoot_mp5_blend02"
	blend XR -70 70
	{ event 5005 0 "muzzle_MP5.txt" }
	fps 30
}
$sequence "crouch_reload_mp5" {
	"Atlas_anims\crouch_reload_mp5_blend01"
	"Atlas_anims\crouch_reload_mp5_blend02"
	blend XR -70 70
	fps 16
}
$sequence "ref_draw_rpg" {
	"Atlas_anims\ref_draw_rpg_blend01"
	"Atlas_anims\ref_draw_rpg_blend02"
	blend XR -70 70
	fps 15
}
$sequence "ref_aim_rpg" {
	"Atlas_anims\ref_aim_rpg_blend01"
	"Atlas_anims\ref_aim_rpg_blend02"
	blend XR -70 70
	fps 10
	loop
}
$sequence "ref_shoot_rpg" {
	"Atlas_anims\ref_shoot_rpg_blend01"
	"Atlas_anims\ref_shoot_rpg_blend02"
	blend XR -70 70
	fps 18
}
$sequence "ref_reload_rpg" {
	"Atlas_anims\ref_reload_rpg_blend01"
	"Atlas_anims\ref_reload_rpg_blend02"
	blend XR -70 70
	fps 13
}
$sequence "crouch_draw_rpg" {
	"Atlas_anims\crouch_draw_rpg_blend01"
	"Atlas_anims\crouch_draw_rpg_blend02"
	blend XR -70 70
	fps 15
}
$sequence "crouch_aim_rpg" {
	"Atlas_anims\crouch_aim_rpg_blend01"
	"Atlas_anims\crouch_aim_rpg_blend02"
	blend XR -70 70
	fps 12
	loop
}
$sequence "crouch_shoot_rpg" {
	"Atlas_anims\crouch_shoot_rpg_blend01"
	"Atlas_anims\crouch_shoot_rpg_blend02"
	blend XR -70 70
	fps 18
}
$sequence "crouch_reload_rpg" {
	"Atlas_anims\crouch_reload_rpg_blend01"
	"Atlas_anims\crouch_reload_rpg_blend02"
	blend XR -70 70
	fps 13
}
$sequence "ref_draw_egon" {
	"Atlas_anims\ref_draw_egon_blend01"
	"Atlas_anims\ref_draw_egon_blend02"
	blend XR -70 70
	fps 15
}
$sequence "ref_aim_egon" {
	"Atlas_anims\ref_aim_egon_blend01"
	"Atlas_anims\ref_aim_egon_blend02"
	blend XR -70 70
	fps 12
	loop
}
$sequence "ref_shoot_egon" {
	"Atlas_anims\ref_shoot_egon_blend01"
	"Atlas_anims\ref_shoot_egon_blend02"
	blend XR -70 70
	fps 18
	loop
}
$sequence "crouch_draw_egon" {
	"Atlas_anims\crouch_draw_egon_blend01"
	"Atlas_anims\crouch_draw_egon_blend02"
	blend XR -70 70
	fps 15
}
$sequence "crouch_aim_egon" {
	"Atlas_anims\crouch_aim_egon_blend01"
	"Atlas_anims\crouch_aim_egon_blend02"
	blend XR -70 70
	fps 12
	loop
}
$sequence "crouch_shoot_egon" {
	"Atlas_anims\crouch_shoot_egon_blend01"
	"Atlas_anims\crouch_shoot_egon_blend02"
	blend XR -70 70
	fps 18
	loop
}
$sequence "ref_draw_squeak" {
	"Atlas_anims\ref_draw_squeak_blend01"
	"Atlas_anims\ref_draw_squeak_blend02"
	blend XR -70 70
	fps 20
}
$sequence "ref_aim_squeak" {
	"Atlas_anims\ref_aim_squeak_blend01"
	"Atlas_anims\ref_aim_squeak_blend02"
	blend XR -70 70
	fps 12
	loop
}
$sequence "ref_shoot_squeak" {
	"Atlas_anims\ref_shoot_squeak_blend01"
	"Atlas_anims\ref_shoot_squeak_blend02"
	blend XR -70 70
	fps 15
}
$sequence "crouch_draw_squeak" {
	"Atlas_anims\crouch_draw_squeak_blend01"
	"Atlas_anims\crouch_draw_squeak_blend02"
	blend XR -70 70
	fps 20
}
$sequence "crouch_aim_squeak" {
	"Atlas_anims\crouch_aim_squeak_blend01"
	"Atlas_anims\crouch_aim_squeak_blend02"
	blend XR -70 70
	fps 12
	loop
}
$sequence "crouch_shoot_squeak" {
	"Atlas_anims\crouch_shoot_squeak_blend01"
	"Atlas_anims\crouch_shoot_squeak_blend02"
	blend XR -70 70
	fps 15
}
$sequence "ref_draw_hive" {
	"Atlas_anims\ref_draw_hive_blend01"
	"Atlas_anims\ref_draw_hive_blend02"
	blend XR -70 70
	fps 12
}
$sequence "ref_aim_hive" {
	"Atlas_anims\ref_aim_hive_blend01"
	"Atlas_anims\ref_aim_hive_blend02"
	blend XR -70 70
	fps 12
	loop
}
$sequence "ref_shoot_hive" {
	"Atlas_anims\ref_shoot_hive_blend01"
	"Atlas_anims\ref_shoot_hive_blend02"
	blend XR -70 70
	fps 17
}
$sequence "crouch_draw_hive" {
	"Atlas_anims\crouch_draw_hive_blend01"
	"Atlas_anims\crouch_draw_hive_blend02"
	blend XR -70 70
	fps 12
}
$sequence "crouch_aim_hive" {
	"Atlas_anims\crouch_aim_hive_blend01"
	"Atlas_anims\crouch_aim_hive_blend02"
	blend XR -70 70
	fps 12
	loop
}
$sequence "crouch_shoot_hive" {
	"Atlas_anims\crouch_shoot_hive_blend01"
	"Atlas_anims\crouch_shoot_hive_blend02"
	blend XR -70 70
	fps 17
}
$sequence "ref_draw_bow" {
	"Atlas_anims\ref_draw_bow_blend01"
	"Atlas_anims\ref_draw_bow_blend02"
	blend XR -70 70
	fps 15
}
$sequence "ref_aim_bow" {
	"Atlas_anims\ref_aim_bow_blend01"
	"Atlas_anims\ref_aim_bow_blend02"
	blend XR -70 70
	fps 10
	loop
}
$sequence "ref_shoot_bow" {
	"Atlas_anims\ref_shoot_bow_blend01"
	"Atlas_anims\ref_shoot_bow_blend02"
	blend XR -70 70
	fps 20
}
$sequence "ref_aim_bowscope" {
	"Atlas_anims\ref_aim_bowscope_blend01"
	"Atlas_anims\ref_aim_bowscope_blend02"
	blend XR -70 70
	fps 6
	loop
}
$sequence "ref_shoot_bowscope" {
	"Atlas_anims\ref_shoot_bowscope_blend01"
	"Atlas_anims\ref_shoot_bowscope_blend02"
	blend XR -70 70
	fps 20
}
$sequence "ref_reload_bow" {
	"Atlas_anims\ref_reload_bow_blend01"
	"Atlas_anims\ref_reload_bow_blend02"
	blend XR -70 70
	fps 11
}
$sequence "crouch_draw_bow" {
	"Atlas_anims\crouch_draw_bow_blend01"
	"Atlas_anims\crouch_draw_bow_blend02"
	blend XR -70 70
	fps 15
}
$sequence "crouch_aim_bow" {
	"Atlas_anims\crouch_aim_bow_blend01"
	"Atlas_anims\crouch_aim_bow_blend02"
	blend XR -70 70
	fps 10
	loop
}
$sequence "crouch_shoot_bow" {
	"Atlas_anims\crouch_shoot_bow_blend01"
	"Atlas_anims\crouch_shoot_bow_blend02"
	blend XR -70 70
	fps 20
}
$sequence "crouch_aim_bowscope" {
	"Atlas_anims\crouch_aim_bowscope_blend01"
	"Atlas_anims\crouch_aim_bowscope_blend02"
	blend XR -70 70
	fps 6
	loop
}
$sequence "crouch_shoot_bowscope" {
	"Atlas_anims\crouch_shoot_bowscope_blend01"
	"Atlas_anims\crouch_shoot_bowscope_blend02"
	blend XR -70 70
	fps 20
}
$sequence "crouch_reload_bow" {
	"Atlas_anims\crouch_reload_bow_blend01"
	"Atlas_anims\crouch_reload_bow_blend02"
	blend XR -70 70
	fps 11
}
$sequence "ref_draw_minigun" {
	"Atlas_anims\ref_draw_minigun_blend01"
	"Atlas_anims\ref_draw_minigun_blend02"
	blend XR -70 70
	fps 15
}
$sequence "ref_aim_minigun" {
	"Atlas_anims\ref_aim_minigun_blend01"
	"Atlas_anims\ref_aim_minigun_blend02"
	blend XR -70 70
	fps 8
	loop
}
$sequence "ref_shoot_minigun" {
	"Atlas_anims\ref_shoot_minigun_blend01"
	"Atlas_anims\ref_shoot_minigun_blend02"
	blend XR -70 70
	{ event 5021 0 "20" }
	fps 30
}
$sequence "crouch_draw_minigun" {
	"Atlas_anims\crouch_draw_minigun_blend01"
	"Atlas_anims\crouch_draw_minigun_blend02"
	blend XR -70 70
	fps 15
}
$sequence "crouch_aim_minigun" {
	"Atlas_anims\crouch_aim_minigun_blend01"
	"Atlas_anims\crouch_aim_minigun_blend02"
	blend XR -70 70
	fps 8
	loop
}
$sequence "crouch_shoot_minigun" {
	"Atlas_anims\crouch_shoot_minigun_blend01"
	"Atlas_anims\crouch_shoot_minigun_blend02"
	blend XR -70 70
	{ event 5021 0 "20" }
	fps 30
}
$sequence "ref_draw_uzis" {
	"Atlas_anims\ref_draw_uzis_blend01"
	"Atlas_anims\ref_draw_uzis_blend02"
	blend XR -70 70
	fps 10
}
$sequence "ref_draw_uzis_left" {
	"Atlas_anims\ref_draw_uzis_left_blend01"
	"Atlas_anims\ref_draw_uzis_left_blend02"
	blend XR -70 70
	fps 11
}
$sequence "ref_aim_uzis" {
	"Atlas_anims\ref_aim_uzis_blend01"
	"Atlas_anims\ref_aim_uzis_blend02"
	blend XR -70 70
	fps 14
	loop
}
$sequence "ref_shoot_uzis_both" {
	"Atlas_anims\ref_shoot_uzis_both_blend01"
	"Atlas_anims\ref_shoot_uzis_both_blend02"
	blend XR -70 70
	{ event 5001 0 "15" }
	{ event 5031 0 "15" }
	fps 24
}
$sequence "ref_shoot_uzis_right" {
	"Atlas_anims\ref_shoot_uzis_right_blend01"
	"Atlas_anims\ref_shoot_uzis_right_blend02"
	blend XR -70 70
	{ event 5001 0 "15" }
	fps 24
}
$sequence "ref_shoot_uzis_left" {
	"Atlas_anims\ref_shoot_uzis_left_blend01"
	"Atlas_anims\ref_shoot_uzis_left_blend02"
	blend XR -70 70
	{ event 5031 0 "15" }
	fps 24
}
$sequence "ref_reload_uzis_right" {
	"Atlas_anims\ref_reload_uzis_right_blend01"
	"Atlas_anims\ref_reload_uzis_right_blend02"
	blend XR -70 70
	fps 14
}
$sequence "ref_reload_uzis_left" {
	"Atlas_anims\ref_reload_uzis_left_blend01"
	"Atlas_anims\ref_reload_uzis_left_blend02"
	blend XR -70 70
	fps 14
}
$sequence "crouch_draw_uzis" {
	"Atlas_anims\crouch_draw_uzis_blend01"
	"Atlas_anims\crouch_draw_uzis_blend02"
	blend XR -70 70
	fps 10
}
$sequence "crouch_draw_uzis_left" {
	"Atlas_anims\crouch_draw_uzis_left_blend01"
	"Atlas_anims\crouch_draw_uzis_left_blend02"
	blend XR -70 70
	fps 11
}
$sequence "crouch_aim_uzis" {
	"Atlas_anims\crouch_aim_uzis_blend01"
	"Atlas_anims\crouch_aim_uzis_blend02"
	blend XR -70 70
	fps 14
	loop
}
$sequence "crouch_shoot_uzis_both" {
	"Atlas_anims\crouch_shoot_uzis_both_blend01"
	"Atlas_anims\crouch_shoot_uzis_both_blend02"
	blend XR -70 70
	{ event 5001 0 "15" }
	{ event 5031 0 "15" }
	fps 24
}
$sequence "crouch_shoot_uzis_right" {
	"Atlas_anims\crouch_shoot_uzis_right_blend01"
	"Atlas_anims\crouch_shoot_uzis_right_blend02"
	blend XR -70 70
	{ event 5001 0 "15" }
	fps 24
}
$sequence "crouch_shoot_uzis_left" {
	"Atlas_anims\crouch_shoot_uzis_left_blend01"
	"Atlas_anims\crouch_shoot_uzis_left_blend02"
	blend XR -70 70
	{ event 5031 0 "15" }
	fps 24
}
$sequence "crouch_reload_uzis_right" {
	"Atlas_anims\crouch_reload_uzis_right_blend01"
	"Atlas_anims\crouch_reload_uzis_right_blend02"
	blend XR -70 70
	fps 14
}
$sequence "crouch_reload_uzis_left" {
	"Atlas_anims\crouch_reload_uzis_left_blend01"
	"Atlas_anims\crouch_reload_uzis_left_blend02"
	blend XR -70 70
	fps 14
}
$sequence "ref_draw_m16" {
	"Atlas_anims\ref_draw_m16_blend01"
	"Atlas_anims\ref_draw_m16_blend02"
	blend XR -70 70
	fps 35
}
$sequence "ref_aim_m16" {
	"Atlas_anims\ref_aim_m16_blend01"
	"Atlas_anims\ref_aim_m16_blend02"
	blend XR -70 70
	fps 12
	loop
}
$sequence "ref_shoot_m16" {
	"Atlas_anims\ref_shoot_m16_blend01"
	"Atlas_anims\ref_shoot_m16_blend02"
	blend XR -70 70
	{ event 5005 0 "muzzle_spaw.txt" }
	fps 30
}
$sequence "ref_shoot_m203" {
	"Atlas_anims\ref_shoot_m203_blend01"
	"Atlas_anims\ref_shoot_m203_blend02"
	blend XR -70 70
	fps 30
}
$sequence "ref_reload_m16" {
	"Atlas_anims\ref_reload_m16_blend01"
	"Atlas_anims\ref_reload_m16_blend02"
	blend XR -70 70
	fps 32
}
$sequence "ref_reload_m203" {
	"Atlas_anims\ref_reload_m203_blend01"
	"Atlas_anims\ref_reload_m203_blend02"
	blend XR -70 70
	fps 35
}
$sequence "crouch_draw_m16" {
	"Atlas_anims\crouch_draw_m16_blend01"
	"Atlas_anims\crouch_draw_m16_blend02"
	blend XR -70 70
	fps 35
}
$sequence "crouch_aim_m16" {
	"Atlas_anims\crouch_aim_m16_blend01"
	"Atlas_anims\crouch_aim_m16_blend02"
	blend XR -70 70
	fps 12
	loop
}
$sequence "crouch_shoot_m16" {
	"Atlas_anims\crouch_shoot_m16_blend01"
	"Atlas_anims\crouch_shoot_m16_blend02"
	blend XR -70 70
	{ event 5005 0 "muzzle_spaw.txt" }
	fps 30
}
$sequence "crouch_shoot_m203" {
	"Atlas_anims\crouch_shoot_m203_blend01"
	"Atlas_anims\crouch_shoot_m203_blend02"
	blend XR -70 70
	fps 30
}
$sequence "crouch_reload_m16" {
	"Atlas_anims\crouch_reload_m16_blend01"
	"Atlas_anims\crouch_reload_m16_blend02"
	blend XR -70 70
	fps 32
}
$sequence "crouch_reload_m203" {
	"Atlas_anims\crouch_reload_m203_blend01"
	"Atlas_anims\crouch_reload_m203_blend02"
	blend XR -70 70
	fps 35
}
$sequence "ref_draw_sniper" {
	"Atlas_anims\ref_draw_sniper_blend01"
	"Atlas_anims\ref_draw_sniper_blend02"
	blend XR -70 70
	fps 15
}
$sequence "ref_aim_sniper" {
	"Atlas_anims\ref_aim_sniper_blend01"
	"Atlas_anims\ref_aim_sniper_blend02"
	blend XR -70 70
	fps 10
	loop
}
$sequence "ref_shoot_sniper" {
	"Atlas_anims\ref_shoot_sniper_blend01"
	"Atlas_anims\ref_shoot_sniper_blend02"
	blend XR -70 70
	{ event 5021 0 "15" }
	fps 20
}
$sequence "ref_aim_sniperscope" {
	"Atlas_anims\ref_aim_sniperscope_blend01"
	"Atlas_anims\ref_aim_sniperscope_blend02"
	blend XR -70 70
	fps 6
	loop
}
$sequence "ref_shoot_sniperscope" {
	"Atlas_anims\ref_shoot_sniperscope_blend01"
	"Atlas_anims\ref_shoot_sniperscope_blend02"
	blend XR -70 70
	{ event 5011 0 "15" }
	fps 20
}
$sequence "ref_reload_sniper" {
	"Atlas_anims\ref_reload_sniper_blend01"
	"Atlas_anims\ref_reload_sniper_blend02"
	blend XR -70 70
	fps 22
}
$sequence "crouch_draw_sniper" {
	"Atlas_anims\crouch_draw_sniper_blend01"
	"Atlas_anims\crouch_draw_sniper_blend02"
	blend XR -70 70
	fps 15
}
$sequence "crouch_aim_sniper" {
	"Atlas_anims\crouch_aim_sniper_blend01"
	"Atlas_anims\crouch_aim_sniper_blend02"
	blend XR -70 70
	fps 10
	loop
}
$sequence "crouch_shoot_sniper" {
	"Atlas_anims\crouch_shoot_sniper_blend01"
	"Atlas_anims\crouch_shoot_sniper_blend02"
	blend XR -70 70
	{ event 5011 0 "15" }
	fps 20
}
$sequence "crouch_aim_sniperscope" {
	"Atlas_anims\crouch_aim_sniperscope_blend01"
	"Atlas_anims\crouch_aim_sniperscope_blend02"
	blend XR -70 70
	fps 6
	loop
}
$sequence "crouch_shoot_sniperscope" {
	"Atlas_anims\crouch_shoot_sniperscope_blend01"
	"Atlas_anims\crouch_shoot_sniperscope_blend02"
	blend XR -70 70
	{ event 5011 0 "15" }
	fps 20
}
$sequence "crouch_reload_sniper" {
	"Atlas_anims\crouch_reload_sniper_blend01"
	"Atlas_anims\crouch_reload_sniper_blend02"
	blend XR -70 70
	fps 22
}
$sequence "ref_draw_saw" {
	"Atlas_anims\ref_draw_saw_blend01"
	"Atlas_anims\ref_draw_saw_blend02"
	blend XR -70 70
	fps 24
}
$sequence "ref_aim_saw" {
	"Atlas_anims\ref_aim_saw_blend01"
	"Atlas_anims\ref_aim_saw_blend02"
	blend XR -70 70
	fps 12
	loop
}
$sequence "ref_shoot_saw" {
	"Atlas_anims\ref_shoot_saw_blend01"
	"Atlas_anims\ref_shoot_saw_blend02"
	blend XR -70 70
	{ event 5005 0 "muzzle_spaw.txt" }
	fps 24
}
$sequence "ref_reload_saw" {
	"Atlas_anims\ref_reload_saw_blend01"
	"Atlas_anims\ref_reload_saw_blend02"
	blend XR -70 70
	fps 30
}
$sequence "crouch_draw_saw" {
	"Atlas_anims\crouch_draw_saw_blend01"
	"Atlas_anims\crouch_draw_saw_blend02"
	blend XR -70 70
	fps 24
}
$sequence "crouch_aim_saw" {
	"Atlas_anims\crouch_aim_saw_blend01"
	"Atlas_anims\crouch_aim_saw_blend02"
	blend XR -70 70
	fps 12
	loop
}
$sequence "crouch_shoot_saw" {
	"Atlas_anims\crouch_shoot_saw_blend01"
	"Atlas_anims\crouch_shoot_saw_blend02"
	blend XR -70 70
	{ event 5005 0 "muzzle_spaw.txt" }
	fps 24
}
$sequence "crouch_reload_saw" {
	"Atlas_anims\crouch_reload_saw_blend01"
	"Atlas_anims\crouch_reload_saw_blend02"
	blend XR -70 70
	fps 30
}
$sequence "deadback" {
	"Atlas_anims\deadback"
	fps 10
}
$sequence "deadsitting" {
	"Atlas_anims\deadsitting"
	fps 10
}
$sequence "deadstomach" {
	"Atlas_anims\deadstomach"
	fps 10
}
$sequence "deadtable" {
	"Atlas_anims\deadtable"
	fps 10
}
$sequence "ShannonCaldwell_v1.2" {
	"Atlas_anims\ShannonCaldwell_v1.2"
	fps 10
}
$sequence "barnaclehit" {
	"Atlas_anims\barnaclehit"
	ACT_BARNACLE_HIT 1
	fps 20
}
$sequence "barnaclepull" {
	"Atlas_anims\barnaclepull"
	ACT_BARNACLE_PULL 1
	fps 25
	loop
}
$sequence "barnaclecrunch" {
	"Atlas_anims\barnaclecrunch"
	ACT_BARNACLE_CHOMP 1
	fps 18
}
$sequence "barnaclechew" {
	"Atlas_anims\barnaclechew"
	ACT_BARNACLE_CHEW 1
	fps 15
	loop
}
$sequence "use_console_loop" {
	"Atlas_anims\use_console_loop"
	fps 10
	loop
}
$sequence "use_console_loop2" {
	"Atlas_anims\use_console_loop2"
	fps 20
	loop
}
$sequence "action_retina" {
	"Atlas_anims\action_retina"
	fps 18
}
$sequence "action_studycart" {
	"Atlas_anims\action_studycart"
	fps 12
}
$sequence "action_wave" {
	"Atlas_anims\action_wave"
	fps 10
}
