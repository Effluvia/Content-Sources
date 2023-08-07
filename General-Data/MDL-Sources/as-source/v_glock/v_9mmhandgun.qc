// Created by Crowbar 0.74

// Stored modelname (without quotes): "valve/models/v_9mmhandgun.mdl"
$modelname "v_9mmhandgun.mdl"
$cd "."
$scale 1.0

$bodygroup "studio"
{
	studio "glock_reference"
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

$cdmaterials "models\v_glock\"


$attachment 0 "Bip01 R Hand" 10 2.25 4.25

$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bip01 L Arm2" 0 -1.95 -1.49 11.368419 2.46 2.85
$hbox 0 "Bip01 L Hand" 0 -0.87 -1.73 4.17 1.03 1.87
$hbox 0 "Bip01 R Arm2" 0 -2.09 -2.99 11.368422 2.6 1.63
$hbox 0 "Bip01 R Hand" 0 -0.87 -2.21 18.139999 3.32 5.04
$hbox 0 "Bone02" -0.65 -8.12 -0.69 3.33 1.11 0.79

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
	fps 14
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
	fps 18
}

$sequence "reload_noshot" {
	"v_9mmhandgun_anims\reload_noshot.smd"
	activity "ACT_VM_RELOAD" 1
	{ event 5004 4 "Weapon_Glock.Special2" }
	{ event 5004 23 "Weapon_Glock.Special1" }
	fadein 0.2
	fadeout 0.2
	fps 18
}

$sequence "draw" {
	"v_9mmhandgun_anims\draw.smd"
	activity "ACT_VM_DRAW" 1
	fadein 0.2
	fadeout 0.2
	fps 16
}

$sequence "holster" {
	"v_9mmhandgun_anims\holster.smd"
	activity "ACT_VM_HOLSTER" 1
	fadein 0.2
	fadeout 0.2
	fps 16
}

$sequence "add_silencer" {
	"v_9mmhandgun_anims\add_silencer.smd"
	fadein 0.2
	fadeout 0.2
	fps 18
}
