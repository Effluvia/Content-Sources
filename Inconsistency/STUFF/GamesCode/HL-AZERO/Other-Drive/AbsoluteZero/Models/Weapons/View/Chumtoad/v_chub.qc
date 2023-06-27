

$modelname ./MDL/v_chub.mdl
$cd ./SMD
$cdtexture ./Textures

$scale 1.0

$origin 0 0 64

// whole body
$body hands "vchub_Template(Hands)1" 
$body toad "vchub_Template(toad)1" 

$texturegroup eye
{
{ "CHB_Eye1.bmp" } { "CHB_Eye(Blink2).bmp" } { "CHB_Eye(Blink3).bmp" }
}

$sequence idle1 "vchub_Idle1(Subtle)" loop fps 16
$sequence fidgetLick "vchub_Fidget1(lick)" fps 16
$sequence fidgetCroak "vchub_Fidget1(Croak)" fps 16
$sequence down "vchub_Down1" fps 21
$sequence up "vchub_Up1" fps 36
$sequence Throw "vchub_Throw1" frame 18 33 fps 24 