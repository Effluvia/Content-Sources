// Created by Crowbar 0.74

$modelname "v_satchel_radio.mdl"
$cd "."
$scale 1.0

$bodygroup "body"
{
	studio "V_R_hand"
}
$bodygroup "body"
{
	studio "new_v_satchel_radio_detonator"
}

$surfaceprop "default"

$contents "solid"


$cdmaterials "models\v_satchel\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bip01 R Forearm" -0.69 -2.17 -2.46 11.368418 2.8 1.95
$hbox 0 "Bip01 R Hand" -0.55 -1.56 -2.45 3.91 2.409613 2.11
$hbox 0 "Bip01 R Finger0" 0 -0.88 -0.92 2.48 0.34 0.45
$hbox 0 "Dummy22" -1.92 -0.76 -2.01 1.72 0.9 9.86

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
