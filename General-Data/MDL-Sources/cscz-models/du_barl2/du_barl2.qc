// Created by Crowbar 0.74

$modelname "props\du_barl2.mdl"

$bodygroup "studio"
{
	studio "du_barl2.smd"
}
$surfaceprop "default"

$contents "solid"

$illumposition 0 -0.005 -25.997

$cdmaterials "models\du_barl2\"

$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hboxset "default"
$hbox 0 "Cylinder01" -17 -17 0 17 16.99 52

$definebone "Cylinder01" "" 0 0.000001 -26 0 0 0 0 0 0 0 0 0
$collisionmodel "du_barl2.smd"
{
	$automass
}


$sequence "idle" {
	"du_barl2_anims\idle.smd"
	fadein 0.2
	fadeout 0.2
	fps 15
	loop
}
