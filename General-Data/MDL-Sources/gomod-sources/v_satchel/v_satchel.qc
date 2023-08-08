// Created by Crowbar 0.74

$modelname "v_satchel.mdl"
$cd "."
$scale 1.0

$bodygroup "studio"
{
	studio "V_L_hand"
}
$bodygroup "studio"
{
	studio "new_v_satchel_charge"
}

$surfaceprop "default"
$contents "solid"
$cdmaterials "models\v_satchel\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bip01 L Forearm" -0.68 -2.18 -1.96 11.368419 2.76 2.44
$hbox 0 "Bip01 L Hand" -0.55 -1.54 -1.85 3.9 4.149753 2.46
$hbox 0 "Bip01 L Finger0" 0 -0.87 -0.47 2.48 0.35 0.9
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
