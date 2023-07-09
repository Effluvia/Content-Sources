// Created by Crowbar 0.74

// Stored modelname (without quotes): "DoD/models/bulb.mdl"
$modelname "bulb.mdl"
$cd "."
$scale 1.0

$bodygroup "bulb"
{
	studio "bulbshade-ref"
	studio "bulb-ref2"
	studio "bulblongwire-ref"
}

$cdmaterials "models\bulb\"

$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "undershade" -9.38 -9.68 -0.01 9.77 9.68 7.13
$hbox 0 "topshade" -9.38 -9.68 -0.01 9.77 9.68 7.44
$hbox 0 "Cylinder01" -2.07 -2.07 0 2.07 2.07 2.55
$hbox 0 "GeoSphere01" -2.97 -6.43 -2.85 2.97 3.12 2.85

$sequence "idle" {
	"bulb_anims\idle"
	ACT_IDLE 1
	fps 10
	loop
}
$sequence "swing" {
	"bulb_anims\swing"
	ACT_IDLE 1
	fps 30
	loop
}
