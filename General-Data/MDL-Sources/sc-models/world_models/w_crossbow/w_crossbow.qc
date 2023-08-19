// Created by Crowbar 0.74

$modelname "w_crossbow.mdl"
$cd "."


$scale 1.0

$bodygroup "body"
{
	studio "w_crossbow_reference"
}

$cdmaterials "world_models\w_crossbow\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bone01" -14.949 -17.215 0 32.296001 18.303999 10.263

$sequence "idle" {
	"w_crossbow_anims\idle"
	fps 30
}
$sequence "onside" {
	"w_crossbow_anims\onside"
	fps 30
}
