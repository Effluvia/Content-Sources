// Created by Crowbar 0.74

// Stored modelname (without quotes): "c:\sierra\half-life\gearbox\models\v_357.mdl"
$modelname "v_357.mdl"
$cd "."


$scale 1.0

$bodygroup "gun"
{
	studio "new_python_gun"
}
$bodygroup "bullets"
{
	studio "new_python_bullets"
}
$bodygroup "speedload"
{
	studio "new_python_speedload"
}
$bodygroup "hands"
{
	studio "new_python_hands_reference"
}
$bodygroup "scope"
{
	blank
	studio "new_python_lasersight"
}

$cdmaterials "models\v_357\"


$attachment 0 "python_master" 0 -14 6

$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bip01 R Forearm" -0.29 -2.04 -1.98 11.55 2.69 1.16
$hbox 0 "Bip01 R Hand" 0 -1.11 -1.85 3.91 0.925514 2.11
$hbox 0 "python_master" -0.78 -13.54 -1.09 1.01 1.46 8.7
$hbox 0 "cylinder_spin" -1.3 -2 -1.42 1.5 0.3 1.42
$hbox 0 "Speedload Master" -0.88 -4.63 -1.03 1.02 0.08 1.16
$hbox 0 "Bip01 L Forearm" -0.41 -2.06 -1.22 11.43 2.67 1.92
$hbox 0 "Bip01 L Hand" 0 -1.05 -2.13 3.79 0.88 1.83
$hbox 0 "Bip01 L Finger0" 0 -0.88 -0.54 2.53 0.41 1.03
$hbox 0 "Bip01 L Finger31" 0 -0.7 -0.53 1.13684 0.45 0.49
$hbox 0 "Bip01 R Finger0" 0 -0.79 -1.08 2.63 0.52 0.48
$hbox 0 "Bip01 R Finger31" 0 -0.76 -0.51 1.22 0.39 0.51
$hbox 0 "sight_master" -1.05 -1.57 -0.76 1.14 1.91 4.72

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
