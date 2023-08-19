// Created by Crowbar 0.74

$modelname "w_egon.mdl"
$cd "."


$scale 1.0

$bodygroup "body"
{
	studio "w_egon_reference"
}

$cdmaterials "world_models\w_egon\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Box02" -28.68 -35.73 -33.130001 12.3 3.93 1.79

$sequence "stayput" {
	"w_egon_anims\stayput"
	fps 1
}
