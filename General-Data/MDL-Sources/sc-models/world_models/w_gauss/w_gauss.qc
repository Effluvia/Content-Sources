// Created by Crowbar 0.74

$modelname "w_gauss.mdl"
$cd "."


$scale 1.0

$bodygroup "body"
{
	studio "w_gauss_new"
}

$cdmaterials "world_models\w_gauss\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bone01" -22.517998 -5.734799 0 20.897999 6.0264 8.181

$sequence "stayput" {
	"w_gauss_anims\stayput"
	fps 30
}
$sequence "onside" {
	"w_gauss_anims\onside"
	fps 30
}
