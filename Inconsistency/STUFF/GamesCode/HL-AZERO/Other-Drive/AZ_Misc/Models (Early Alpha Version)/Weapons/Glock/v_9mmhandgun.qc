
$modelname "./v_9mmhandgun.mdl"
$cd "."
$cdtexture "."
$scale 1.0
$origin 0 -1 0

$body studio "v_9mmhandgun_glock_reference"
$body hands "v_9mmhandgun_Bare_Hands_Full"

$bodygroup silencer
{
blank
studio "v_9mmhandgun_silencer_reference"
}

$flags 256


$attachment 0 "Bip01 R Hand" 10 2.25 4.25

$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bip01 R Arm2" 0 -1.7 -1.69 11.368421 2.72 1.77
$hbox 0 "Bip01 R Hand" -0.09 -1.54 -2.23 10.45 2.24 4.11
$hbox 0 "Bone02" -0.28 -7.35 -0.41 3.4 1.68 0.99
$hbox 0 "Bip01 L Arm2" 0 -1.94 -1.49 11.368419 2.45 2.83
$hbox 0 "Bip01 L Hand" -0.21 -1.05 -2.17 4.12 0.95 1.87

$sequence "idle3" {
	"v_9mmhandgun_anims\idle3"
	fps 10
	loop
}
$sequence "idle3" {
	"v_9mmhandgun_anims\idle3_2"
	fps 10
	loop
}
$sequence "idle3" {
	"v_9mmhandgun_anims\idle3_3"
	fps 10
	loop
}
$sequence "shoot" {
	"v_9mmhandgun_anims\shoot"
	{ event 5001 0 "11" }
	fps 25
}
$sequence "shoot_empty" {
	"v_9mmhandgun_anims\shoot_empty"
	{ event 5001 0 "11" }
	fps 25
}
$sequence "reload" {
	"v_9mmhandgun_anims\reload"
{ event 5004 7 "items/9mmclip2.wav" }
{ event 5004 12 "items/9mmclip1.wav" }
{ event 5004 20 "items/guncock1.wav" } 
	fps 14
	
}
$sequence "reload_noshot" {
	"v_9mmhandgun_anims\reload_noshot"
	{ event 5004 4 "items/9mmclip2.wav" }
	{ event 5004 23 "items/9mmclip1.wav" }
	fps 18
}
$sequence "draw" {
	"v_9mmhandgun_anims\draw"
	fps 16
}
$sequence "holster" {
	"v_9mmhandgun_anims\holster"
	fps 16
}
$sequence "add_silencer" {
	"v_9mmhandgun_anims\add_silencer"
	fps 18
}
$sequence "add_silencer" {
	"v_9mmhandgun_anims\add_silencer_2"
	fps 18
}
