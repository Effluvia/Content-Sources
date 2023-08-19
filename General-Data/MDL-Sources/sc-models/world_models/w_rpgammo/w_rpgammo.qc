// Created by Crowbar 0.74

$modelname "w_rpgammo.mdl"
$cd "."


$scale 1.0

$bodygroup "studio"
{
	studio "rpgammo_new"
}

$cdmaterials "world_models\w_rpgammo\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bone01" -3.888 -2.8 0 5.32 1.536 1.4

$sequence "idle" {
	"w_rpgammo_anims\idle"
	fps 30
}
