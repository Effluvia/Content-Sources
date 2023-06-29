$modelname "spx_crossbreed\gibs\namGrunt_gib3.mdl"

$bodygroup studio
{
studio "namGrunt_gib3.smd"
}

$cdmaterials "models\spx_crossbreed"
$cliptotextures

$scale 1.0

$surfaceprop "zombieflesh"

$sequence ragdoll "ragdoll" ACT_DIERAGDOLL 1 fps 30.00
$collisionjoints "namGrunt_gib3_phys.smd" {

	$mass 15.0
	$inertia 2.0
	$damping 0.01
	$rotdamping 1.50
	$rootbone "bip01"

//	$jointconstrain "bip01_r_arm1" x limit -15.00 30.00 0.20
//	$jointconstrain "bip01_r_arm1" y limit -45.00 64.00 0.20
//	$jointconstrain "bip01_r_arm1" z limit -73.00 12.00 0.20

	$jointconstrain "bip01_r_arm2" x limit 0.00 0.00 0.00
	$jointconstrain "bip01_r_arm2" y limit -40.00 10.00 0.20
	$jointconstrain "bip01_r_arm2" z limit -100.00 20.00 0.20

	$jointconstrain "bip01_r_hand" x limit -60.00 60.00 0.20
	$jointconstrain "bip01_r_hand" y limit -30.00 30.00 0.20
	$jointconstrain "bip01_r_hand" z limit -90.00 70.00 0.20
}

