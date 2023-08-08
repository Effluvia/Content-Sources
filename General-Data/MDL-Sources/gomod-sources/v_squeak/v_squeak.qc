// Created by Crowbar 0.74

// Stored modelname (without quotes): "c:\sierra\half-life\barney\models\v_squeak.mdl"
$modelname "v_squeak.mdl"
$cd "."
$scale 1.0

$bodygroup "body"
{
	studio "barney_LR_hands"
}
$bodygroup "body"
{
	studio "new_SQUEAK_reference"
}

$surfaceprop "default"

$contents "solid"


$cdmaterials "models\v_squeak\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bip01 L Hand" 0 -1.04 -1.85 3.9 0.82 1.71
$hbox 0 "Bip01 L Finger0" 0 -0.87 -0.47 2.373625 0.36 0.9
$hbox 0 "L_wristbone" -1.84 -2.51 -3.31 2.57 2.48 9.18
$hbox 0 "Bip01 R Forearm" -0.68 -2.18 -2.46 11.88 2.76 1.95
$hbox 0 "Bip01 R Hand" 0 -1.06 -1.71 3.9 3.04147 2.671541
$hbox 0 "Bip01 R Finger0" 0 -0.87 -0.92 2.373624 0.36 0.45
$hbox 0 "Bip02 Spine" -1.9 -4.38 -2.03 6.01 1.85 2.23
$hbox 0 "Bip02 Neck" -0.62 -0.23 -1.37 0.954034 1.681429 1.57
$hbox 0 "Bip02 R Forearm" -0.19 -0.5 -0.46 3.22 0.52 0.63
$hbox 0 "Bip02 Neck1" 0 -0.84 -1.26 1.42 1.41 1.46
$hbox 0 "Bip02 Head" 0 -0.73 -0.45 1.44 0.62 0.69

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
