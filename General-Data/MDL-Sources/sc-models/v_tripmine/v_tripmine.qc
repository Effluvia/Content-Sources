// Created by Crowbar 0.74

$modelname "v_tripmine.mdl"
$cd "."
$scale 1.0

$bodygroup "hands"
{
	studio "v_hands_HDskel_arms2arms"
	blank
}
$bodygroup "mine"
{
	studio "V_tripmine_reference"
	studio "w_tripmine_Reference"
}

$surfaceprop "default"
$contents "solid"
$cdmaterials "models\v_tripmine\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bip01 L Forearm" 0 -2.09 -1.94 12.68 1.91 1.99
$hbox 0 "Bip01 L Hand" 0 -0.94 -2.08 3.93 1.1 1.78
$hbox 0 "Bip01 L Finger0" 0 -0.89 -0.72 2.088058 0.6 0.76
$hbox 0 "Bip01 R Forearm" 0 -2.09 -2.01 12.68 1.93 1.93
$hbox 0 "Bip01 R Hand" 0 -0.94 -1.77 3.93 1.1 2.09
$hbox 0 "Bip01 R Finger0" 0 -0.84 -0.76 2.19 0.65 0.72
$hbox 0 "tripmine" -11.14 -3.78 -8.17 11.18 7.17 1.97

$sequence "idle1" {
	"v_tripmine_anims\idle1.smd"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}

$sequence "idle2" {
	"v_tripmine_anims\idle2.smd"
	activity "ACT_VM_IDLE" 2
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}

$sequence "arm1" {
	"v_tripmine_anims\arm1.smd"
	activity "ACT_VM_PRIMARYATTACK" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 60
}

$sequence "place" {
	"v_tripmine_anims\place.smd"
	activity "ACT_VM_SECONDARYATTACK" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 30
}

$sequence "fidget" {
	"v_tripmine_anims\fidget.smd"
	activity "ACT_VM_FIDGET" 1
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}

$sequence "holster" {
	"v_tripmine_anims\holster.smd"
	activity "ACT_VM_HOLSTER" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 30
}

$sequence "draw" {
	"v_tripmine_anims\draw.smd"
	activity "ACT_VM_DRAW" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 30
}
