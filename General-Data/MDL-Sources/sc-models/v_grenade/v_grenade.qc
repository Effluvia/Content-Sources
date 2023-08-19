// Created by Crowbar 0.74

$modelname "v_grenade.mdl"
$cd "."
$scale 1.0

$bodygroup "SVEN"
{
	studio "Sven_Hands_ref"
}
$bodygroup "PINEAPPLE"
{
	studio "Mk2Nade_ref"
}

$cdmaterials "models\v_grenade\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bip01 L UpperArm" -2.18 -1.82 -2.75 11.368427 3.03 2.35
$hbox 0 "Bip01 L Hand" -0.33 -1.22 -1.71 3.87 0.82 2
$hbox 0 "Bip01 L Finger0" 0 -0.75 -0.53 2.373625 0.26 0.81
$hbox 0 "L_Arm_bone" -2.28 -2.51 -2.49 2.33 2.26 9.09
$hbox 0 "Bip01 R UpperArm" -2.18 -1.82 -2.35 11.368425 3.03 2.75
$hbox 0 "Bip01 R Forearm" -1.1 -1.82 -1.8 11.368418 2.24 2.19
$hbox 0 "Bip01 R Hand" -0.33 -1.22 -2 3.87 0.82 1.71
$hbox 0 "Bip01 R Finger0" 0 -0.75 -0.81 2.373624 0.26 0.53
$hbox 0 "R_Arm_bone" -2.15 -2.51 -2.47 2.45 2.26 9.11
$hbox 0 "pine" -2.305521 -1.35 -2.4 1.31 1.26 2.35

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
