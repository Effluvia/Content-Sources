// Created by Crowbar 0.74

// Stored modelname (without quotes): "c:\sierra\half-life\gearbox\models\v_grenade.mdl"
$modelname "v_grenade.mdl"
$cd "."


$scale 1.0

$bodygroup "body"
{
	studio "grenade"
}
$bodygroup "body"
{
	studio "v_grenade_hands_reference"
}

$cdmaterials "models\v_grenade\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bip01 R Arm2" -0.39 -2.11 -2.02 11.43 2.59 1.12
$hbox 0 "Bip01 R Hand" 0 -1.14 -1.88 3.8 0.965987 2.12
$hbox 0 "Dummy22" -1.01 -1.22 -2.13 1.62 1.92 3.11
$hbox 0 "Bip01 L Arm2" -0.4 -2.11 -1.22 11.42 2.59 1.92
$hbox 0 "Bip01 L Hand" 0 -1.05 -2.12 3.79 0.88 1.87
$hbox 0 "Bip01 L Finger0" 0 -0.84 -0.55 2.54 0.46 0.95
$hbox 0 "Bip01 R Finger0" 0 -0.81 -1.11 2.54 0.49 0.46

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
