//$cd "C:\Programming\heart of evil source\modelsrc\updated\tripmine"
$modelname "tripmine\w_tripmine.mdl"
$scale 0.75
$model "mine" "tripmine_boned_world.smd"
$cdmaterials "models\tripmine\v_tripmine\"
//$hboxset "default"
//$hbox 0 "Weapon_Bone" -11.445  -3.916  -4.224  11.469  7.327  5.435
// Model uses material "side.vmt"
// Model uses material "box.vmt"
// Model uses material "light.vmt"
// Model uses material "face.vmt"
// Model uses material "lensCHROME.vmt"
$surfaceprop "metal"
$illumposition -2.517 -0.012 0.669

$sequence idle "world" loop ACT_IDLE 1 fps 30.00
//$sequence world "world" loop ACT_TRIPMINE_WORLD 1 fps 30.00
//$sequence ground "ground" loop ACT_TRIPMINE_GROUND 1 fps 30.00

$collisionmodel "tripmine_boned_world.smd"
{
    $mass 5.0
}

