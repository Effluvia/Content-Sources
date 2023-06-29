$modelname "gibs\huey_part_left_strut.mdl"
$cdmaterials "models\huey\"
$model "studio" "left_strut.smd"

$surfaceprop "metal"

$sequence idle1 "idle1" fps 30.00

$collisionmodel "left_strut.smd" {
	$mass 500.0
}

$keyvalues
{
	huey_chunk
	{
		"offset" "39 -140 36"
	}
}

