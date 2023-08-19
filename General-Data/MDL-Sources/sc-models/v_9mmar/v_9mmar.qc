// Created by Crowbar 0.74

$modelname "v_9mmar.mdl"
$cd "."
$scale 1.0

$bodygroup "studio"
{
	studio "megaref"
}
$bodygroup "lhand"
{
	studio "HANDL"
}
$bodygroup "rhand"
{
	studio "HANDR"
}

$surfaceprop "default"

$contents "solid"

$cdmaterials "models\v_9mmar\"

$attachment 0 "MP5-N" -12.5 -0.5 0

$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Magazine" -2.84 -6.1 -0.51 0.58 0.66 0.51
$hbox 0 "MP5-N" -13.14 -6.18 -1.08 13.14 3.13 0.94
$hbox 0 "L_lowerarm" -9.87 -1.81 -1.51 0.37 1.52 1.86
$hbox 0 "L_Hand" -3.368393 -1.79 -0.46 0 1.44 1.63
$hbox 0 "r_thumb04" -1.65 -0.53 -0.72 0 1.03 0.45
$hbox 0 "R_lowerarm" -0.37 -1.53 -1.86 9.65 1.8 1.51
$hbox 0 "R_Hand" 0 -1.45 -1.63 3.58 1.96 0.46
$hbox 0 "r_thumb" 0 -1.04 -0.45 1.64 0.37 0.66

$sequence "longidle" {
	"v_9mmar_anims\longidle.smd"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	fps 15
}

$sequence "idle1" {
	"v_9mmar_anims\idle.smd"
	activity "ACT_VM_IDLE" 2
	fadein 0.2
	fadeout 0.2
	fps 25
}

$sequence "grenade" {
	"v_9mmar_anims\launch.smd"
	activity "ACT_VM_SECONDARYATTACK" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 55
}

$sequence "reload" {
	"v_9mmar_anims\reload.smd"
	activity "ACT_VM_RELOAD" 1
	{ event 5004 5 "Weapon_MP5.Special1" }
	{ event 5004 65 "Weapon_MP5.Special2" }
	fadein 0.2
	fadeout 0.2
	fps 55
}

$sequence "deploy" {
	"v_9mmar_anims\deploy.smd"
	activity "ACT_VM_DRAW" 1
	fadein 0.2
	fadeout 0.2
	fps 35
}

$sequence "shoot" {
	"v_9mmar_anims\fire1.smd"
	activity "ACT_VM_PRIMARYATTACK" 1
	{ event 5001 0 "20" }
	fadein 0.2
	fadeout 0.2
	snap
	fps 10
}

$sequence "shoot2" {
	"v_9mmar_anims\fire2.smd"
	activity "ACT_VM_PRIMARYATTACK" 2
	{ event 5001 0 "20" }
	fadein 0.2
	fadeout 0.2
	snap
	fps 10
}

$sequence "shoot3" {
	"v_9mmar_anims\fire3.smd"
	activity "ACT_VM_PRIMARYATTACK" 3
	{ event 5001 0 "20" }
	fadein 0.2
	fadeout 0.2
	snap
	fps 10
}
