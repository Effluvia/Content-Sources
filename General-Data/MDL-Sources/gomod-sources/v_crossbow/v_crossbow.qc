// Created by Crowbar 0.74

// Stored modelname (without quotes): "c:\sierra\half-life\barney\models\v_crossbow.mdl"
$modelname "v_crossbow.mdl"
$cd "."
$scale 1.0

$bodygroup "body"
{
	studio "compound_scope"
}
$bodygroup "body"
{
	studio "barney_xbow_hand"
}


$surfaceprop "default"

$contents "solid"


$cdmaterials "models\v_crossbow\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Xbow biped R Hand" 0 0 0 3.791503 1.946522 4.704311
$hbox 0 "crossbow parent" -3.94 -20.540001 -5.31 3.96 6.86 3.87
$hbox 0 "grip" -4.000001 -16.859829 -11.834427 4 0.554309 0.485728
$hbox 0 "Slide" -1.64 -3.389289 -3.6 4.05 1.69 0.44
$hbox 0 "Xbow biped L Arm2" -0.68 -2.18 -1.96 11.88 2.76 2.44
$hbox 0 "Xbow biped L Hand" 0 -1.04 -1.85 3.9 0.82 1.71
$hbox 0 "Xbow biped L Finger0" 0 -0.87 -0.47 2.51 0.35 0.9

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
