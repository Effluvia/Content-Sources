// Created by Crowbar 0.74

$modelname "v_9mmhandgun.mdl"
$cd "."
$scale 1.0

$bodygroup "studio"
{
	studio "v_beretta_ref"
}
$bodygroup "studio"
{
	studio "ref_v_hand_left"
}
$bodygroup "studio"
{
	studio "ref_v_hand_right"
}

$surfaceprop "default"

$contents "solid"


$cdmaterials "models\v_9mmhandgun\"


$attachment 0 "CTRL_Beretta_Slide" 6.4 0.8 0

$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bip01 L Forearm" 0 -1.53 -1.43 9.29 1.4 1.45
$hbox 0 "Bip01 L Hand" 0 -0.69 -1.52 2.88 0.8 1.31
$hbox 0 "Bip01 L Finger0" 0 -0.66 -0.53 1.529664 0.44 0.56
$hbox 0 "Bip01 R Forearm" 0 -1.54 -1.48 9.29 1.41 1.42
$hbox 0 "Bip01 R Hand" 0 -0.79 -1.3 2.93 0.82 2.975479
$hbox 0 "Bip01 R Finger0" 0 -0.62 -0.56 1.6 0.48 0.53
$hbox 0 "CTRL_Beretta_ROOT" 0 -1.664594 -0.793712 1.497563 2.938232 2.464076
$hbox 0 "CTRL_Beretta_Slide" -1.09 0 -0.44 5.94 1.78 1.491739
$hbox 0 "dummy_beretta_otherbits" -5.43 -1.86 -0.51 2.6 2.83 0.59

$sequence "idle1" {
	"v_9mmhandgun_anims\idle1.smd"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	fps 16
	loop
}

$sequence "idle2" {
	"v_9mmhandgun_anims\idle2.smd"
	activity "ACT_VM_IDLE" 2
	fadein 0.2
	fadeout 0.2
	fps 16
	loop
}

$sequence "idle3" {
	"v_9mmhandgun_anims\idle3.smd"
	activity "ACT_VM_IDLE" 3
	fadein 0.2
	fadeout 0.2
	fps 16
	loop
}

$sequence "shoot" {
	"v_9mmhandgun_anims\shoot.smd"
	activity "ACT_VM_PRIMARYATTACK" 1
	{ event 5001 0 "11" }
	fadein 0.2
	fadeout 0.2
	snap
	fps 25
}

$sequence "shoot_empty" {
	"v_9mmhandgun_anims\shoot_empty.smd"
	activity "ACT_GLOCK_SHOOTEMPTY" 1
	{ event 5001 0 "11" }
	fadein 0.2
	fadeout 0.2
	snap
	fps 25
}

$sequence "reload" {
	"v_9mmhandgun_anims\reload.smd"
	activity "ACT_GLOCK_SHOOT_RELOAD" 1
	{ event 5004 4 "Weapon_Glock.Special2" }
	{ event 5004 23 "Weapon_Glock.Special1" }
	fadein 0.2
	fadeout 0.2
	fps 21
}

$sequence "reload_noshot" {
	"v_9mmhandgun_anims\reload_empty.smd"
	activity "ACT_VM_RELOAD" 1
	{ event 5004 4 "Weapon_Glock.Special2" }
	{ event 5004 23 "Weapon_Glock.Special1" }
	fadein 0.2
	fadeout 0.2
	fps 21
}

$sequence "draw" {
	"v_9mmhandgun_anims\draw.smd"
	activity "ACT_VM_DRAW" 1
	fadein 0.2
	fadeout 0.2
	fps 19
}

$sequence "holster" {
	"v_9mmhandgun_anims\holster.smd"
	activity "ACT_VM_HOLSTER" 1
	fadein 0.2
	fadeout 0.2
	fps 12
}
