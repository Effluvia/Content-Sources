/*
==============================================================================

player

==============================================================================
*/

$modelname ./MDL/player.mdl
$cd ./SMD
$cdtexture ./Textures

$scale 1.0
$origin 0 0 36

/*
$bodygroup head
{
studio "player_template_biped(Gordon_Head)1"
studio "player_template_biped(Helmet)1"
studio "PLAYER_Template_Biped(female)2"
studio "player_template_biped(HelmetF)1"

}
*/

$controller mouth "bone02" ZR 0 30

$include "./player_shared.qc"

