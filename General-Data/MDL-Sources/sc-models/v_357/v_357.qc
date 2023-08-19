// Created by Crowbar 0.74

$modelname "v_357.mdl"
$cd "."
$scale 1.0

$bodygroup "gun"
{
	studio "357_new"
}
$bodygroup "gun"
{
	studio "HANDL"
}
$bodygroup "gun"
{
	studio "HANDR"
}
$bodygroup "gun"
{
	studio "chrome"
}

$surfaceprop "default"

$contents "solid"

$cdmaterials "models\v_357\"

$attachment 0 "Colt_Python" 0 -11 2.5

$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Colt_Python" -0.68 -10.2 -41.480564 0.68 18.416145 3.93
$hbox 0 "Quickloader" -0.97 -0.99 -0.97 0.97 0.99 0.97
$hbox 0 "sixshooter" -1.12 -0.91 -1.07 1.12 0.91 1.07
$hbox 0 "BulletsHelper" 0 -43.582081 0 18.098585 0 4.482939
$hbox 0 "Bullet1" -4.68 -18.040001 0 0 0 44.470001
$hbox 0 "bullet01" -4.14 -18.030001 0 0 0 44.16
$hbox 0 "bullet2" -4.13 -17.99 0 0 0 43.529999
$hbox 0 "bullet3" -4.69 -17.969999 0 0 0 43.209999
$hbox 0 "bullet4" -5.23 -17.99 0 0 0 43.529999
$hbox 0 "bullet5" -5.23 -18.030001 0 0 0 44.16
$hbox 0 "L_lowerarm" -9.87 -1.81 -1.51 0.37 1.52 1.86
$hbox 0 "L_Hand" -3.368393 -1.79 -0.46 0 1.44 1.63
$hbox 0 "r_thumb04" -1.65 -0.53 -0.72 0 1.03 0.45
$hbox 0 "R_lowerarm" -0.37 -1.53 -1.86 9.65 1.8 1.51
$hbox 0 "R_Hand" 0 -1.45 -1.63 3.58 1.96 0.46
$hbox 0 "r_thumb" 0 -1.04 -0.45 1.64 0.37 0.66

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
