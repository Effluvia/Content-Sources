$modelname "Vehicles/choreo_vehicle_name0e1_crash.mdl"
$cdmaterials "models\Blackout"

$bodygroup studio
{
studio "vehicle.smd"
}

$attachment "vehicle_passenger_feet0" "Base" 0.00 0.00 0.00 rotate -90 -90 0
$attachment "vehicle_driver_eyes" "View" 0.00 0.00 0.00 rotate -90 -90 0
$attachment "exit" "Base" 0.00 -0.00 72.00 rotate 90 90 0

$hboxset "entryboxes"
$hbox 0 "Base"	   -80.00    0.00 -100.00    80.00   96.00    0.00
$hbox 0 "Base"	   -80.00    0.00 -200.00    80.00   96.00 -100.00
$hbox 0 "Base"	   -80.00    0.00 -300.00    80.00   96.00 -200.00
$hbox 0 "Base"	   -80.00    0.00 -400.00    80.00   96.00 -300.00
$hbox 0 "Base"	   -80.00    0.00 -500.00    80.00   96.00 -400.00
$hbox 0 "Base"	   -80.00    0.00 -600.00    80.00   96.00 -500.00

$animation a_idle "seq_all" frame 55 125
$sequence idle "a_idle" fps 20 snap loop

$animation a_enter1 "seq_all" frame 25 30
$sequence enter1 "a_enter1" fps 30 snap
$animation a_enter2 "seq_all" frame 20 30
$sequence enter2 "a_enter2" fps 30 snap
$animation a_enter3 "seq_all" frame 15 30
$sequence enter3 "a_enter3" fps 30 snap
$animation a_enter4 "seq_all" frame 10 30
$sequence enter4 "a_enter4" fps 30 snap
$animation a_enter5 "seq_all" frame 5 30
$sequence enter5 "a_enter5" fps 30 snap
$animation a_enter6 "seq_all" frame 0 30
$sequence enter6 "a_enter6" fps 30 snap

$animation a_exit "seq_all" frame 125 160
$sequence exit "a_exit" fps 20 snap fadeout 0

$collisionmodel "vehicle.smd"
{
     // Mass in kilograms
     $mass 1000
}

$keyvalues
{
    "vehicle_entry"
    {
        // Entries are: Key = enter animation, Value = Hitbox group
        "enter1" "0"
        "enter2" "1"
        "enter3" "2"
        "enter4" "3"
        "enter5" "4"
        "enter6" "5"
  }
    "vehicle_exit"
    {
        // Entries are: Key = exit animation, Value = "upright" or "upsidedown"
        "exit" "upright"
    }
}

