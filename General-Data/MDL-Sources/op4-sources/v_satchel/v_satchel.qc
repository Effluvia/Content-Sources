// Created by Crowbar 0.74

// Stored modelname (without quotes): "c:\sierra\half-life\gearbox\models\v_satchel.mdl"
$modelname "v_satchel.mdl"
$cd "."


$scale 1.0

$bodygroup "studio"
{
	studio "new_v_satchel_hand_reference"
}
$bodygroup "studio"
{
	studio "new_v_satchel_charge"
}

$cdmaterials "models\v_satchel\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bip01 L Forearm" -0.4 -2.11 -1.22 11.42 2.59 1.92
$hbox 0 "Bip01 L Hand" 0 -1.05 -2.12 3.79 4.149753 1.87
$hbox 0 "Bip01 L Finger0" 0 -0.84 -0.55 2.54 0.46 0.95
$hbox 0 "Bip01 L Finger31" 0 -0.7 -0.46 1.13684 0.45 0.55
$hbox 0 "Bip01 R Forearm" -0.39 -2.11 -2.02 11.43 2.59 1.12
$hbox 0 "Bip01 R Hand" 0 -1.14 -1.88 3.8 0.78 2.12
$hbox 0 "Bip01 R Finger0" 0 -0.81 -1.11 2.53 0.49 0.46
$hbox 0 "Bip01 R Finger31" 0 -0.8 -0.56 1.23 0.36 0.58
$hbox 0 "Dummy22" -0.464522 -3.607854 0 2.23986 2.21074 3.768721
$hbox 0 "Bone03" -7.4 -1.64 -4.61 2.84 11.61 5.32

$sequence "idle1" {
	"v_satchel_anims\idle1.smd"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	fps 20
	loop
}

$sequence "fidget1" {
	"v_satchel_anims\fidget1.smd"
	activity "ACT_VM_FIDGET" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "draw" {
	"v_satchel_anims\draw.smd"
	activity "ACT_VM_DRAW" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "drop" {
	"v_satchel_anims\drop.smd"
	activity "ACT_VM_HOLSTER" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}
