// Created by Crowbar 0.74

// Stored modelname (without quotes): "E:\temp\helm\barney_helmet.mdl"
$modelname "items\helmet_mk4.mdl"
$cd "."
$scale 0.5

$bodygroup "w_barney_helmet"
{
	studio "barney_helmet"
}

$cdmaterials "models\barney_helmet\"
$surfaceprop "default"

$contents "solid"

$cbox 0 0 0 0 0 0

$bbox -7.36 -6.34 -11.58 7.24 9.25 8.38

$hbox 0 "Object02" -7.36 -6.34 -11.58 7.24 9.25 8.38

$sequence "idle" {
	"barney_helmet_anims\idle"
	fps 30
}
