// Created by Crowbar 0.74

// Stored modelname (without quotes): "valve/models/v_shotgun.mdl"
$modelname "v_shotgun.mdl"
$cd "."
$scale 1.0

$bodygroup "studio"
{
	studio "v_shotgun_reference"
}
$bodygroup "studio"
{
	studio "shell"
}

$cdmaterials "models\v_shotgun\"


$attachment 0 "Bip01 R Hand" 30 1 6

$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bip01 L Arm2" 0 -2.03 -2.94 12.71 1.85 2.2
$hbox 0 "Bip01 L Hand" 0 -0.79 -1.77 4.12 1.25 1.89
$hbox 0 "Bip01 L Finger0" -0.12 -1.22 -0.32 1.44 0.66 0.84
$hbox 0 "Bip01 R Hand" 0 -0.62 0 26.43 1.53 6.26
$hbox 0 "Bone01" -1.9 -2.49 -1.55 7.09 1.48 1.7

$sequence "sm_idle" {
	"v_shotgun_anims\sm_idle.smd"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 9
	loop
}

$sequence "shoot" {
	"v_shotgun_anims\shoot.smd"
	activity "ACT_VM_PRIMARYATTACK" 1
	{ event 5001 0 "31" }
	fadein 0.2
	fadeout 0.2
	snap
	fps 20
}

$sequence "shoot_big" {
	"v_shotgun_anims\shoot_big.smd"
	activity "ACT_VM_SECONDARYATTACK" 1
	{ event 5001 0 "41" }
	fadein 0.2
	fadeout 0.2
	snap
	fps 13
}

$sequence "reload" {
	"v_shotgun_anims\reload.smd"
	activity "ACT_VM_RELOAD" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 16
}

$sequence "pump" {
	"v_shotgun_anims\pump.smd"
	activity "ACT_SHOTGUN_PUMP" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 13
}

$sequence "start_reload" {
	"v_shotgun_anims\start_reload.smd"
	activity "ACT_SHOTGUN_RELOAD_START" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 12
}

$sequence "draw" {
	"v_shotgun_anims\draw.smd"
	activity "ACT_VM_DRAW" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 24
}

$sequence "reholster" {
	"v_shotgun_anims\reholster.smd"
	activity "ACT_VM_HOLSTER" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 30
}

$sequence "idle4" {
	"v_shotgun_anims\idle4.smd"
	activity "ACT_SHOTGUN_IDLE4" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 9
	loop
}

$sequence "deepidle" {
	"v_shotgun_anims\deepidle.smd"
	activity "ACT_SHOTGUN_IDLE_DEEP" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 12
	loop
}
