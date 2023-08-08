// Created by Crowbar 0.74

$modelname "v_357.mdl"
$cd "."
$scale 1.0

$bodygroup "gun"
{
	studio "v_python_reference"
}
$bodygroup "hands"
{
	studio "v_barney_hands_reference"
}
$bodygroup "scope"
{
	blank
	studio "v_python_laser_reference"
}


$surfaceprop "default"

$contents "solid"

$cdmaterials "models\v_357\"

$attachment 0 "python" 0 -14 6

$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bip01 R Hand" 0 -1.06 -1.72 5.941133 1.084807 3.855314
$hbox 0 "python" -0.73 -10.38 -5.62 1.07 5.16 3.01
$hbox 0 "revolver" -1.92 -1.99 -57.742607 1.08 22.403749 3.27
$hbox 0 "speed_loader" -1.3 -1.3 0 1.3 1.3 3.15
$hbox 0 "Bip01 L Hand" 0 -1.04 -1.85 3.9 0.82 1.71
$hbox 0 "Bip01 L Finger0" 0 -0.87 -0.47 2.373625 0.36 0.9
$hbox 0 "L_Arm_bone" -1.85 -2.51 -3.31 2.57 2.48 9.18
$hbox 0 "Bip01 R Finger0" 0 -0.85 -0.92 2.373626 0.37 0.45
$hbox 0 "R_Arm_bone" -2.43 -2.52 -3.29 1.98 2.53 9.2
$hbox 0 "lasersight_mesh" -1.36 -11.65 -2.66 0.79 0 1.84

$sequence "idle1" {
	"v_357_anims\idle1.smd"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}

$sequence "fidget1" {
	"v_357_anims\fidget1.smd"
	activity "ACT_VM_FIDGET" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "fire1" {
	"v_357_anims\fire1.smd"
	activity "ACT_VM_PRIMARYATTACK" 1
	{ event 5001 0 "31" }
	fadein 0.2
	fadeout 0.2
	snap
	fps 30
}

$sequence "reload" {
	"v_357_anims\reload.smd"
	activity "ACT_VM_RELOAD" 1
	{ event 5004 72 "Weapon_357.Reload" }
	fadein 0.2
	fadeout 0.2
	fps 36
}

$sequence "holster" {
	"v_357_anims\holster.smd"
	activity "ACT_VM_HOLSTER" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "draw" {
	"v_357_anims\draw.smd"
	activity "ACT_VM_DRAW" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "idle2" {
	"v_357_anims\idle2.smd"
	activity "ACT_VM_IDLE" 2
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}

$sequence "idle3" {
	"v_357_anims\idle3.smd"
	activity "ACT_VM_IDLE" 3
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}
