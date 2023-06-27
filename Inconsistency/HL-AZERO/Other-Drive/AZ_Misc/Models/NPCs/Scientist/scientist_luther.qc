
$modelname "MDL\scientist_e3.mdl"
$cd ".\SMD"
$cdtexture ".\Textures"
$scale 1.05
$cliptotextures

$bbox -16.000000 -16.000000 0.000000 16.000000 16.000000 72.000000
$cbox 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000
$eyeposition 0.000000 0.000000 60.000000


//reference mesh(es)
$body "studio" "Scientist_Template_Pose(Headless_Body)1"

$bodygroup "heads"
{
studio "Scientist_Template_Pose(Just_the_Head)1"
studio "Scientist_Einstien_Head_Template_Pose1"
studio "Scientist_luther_Head_Template_Pose1"
}

// 2 skin families
$texturegroup skinfamilies 
{
{ "Torso-Fnt.bmp"  "Torso-Bk.bmp" "Pnts-fnt.bmp" "Hd_Lft1.bmp" "Hd_Bck1.bmp" "Neck_Bck1.bmp" "Face.bmp" "Foot_L.bmp" "Foot_R.bmp" }
{ "Torso-Fnt-PostD.bmp"  "Torso-Bk-PostD.bmp" "Pnts-fnt-Post2.bmp" "Hd_Lft1-PostD.bmp" "Hd_Bck1-PostD.bmp" "Neck_Bck1-PostD.bmp" "Face-PostD.bmp" "Foot_L-PostD.bmp" "Foot_R-PostD.bmp" }
{ "Torso-Fnt-Corpse.bmp"  "Torso-Bk-Corpse.bmp" "Pnts-fnt.bmp" "Hd_Lft1-PostD.bmp" "Hd_Bck1-PostD.bmp" "Neck_Bck1-PostD.bmp" "Face-PostD.bmp" "Foot_L-PostD.bmp" "Foot_R-PostD.bmp" }
}

// 2 bone controller(s)
$controller 0 "Bip02 Head" XR -60.000000 60.000000
$controller 4 "Bone01" ZR 0.000000 45.000000

$include ".//scientist_shared.qc"