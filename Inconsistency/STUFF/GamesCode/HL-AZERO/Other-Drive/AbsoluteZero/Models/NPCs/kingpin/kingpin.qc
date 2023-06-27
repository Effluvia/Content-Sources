/*

kingpin.qc 

*/

$scale 1.5

// kingpin.smd
$modelname ./MDL/kingpin.mdl
$cd ./SMD/
$cdtexture ./Textures/

$body studio "reference"

$eyeposition 0 0 55
//$eyeposition 0 14 55

$sequence idle1 "idle1" fps 22 LOOP origin 0 30 0 ACT_IDLE 1
$sequence walk "walk2" LX fps 30 LOOP ACT_WALK 1
$sequence run "run" LX LOOP ACT_RUN 1
$sequence small_flinch1 "small_flinch1" fps 30 origin 0 30 0 ACT_SMALL_FLINCH 2
$sequence small_flinch2 "small_flinch2" fps 30 origin 0 30 0 ACT_SMALL_FLINCH 1
$sequence attack_both "attack_both" fps 30 origin 0 30 0 ACT_MELEE_ATTACK1 1
$sequence attack_left "attack_left" fps 30 origin 0 30 0 ACT_MELEE_ATTACK1 2
$sequence attack_right "attack_right" fps 30 origin 0 30 0 ACT_MELEE_ATTACK1 3
$sequence mage_start "mage_start" fps 30 origin 0 30 0 
$sequence mage_loop "mage_loop" fps 30 loop origin 0 30 0 
$sequence mage_end "mage_end" fps 30 origin 0 30 0 
$sequence die_forward "die_forward" fps 30 origin 0 30 0 ACT_DIEFORWARD 1
$sequence die_headshot "die_headshot" fps 30 origin 0 30 0 ACT_DIE_HEADSHOT 1
$sequence die_simple "die_simple" fps 30 origin 0 30 0 ACT_DIESIMPLE 1
