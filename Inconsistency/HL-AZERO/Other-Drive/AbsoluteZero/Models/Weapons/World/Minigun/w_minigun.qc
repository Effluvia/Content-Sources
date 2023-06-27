$modelname "./mdl/w_minigun.mdl"
$cd ".\smd"
$cdtexture ".\textures"
$scale 1.0

$bbox 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000
$cbox 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000
$eyeposition 0.000000 0.000000 0.000000


//reference mesh(es)
$body "body" "reference"


// 1 hit box(es)
$hbox 0 "Cylinder01" -10.650000 -9.870000 -1.520000 3.970000 5.900000 74.070000

// 2 animation sequence(s)
$sequence "stayput" "stayput" fps 30 
$sequence "onside" "onside" fps 30 
