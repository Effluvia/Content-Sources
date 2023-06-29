$modelname "gibs\Hgibs_Skull1.mdl"
$cdmaterials "models\gibs\Hgibs\"
$model "studio" "HGIB_Skull1.smd"

$surfaceprop "bloodyflesh"

$illumposition -0.259 0.550 0.140

$sequence idle1 "idle1" fps 30.00

$collisionmodel "HGIB_Skull1.smd" {
	$mass 2.0
}
