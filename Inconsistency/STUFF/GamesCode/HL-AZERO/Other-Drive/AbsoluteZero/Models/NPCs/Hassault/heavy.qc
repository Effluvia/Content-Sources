/*
==============================================================================

human heavy weapons

==============================================================================
*/

$modelname ./MDL/hassault.mdl
$cd ./SMD
$cdtexture ./Textures
$scale 1
$attachment 0 "20mm cannon" 0 -31.000000 2

$root "Bip01"

$eyeposition 0 0 70
//$eyeposition 0 4 70

$controller 0 "bip01 head" XR -70 40

// whole body
$body studio "reference(final)"

$bodygroup weapons 
{
studio "gunreference"
blank
}

$include "./heavy_shared.qc"