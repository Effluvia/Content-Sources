// Created by Crowbar 0.74

// Stored modelname (without quotes): "valve/models/w_suit.mdl"
$modelname "w_suit.mdl"
$cd "."
$scale 1.0

$bodygroup "studio"
{
	studio "wrld_suit"
}

$surfaceprop "default"

$contents "solid"

$cdmaterials "models\w_blue_suit\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Player_Mesh" -9.86 -51.439999 -9.07 19.24 17.459999 6.2

$sequence "idle" {
	"w_blue_suit_anims\idle"
	fps 30
}
