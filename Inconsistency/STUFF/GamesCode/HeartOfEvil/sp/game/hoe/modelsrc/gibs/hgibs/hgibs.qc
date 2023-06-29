$modelname "gibs\Hgibs.mdl"
$cdmaterials "models\gibs\Hgibs\"
$bodygroup "parts" 
{
studio "HGIB_Skull1.smd"
studio "HGIB_HMeat1.smd"
studio "HGIB_Guts1.smd"
studio "HGIB_Lung1.smd"
studio "HGIB_Legbone1.smd"
studio "HGIB_B_Gib1.smd"
studio "HGIB_B_Bone1.smd"
studio "Flesh1.smd"
studio "Flesh2.smd"
studio "Flesh3.smd"
studio "Flesh4.smd"
}

$hboxset "default"
$hbox 0 "Dummy01" -6.997  -1.477  -2.911  3.110  4.273  1.216
$hbox 0 "Skull1" 0.000  -3.020  -6.114  11.238  8.988  6.116
$hbox 0 "Object13" -6.849  -10.148  -4.819  6.849  10.148  4.819
$hbox 0 "Object11" -3.271  -14.973  -3.240  3.271  14.973  3.240
$hbox 0 "Lung01" -6.691  -13.082  -5.117  7.025  11.970  5.234
$hbox 0 "Leg_Bone01" -5.313  -19.306  -3.122  5.313  19.306  3.122
$hbox 0 "Big_Gib01" -5.740  -13.665  -5.820  5.741  13.664  5.820
$hbox 0 "Spinal01" -8.775  -18.477  -7.482  4.924  12.394  5.187
$hbox 0 "Object03" -6.193  -12.762  -1.677  6.193  12.762  1.677
$hbox 0 "Object12" -7.621  -16.904  -3.333  7.621  16.904  3.334
$hbox 0 "Object07" -3.866  -7.335  -4.180  3.866  7.335  4.180
$hbox 0 "Object16" -4.688  -17.392  -1.442  4.688  17.392  1.442

// Model uses material "Skull1.vmt"
// Model uses material "Side_Bone1.vmt"
// Model uses material "Skin_Side1.vmt"
// Model uses material "Neck_1.vmt"
// Model uses material "HMEAT1_Front1.vmt"
// Model uses material "GUTS1_Top1.vmt"
// Model uses material "Lung1.vmt"
// Model uses material "Rib1.vmt"
// Model uses material "G_legbone1.vmt"
// Model uses material "B_Gib1.vmt"
// Model uses material "B_Bone1.vmt"
// Model uses material "Flesh1.vmt"
// Model uses material "Flesh1(Top).vmt"
// Model uses material "Flesh2.vmt"

$surfaceprop "bloodyflesh"

$illumposition -0.259 0.550 0.140

$sequence idle1 "idle1" fps 30.00

$collisionmodel "phymodel.smd" {

	$concave
	$mass 24.0
	$inertia 1.00
	$damping 0.00
	$rotdamping 0.00
}
