$modelname "charlie\charlie_byarms.mdl"
$cdmaterials "models\charlie"
$cliptotextures

$surfaceprop "flesh"

$scale 1.0

$eyeposition 0.000000 0.000000 63.000000

// 1 attachment
//$attachment 0 "Bip01 R Hand" 30.000000 7.5000000 10.00000
$attachment "head" "Bip01 Head" 0.00 0.00 0.00 rotate 0 90 90
$attachment "neck" "Bip01 Neck" 0.500000 1.000000 0.00000

// These are needed for head-turning
$attachment "eyes" "Bip01 Head" 0.00 -0.00 -0.00 rotate -0.00 81.00 90.00
$attachment "forward" "Bip01 Neck" 0.00 -0.00 0.00 rotate -0.00 81.00 90.00

// Left-hand attachment for tossing grenades
$attachment "lefthand" "Bip01 L Hand" 6.00 0.00 0.00 rotate 0.00 0.00 0.00

// Attachment for hoe_corpse_marker
$attachment "CorpseMarker" "Bip01 Pelvis" 0.00 0.00 0.00 rotate 0.00 0.00 0.00

// Water splash attachments
$attachment "left_foot" "Bip01 L Foot" 5.80 2.00 -2.00 rotate -0 0 0
$attachment "right_foot" "Bip01 R Foot" 5.00 2.00 2.00 rotate 0 -0 -0

// Corpse rope attachments
$attachment "rope_righthand" "Bip01 R Hand" 3.00 0.00 0.00 rotate 0 0 0
$attachment "rope_neck" "Bip01 Spine2" 10.00 -2.00 -0.00 rotate -0 -0 0

// 2 bone controllers
//$controller 0 "Bip01 Head" XR -30 30
//$controller mouth "Bone05" ZR 0 25

// Serves as a hint to the game code that the named bone will be used for
// bone merges during the game. If this flag is not present, the bone can
// still be used but you'll get performance warnings. Weapons have a bone
// with the same name.
$bonemerge "Bip01 R Hand"

// 20 hit boxes
$hbox 3 "Bip01 Pelvis" -4.690000 -6.440000 -6.750000  5.000000 5.560000 6.750000
$hbox 6 "Bip01 L Leg" 2.660000 -3.690000 -3.090000  18.160000 4.880000 3.310000
$hbox 6 "Bip01 L Leg1" 0.380000 -3.970000 -2.840000  17.600000 4.000000 2.940000
$hbox 6 "Bip01 L Foot" -0.590000 -2.340000 -2.630000  3.790000 8.000000 2.190000
$hbox 7 "Bip01 R Leg" 2.470000 -3.690000 -3.160000  18.129999 4.880000 3.380000
$hbox 7 "Bip01 R Leg1" 0.310000 -3.970000 -2.840000  17.600000 3.940000 2.970000
$hbox 7 "Bip01 R Foot" -0.560000 -2.340000 -2.190000  3.810000 8.000000 2.660000
$hbox 3 "Bip01 Spine1" -1.250000 -8.500000 -5.500000  5.830000 5.500000 5.500000
$hbox 2 "Bip01 Spine2" -0.060000 -8.530000 -7.0000  7.000000 7.000000 7.0000
$hbox 2 "Bip01 Spine3" -2.250000 -4.00000 -6.310000  5.500000 5.090000 6.310000
$hbox 1 "Bip01 Neck" -3.110000 -1.500000 -3.000000  2.050000 3.500000 3.000000
$hbox 1 "Bip01 Head" -1.0000 -1.5000 -3.000000  9.0000 7.000000 3.000000
$hbox 4 "Bip01 L Arm" 0.940000 -4.880000 -4.130000  5.440000 2.280000 3.500000
$hbox 4 "Bip01 L Arm1" -2.160000 -4.340000 -2.560000  11.560000 1.410000 2.380000
$hbox 4 "Bip01 L Arm2" 0.590000 -1.810000 -2.190000  12.750000 2.840000 2.410000
$hbox 4 "Bip01 L Hand" 3.000000 -1.000000 -2.000000  6.000000 1.500000 3.500000
$hbox 5 "Bip01 R Arm" 1.380000 -4.840000 -3.560000  5.690000 2.310000 4.310000
$hbox 5 "Bip01 R Arm1" -1.880000 -4.470000 -2.160000  11.810000 1.280000 2.810000
$hbox 5 "Bip01 R Arm2" 0.440000 -1.970000 -2.340000  12.590000 2.720000 2.220000
$hbox 5 "Bip01 R Hand" 3.000000 -1.000000 -2.000000  6.000000 1.500000 3.500000

$bodygroup head
{
studio "ref_Head_NVA_Brown.smd"
studio "ref_Head_NVA_Green.smd"
studio "ref_Head_VC1.smd"
studio "ref_Head_VC2.smd"
studio "ref_Neck.smd"

studio "ref_Head_NVA_Brown_Off.smd"
studio "ref_Head_NVA_Brown_Off.smd"
studio "ref_Head_VC1_Off.smd"
studio "ref_Head_VC2_Off.smd"
}

