// Created by Crowbar 0.74

// Stored modelname (without quotes): "valve/models/v_357.mdl"
$modelname "v_357.mdl"
$cd "."
$scale 1.0

$bodygroup "gun"
{
	studio "python_gun"
}
$bodygroup "bullets"
{
	studio "python_bullets"
}
$bodygroup "speedload"
{
	studio "python_speedload"
}
$bodygroup "hands"
{
	studio "python_hands"
}
$bodygroup "scope"
{
	blank
	studio "python_lasersight"
}

$cdmaterials "models\v_python\"


$attachment 0 "python_master" 0 -14 6

$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bip01 R Arm2" 0 -2.8 -4 11.368423 3.02 1.73
$hbox 0 "Bip01 R Hand" 0 -0.87 -1.79 3.89 1.03 1.94
$hbox 0 "python_master" -0.78 -13.55 -1.09 1.01 1.46 8.69
$hbox 0 "cylinder_spin" -1.3 -2 -1.43 1.5 0.29 1.41
$hbox 0 "Speedload Master" -1.15 -4.19 -1.18 1.15 0.08 1.16
$hbox 0 "Bip01 L Arm2" 0 -2.8 -1.73 11.68 3.02 4
$hbox 0 "Bip01 L Hand" 0 -0.87 -1.94 3.89 1.03 1.79
$hbox 0 "sight_master" -1.05 -1.58 -0.77 1.14 1.9 4.71

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