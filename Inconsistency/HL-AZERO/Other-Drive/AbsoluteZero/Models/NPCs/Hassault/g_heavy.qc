/*
==============================================================================

german human heavy weapons

==============================================================================
*/

$modelname ./MDL/g_hassault.mdl
$cd ./SMD
$cdtexture ./Textures
$texrendermode "green_material.bmp" "chrome" 
$texrendermode "white_material.bmp" "chrome" 
$scale 1.0
$attachment 0 "20mm cannon" 0 -31.000000 2
$eyeposition 0 0 70

// whole body
$body studio "reference(german)"

$bodygroup weapons 
{
studio "gunreference"
blank
}

$include "./heavy_shared.qc"