$modelname "gibs\huey_part_cabin.mdl"
$cdmaterials "models\huey\"
$model "studio" "cabin.smd"

$surfaceprop "metal"

$attachment "sparks" "origin" 15.00 3.20 61.20 rotate 90 90 0

$sequence idle1 "idle1" fps 30.00

$collisionmodel "cabin_physbox.smd" {
	$concave
	$mass 500.0
}

$keyvalues
{
	huey_chunk
	{
		"offset" "-26 -103 64"
	}
}
