$modelname "w_grenade.mdl"

// If you change the scale of this, the grenade won't line up with the player model's hand in thirdperson
//$scale 0.65
$scale 0.5

$model "studio" "grenade.smd"
$cdmaterials "models\world_models\wrld_grenade\"

//$hboxset "default"
//$hbox 0 "Line02" -7.093  -8.612  -2.965  2.229  5.675  5.565

// Model uses material "grenade_side.vmt"
// Model uses material "grenade_skin.vmt"

$surfaceprop "metal"
//$staticprop

$illumposition -0.597 0.342 4.000

$sequence idle "idle" ACT_IDLE 1 fps 30.00

// auto-create a collisionmodel
$collisionmodel "grenade.smd"
{
    $mass 1.0
    $inertia 1.00
    $damping 0.00
    $rotdamping 0.00
}

