$modelname "Vehicles\choreo_vehicle_namb2_ladder.mdl"
$cdmaterials "models\huey\ladder"
$cliptotextures

$scale 1.0
//$surfaceprop "popcan"

$bodygroup studio
{
studio "ref_ladder.smd"
}

$attachment "passenger" "bone38" 0.00 -12.40 10.00 rotate 90 90 0

$sequence idle "seq_idle" fps 1
$sequence unwind "seq_unwind" fps 120 
$sequence unwound "seq_unwound" fps 1

$keyvalues
{
    vehicle_npc_passengers
    {
        role_sarge
        {
            target_attachment passenger
            entry
            {
                animation ladder_mount
                priority 1
            }
            idle
            {
                animation ladder_idle
                priority 1
            }
       }
    }
}

