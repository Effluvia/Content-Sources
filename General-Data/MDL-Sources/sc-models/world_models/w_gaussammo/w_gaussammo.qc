// Created by Crowbar 0.74

$modelname "w_gaussammo.mdl"
$cd "."


$scale 1.0

$bodygroup "studio"
{
	studio "gaussammo_new"
}

$cdmaterials "world_models\w_gaussammo\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bone01" -4.86 -3.5 0 6.65 1.92 1.75

$sequence "idle" {
	"w_gaussammo_anims\idle"
	fps 30
}
