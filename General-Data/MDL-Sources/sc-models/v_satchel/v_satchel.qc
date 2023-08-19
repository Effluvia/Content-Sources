// Created by Crowbar 0.74

$modelname "v_satchel.mdl"
$cd "."
$scale 1.0

$bodygroup "body"
{
	studio "satchel_charge"
}
$bodygroup "body"
{
	studio "v_hands_HDskel_arms2arms"
}

$surfaceprop "default"

$contents "solid"


$cdmaterials "models\v_satchel\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bip01 L Forearm" 0 -2.09 -1.94 12.68 1.91 1.99
$hbox 0 "Bip01 L Hand" 0 -0.94 -2.08 3.93 3.999003 1.78
$hbox 0 "psx_v_satchel" -4.87 -5.92 -4.66 5.04 4.31 4.88
$hbox 0 "Bone01" -0.836786 -0.24 -2.177715 0.44 1.124343 0.03
$hbox 0 "Bip01 L Finger0" 0 -0.89 -0.72 2.088058 0.6 0.76

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
