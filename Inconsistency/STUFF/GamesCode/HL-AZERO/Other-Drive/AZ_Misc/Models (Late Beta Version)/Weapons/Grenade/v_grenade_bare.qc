/*
==============================================================================
==============================================================================
*/

$modelname "MDL/v_grenade_bare.mdl"
$cd "SMD/"
$cdtexture "Textures/"
$scale 1.0
$cliptotextures


$bbox 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000
$cbox 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000
$eyeposition 0.000000 0.000000 0.000000

$origin 0 0 64

//reference mesh(es)
$body "body" "grenade"

$body "body" "grenade_hand_bare"



// 6 hit box(es)
$hbox 0 "Bip01 R Arm1" 0.000000 -2.810000 -3.310000 12.350000 3.490000 2.620000
$hbox 0 "Bip01 R Arm2" 0.000000 -2.610000 -3.140000 11.368420 3.210000 2.340000
$hbox 0 "Bip01 R Hand" 0.000000 -0.870000 -1.790000 3.890000 1.030000 1.940000
$hbox 0 "Dummy22" -1.010000 -1.220000 -2.130000 1.620000 1.920000 3.110000
$hbox 0 "Bip01 L Arm2" 0.000000 -2.280000 -2.120000 11.680000 2.740000 2.380000
$hbox 0 "Bip01 L Hand" 0.000000 -0.870000 -1.940000 3.890000 1.030000 1.790000

// 8 animation sequence(s)
$sequence idle "grenade_idle1(sway)" loop fps 30
$sequence fidget "grenade_fidget1(thumb)" loop fps 30
$sequence pinpull "grenade_pinpull" fps 45

$sequence throw1 "grenade_throw1(toss)" fps 18 
$sequence throw2 "grenade_throw2(medium)" fps 20
$sequence throw3 "grenade_throw3(hard)" fps 30 

$sequence holster "grenade_holster" fps 30
$sequence draw "grenade_draw" fps 30

// End of QC script.
