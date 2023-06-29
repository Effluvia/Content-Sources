// Output .MDL
$modelname "barneyzombiehead.mdl"

// Directory of .SMD

// Directory of .BMP
$cdmaterials "models\barneyzombie\Head"
$cliptotextures

$surfaceprop "bloodyflesh"

$scale 1.0

$hbox 1 "joint1" -5 -5 -5 5 5 5

// bodygroups: use relative path from $cd
$bodygroup head
{
studio "ref_Head.smd"
}

// sequences: all sequences are in $cd
$sequence idle "seq_idle" loop fps 1

