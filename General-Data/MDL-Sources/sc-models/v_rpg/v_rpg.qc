// Created by Crowbar 0.74

$modelname "v_rpg.mdl"
$cd "."
$scale 1.0

$bodygroup "body"
{
	studio "v_rpg_reference"
}
$bodygroup "studio"
{
	studio "v_hands_hdskel_arms2arms"
}

$surfaceprop "default"

$contents "solid"

$cdmaterials "models\v_rpg\"

$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Cylinder01" -0.76 -1.02 0 0.77 1.05 3.44
$hbox 0 "Rocket" -1.85 -1.86 -6.38 1.86 1.86 7.03
$hbox 0 "Bip01 L Forearm" 0 -2.09 -1.94 12.68 1.91 1.99
$hbox 0 "Bip01 L Hand" 0 -0.94 -2.08 3.93 1.1 1.78
$hbox 0 "Bip01 L Finger0" 0 -0.89 -0.72 2.088058 0.6 0.76
$hbox 0 "Bip01 R Forearm" 0 -2.09 -2.01 12.68 1.93 1.93
$hbox 0 "Bip01 R Hand" 0 -0.94 -1.77 3.93 1.1 2.09
$hbox 0 "Bip01 R Finger0" 0 -0.84 -0.76 2.19 0.65 0.72

$sequence "idle" {
	"v_rpg_anims\idle.smd"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	fps 15
	loop
}

$sequence "fidget" {
	"v_rpg_anims\fidget.smd"
	activity "ACT_VM_FIDGET" 1
	fadein 0.2
	fadeout 0.2
	fps 15
	loop
}

$sequence "reload" {
	"v_rpg_anims\reload.smd"
	activity "ACT_VM_RELOAD" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 30
}

$sequence "fire" {
	"v_rpg_anims\fire.smd"
	activity "ACT_VM_PRIMARYATTACK" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 30
}

$sequence "holster1" {
	"v_rpg_anims\holster1.smd"
	activity "ACT_VM_HOLSTER" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 30
}

$sequence "draw1" {
	"v_rpg_anims\draw1.smd"
	activity "ACT_VM_DRAW" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "holster2" {
	"v_rpg_anims\holster2.smd"
	activity "ACT_RPG_HOLSTER_UNLOADED" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 30
}

$sequence "draw2" {
	"v_rpg_anims\draw2.smd"
	activity "ACT_RPG_DRAW_UNLOADED" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "idle2" {
	"v_rpg_anims\idle2.smd"
	activity "ACT_RPG_IDLE_UNLOADED" 1
	fadein 0.2
	fadeout 0.2
	fps 15
	loop
}

$sequence "fidget2" {
	"v_rpg_anims\fidget.smd"
	activity "ACT_RPG_FIDGET_UNLOADED" 1
	fadein 0.2
	fadeout 0.2
	fps 15
	loop
}
