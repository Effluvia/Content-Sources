// Created by Crowbar 0.74

$modelname "v_shotgun.mdl"
$cd "."
$scale 1.0

$bodygroup "studio"
{
	studio "refhoorah1"
}
$bodygroup "studio"
{
	studio "refhoorah2"
}
$bodygroup "studio"
{
	studio "HANDL"
}
$bodygroup "studio"
{
	studio "HANDR"
}

$surfaceprop "default"

$contents "solid"

$cdmaterials "models\v_shotgun\"


$attachment 0 "SPAS" 0 2 -30

$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "SPAS" -1.21 -7.61 -28.08 1.73 5.38 19.690001
$hbox 0 "shell.vtf" -0.38 -0.35 -1.83 1.27 1.18 1.83
$hbox 0 "Pump" -1.33 -2.48 -8.04 2.21 3.02 16.315371
$hbox 0 "Ejector" 0 0 -2.18 1.8 1.54 2.18
$hbox 0 "Holdopen" 0 0 0 2.35 2.18 8.26
$hbox 0 "L_lowerarm" -19.74 -3.62 -3.02 0.75 3.05 3.72
$hbox 0 "L_Hand" -6.736786 -3.58 -0.93 0 2.88 3.26
$hbox 0 "Bone24" -2.337132 -0.46 -0.61 0 0.67 0.76
$hbox 0 "Bone25" -1.572148 -0.66 -0.65 0.36 0.6 0.67
$hbox 0 "Bone28" -2.637216 -0.57 -0.77 0 0.69 0.84
$hbox 0 "Bone29" -1.696686 -0.74 -0.92 0.51 0.69 0.53
$hbox 0 "Bone30" -2.04 -0.62 -0.74 0.52 0.53 0.49
$hbox 0 "Bone32" -2.92667 -0.79 -0.71 0 0.67 0.82
$hbox 0 "Bone33" -1.71 -0.71 -0.73 0.48 0.81 0.88
$hbox 0 "Bone34" -2.44 -0.59 -0.7 0 0.54 0.84
$hbox 0 "Bone36" -2.666344 -0.55 -0.61 0 0.5 0.74
$hbox 0 "Bone37" -1.461878 -0.75 -0.74 0.46 0.71 0.76
$hbox 0 "Bone38" -2.05 -0.57 -0.66 0.18 0.53 0.66
$hbox 0 "r_thumb04" -3.3 -1.06 -1.44 0 2.06 0.9
$hbox 0 "R_thumb03" -2.38 -0.68 -0.9 0 0.8 0.93
$hbox 0 "r_thumb05" -2.52 -0.78 -0.88 0 0.89 1
$hbox 0 "R_lowerarm" -0.75 -3.07 -3.72 19.309999 3.6 3.02
$hbox 0 "R_Hand" 0 -2.9 -3.26 7.17 3.92 0.93
$hbox 0 "Bone03" 0 -0.66 -0.67 2.337132 0.6 0.66
$hbox 0 "Bone04" -0.26 -0.59 -0.67 1.572146 0.55 0.57
$hbox 0 "Bone07" 0 -0.73 -0.52 2.637216 0.7 0.96
$hbox 0 "Bone08" -0.42 -0.71 -0.53 1.696686 0.69 0.76
$hbox 0 "Bone11" -0.3 -0.83 -0.81 2.926672 0.67 0.91
$hbox 0 "Bone12" -0.35 -0.71 -0.88 2.02 0.69 0.59
$hbox 0 "Bone13" 0 -0.56 -0.84 2.44 0.57 0.2
$hbox 0 "Bone15" 0 -0.75 -0.74 2.666344 0.71 0.75
$hbox 0 "Bone16" -0.37 -0.7 -0.76 1.68 0.73 0.64
$hbox 0 "Bone17" -0.18 -0.55 -0.66 2.05 0.55 0.47
$hbox 0 "r_thumb" 0 -2.08 -0.9 3.29 0.75 1.33
$hbox 0 "r_thumb3" -0.11 -0.91 -1 2.51 0.76 0.88

$sequence "sm_idle" {
	"v_shotgun_anims\sm_idle.smd"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 45
	loop
}

$sequence "shoot" {
	"v_shotgun_anims\shoot1.smd"
	activity "ACT_VM_PRIMARYATTACK" 1
	{ event 5001 0 "31" }
	fadein 0.2
	fadeout 0.2
	snap
	fps 50
}

$sequence "shoot_big" {
	"v_shotgun_anims\shoot_double.smd"
	activity "ACT_VM_SECONDARYATTACK" 1
	{ event 5001 0 "41" }
	fadein 0.2
	fadeout 0.2
	snap
	fps 50
}

$sequence "reload" {
	"v_shotgun_anims\reload_middle.smd"
	activity "ACT_VM_RELOAD" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 50
}

$sequence "pump" {
	"v_shotgun_anims\reload_end.smd"
	activity "ACT_SHOTGUN_PUMP" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 55
}

$sequence "start_reload" {
	"v_shotgun_anims\reload_start.smd"
	activity "ACT_SHOTGUN_RELOAD_START" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 55
}

$sequence "draw" {
	"v_shotgun_anims\draw.smd"
	activity "ACT_VM_DRAW" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 45
}

$sequence "reholster" {
	"v_shotgun_anims\reholster.smd"
	activity "ACT_VM_HOLSTER" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 45
}

$sequence "idle4" {
	"v_shotgun_anims\idle4.smd"
	activity "ACT_SHOTGUN_IDLE4" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 35
	loop
}

$sequence "deepidle" {
	"v_shotgun_anims\deepidle.smd"
	activity "ACT_SHOTGUN_IDLE_DEEP" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 35
	loop
}
