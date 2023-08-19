// Created by Crowbar 0.74

$modelname "w_satchel.mdl"
$cd "."


$scale 1.0

$bodygroup "studio"
{
	studio "w_satchel"
}

$cdmaterials "world_models\w_satchel\"
$surfaceprop "default"

$contents "solid"

$illumposition 0.411 1.114 9.644

$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "w_satchel" -8.28 -5.94 -7.64 6.69 5.94 12.63

$sequence "idle" {
	"w_satchel_anims\idle"
	fps 30
}
$sequence "onback" {
	"w_satchel_anims\onback"
	fps 30
}
