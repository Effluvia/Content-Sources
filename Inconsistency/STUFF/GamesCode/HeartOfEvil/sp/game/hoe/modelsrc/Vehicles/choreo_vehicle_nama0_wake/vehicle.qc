$modelname "Vehicles/choreo_vehicle_nama0_wake.mdl"
$cdmaterials "models\Blackout"

$bodygroup studio
{
studio "vehicle.smd"
}

$hboxset "default"
$hbox 0 "GhostAnim_Base"	    -2.00   -2.00   -2.00     2.00    2.00    2.00

$hboxset "entryboxes"
$hbox 0 "GhostAnim_Base"	    -2.00   -2.00   -2.00     2.00    2.00    2.00

$attachment "vehicle_driver_eyes" "GhostAnim_View" 0.00 0.00 0.00 rotate -90 -90 0
$attachment "exit1" "GhostAnim_Base" -60.00 28.00 5.00 rotate 0 0 0

// 1 sequences
$sequence enter1 "seq_enter1" fps 30 snap
$sequence exit1 "seq_exit1" fps 25 fadeout 0

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
        "exit1"	"upright"
    }
}

