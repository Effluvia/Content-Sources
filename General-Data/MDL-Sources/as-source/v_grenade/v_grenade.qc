// Created by Crowbar 0.74

// Stored modelname (without quotes): "valve/models/v_grenade.mdl"
$modelname "v_grenade.mdl"
$cd "."
$scale 1.0

$bodygroup "body"
{
	studio "grenade"
}
$bodygroup "body"
{
	studio "grenade_hand"
}

$cdmaterials "models\v_grenade\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bip01 R Arm1" 0 -2.81 -3.31 12.35 3.49 2.62
$hbox 0 "Bip01 R Arm2" 0 -2.61 -3.14 11.36842 3.21 2.34
$hbox 0 "Bip01 R Hand" 0 -0.87 -1.79 3.89 1.03 1.94
$hbox 0 "Dummy22" -1.01 -1.22 -2.13 1.62 1.92 3.11
$hbox 0 "Bip01 L Arm2" 0 -2.28 -2.12 11.68 2.74 2.38
$hbox 0 "Bip01 L Hand" 0 -0.87 -1.94 3.89 1.03 1.79

$sequence "idle" {
	"v_grenade_anims\idle.smd"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 30
	loop
}

$sequence "fidget" {
	"v_grenade_anims\fidget.smd"
	activity "ACT_VM_FIDGET" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 30
	loop
}

$sequence "pinpull" {
	"v_grenade_anims\pinpull.smd"
	activity "ACT_VM_PRIMARYATTACK" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 50
}

$sequence "throw1" {
	"v_grenade_anims\throw1.smd"
	activity "ACT_HANDGRENADE_THROW1" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 25
}

$sequence "throw2" {
	"v_grenade_anims\throw2.smd"
	activity "ACT_HANDGRENADE_THROW2" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 30
}

$sequence "throw3" {
	"v_grenade_anims\throw3.smd"
	activity "ACT_HANDGRENADE_THROW3" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 40
}

$sequence "holster" {
	"v_grenade_anims\holster.smd"
	activity "ACT_VM_HOLSTER" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 30
}

$sequence "draw" {
	"v_grenade_anims\draw.smd"
	activity "ACT_VM_DRAW" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}
