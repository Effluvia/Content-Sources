$modelname ".\MDL\houndeye.mdl"
$cd ".\SMD"
$cdtexture ".\Textures"

// whole body
$body studio "reference2"

//$origin 0 -4 0 // this is to fix the fact that his head sticks out of the bounding box

$eyeposition 0 0 27
//$eyeposition 0 22 27

$texturegroup skinfamilies 
{
{ "eye.bmp" }
{ "eye1.bmp" }
{ "eye2.bmp" }
{ "eye3.bmp" }
}

$sequence idle1 "idle1" ACT_IDLE 1 
$sequence idle2 "idle2" LOOP ACT_IDLE 20 
$sequence idle3 "idle3" ACT_IDLE 1 

$sequence run "run" LX loop FPS 30 ACT_RUN 4 origin 0 9 0
$sequence run2 "run2" LX loop FPS 30 ACT_RUN 2 origin 0 9 0
$sequence run3 "run3" LX loop FPS 30 ACT_RUN 1 origin 0 9 0

$sequence die "die_simple" fps 20 ACT_DIESIMPLE 1 { event 2001 5 }
$sequence die1 "die_forward" fps 20 ACT_DIEFORWARD 1  { event 2001 10 }
$sequence die2 "die_backward" fps 20 ACT_DIEBACKWARD 2 { event 2001 8 }
$sequence die3 "die_backward2" fps 20 ACT_DIEBACKWARD 1  { event 2001 7 }

//$sequence attack2 "attack2" fps 25 ACT_RANGE_ATTACK1 1 { event 3 22 }

$sequence attack "attack" fps 25 ACT_RANGE_ATTACK1 1  { event 2 5 } { event 3 59 }


$sequence flinch_small "flinch" fps 30 ACT_SMALL_FLINCH 1
$sequence flinch_small2 "flinch2" fps 30 ACT_SMALL_FLINCH 1
$sequence flinchbig "flinchbig" fps 30 ACT_BIG_FLINCH 1 
$sequence die_crumple "die2" fps 30 ACT_DIESIMPLE 1
//$sequence running_jump "jump" LX fps 30 ACT_LEAP 1 
//breaks code
$sequence walk_limp "walk" LX LOOP fps 30
$sequence walk2 "walk2" LX LOOP FPS 30 ACT_WALK 1
$sequence leaderlook "leaderlook" ACT_GUARD 1 { event 1 10 } { event 1 20 }

$sequence sleep "sleepidle" LOOP ACT_CROUCHIDLE 1
$sequence gotosleep "laydown" ACT_CROUCH 1 
$sequence sleeptostand "sleeptostand" ACT_STAND 1

$sequence madidle "madidle" LOOP ACT_IDLE_ANGRY 2
$sequence madidle2 "madidle2" LOOP FPS 37 ACT_IDLE_ANGRY 3 { event 4 7 } { event 7 7 } { event 4 17 } { event 7 17 } { event 5 34 } { event 7 34 }
$sequence madidle3 "madidle3" LOOP ACT_IDLE_ANGRY 1

$sequence inspect "inspect" FPS 10 ACT_INSPECT_WALL 1
$sequence eat "eat" ACT_EAT 1

$sequence 180l "180l" ACT_TURN_LEFT 1 FPS 30 LOOP
$sequence 180r "180r" ACT_TURN_RIGHT 1 FPS 30 LOOP 

$sequence jumpback "jumpback" ACT_LEAP 1 LX LY LZ { event 6 10 }

$sequence wakefast "jump_up" ACT_HOP 1
$sequence whimper "submisidle" loop fps 35
$sequence jumpwindow "jump_through_window" fps 28 origin 0 0 20

