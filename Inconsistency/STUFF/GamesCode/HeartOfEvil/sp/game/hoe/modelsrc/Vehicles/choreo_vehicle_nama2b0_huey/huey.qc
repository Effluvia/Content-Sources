$modelname "Vehicles\choreo_vehicle_nama2b0_huey.mdl"
$cdmaterials "models\huey"
$cliptotextures

$scale 1.0

"$surfaceprop" "metal"

$eyeposition 0.000000 0.000000 0.000000

// attachments
$attachment "grenadebase" "grenade" 0.000000 0.000000 0.000000 
$attachment "grenademuzzle" "grenadeend" 0.000000 0.000000 0.000000 
$attachment "leftgunbase" "leftgun2" 0.00 0.00 -2.00 rotate -90 -90 0
$attachment "leftgunmuzzle" "leftgun2" 0.00 0.70 22.00 rotate -90 -90 0
$attachment "rightgunbase" "leftgun2" 4.00 180.00 -8.00 rotate -90 -90 0
$attachment "rightgunmuzzle" "leftgun2" 4.00 180.00 18.00 rotate -90 -90 0
$attachment "pgunbase" "gruntarms" 4.50 0.00 0.00 rotate 0 0 0 
$attachment "pgunmuzzle" "gruntgun" 4.50 -1.30 20.00 rotate -90 0 0
$attachment "exhaust" "toprotor" 30.00 -20.00 -120.00 rotate 0 0 0
$attachment "leftmissile" "center" 66.00 -8.00 17.00 rotate 0 0 0
$attachment "rightmissile" "center" -67.00 -5.00 17.00 rotate 0 0 0

// player eyes
$attachment "vehicle_driver_eyes" "vehicle_driver_eyes" 0.00 0.00 0.00 rotate -90 -90 0

// player attachment
$attachment "vehicle_feet_passenger0" "center" 30.00 -15.00 30.00 rotate -90 -90 0

// NPC passenger attachments
$attachment "passenger_barney" "center" -30.00 -12.00 12.00 rotate -90 -90 0
$attachment "passenger_sarge" "center" 0.00 -17.00 18.00 rotate -90 -90 0
$attachment "passenger_pilot" "center" 22.00 -10.00 90.00 rotate -90 -90 0
$attachment "passenger_copilot" "center" -22.00 -10.00 90.00 rotate -90 -90 0

// bone controllers
$controller 0 "leftgun2" YR 0 45
$controller 1 "grunt" YR -100 100
$controller 2 "gruntarms" XR -18 40
$controller 3 "grenade" XR -45 45

// This is to slide the grunt left/right, but only 4 bone controllers are allowed
//$controller 4 "grunt" X -20 20

// 15 hit boxes
$hboxset "default"
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

// C++ expects the name "entryboxes"
$hboxset "entryboxes"
$hbox 0 "center"	    30.00    4.00    0.00   110.00   68.00   80.00

$bodygroup body
{
studio "Ref_Huey_Body.smd"
}

$bodygroup speakers
{
studio "ref_speakers.smd"
}

// MUST BE THE FIRST SEQUENCE
$sequence idle1 "seq_neutral_pose" loop fps 17 {
    activity ACT_IDLE 1
}

// rotating blades
// NOTE: ideally I could play "enter1" as a new animation layer on top of
// whatever animation the model was playing.  As a workaround, I create a
// spinning-blades animation that plays on top of all sequences.
$animation a_neutral seq_neutral_pose frames 0 0
$animation a_spin_blades idle1 subtract a_neutral 0
$sequence spin_blades "a_spin_blades" loop fps 17 {
    delta
    autoplay
    hidden
}

// player entry
$sequence enter1 "seq_enter1" fps 15 {
    snap
}

$collisionmodel "physbox.smd"
{
     $concave

     // Mass in kilograms
     $mass 1000
}

$keyvalues
{
    "vehicle_entry"
    {
        // Entries are: Key = enter animation, Value = Hitbox group
        "enter1"	"0"
    }
    vehicle_npc_passengers
    {
        role_barney
        {
            target_attachment passenger_barney
            entry
            {
                animation get_in_huey
                priority 1
            }
        }
        role_sarge
        {
            target_attachment passenger_sarge
            entry
            {
                animation get_in_huey
                priority 1
            }
        }
    }
}

