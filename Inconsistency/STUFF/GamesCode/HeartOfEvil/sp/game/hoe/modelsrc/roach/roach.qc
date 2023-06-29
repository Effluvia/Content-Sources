$cdmaterials "models\cockroach\"

// HOE: Added polys to bottom for namf5 headroach
$model "body" "reference.smd"

$hboxset "default"
$hbox 0 "Dummy01" -2.591  -1.186  0.000  2.504  5.386  2.177
$hbox 0 "Dummy15" -2.112  -4.503  0.000  0.157  0.000  3.072
$hbox 0 "Dummy16" -0.119  -4.657  0.000  2.497  0.000  3.135

// Model uses material "top.vmt"
// Model uses material "Untitled-5.vmt"

$surfaceprop "default"

$illumposition 1.775 -0.135 2.192

// HOE: added to avoid warnings
$eyeposition 0 0 1

$sequence idle "idle" loop ACT_IDLE 1 fps 8.00
$sequence run "run" loop ACT_WALK 1 fps 30.00
