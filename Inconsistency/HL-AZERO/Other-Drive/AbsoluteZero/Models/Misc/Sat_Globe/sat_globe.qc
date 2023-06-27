$modelname ".\MDL\sat_globe.mdl"
$cd ".\SMD"
$cdtexture ".\Textures"

$body "studio" "sat_globe_ref"

$texturegroup skinfamilies 
{
{ "array.bmp"  "offline.bmp"  "topo.bmp" }
{ "activated.bmp"  "online.bmp"  "Copy of topo.bmp" }
}

$sequence "orbit" "orbit" fps 5 loop 

