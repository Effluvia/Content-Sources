// Output .MDL
$modelname "goddess.mdl"

// Directory of .SMD

// Directory of .BMP
$cdmaterials "models\goddess"
$cliptotextures

// Write textures into a ???T.mdl
//$externaltextures

$scale 1.0
$staticprop

// bodygroups: use relative path from $cd
$bodygroup body
{
studio "goddess.smd"
}

// sequences: all sequences are in $cd
$sequence seq-name "idle1" loop fps 30 ACT_IDLE 1

