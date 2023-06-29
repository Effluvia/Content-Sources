$modelname "gibs\huey_part_left_gun.mdl"
$cdmaterials "models\huey\"
$model "studio" "left_gun.smd"

$surfaceprop "metal"

$sequence idle1 "idle1" fps 30.00

$collisionmodel "left_gun.smd" {
	$mass 50.0
}

$keyvalues
{
	huey_chunk
	{
		"offset" "66 -132 3"
	}
}

