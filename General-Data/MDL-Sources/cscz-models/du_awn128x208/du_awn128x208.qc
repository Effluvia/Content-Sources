// Created by Crowbar 0.74

$modelname "props\du_awn128x208.mdl"

$bodygroup "studio"
{
	studio "du_awn128x208.smd"
}


$surfaceprop "default"

$contents "solid"

$illumposition 0 0 11

$cdmaterials "models\dedust2cz\"

$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hboxset "default"
$hbox 0 "Box01" -64 -104 -28 64 104 50

$definebone "Box01" "" 0 0 0 0 0 0 0 0 0 0 0 0


$sequence "idle" {
	"du_awn128x208_anims\idle.smd"
	fadein 0.2
	fadeout 0.2
	fps 15
	loop
}

$collisionmodel "du_awn128x208.smd"
{
	$automass
}
