$modelname ".\mdl_bare\v_rpg.mdl"

$cd ".\SMD"
$cdtexture ".\Textures"
$scale 1.0

$origin 0 0 64

$body "Launcher" "rpg_launcher"
$body "Rocket" "rpg_rocket"
$body "Hands" "rpg_hand_bare"

$sequence idle "rpg_idle1(simple)" loop fps 30				
$sequence reload "rpg_fire1(reload)" fps 30
$sequence fire "rpg_fire2(empty)" fps 30
$sequence holster "rpg_holster1(loaded_slarty)" fps 30
$sequence draw "rpg_draw1(loaded_slarty)" fps 30
$sequence idle_empty "rpg_idle2(unloaded_simple)" loop fps 30	
$sequence holster_empty "rpg_holster2(unloaded_slarty)" fps 30
$sequence draw_empty "rpg_draw2(unloaded_slarty)" fps 30

