$modelname "spx_crossbreed\gibs\namGrunt_gib2.mdl"

$bodygroup studio
{
studio "namGrunt_gib2.smd"
}

$cdmaterials "models\spx_crossbreed"
$cliptotextures

$scale 1.0

$surfaceprop "zombieflesh"

$sequence ragdoll "ragdoll" ACT_DIERAGDOLL 1 fps 30.00
$collisionjoints "namGrunt_gib2_phys.smd" {

	$mass 50.0
	$inertia 5.00
	$damping 0.01
	$rotdamping 1.50
	$rootbone "bip01"

	$jointconstrain "bip01_r_leg" x limit -25.00 20.00 0.20
	$jointconstrain "bip01_r_leg" y limit -40.00 20.00 0.20
	$jointconstrain "bip01_r_leg" z limit -37.00 63.00 0.20

	$jointconstrain "bip01_r_leg1" x limit 0.00 0.00 0.00
	$jointconstrain "bip01_r_leg1" y limit 0.00 0.00 0.00
	$jointconstrain "bip01_r_leg1" z limit -95.00 4.00 0.20

	$jointconstrain "bip01_l_leg" x limit -25.00 20.00 0.20
	$jointconstrain "bip01_l_leg" y limit -20.00 40.00 0.20
	$jointconstrain "bip01_l_leg" z limit -37.00 63.00 0.20

	$jointconstrain "bip01_l_leg1" x limit 0.00 0.00 0.00
	$jointconstrain "bip01_l_leg1" y limit 0.00 0.00 0.00
	$jointconstrain "bip01_l_leg1" z limit -95.00 4.00 0.20

	$jointconstrain "bip01_l foot" x limit -30.00 30.00 0.20
	$jointconstrain "bip01_l foot" y limit -30.00 30.00 0.20
	$jointconstrain "bip01_l foot" z limit -15.00 35.00 0.20

	$jointconstrain "bip01_r foot" x limit -30.00 30.00 0.20
	$jointconstrain "bip01_r foot" y limit -30.00 30.00 0.20
	$jointconstrain "bip01_r foot" z limit -15.00 35.00 0.20
}

