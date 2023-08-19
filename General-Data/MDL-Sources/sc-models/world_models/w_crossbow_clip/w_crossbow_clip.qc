// Created by Crowbar 0.74

$modelname "w_crossbow_clip.mdl"
$cd "."


$scale 1.0

$bodygroup "studio"
{
	studio "crossbow_clip"
}

$cdmaterials "world_models\w_crossbow_clip\"


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bone01" -6.468661 -4.6585 0 8.851151 2.55552 2.32925

$sequence "idle" {
	"w_crossbow_clip_anims\idle"
	fps 30
}
