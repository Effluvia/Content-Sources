$modelname "MDL\g_scientist.mdl"
$cd ".\SMD"
$cdtexture ".\Textures"
$scale 1.05
$cliptotextures

$bbox -16.000000 -16.000000 0.000000 16.000000 16.000000 72.000000
$cbox 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000
$eyeposition 0.000000 0.000000 60.000000

//reference mesh(es)
$body "studio" "Scientist_Template_Pose(German)"

// 2 bone controller(s)
$controller 0 "Bip02 Head" XR -60.000000 60.000000

$include ".//scientist_shared.qc"