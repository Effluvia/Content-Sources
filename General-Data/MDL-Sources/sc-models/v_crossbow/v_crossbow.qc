// Created by Crowbar 0.74

$modelname "v_crossbow.mdl"
$cd "."
$scale 1.0

$bodygroup "body"
{
	studio "v_crossbow_reference"
}
$bodygroup "body"
{
	studio "v_rhand_HDskel"
}

$surfaceprop "default"

$contents "solid"

$cdmaterials "models\v_crossbow\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "crossbow parent" -3.94 -20.639999 -5.3 3.95 6.86 3.59
$hbox 0 "grip" -4 -10.388143 -2.752418 4 5.101495 0.48573
$hbox 0 "Slide" -1.65 -3.389288 -3.59 4.04 1.69 0.45
$hbox 0 "Bip01 L Forearm" 0 -2.09 -1.94 12.68 1.91 1.99
$hbox 0 "Bip01 L Hand" 0 -0.94 -2.08 3.93 1.1 1.78
$hbox 0 "Bip01 L Finger0" 0 -0.89 -0.72 2.088058 0.6 0.76

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
	fps 35
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
