// Created by Crowbar 0.74

// Stored modelname (without quotes): "c:/sierra/half-life/valve/models/w_357ammo.mdl"
$modelname "w_357ammo.mdl"
$cd "."


$scale 1.0

$bodygroup "studio"
{
	studio "w_python_ammo_reference"
}

$cdmaterials "world_models\w_357ammo\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Box02" -6.08 -2.69 -2.11 5.53 2.69 5.62

$sequence "idle" {
	"w_357ammo_anims\idle"
	fps 30
}
