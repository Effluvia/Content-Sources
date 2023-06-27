$modelname "mdl\v_squeak.mdl"
$cd ".\smd"
$cdtexture ".\textures"
$scale 1.0
$cliptotextures

$body "body" "SQUEAK_Template_Biped(Hand)1"
$body "body" "SQUEAK_Template_Biped(Weapon)1" reverse

$texturegroup eye
{
{ "SQK_Eye1.BMP" } { "SQK_Eye1(Blink2).BMP" } { "SQK_Eye1(Blink3).BMP" }
}

$origin 0 0 64

$sequence idle1 "SQUEAK_Idle1(Subtle)" loop fps 16
$sequence fidgetfit "SQUEAK_Fidget1(Fit)" fps 16
$sequence fidgetnip "SQUEAK_Fidget1(nip)" fps 16
$sequence down "SQUEAK_Down1" fps 16
$sequence up "SQUEAK_Up1" fps 30
//old 24
$sequence throw "SQUEAK_Throw1" fps 30
//anim buggy
//$sequence throw2 "SQUEAK_Throw2" fps 24
