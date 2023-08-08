// Created by Crowbar 0.74

// Stored modelname (without quotes): "c:\sierra\half-life\barney\models\v_egon.mdl"
$modelname "v_egon.mdl"
$cd "."
$scale 1.0

$bodygroup "body"
{
	studio "new_egon_newgun"
}
$bodygroup "body"
{
	studio "V_L_hand"
}


$surfaceprop "default"

$contents "solid"

$cdmaterials "models\v_egon\"

$attachment 0 "Gauss" -10 2.5 0

$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Gauss" -13.72 -1.26 -6.19 2.15 3.63 9.78
$hbox 0 "Bip01 L Forearm" -0.69 -2.16 -1.97 11.89 2.8 2.44
$hbox 0 "Bip01 L Hand" -0.56 -1.15 -1.87 3.91 0.82 1.69
$hbox 0 "Bip01 L Finger0" 0 -0.89 -0.46 2.5 0.34 0.91

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
