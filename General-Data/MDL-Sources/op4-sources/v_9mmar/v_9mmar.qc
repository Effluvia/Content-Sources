// Created by Crowbar 0.74

// Stored modelname (without quotes): "c:\sierra\half-life\gearbox\models\v_9mmAR.mdl"
$modelname "v_9mmar.mdl"
$cd "."
$scale 1.0

$bodygroup "studio"
{
	studio "new_v_mp5_reference"
}
$bodygroup "studio"
{
	studio "new_v_mp5_hands_reference"
}

$cdmaterials "models\v_9mmar\"


$attachment 0 "Bip01 R Hand" 10 -1.75 8

$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bip01 R Forearm" -0.44 -2.13 -1.99 11.39 2.58 1.14
$hbox 0 "Bip01 R Hand" 0 -1.17 -1.85 3.789474 0.75 2.13
$hbox 0 "Bone01" -1.68 -11.16 -12.98 1.11 2.95 5.59
$hbox 0 "Bone03" -1.22 -0.73 -4.48 0 6.68 0.54
$hbox 0 "Bip01 L Forearm" -0.37 -1.71 -1.58 11.37 1.91 2.56
$hbox 0 "Bip01 L Hand" 0 -0.96 -2.1 3.789474 0.97 1.89
$hbox 0 "Bip01 L Finger0" 0 -0.9 -0.65 2.54 0.4 0.86
$hbox 0 "Bip01 L Finger31" 0 -0.62 -0.42 1.16 0.53 0.6
$hbox 0 "Bip01 R Finger0" 0 -0.85 -1.15 2.47 0.45 0.42
$hbox 0 "Bip01 R Finger31" 0 -0.73 -0.53 1.136842 0.42 0.48

$sequence "longidle" {
	"v_9mmar_anims\longidle"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	fps 8
}
$sequence "idle1" {
	"v_9mmar_anims\idle1"
	activity "ACT_VM_IDLE" 2
	fadein 0.2
	fadeout 0.2
	fps 35
}
$sequence "grenade" {
	"v_9mmar_anims\grenade"
	activity "ACT_VM_SECONDARYATTACK" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 30
}
$sequence "reload" {
	"v_9mmar_anims\reload"
	activity "ACT_VM_RELOAD" 1
	{ event 5004 5 "Weapon_MP5.Special1" }
	{ event 5004 24 "Weapon_MP5.Special2" }
	fadein 0.2
	fadeout 0.2
	fps 30
}
$sequence "deploy" {
	"v_9mmar_anims\deploy"
	activity "ACT_VM_DRAW" 1
	fadein 0.2
	fadeout 0.2
	fps 12
}
$sequence "deploy2" {
	"v_9mmar_anims\deploy_2"
	activity "ACT_VM_DRAW" 2
	fadein 0.2
	fadeout 0.2
	fps 12
}
$sequence "shoot" {
	"v_9mmar_anims\shoot"
	activity "ACT_VM_PRIMARYATTACK" 1
	{ event 5001 0 "20" }
	fadein 0.2
	fadeout 0.2
	snap
	fps 10
}
$sequence "shoot2" {
	"v_9mmar_anims\shoot_2"
	activity "ACT_VM_PRIMARYATTACK" 2
	{ event 5001 0 "20" }
	fadein 0.2
	fadeout 0.2
	snap
	fps 10
}
$sequence "shoot3" {
	"v_9mmar_anims\shoot_3"
	activity "ACT_VM_PRIMARYATTACK" 3
	{ event 5001 0 "20" }
	fadein 0.2
	fadeout 0.2
	snap
	fps 10
}
