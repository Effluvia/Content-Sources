$modelname "gibs\huey_part_roof.mdl"
$cdmaterials "models\huey\"
$model "studio" "roof.smd"

$surfaceprop "metal"

$sequence idle1 "idle1" fps 30.00

$collisionmodel "roof_physbox.smd" {
	$concave
	$mass 50.0
}

$keyvalues
{
	huey_chunk
	{
		"offset" "-9 -51 76"
	}
}

