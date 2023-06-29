$modelname "Vehicles/choreo_vehicle_namb0_huey_exit.mdl"
$cdmaterials "models\Blackout"

$bodygroup studio
{
studio "vehicle.smd"
}

$hboxset "entryboxes"

$attachment "vehicle_passenger_feet0" "Base" 0.00 0.00 0.00 rotate -90 -90 0
$attachment "vehicle_driver_eyes" "View" 0.00 0.00 0.00 rotate -90 -90 0
$attachment "exit1" "Base" 0.00 0.00 0.00 rotate -90 -90 0

// 1 sequences
$sequence enter1 "seq_enter1" fps 30 snap
$sequence exit1 "seq_exit1" fps 30 fadeout 0

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
    }
    "vehicle_exit"
    {
        // Entries are: Key = exit animation, Value = "upright" or "upsidedown"
        "exit1" "upright"
    }
}

