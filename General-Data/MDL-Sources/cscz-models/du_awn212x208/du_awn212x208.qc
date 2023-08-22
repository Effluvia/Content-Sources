// Created by Crowbar 0.74

$modelname "props\du_awn212x208.mdl"

$bodygroup "studio"
{
	studio "du_awn212x208.smd"
}


$surfaceprop "default"

$contents "solid"

$illumposition 0 -0.051 11

$cdmaterials "models\dedust2cz\"

$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hboxset "default"
$hbox 0 "Box02" -158.970001 -104 -28 52.990002 104 50

$definebone "Box02" "" 53.040867 0 0 0 0 0 0 0 0 0 0 0


$sequence "idle" {
	"du_awn212x208_anims\idle.smd"
	fadein 0.2
	fadeout 0.2
	fps 15
	loop
}

$collisionmodel "du_awn212x208.smd"
{
	$automass

}
