// Created by Crowbar 0.74

$modelname "items\hevsuit.mdl"
$cd "."
$scale 1.0

$bodygroup "body"
{
	studio "HEV_suit_torso_ref"
}
$bodygroup "body"
{
	studio "HEV_suit_legs_ref"
}

$cdmaterials "models\w_suit\"
$surfaceprop "default"

$contents "solid"

$cbox 0 0 0 0 0 0

$bbox -10.4 -4.82 -3.73 15.16 61.34 14.72


$hbox 0 "Bip01 R Foot" -10.4 -4.82 -3.73 15.16 61.34 14.72

$sequence "idle" {
	"w_suit_anims\idle"
	fps 30
}
