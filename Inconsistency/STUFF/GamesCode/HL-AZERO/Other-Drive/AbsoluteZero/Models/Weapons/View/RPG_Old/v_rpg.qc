$modelname ".\mdl\v_rpg.mdl"

$cd ".\SMD"
$cdtexture ".\Textures"
$scale 1.0

$origin 0 0 64

$body "Launcher" "rpg_launcher"
$body "Rocket" "rpg_rocket"
$body "Hands" "rpg_hand" reverse

$sequence idle "rpg_idle1(loaded_slarty)" loop fps 15				
$sequence fidget1 "rpg_fidget1(loaded_screen_slarty)" loop fps 15
$sequence reload "rpg_reload_slarty" fps 30
$sequence fire "rpg_fire_slarty" fps 30
$sequence holster1 "rpg_holster1(loaded_slarty)" fps 30
$sequence draw1 "rpg_draw1(loaded_slarty)" fps 30
$sequence holster2 "rpg_holster2(unloaded_slarty)" fps 30
$sequence draw2 "rpg_draw2(unloaded_slarty)" fps 30
$sequence fidget2 "rpg_fidget1(unloaded_screen_slarty)" fps 15
