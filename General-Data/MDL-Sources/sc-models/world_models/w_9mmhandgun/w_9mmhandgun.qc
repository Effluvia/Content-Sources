// Created by Crowbar 0.74

$modelname "w_9mmhandgun.mdl"
$cd "."


$scale 1.0

$bodygroup "studio"
{
	studio "w_beretta"
}

$cdmaterials "world_models\w_9mmhandgun\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bone01" -10.43 -2.8 -0.04 7.59 9.45 2.87

$sequence "idle" {
	"w_9mmhandgun_anims\idle"
	fps 30
}
