$modelname "gibs\Hgibs_Guts1.mdl"
$cdmaterials "models\gibs\Hgibs\"
$model "studio" "HGIB_Guts1.smd"

$surfaceprop "bloodyflesh"

$illumposition -0.259 0.550 0.140

$sequence idle1 "idle1" fps 30.00

$collisionmodel "HGIB_Guts1.smd" {
	$mass 2.0
}
