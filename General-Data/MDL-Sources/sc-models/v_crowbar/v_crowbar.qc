// Created by Crowbar 0.74

$modelname "v_crowbar.mdl"
$cd "."
$scale 1.0

$bodygroup "hands"
{
	studio "Sven_Hands_right_ref"
}
$bodygroup "studio"
{
	studio "Crowbar_ref"
}

$surfaceprop "default"

$contents "solid"

$cdmaterials "models\v_crowbar\"

$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bip01 R Arm1" -4.37 -3.65 -4.71 22.736847 6.06 5.51
$hbox 0 "Bip01 R Arm2" -2.21 -3.65 -3.6 22.736832 4.48 4.38
$hbox 0 "Bip01 R Hand" -0.66 -4.07 -17.040001 15.21 6.66 34.84
$hbox 0 "Bip01 R Finger0" 0 -1.35 -1.62 5.42 0.8 1.27
$hbox 0 "Bip01 R Finger01" 0 -0.66 -0.77 3.15 0.95 1.36
$hbox 0 "Bip01 R Finger1" -0.17 -1.13 -0.83 2.49 0.63 0.89
$hbox 0 "Bip01 R Finger11" -0.02 -1.02 -0.86 2.273682 0.64 0.83
$hbox 0 "Bip01 R Finger12" -0.37 -0.84 -0.71 2.29 0.53 0.6
$hbox 0 "Bip01 R Finger2" 0 -1.18 -0.97 2.91 0.7 0.81
$hbox 0 "Bip01 R Finger22" -0.17 -1.01 -0.7 2.55 0.49 0.64
$hbox 0 "Bip01 R Finger3" 0 -1.15 -1.03 2.69 0.54 0.66
$hbox 0 "Bip01 R Finger31" 0 -0.98 -1 2.47 0.55 0.66
$hbox 0 "Bip01 R Finger32" -0.1 -0.78 -0.78 2.9 0.49 0.52
$hbox 0 "Bip01 R Finger4" 0 -1.06 -0.73 2.25 0.53 0.74
$hbox 0 "Bip01 R Finger41" 0 -0.87 -0.51 2.16 0.51 0.83
$hbox 0 "Bip01 R Finger42" 0 -0.74 -0.22 1.68 0.43 0.82
$hbox 0 "R_Arm_bone" -4.31 -5.04 -4.94 4.91 4.52 18.23

$sequence "idle1" {
	"v_crowbar_anims\idle1.smd"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	fps 13
}

$sequence "draw" {
	"v_crowbar_anims\draw.smd"
	activity "ACT_VM_DRAW" 1
	fadein 0.2
	fadeout 0.2
	fps 24
}

$sequence "holster" {
	"v_crowbar_anims\holster.smd"
	fadein 0.2
	fadeout 0.2
	fps 24
}

$sequence "attack1" {
	"v_crowbar_anims\attack1.smd"
	activity "ACT_VM_HITCENTER" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 22
}

$sequence "attack1miss" {
	"v_crowbar_anims\attack1miss.smd"
	activity "ACT_VM_MISSCENTER" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 22
}

$sequence "attack2" {
	"v_crowbar_anims\attack2.smd"
	activity "ACT_VM_MISSCENTER" 2
	fadein 0.2
	fadeout 0.2
	snap
	fps 22
}

$sequence "attack2hit" {
	"v_crowbar_anims\attack2hit.smd"
	activity "ACT_VM_HITCENTER" 2
	fadein 0.2
	fadeout 0.2
	snap
	fps 22
}

$sequence "attack3" {
	"v_crowbar_anims\attack3.smd"
	activity "ACT_VM_MISSCENTER" 3
	fadein 0.2
	fadeout 0.2
	snap
	fps 24
}

$sequence "attack3hit" {
	"v_crowbar_anims\attack3hit.smd"
	activity "ACT_VM_HITCENTER" 3
	fadein 0.2
	fadeout 0.2
	snap
	fps 24
}

$sequence "idle2" {
	"v_crowbar_anims\idle2.smd"
	fadein 0.2
	fadeout 0.2
	fps 15
	loop
}

$sequence "idle3" {
	"v_crowbar_anims\idle3.smd"
	fadein 0.2
	fadeout 0.2
	fps 15
	loop
}
