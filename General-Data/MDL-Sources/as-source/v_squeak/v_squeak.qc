// Created by Crowbar 0.74

// Stored modelname (without quotes): "valve/models/v_squeak.mdl"
$modelname "v_squeak.mdl"
$cd "."
$scale 1.0

$bodygroup "body"
{
	studio "SQUEAK_Template_Biped(Hand)1"
}
$bodygroup "body"
{
	studio "SQUEAK_Template_Biped(Weapon)1"
}

$cdmaterials "models\v_squeak\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bip01 L Arm2" 0 -2.8 -1.73 11.68 3.02 4
$hbox 0 "Bip01 L Hand" 0 -0.87 -1.94 3.89 1.03 1.79
$hbox 0 "Bip01 R Arm2" 0 -2.8 -4 11.368423 3.02 1.73
$hbox 0 "Bip01 R Hand" 0 -0.87 -1.79 3.89 2.959115 2.02
$hbox 0 "Bip01 R Finger0" 0 -0.59 -0.63 2.57 0.44 0.63
$hbox 0 "Bip02 Spine" -1.9 -4.38 -2.04 6.01 1.85 2.23
$hbox 0 "Bip02 Neck" -0.63 -0.23 -1.38 0.954035 1.681428 1.57
$hbox 0 "Bip02 R Arm2" -0.19 -0.51 -0.45 3.22 0.52 0.64
$hbox 0 "Bip02 Neck1" 0 -0.84 -1.26 1.41 1.41 1.45
$hbox 0 "Bip02 Head" 0 -0.72 -0.46 1.43 0.63 0.68

$sequence "idle1" {
	"v_squeak_anims\idle1.smd"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	fps 16
	loop
}

$sequence "fidgetfit" {
	"v_squeak_anims\fidgetfit.smd"
	activity "ACT_VM_FIDGET" 1
	fadein 0.2
	fadeout 0.2
	fps 16
}

$sequence "fidgetnip" {
	"v_squeak_anims\fidgetnip.smd"
	activity "ACT_VM_FIDGET" 2
	fadein 0.2
	fadeout 0.2
	fps 16
}

$sequence "down" {
	"v_squeak_anims\down.smd"
	activity "ACT_VM_HOLSTER" 1
	fadein 0.2
	fadeout 0.2
	fps 16
}

$sequence "up" {
	"v_squeak_anims\up.smd"
	activity "ACT_VM_DRAW" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "throw" {
	"v_squeak_anims\throw.smd"
	activity "ACT_VM_PRIMARYATTACK" 1
	fadein 0.2
	fadeout 0.2
	fps 24
}
