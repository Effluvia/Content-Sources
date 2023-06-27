$modelname "MDL_E3\scientist.mdl"
$cd ".\SMD"
$cdtexture ".\Textures"

// head controllers
$controller 0 "Bip02 Head" XR -60 60
$controller 1 "Bip02 Head" ZR -50 30

// mouth controllers
//old value was 45!
$controller 4 "Bone01" ZR 0.000000 20.000000

// scale scientist model up
$scale 1.05

// whole body
$body "studio" "Scientist_Template_Pose(Headless_Body)_legitmesh"

$bodygroup "heads"
{
studio "Scientist_Template_Pose(Just_the_Head)1" reverse
studio "Scientist_Einstien_Head_Template_Pose1"
studio "Scientist_Luther_Head_Template_Pose1"
}

$bodygroup "needle"
{
blank
studio "Scientist_Template_Pose(Needle)"
}

$texturegroup skinfamilies 
{
{ "Torso-Fnt.bmp"  "Torso-Bk.bmp" "Pnts-fnt.bmp" "Foot_L.bmp" "Foot_R.bmp" }
{ "Torso-Fnt-PostD.bmp"  "Torso-Bk-PostD.bmp" "Pnts-fnt-Post2.bmp" "Foot_L-PostD.bmp" "Foot_R-PostD.bmp" }
{ "Torso-Fnt-Corpse.bmp"  "Torso-Bk-Corpse.bmp" "Pnts-fnt.bmp" "Foot_L-PostD.bmp" "Foot_R-PostD.bmp" }
}

$include "scientist_shared2.qc"

