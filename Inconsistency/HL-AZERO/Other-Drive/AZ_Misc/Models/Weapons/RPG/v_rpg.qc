$modelname ".\mdl\v_rpg.mdl"

$cd ".\SMD"
$cdtexture ".\Textures"
$scale 1.0

$origin -1 0 2

$body "Launcher" "rpg_launcher"
$body "Rocket" "rpg_rocket"
$body "Hands" "rpg_hand" reverse

$sequence "idle" "idle" fps 15 loop 
$sequence "fidget" "fidget" fps 15 loop 
$sequence "reload" "reload" fps 30 
$sequence "fire" "fire" fps 30 
$sequence "holster1" "holster1" fps 30 
$sequence "draw1" "draw1" fps 30 
$sequence "holster2" "holster2" fps 30 
$sequence "draw2" "draw2" fps 30 
$sequence "idle2" "idle2" fps 15 loop 
//$sequence "fidget2" "fidget2" fps 15 loop 
