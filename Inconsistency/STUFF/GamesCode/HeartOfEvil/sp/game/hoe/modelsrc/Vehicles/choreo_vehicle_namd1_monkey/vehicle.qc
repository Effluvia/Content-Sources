$modelname "Vehicles/choreo_vehicle_namd1_monkey.mdl"
$cdmaterials "models\Blackout"

$bodygroup studio
{
studio "vehicle.smd"
}

$attachment "vehicle_passenger_feet0" "Base" 0.00 0.00 0.00 rotate -90 -90 0
$attachment "vehicle_driver_eyes" "View" 0.00 0.00 0.00 rotate -90 -90 0
$attachment "exit" "Base" 0.00 -0.00 72.00 rotate 90 90 0

$animation a_idle "seq_all" frame 10 10 numframes 2
$sequence idle "a_idle" fps 1 snap

$animation a_enter "seq_all" frame 0 10
$sequence enter "a_enter" fps 8 snap

$animation a_exit "seq_all" frame 40 160
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
        "enter" "0"
    }
    "vehicle_exit"
    {
        // Entries are: Key = exit animation, Value = "upright" or "upsidedown"
        "exit" "upright"
    }
}

