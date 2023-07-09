// Created by Crowbar 0.74

// Stored modelname (without quotes): "C:\Documents and Settings\Administrator\Skrivebord\Models\nanaki"
$modelname "tree.mdl"
$cd "."
$scale 1.0

$bodygroup "body"
{
	studio "reference"
}

$cdmaterials "models\tree\"

$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "joint1" -12.82 -128 -13.18 13.45 0 15.78
$hbox 0 "joint2" -129.320007 -103.910004 -113.690002 107.900002 92.440002 120.160004

$sequence "seq-name" {
	"tree_anims\seq-name"
	ACT_IDLE 1
	fps 60
	loop
}
