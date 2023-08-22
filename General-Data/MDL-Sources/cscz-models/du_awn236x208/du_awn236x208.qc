// Created by Crowbar 0.74

$modelname "props\du_awn236x208.mdl"

$bodygroup "studio"
{
	studio "du_awn236x208.smd"
}


$surfaceprop "default"

$contents "solid"

$illumposition 0 -0.018 11

$cdmaterials "models\dedust2cz\"

$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hboxset "default"
$hbox 0 "Box02" -176.979996 -104 -28 58.990002 104 50

$definebone "Box02" "" 59.01302 0 0 0 0 0 0 0 0 0 0 0


$sequence "idle" {
	"du_awn236x208_anims\idle.smd"
	fadein 0.2
	fadeout 0.2
	fps 15
	loop
}

$collisionmodel "du_awn236x208.smd"
{
	$automass

}
