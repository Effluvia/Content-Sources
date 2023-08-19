// Created by Crowbar 0.74

$modelname "v_squeak.mdl"
$cd "."
$scale 1.0

$bodygroup "body"
{
	studio "v_hands_HDskel_arms2arms"
}
$bodygroup "body"
{
	studio "v_snark_reference"
}

$surfaceprop "default"
$contents "solid"
$cdmaterials "models\v_squeak\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bip01 R Forearm" -0.11 -1.65 -1.71 11.74 2.13 2.12
$hbox 0 "Bip01 R Hand" 0 -0.62 -1.74 3.8 3.861825 2.664376
$hbox 0 "Bip01 L Forearm" -0.11 -1.65 -2.12 11.74 2.13 1.71
$hbox 0 "Bip01 L Hand" 0 -0.62 -1.71 3.8 0.92 1.74
$hbox 0 "Bip02 L Thigh" 0 -0.05 -1.49 2.94 1.65 0.23
$hbox 0 "Bip02 L Calf" -0.01 -0.62 -0.52 2.72 0.75 0.78
$hbox 0 "Bip02 R Thigh" 0 -0.65 -0.04 2.78 1.15 1.88
$hbox 0 "Bip02 Spine" -2.89 -4.62 -2.92 6.57 2.11 2.99
$hbox 0 "Bip02 L UpperArm" 0 -1.06 -2.19 1.9 0.23 0
$hbox 0 "Bip02 L Forearm" 0 -1.01 -1.99 3.86 0.16 0
$hbox 0 "Bip02 R UpperArm" 0 -0.94 0 2.07 0.47 1.61
$hbox 0 "Bip02 R Forearm" 0 -0.66 -0.1 3.77 0.42 1.24
$hbox 0 "Bip02 Head" -0.153153 -1.59 -0.94 1.9 0.57 1.3
$hbox 0 "Dummy18" -1.07 -1.01 -0.37 0.15 0.5 1.01

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
