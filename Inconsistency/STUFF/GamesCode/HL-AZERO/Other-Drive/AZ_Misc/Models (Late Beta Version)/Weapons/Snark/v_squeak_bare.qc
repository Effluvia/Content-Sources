$modelname "mdl\v_squeak_bare.mdl"
$cd ".\smd"
$cdtexture ".\textures"
$scale 1.0
$cliptotextures

$body "body" "SQUEAK_Template_Biped(Sleeve)1"
$body "body" "python_hands"
$body "body" "SQUEAK_Template_Biped(Weapon)1" reverse



$sequence "idle1" "idle1" fps 16 loop 
$sequence "fidgetfit" "fidgetfit" fps 16 
$sequence "fidgetnip" "fidgetnip" fps 16 
$sequence "down" "down" fps 16 
$sequence "up" "up" fps 30 
$sequence "throw" "throw" fps 24 

