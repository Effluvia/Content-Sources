$modelname "spx_crossbreed\gibs\Male2_gib1.mdl"

$bodygroup studio
{
studio "Male2_gib1.smd"
}

$cdmaterials "models\spx_crossbreed"
$cliptotextures

$scale 1.0

$surfaceprop "zombieflesh"

$sequence ragdoll "ragdoll" ACT_DIERAGDOLL 1 fps 30.00
$collisionjoints "Male1_gib1_phys.smd" {

	$mass 50.0
	$inertia 5.00
	$damping 0.01
	$rotdamping 1.50
	$rootbone "bip01"

	$jointconstrain "bip01_spine2" x limit -60.00 60.00 0.20
	$jointconstrain "bip01_spine2" y limit -25.00 25.00 0.20
	$jointconstrain "bip01_spine2" z limit -25.00 40.00 0.20

	$jointconstrain "bip01_l_arm1" x limit -30.00 15.00 0.20
	$jointconstrain "bip01_l_arm1" y limit -67.00 30.00 0.20
	$jointconstrain "bip01_l_arm1" z limit -73.00 12.00 0.20

	$jointconstrain "bip01_l_arm2" x limit 0.00 0.00 0.00
	$jointconstrain "bip01_l_arm2" y limit -10.00 40.00 0.20
	$jointconstrain "bip01_l_arm2" z limit -100.00 20.00 0.20

	$jointconstrain "bip01_l_hand" x limit -60.00 60.00 0.20
	$jointconstrain "bip01_l_hand" y limit -30.00 30.00 0.20
	$jointconstrain "bip01_l_hand" z limit -90.00 70.00 0.20

	$jointconstrain "bip01 r arm1" x limit -15.00 30.00 0.20
	$jointconstrain "bip01 r arm1" y limit -45.00 64.00 0.20
	$jointconstrain "bip01 r arm1" z limit -73.00 12.00 0.20

	$jointconstrain "bip01 r arm2" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 r arm2" y limit -40.00 10.00 0.20
	$jointconstrain "bip01 r arm2" z limit -100.00 20.00 0.20

	$jointconstrain "bip01 r hand" x limit -60.00 60.00 0.20
	$jointconstrain "bip01 r hand" y limit -30.00 30.00 0.20
	$jointconstrain "bip01 r hand" z limit -90.00 70.00 0.20

	$jointrotdamping "bip01_head" 5.00
	$jointconstrain "bip01_head" x limit -20.00 20.00 0.20
	$jointconstrain "bip01_head" y limit -25.00 25.00 0.20
	$jointconstrain "bip01_head" z limit -13.00 30.00 0.20
}