$bodygroup body
{
studio "ref_body_NVA_Brown.smd"
studio "ref_body_NVA_Green.smd"
studio "ref_body_VC1.smd"
studio "ref_body_VC2.smd"
}

// Sequences

// Corpse Sequences
$sequence lying_on_back "seq_lying_on_back" fps 1 
$sequence lying_on_side "seq_lying_on_side_ragdoll" fps 1 
$sequence lying_on_stomach "seq_lying_on_stomach" fps 1 
$sequence hanging_byfeet "seq_hanging_byfeet" loop fps 10
$sequence hanging_byarms "seq_hanging_byarms" loop fps 10
$sequence hanging_byneck "seq_hanging_byneck" fps 10
$sequence deadsitting "seq_deadsitting_ragdoll" fps 1
$sequence deadseated "seq_deadseated" fps 1

// collision_model.smd and $collisionjoints taken from hl1 barney model
$sequence ragdoll "ragdoll_pose" ACT_DIERAGDOLL 1 fps 30.00
$collisionjoints "collision_model.smd" {

	$mass 115.0
	$inertia 10.00
	$damping 0.01
	$rotdamping 1.50
	$rootbone "bip01"

	$jointconstrain "bip01 spine2" x limit -60.00 60.00 0.20
	$jointconstrain "bip01 spine2" y limit -25.00 25.00 0.20
	$jointconstrain "bip01 spine2" z limit -25.00 30.00 0.20

	$jointconstrain "bip01 r arm1" x limit -40.00 40.00 0.00
	$jointconstrain "bip01 r arm1" y limit -65.00 105.00 0.20
	$jointconstrain "bip01 r arm1" z limit -73.00 12.00 0.20

	$jointconstrain "bip01 r arm2" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 r arm2" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 r arm2" z limit -120.00 4.00 0.20

	$jointconstrain "bip01 l arm1" x limit -40.00 40.00 0.00
	$jointconstrain "bip01 l arm1" y limit -105.00 65.00 0.20
	$jointconstrain "bip01 l arm1" z limit -73.00 12.00 0.20

	$jointconstrain "bip01 l arm2" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 l arm2" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 l arm2" z limit -120.00 4.00 0.20

	$jointconstrain "bip01 l hand" x limit -60.00 60.00 0.20
	$jointconstrain "bip01 l hand" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 l hand" z limit -30.00 20.00 0.20

	$jointconstrain "bip01 r hand" x limit -60.00 60.00 0.20
	$jointconstrain "bip01 r hand" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 r hand" z limit -30.00 20.00 0.20

	$jointconstrain "bip01 r leg" x limit -25.00 20.00 0.20
	$jointconstrain "bip01 r leg" y limit -40.00 20.00 0.20
	$jointconstrain "bip01 r leg" z limit -37.00 83.00 0.20

	$jointconstrain "bip01 r leg1" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 r leg1" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 r leg1" z limit -95.00 4.00 0.20

	$jointrotdamping "bip01 head" 5.00
	$jointconstrain "bip01 head" x limit -20.00 20.00 0.20
	$jointconstrain "bip01 head" y limit -25.00 25.00 0.20
	$jointconstrain "bip01 head" z limit -13.00 30.00 0.20

	$jointconstrain "bip01 l leg" x limit -25.00 20.00 0.20
	$jointconstrain "bip01 l leg" y limit -20.00 40.00 0.20
	$jointconstrain "bip01 l leg" z limit -37.00 83.00 0.20

	$jointconstrain "bip01 l leg1" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 l leg1" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 l leg1" z limit -95.00 4.00 0.20

	$jointconstrain "bip01 l foot" x limit -20.00 20.00 0.00
	$jointconstrain "bip01 l foot" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 l foot" z limit -15.00 35.00 0.20

	$jointconstrain "bip01 r foot" x limit -20.00 20.00 0.00
	$jointconstrain "bip01 r foot" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 r foot" z limit -15.00 35.00 0.20

	// The whole point of this separate model is that the
	// legs don't collide with each other.
	$jointcollide "Bip01 Pelvis" "Bip01 L Arm1"
	$jointcollide "Bip01 Pelvis" "Bip01 L Arm2"
	$jointcollide "Bip01 Pelvis" "Bip01 L Hand"
	$jointcollide "Bip01 Pelvis" "Bip01 R Arm1"
	$jointcollide "Bip01 Pelvis" "Bip01 R Arm2"
	$jointcollide "Bip01 Pelvis" "Bip01 R Hand"

	$jointcollide "Bip01 Spine2" "Bip01 L Arm2"
	$jointcollide "Bip01 Spine2" "Bip01 L Hand"
	$jointcollide "Bip01 Spine2" "Bip01 R Arm2"
	$jointcollide "Bip01 Spine2" "Bip01 R Hand"

	$jointcollide "Bip01 Head" "Bip01 L Arm1"
	$jointcollide "Bip01 Head" "Bip01 L Arm2"
	$jointcollide "Bip01 Head" "Bip01 L Hand"
	$jointcollide "Bip01 Head" "Bip01 R Arm1"
	$jointcollide "Bip01 Head" "Bip01 R Arm2"
	$jointcollide "Bip01 Head" "Bip01 R Hand"
}
