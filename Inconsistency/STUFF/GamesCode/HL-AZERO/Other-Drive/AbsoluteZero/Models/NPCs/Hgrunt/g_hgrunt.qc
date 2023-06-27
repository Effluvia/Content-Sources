/*
==============================================================================

human grunt

==============================================================================
*/

$modelname ./MDL/g_hgrunt.mdl
$cd ./SMD
$cdtexture ./BMP
$texrendermode "metal_green.bmp" "chrome" 
$texrendermode "metal_white.bmp" "chrome" 
$scale 1

$root "Bip01"

// whole body
$body body "ref_nohead(german)"

//same with the german grunt? all this does it open it up to loading no head on the grunt!
//to be perfectly honest, it should just be one big reference with both the head and the body.

$body head "ref_head(german)"

//OLD: Restore if we blank on making a decapped version of the head (which I doubt we would)
//$bodygroup head
//{
//studio "ref_head(german)"
//}

$bodygroup weapons 
{
studio "Old_Mp5_RiggedToHand"
blank
}

$include "./hgrunt_shared.qc"
