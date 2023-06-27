$modelname "MDL\g_scientist.mdl"
$cd ".\SMD"
$cdtexture ".\Textures"
$texrendermode "white.bmp" "chrome" 
$texrendermode "pink.bmp" "chrome" 

// head controllers
$controller 0 "Bip02 Head" XR -60 60
$controller 1 "Bip02 Head" ZR -50 30

// mouth controllers

// scale scientist model up
$scale 1.05

// whole body
$body "studio" "Scientist_Template_Pose(German)"

$bodygroup "heads"
{
blank
blank
blank
}

$bodygroup "needle"
{
blank
studio "Scientist_Template_Pose(Needle)"
}

$include "scientist_shared2.qc"

