$modelname "MDL\floater.mdl"
$cd ".\SMD"
$cdtexture ".\Textures"

$body "body" "reference" reverse

$sequence "idle1" "idle1" fps 15 loop ACT_IDLE 1 
$sequence "turn_left" "turn_left" fps 15 ACT_TURN_LEFT 1 
$sequence "turn_right" "turn_right" fps 15 ACT_TURN_RIGHT 1 
$sequence "shoot" "shoot" fps 30 loop ACT_RANGE_ATTACK1 1 
$sequence "float_melee" "float_melee" fps 30 loop ACT_MELEE_ATTACK1 1 
$sequence "die" "die"  fps 30 ACT_DIE_HEADSHOT 1 
$sequence "fall_loop" "fall_loop" fps 15 loop 
$sequence "fall_die" "fall_die" fps 20 ACT_DIESIMPLE 1
$sequence "flinch" "flinch" fps 15 ACT_SMALL_FLINCH 5 
