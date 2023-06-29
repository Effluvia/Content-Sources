$modelname "gibs\huey_part_engine.mdl"
$cdmaterials "models\huey\"
$model "studio" "engine.smd"

$surfaceprop "metal"

$attachment "damage" "origin" 0.00 5.80 -79.00 rotate 90 90 0

$sequence idle1 "idle1" fps 30.00

$collisionmodel "engine_physbox.smd" {
	$concave
	$mass 500.0
}

$keyvalues
{
	huey_chunk
	{
		"offset" "-7 -61 -30"
	}
}
