// Created by Crowbar 0.74

$modelname "w_rpg.mdl"
$cd "."


$scale 1.0

$bodygroup "body"
{
	studio "w_rpg_reference"
}

$cdmaterials "world_models\w_rpg\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bone01" -32.560001 -8.26 0 27.790001 12.73 11.38

$sequence "idle1" {
	"w_rpg_anims\idle1"
	fps 30
	loop
}
