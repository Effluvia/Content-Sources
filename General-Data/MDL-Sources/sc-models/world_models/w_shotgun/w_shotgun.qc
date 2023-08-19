// Created by Crowbar 0.74

$modelname "w_shotgun.mdl"
$cd "."


$scale 1.0

$bodygroup "studio"
{
	studio "w_spas12_new"
}

$cdmaterials "world_models\w_shotgun\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bone01" -23.101198 -3.7341 0 15.632998 7.460099 3.2076

$sequence "longidle" {
	"w_shotgun_anims\longidle"
	fps 10
}
