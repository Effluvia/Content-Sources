// Created by Crowbar 0.74

// Stored modelname (without quotes): "chair_cafe/chair_cafe.mdl"
$modelname "ctyl.mdl"
$cd "."
$scale 1.0

$bodygroup "body"
{
	studio "chair_cafe"
}

$cdmaterials "models\ctyl\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "chair_cafe" -9.51 -9.65 0 9.35 11.37 35.669998

$sequence "idle" {
	"ctyl_anims\idle"
	fps 1
	loop
}
