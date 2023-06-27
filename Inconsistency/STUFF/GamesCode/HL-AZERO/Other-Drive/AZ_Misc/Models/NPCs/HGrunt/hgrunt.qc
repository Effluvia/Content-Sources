/*
==============================================================================

human grunt

==============================================================================
*/

$modelname mdl/hgrunt.mdl
$cd ./smd
$cdtexture ./textures
$externaltextures
$scale 1

$root "Bip01"

// whole body
$body body "reference"


$bodygroup weapons 
{
studio "gunreference(mp5sd)"
blank
}

$include "./hgrunt_shared.qc"
