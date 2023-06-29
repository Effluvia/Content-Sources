$modelname "w_squeak.mdl"
$model "studio" "WORLDSQUEAK_Template_Biped1.smd"
$cdmaterials "models\v_squeak\"
$cliptotextures

$hboxset "default"
$hbox 1 "Bip01 Pelvis" -1.516  -2.282  -2.321  0.000  0.000  2.321
$hbox 1 "Bip01 Spine" -0.809  -4.146  -3.793  7.154  3.139  3.606
$hbox 1 "Bip01 Neck" -2.027  -1.925  -2.553  1.311  2.671  2.366
$hbox 1 "Bip01 Head" 0.000  -1.049  -1.842  2.623  1.281  1.655

// Model uses material "SQK_Side1.vmt"
// Model uses material "SQK_Eye1.vmt"

$surfaceprop "alienflesh"

$illumposition -0.090 -1.661 5.095

$sequence idle "idle" ACT_IDLE 1 fps 16.00
$sequence fidget "fidget" ACT_FIDGET 1 fps 16.00
$sequence jump "jump" ACT_JUMP 1 fps 16.00
$sequence run "run" LX loop ACT_RUN 1 fps 45.00

// HOE added $eyeposition and $bbox
$eyeposition 0.000000 0.000000 4.000000
$bbox -4.000000 -4.000000 0.000000 6.000000 4.000000 10.000000

