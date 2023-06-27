/*
==============================================================================

human grunt

==============================================================================
*/

$modelname ./MDL/hgrunt.mdl
$cd ./SMD
$cdtexture ./BMP
$scale 1

$root "Bip01"

// whole body
$body body "ref_nohead"

//why did the grunt have a blank bodygroup for the heads? why was it there? what would the purpose be for it?

$bodygroup head
{
studio "ref_head"
studio "blownuphead"
}

$bodygroup weapons 
{
studio "Old_Mp5_RiggedToHand"
blank
}

$include "./hgrunt_shared.qc"
