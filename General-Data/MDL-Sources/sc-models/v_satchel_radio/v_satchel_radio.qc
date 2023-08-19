// Created by Crowbar 0.74

$modelname "v_satchel_radio.mdl"
$cd "."
$scale 1.0

$bodygroup "body"
{
	studio "v_hands_HDskel_arms2arms"
}
$bodygroup "body"
{
	studio "satchel_radio_detonator"
}

$surfaceprop "default"

$contents "solid"

$cdmaterials "models\v_satchel_radio\"

$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bip01 R Forearm" 0 -2.09 -2.01 12.68 1.93 1.93
$hbox 0 "Bip01 R Hand" 0 -0.94 -1.77 3.93 1.1 2.09
$hbox 0 "Bip01 R Finger0" 0 -0.84 -0.76 2.19 0.65 0.72
$hbox 0 "Cylinder02" -0.49 -0.69 -6.01 3.14 0.98 5.86

$sequence "idle1" {
	"v_satchel_radio_anims\idle1.smd"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}

$sequence "fidget1" {
	"v_satchel_radio_anims\fidget1.smd"
	activity "ACT_VM_FIDGET" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "draw" {
	"v_satchel_radio_anims\draw.smd"
	activity "ACT_VM_DRAW" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "fire" {
	"v_satchel_radio_anims\fire.smd"
	activity "ACT_VM_PRIMARYATTACK" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "holster" {
	"v_satchel_radio_anims\holster.smd"
	activity "ACT_VM_HOLSTER" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}
