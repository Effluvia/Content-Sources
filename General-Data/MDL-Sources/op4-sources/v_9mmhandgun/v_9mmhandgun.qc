// Created by Crowbar 0.74

// Stored modelname (without quotes): "c:\sierra\half-life\gearbox\models\v_9mmhandgun.mdl"
$modelname "v_9mmhandgun.mdl"
$cd "."


$scale 1.0

$bodygroup "studio"
{
	studio "v_glock_reference"
}
$bodygroup "studio"
{
	studio "v_glock_hands_reference"
}
$bodygroup "studio"
{
	studio "clip_reference"
}
$bodygroup "silencer"
{
	blank
	studio "glock_reference(silencer)"
}

$cdmaterials "models\v_9mmhandgun\"


$attachment 0 "Bip01 R Hand" 10 2.25 4.25

$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bip01 R Arm2" -0.39 -2.11 -2.02 11.43 2.59 1.12
$hbox 0 "Bip01 R Hand" 0 -1.14 -1.88 18.139999 3.32 5.04
$hbox 0 "Bone02" 0 -7.89 -0.3 4.16 1.36 1.17
$hbox 0 "Bip01 L Arm2" -0.4 -2.11 -1.22 11.42 2.59 1.92
$hbox 0 "Bip01 L Hand" 0 -1.05 -2.12 3.79 0.88 1.87
$hbox 0 "Bip01 L Finger0" 0 -0.84 -0.55 2.54 0.46 0.95
$hbox 0 "Bip01 L Finger1" 0 -0.7 -0.89 1.14 0.37 0.29
$hbox 0 "Bip01 L Finger11" 0 -0.7 -1.02 1.136841 0.35 0
$hbox 0 "Bip01 L Finger31" 0 -0.7 -0.46 1.13684 0.45 0.55
$hbox 0 "Bip01 R Finger0" 0 -0.81 -1.11 2.53 0.49 0.46
$hbox 0 "Bip01 R Finger11" 0 -0.8 0 1.13684 0.25 1.01
$hbox 0 "Bip01 R Finger31" 0 -0.8 -0.56 1.23 0.36 0.58

$sequence "idle1" {
	"v_9mmhandgun_anims\idle1"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	fps 16
	loop
}
$sequence "idle2" {
	"v_9mmhandgun_anims\idle2"
	activity "ACT_VM_IDLE" 2
	fadein 0.2
	fadeout 0.2
	fps 16
	loop
}
$sequence "idle3" {
	"v_9mmhandgun_anims\idle3"
	activity "ACT_VM_IDLE" 3
	fadein 0.2
	fadeout 0.2
	fps 14
	loop
}
$sequence "shoot" {
	"v_9mmhandgun_anims\shoot"
	activity "ACT_VM_PRIMARYATTACK" 1
	{ event 5001 0 "11" }
	fadein 0.2
	fadeout 0.2
	snap
	fps 25
}
$sequence "shoot_empty" {
	"v_9mmhandgun_anims\shoot_empty"
	activity "ACT_GLOCK_SHOOTEMPTY" 1
	{ event 5001 0 "11" }
	fadein 0.2
	fadeout 0.2
	snap
	fps 25
}
$sequence "reload" {
	"v_9mmhandgun_anims\reload"
	activity "ACT_GLOCK_SHOOT_RELOAD" 1
	{ event 5004 4 "Weapon_Glock.Special2" }
	{ event 5004 23 "Weapon_Glock.Special1" }
	fadein 0.2
	fadeout 0.2
	fps 18
}
$sequence "reload_noshot" {
	"v_9mmhandgun_anims\reload_noshot"
	activity "ACT_VM_RELOAD" 1
	{ event 5004 4 "Weapon_Glock.Special2" }
	{ event 5004 23 "Weapon_Glock.Special1" }
	fadein 0.2
	fadeout 0.2
	fps 18
}
$sequence "draw" {
	"v_9mmhandgun_anims\draw"
	activity "ACT_VM_DRAW" 1
	fadein 0.2
	fadeout 0.2
	fps 16
}
$sequence "holster" {
	"v_9mmhandgun_anims\holster"
	activity "ACT_VM_HOLSTER" 1
	fadein 0.2
	fadeout 0.2
	fps 16
}
$sequence "add_silencer" {
	"v_9mmhandgun_anims\add_silencer"
	fps 18
}
