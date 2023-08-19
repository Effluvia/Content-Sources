// Created by Crowbar 0.74

$modelname "v_egon.mdl"
$cd "."
$scale 1.0

$bodygroup "body"
{
	studio "v_egon_reference"
}
$bodygroup "body"
{
	studio "Sven_Hands_left_ref"
}


$surfaceprop "default"

$contents "solid"

$cdmaterials "models\v_egon\"

$attachment 0 "egon" 0 0 17

$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "egon" -6.21 -2.38 0 9.72 3.4 16.98
$hbox 0 "glow" -2.43 -0.85 -1.93 3.4 3.33 1.99
$hbox 0 "Bip01 L UpperArm" -2.18 -1.82 -2.75 11.368431 3.02 2.35
$hbox 0 "Bip01 L Hand" -0.33 -1.22 -1.7 3.87 0.82 2
$hbox 0 "Bip01 L Finger0" 0 -0.75 -0.53 2.373627 0.26 0.81
$hbox 0 "L_Arm_bone" -2.28 -2.52 -2.49 2.33 2.26 9.09

$sequence "idle1" {
	"v_egon_anims\idle1.smd"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}

$sequence "fidget1" {
	"v_egon_anims\fidget1.smd"
	activity "ACT_VM_FIDGET" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "altfireon" {
	"v_egon_anims\altfireon.smd"
	activity "ACT_VM_SECONDARYATTACK" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "altfirecycle" {
	"v_egon_anims\altfirecycle.smd"
	activity "ACT_VM_SECONDARYATTACK" 2
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}

$sequence "altfireoff" {
	"v_egon_anims\altfireoff.smd"
	activity "ACT_VM_SECONDARYATTACK" 3
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "fire1" {
	"v_egon_anims\fire1.smd"
	activity "ACT_VM_PRIMARYATTACK" 1
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}

$sequence "fire2" {
	"v_egon_anims\fire2.smd"
	activity "ACT_VM_PRIMARYATTACK" 2
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}

$sequence "fire3" {
	"v_egon_anims\fire3.smd"
	activity "ACT_VM_PRIMARYATTACK" 3
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}

$sequence "fire4" {
	"v_egon_anims\fire4.smd"
	activity "ACT_VM_PRIMARYATTACK" 4
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}

$sequence "draw" {
	"v_egon_anims\draw.smd"
	activity "ACT_VM_DRAW" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "holster" {
	"v_egon_anims\holster.smd"
	activity "ACT_VM_HOLSTER" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}
