// Created by Crowbar 0.74

// Stored modelname (without quotes): "valve/models/v_rpg.mdl"
$modelname "v_rpg.mdl"
$cd "."
$scale 1.0

$bodygroup "body"
{
	studio "rpg_rocket"
}
$bodygroup "body"
{
	studio "blowpipe"
}
$bodygroup "body"
{
	studio "rpg_hand"
}

$cdmaterials "models\v_rpg\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "rocket_master" -1.86 -8.62 -1.65 1.93 4.14 2.14
$hbox 0 "Bip01 R Hand" 0 0 0 8.085783 1.563943 7.994959
$hbox 0 "Dummy22" -3.07 -10.94 -5.99 5.37 10.85 2.29
$hbox 0 "Bip01 L Arm2" 0 -2.03 -1.39 11.32 2.41 2.98
$hbox 0 "Bip01 L Hand" 0 -0.93 -1.85 4.18 1.04 1.78

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
	"v_rpg_anims\fidget2.smd"
	activity "ACT_RPG_FIDGET_UNLOADED" 1
	fadein 0.2
	fadeout 0.2
	fps 15
	loop
}
