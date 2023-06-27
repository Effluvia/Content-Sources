$modelname "MDL\v_minigun.mdl"
$cd ".\SMD"
$cdtexture ".\Textures"
$scale 1.0


$bbox 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000
$cbox 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000
$eyeposition 0.000000 0.000000 0.000000


//reference mesh(es)
$body "body" "v_reference"



// 1 attachment(s)
$attachment 0 "Bip01 R Hand001" -70.000000 -20.00000 6.250000


$sequence "idle1" "idle1" fps 8
$sequence "idle_long" "idle_long" fps 15
$sequence "spin_up" "spin_up" fps 30
$sequence "spin_loop" "spin_loop" fps 30
$sequence "spin_fire" "spin_fire" fps 40 { event 5001 0 "20" }
$sequence "spin_down" "spin_down" fps 30
$sequence "draw" "draw" fps 15
$sequence "holster" "holster" fps 15

// End of QC script.
