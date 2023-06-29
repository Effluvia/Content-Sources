$modelname "can.mdl"
$cdmaterials "models\can"
$cliptotextures

$scale 0.5
$staticprop
$surfaceprop "popcan"

// 0 attachments

// 0 bone controllers

// 1 hit boxes
//$hbox 0 "Box01" -16.950001 -5.760000 -0.430000  10.070000 12.770000 14.180000

$bodygroup studio
{
studio "gensoda1.smd"
}

$texturegroup skinfamilies
{
 {
  "canbutt.vmt"
  "cantop.vmt"
  "gensoda1.vmt"
 }
 {
  "canbutt.vmt"
  "cantop.vmt"
  "gensoda2.vmt"
 }
 {
  "canbutt.vmt"
  "cantop.vmt"
  "gensoda3.vmt"
 }
 {
  "canbutt.vmt"
  "cantop.vmt"
  "gensoda4.vmt"
 }
 {
  "canbutt.vmt"
  "cantop.vmt"
  "gensoda5.vmt"
 }
 {
  "canbutt.vmt"
  "cantop.vmt"
  "gensoda6.vmt"
 }
}

// 1 sequences
$sequence idle "idle" fps 30 

// auto-create a collisionmodel
$collisionmodel "gensoda1.smd"
{
    $mass 1.0
}


