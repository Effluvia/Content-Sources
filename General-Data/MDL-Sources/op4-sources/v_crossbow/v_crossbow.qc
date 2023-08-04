// Created by Crowbar 0.74

// Stored modelname (without quotes): "c:\sierra\half-life\gearbox\models\v_crossbow.mdl"
$modelname "v_crossbow.mdl"
$cd "."


$scale 1.0

$bodygroup "body"
{
	studio "compound_scope"
}
$bodygroup "body"
{
	studio "v_crossbow_hands_reference"
}

$cdmaterials "models\v_crossbow\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Xbow biped R Arm2" -0.39 -2.11 -2.02 11.43 2.59 1.12
$hbox 0 "Xbow biped R Hand" 0 -1.14 -1.88 3.8 1.946522 4.704311
$hbox 0 "crossbow parent" -3.94 -20.540001 -5.31 3.96 6.86 3.87
$hbox 0 "grip" -4.000001 -16.859829 -11.834427 4 0.554309 0.485728
$hbox 0 "Slide" -1.64 -3.389289 -3.6 4.05 1.69 0.44
$hbox 0 "Xbow biped L Arm2" -0.4 -2.11 -1.22 11.42 2.59 1.92
$hbox 0 "Xbow biped L Hand" 0 -1.05 -2.12 3.79 0.88 1.87
$hbox 0 "Xbow biped L Finger0" 0 -0.84 -0.53 2.53 0.35 0.95
$hbox 0 "Xbow biped L Finger31" 0 -0.7 -0.46 1.13684 0.45 0.55
$hbox 0 "Xbow biped R Finger0" 0 -0.81 -1.11 2.53 0.49 0.46
$hbox 0 "Xbow biped R Finger31" 0 -0.8 -0.56 1.23 0.36 0.58

$sequence "idle1" {
	"v_crossbow_anims\idle1.smd"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}

$sequence "idle2" {
	"v_crossbow_anims\idle2.smd"
	activity "ACT_CROSSBOW_IDLE_UNLOADED" 1
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}

$sequence "fidget1" {
	"v_crossbow_anims\fidget1.smd"
	activity "ACT_VM_FIDGET" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "fidget2" {
	"v_crossbow_anims\fidget2.smd"
	activity "ACT_CROSSBOW_FIDGET_UNLOADED" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "fire1" {
	"v_crossbow_anims\fire1.smd"
	activity "ACT_VM_PRIMARYATTACK" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 60
}

$sequence "fire2" {
	"v_crossbow_anims\fire2.smd"
	fadein 0.2
	fadeout 0.2
	snap
	fps 30
}

$sequence "fire3" {
	"v_crossbow_anims\fire3.smd"
	activity "ACT_VM_SECONDARYATTACK" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 30
}

$sequence "reload" {
	"v_crossbow_anims\reload.smd"
	activity "ACT_VM_RELOAD" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "draw1" {
	"v_crossbow_anims\draw1.smd"
	activity "ACT_VM_DRAW" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "draw2" {
	"v_crossbow_anims\draw2.smd"
	activity "ACT_CROSSBOW_DRAW_UNLOADED" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "holster1" {
	"v_crossbow_anims\holster1.smd"
	activity "ACT_VM_HOLSTER" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 30
}

$sequence "holster2" {
	"v_crossbow_anims\holster2.smd"
	activity "ACT_CROSSBOW_HOLSTER_UNLOADED" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 30
}
