$modelname "huey\huey.mdl"
$cdmaterials "models\huey"
$cliptotextures

$scale 1.0

"$surfaceprop" "metal"

$eyeposition 0.000000 0.000000 77.000000

// attachments
$attachment "grenadebase" "grenade" 0.000000 0.000000 0.000000 
$attachment "grenademuzzle" "grenadeend" 0.000000 0.000000 0.000000 
$attachment "leftgunbase" "leftgun2" 0.00 0.00 -2.00 rotate -90 -90 0
$attachment "leftgunmuzzle" "leftgun2" 0.00 0.70 22.00 rotate -90 -90 0
$attachment "rightgunbase" "leftgun2" 4.00 180.00 -8.00 rotate -90 -90 0
$attachment "rightgunmuzzle" "leftgun2" 4.00 180.00 18.00 rotate -90 -90 0
$attachment "pgunbase" "gruntarms" 4.50 0.00 -0.00 rotate -90 90 0
$attachment "pgunmuzzle" "gruntgun" 4.50 -1.30 20.00 rotate -90 0 0
$attachment "exhaust" "center" 0.00 72.00 -120.00 rotate 90 90 0
$attachment "leftmissile" "center" 66.00 -8.00 17.00 rotate 0 0 0
$attachment "rightmissile" "center" -67.00 -5.00 17.00 rotate 0 0 0

// NPC passenger attachments
$attachment "passenger_pilot" "center" 22.00 -10.00 90.00 rotate -90 -90 0
$attachment "passenger_copilot" "center" -22.00 -10.00 90.00 rotate -90 -90 0

// Rappel attachments
$attachment "rappel_left" "center" 56.50 7.97 39.30 rotate -0 -180 90
$attachment "rappel_right" "center" -56.50 7.97 39.30 rotate 0 0 -90
$attachment "rappel0" "center" 54.00 -24.00 64.00 rotate 0 0 0
$attachment "rappel1" "center" -54.00 -24.00 64.00 rotate 0 0 0
$attachment "rappel2" "center" 54.00 -24.00 12.00 rotate 0 0 0
$attachment "rappel3" "center" -54.00 -24.00 12.00 rotate 0 0 0

// namb2 ladder
$attachment "ladder" "center" 60.00 8.00 45.00 rotate -0 0 -90

// bone controllers
$controller 0 "leftgun2" YR 0 45
$controller 1 "grunt" YR -100 100
$controller 2 "gruntarms" XR -18 40
$controller 3 "grenade" XR -45 45

// This is to slide the grunt left/right
$poseparameter "grunt_slide" -1 1
$weightlist grunt { "grunt" 1.0 }
$animation a_idle0 idle1
$animation a_grunt_left "seq_grunt.smd" frame 0 0 subtract a_idle0 0 weightlist grunt
$animation a_grunt_right "seq_grunt.smd" frame 1 1 subtract a_idle0 0 weightlist grunt
$sequence grunt_slide { a_grunt_left a_grunt_right blend grunt_slide -1 1 } weightlist grunt delta autoplay

// 15 hit boxes
$hbox 0 "center" -60.000000 -8.000000 -40.000000  60.000000 70.000000 8.000000
$hbox 0 "center" -40.000000 -4.000000 -80.000000  40.000000 64.000000 -40.000000
$hbox 0 "center" -20.000000 8.000000 -120.000000  20.000000 56.000000 -80.000000
$hbox 0 "center" -16.000000 56.000000 -120.000000  16.000000 86.000000 -80.000000
$hbox 0 "center" -16.000000 64.000000 -80.000000  16.000000 86.000000 -40.000000
$hbox 0 "center" -32.000000 70.000000 -40.000000  32.000000 90.000000 16.000000
$hbox 0 "center" -60.000000 64.000000 8.000000  60.000000 70.000000 64.000000
$hbox 0 "center" -56.000000 -8.000000 64.000000  56.000000 70.000000 100.000000
$hbox 0 "center" -60.000000 -8.000000 8.000000  60.000000 8.000000 64.000000
$hbox 0 "center" -52.000000 0.000000 100.000000  52.000000 32.000000 144.000000
$hbox 0 "center" -52.000000 32.000000 100.000000  52.000000 60.000000 122.000000
$hbox 0 "center" -16.000000 16.000000 -240.000000  16.000000 64.000000 -120.000000
$hbox 0 "center" -8.000000 32.000000 -380.000000  8.000000 68.000000 -240.000000
$hbox 0 "center" -80.000000 -12.000000 -42.000000  -60.000000 4.000000 16.000000
$hbox 0 "center" 80.000000 -12.000000 -42.000000  60.000000 4.000000 16.000000
$hbox 1 "center" -24.000000 8.000000 20.000000  24.000000 64.000000 64.000000
$hbox 1 "grunt" -8.000000 8.000000 0.000000  8.000000 -16.000000 56.000000
$hbox 1 "grunt" 0.000000 -16.000000 8.000000  8.000000 -24.000000 32.000000

$bodygroup body
{
studio "Ref_Huey_Body.smd"
}

$bodygroup grunt
{
blank
studio "Ref_Grunt.smd"
studio "Ref_MikeForce.smd"
}

$bodygroup grenade
{
blank
studio "Ref_Grenade.smd"
}

$sequence idle1 "idle1" loop fps 17 {
    activity ACT_IDLE 1
}

$weightlist blades toprotor 1.0 tailrotor 1.0
$weightlist noblades center 1.0 toprotor 0.0 tailrotor 0.0
$sequence layer_spin_blades idle1 subtract layer_spin_blades 0 loop fps 17 realtime delta
$sequence crashing seq_crashing loop fps 30 {
    activity ACT_HUEY_CRASHING 1
    addlayer layer_spin_blades
//    weightlist noblades
}

$collisionmodel "physbox.smd"
{
     $concave

     // Mass in kilograms
     $mass 2400 // same as combine heli
}

