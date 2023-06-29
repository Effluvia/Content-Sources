$modelname "gibs\huey_part_tail.mdl"
$cdmaterials "models\huey\"
$model "studio" "tail.smd"

$surfaceprop "metal"

$attachment "damage" "origin" 0.00 -55.00 181.00 rotate -90 -90 0

$sequence idle1 "idle1" fps 30.00

$collisionmodel "tail_physbox.smd" {
	$concave
	$mass 500.0
}

$keyvalues
{
	huey_chunk
	{
		"offset" "-8 -40 -282"
	}
}

