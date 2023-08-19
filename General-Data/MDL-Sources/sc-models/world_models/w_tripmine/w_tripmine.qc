// Created by Crowbar 0.74

$modelname "w_tripmine.mdl"
$cd "."

$surfaceprop "default"

$contents "solid"
$scale 1.0

$bodygroup "studio"
{
	studio "tripmine_world_ref"
}

$cdmaterials "world_models\w_tripmine\"


$cbox 0 0 0 0 0 0

$bbox -7.96 -4.22 -3.93 7.99 4.26 3.96

$sequence "world" {
	"w_tripmine_anims\world.smd"
	activity "ACT_TRIPMINE_WORLD" 1
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}

$sequence "ground" {
	"w_tripmine_anims\ground.smd"
	activity "ACT_TRIPMINE_GROUND" 1
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}
