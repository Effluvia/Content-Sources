// Created by Crowbar 0.74

// Stored modelname (without quotes): "c:\sierra\half-life\gearbox\models\v_satchel_radio.mdl"
$modelname "v_satchel_radio.mdl"
$cd "."


$scale 1.0

$bodygroup "body"
{
	studio "new_v_satchel_radio_hand"
}
$bodygroup "body"
{
	studio "new_v_satchel_radio_detonator"
}

$cdmaterials "models\v_satchel_radio\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bip01 R Forearm" -0.4 -2.07 -2.02 11.44 2.65 1.12
$hbox 0 "Bip01 R Hand" -1.63 -1.15 -1.84 3.8 2.409613 2.14
$hbox 0 "Bip01 R Finger0" 0 -0.84 -1.12 2.373624 0.35 0.43
$hbox 0 "Bip01 R Finger01" 0 -0.79 -0.78 1.39 0.58 0.46
$hbox 0 "Bip01 R Finger31" 0 -0.8 -0.5 1.19 0.35 0.63
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
