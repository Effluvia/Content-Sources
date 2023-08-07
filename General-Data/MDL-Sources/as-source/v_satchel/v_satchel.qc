// Created by Crowbar 0.74

// Stored modelname (without quotes): "valve/models/v_satchel.mdl"
$modelname "v_satchel.mdl"
$cd "."
$scale 1.0

$bodygroup "body"
{
	studio "satchel_hand_fix"
}
$bodygroup "body"
{
	studio "satchel_charge"
}

$cdmaterials "models\v_satchel\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bip01 L Arm2" 0 -2.03 -1.9 11.8 1.73 2.96
$hbox 0 "Bip01 L Hand" 0 -0.86 -1.85 4.17 4.149754 1.78
$hbox 0 "Bip01 L Finger0" 0 -0.45 -0.62 2.09 0.74 0.58
$hbox 0 "Dummy22" -0.464522 -3.607854 0 2.239859 2.21074 3.768721
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

