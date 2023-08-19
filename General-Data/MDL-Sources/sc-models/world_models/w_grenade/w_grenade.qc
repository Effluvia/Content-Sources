// Created by Crowbar 0.74

$modelname "w_grenade.mdl"
$cd "."


$scale 1.0

$bodygroup "NADE"
{
	studio "w_mk2_ref"
	studio "w_mk2_thrown_ref"
}

$cdmaterials "world_models\w_grenade\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Line02" -3.311847 -4.763285 -2.991087 3.68874 7.152947 3.039201

$sequence "idle" {
	"w_grenade_anims\idle.smd"
	activity "ACT_IDLE" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "roll1" {
	"w_grenade_anims\roll1.smd"
	activity "ACT_GRENADE_ROLL" 1
	fadein 0.2
	fadeout 0.2
	fps 15
	loop
}

$sequence "roll2" {
	"w_grenade_anims\roll2.smd"
	activity "ACT_GRENADE_ROLL" 2
	fadein 0.2
	fadeout 0.2
	fps 15
	loop
}

$sequence "toss" {
	"w_grenade_anims\toss_1.smd"
	activity "ACT_GRENADE_TOSS" 1
	fadein 0.2
	fadeout 0.2
	fps 10
	loop
}

$sequence "toss2" {
	"w_grenade_anims\toss_2.smd"
	activity "ACT_GRENADE_TOSS" 2
	fadein 0.2
	fadeout 0.2
	fps 10
	loop
}

$sequence "toss3" {
	"w_grenade_anims\toss_3.smd"
	activity "ACT_GRENADE_TOSS" 3
	fadein 0.2
	fadeout 0.2
	fps 10
	loop
}

$sequence "toss4" {
	"w_grenade_anims\toss2.smd"
	activity "ACT_GRENADE_TOSS" 4
	fadein 0.2
	fadeout 0.2
	fps 10
	loop
}
