// Created by Crowbar 0.74

$modelname "w_argrenade.mdl"
$cd "."


$scale 1.0

$bodygroup "studio"
{
	studio "m4_grenade_ammo_reference"
}

$cdmaterials "world_models\w_argrenade\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bone01" -1.79 -1.67 0 4.96 6.08 7.04

$sequence "idle" {
	"w_argrenade_anims\idle"
	fps 30
}
