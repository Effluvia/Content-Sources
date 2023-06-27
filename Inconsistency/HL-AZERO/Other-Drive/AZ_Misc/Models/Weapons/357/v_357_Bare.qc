$modelname "mdl\v_357_bare.mdl"
$cd ".\smd"
$cdtexture ".\textures"
$scale 1.0
$cliptotextures

$body "gun" "python_gun"
$body "bullets" "python_bullets"
$body "speedload" "python_speedload"
$body "hands" "python_hands_bare"


$attachment 0 "python_master" 0.000000 -14.000000 6.000000

$sequence "idle1" "idle1" fps 30 loop 
$sequence "fidget1" "fidget1" fps 30 
$sequence "fire1" "fire1" fps 30 { event 5001 0 "31" } 
$sequence "reload" "reload" fps 36 { event 5004 70 "weapons/357_reload1.wav" } 
$sequence "holster" "holster" fps 30 
$sequence "draw" "draw" fps 30 
$sequence "idle2" "idle2" fps 30 loop 
$sequence "idle3" "idle3" fps 30 loop 

