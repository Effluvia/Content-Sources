//$cd "C:\Programming\heart of evil source\modelsrc\hl1 source\chumtoad"
$modelname "chumtoad.mdl"
$bodygroup studio
{
studio "Chub_Template_Biped1.smd"
}
$bodygroup engine
{
blank
studio "engine.smd"
}
$cdmaterials "models\chubtoad\"
$texturegroup skinfamilies
{
 {
  "CHB_Side1.vmt"
  "CHB_Eye1.vmt"
 }
 {
  "CHB_Side1.vmt"
  "CHB_Eye(Blink2).vmt"
 }
 {
  "CHB_Side1.vmt"
  "CHB_Eye(Blink3).vmt"
 }
}

$attachment "exhaust" "Engine" 4.00 3.00 -5.00 rotate 90 90 0
$attachment "headlight" "Engine" 0.00 1.00 4.00 rotate -90 -90 0

$hboxset "default"
$hbox 0 "Bip01 L Leg" -2.132  -3.213  -0.662  2.970  0.968  1.687
$hbox 0 "Bip01 L Leg1" -2.143  -0.844  -0.400  6.698  1.659  1.209
$hbox 0 "Bip01 L Foot" 0.000  -3.532  -2.631  1.723  2.638  2.688
$hbox 0 "Bip01 R Leg" -2.241  -3.184  -1.672  2.970  0.919  1.166
$hbox 0 "Bip01 R Leg1" -1.956  -0.921  -1.178  6.698  1.655  0.353
$hbox 0 "Bip01 R Foot" 0.000  -3.105  -1.855  1.738  2.638  2.937
$hbox 0 "Bip01 Spine" -2.994  -3.873  -5.321  4.335  4.180  5.374
$hbox 0 "Bip01 Spine1" 0.000  -3.872  -5.934  5.324  3.161  5.986
$hbox 0 "Bip01 Neck" -2.577  -0.000  -3.293  0.532  2.188  3.487
$hbox 0 "Bip01 Head" 0.000  -1.486  -4.110  3.126  0.755  4.163
$hbox 0 "Bip01 L Arm2" -0.706  -1.022  -0.538  2.143  0.970  0.879
$hbox 0 "Bip01 L Hand" -0.223  -0.304  -0.827  1.334  0.878  1.036
$hbox 0 "Bip01 L Finger0" 0.000  -0.217  -1.120  2.385  0.893  0.733
$hbox 0 "Bip01 R Arm2" -0.566  -1.120  -0.984  2.143  0.859  0.480
$hbox 0 "Bip01 R Hand" -0.110  -0.247  -1.210  1.384  0.946  0.620
$hbox 0 "Bip01 R Finger0" -0.112  -0.172  -1.118  2.438  0.944  0.743
$hbox 0 "Bone13" -2.713  -1.056  -1.288  0.000  0.000  1.097

// Model uses material "CHB_Side1.vmt"
// Model uses material "CHB_Eye1.vmt"
// Model uses material "CHB_Tounge1.vmt"
// Model uses material "CHB_Eye(Blink2).vmt"
// Model uses material "CHB_Eye(Blink3).vmt"
$surfaceprop "alienflesh"
$eyeposition 0.000 0.000 8.000
$illumposition 0.837 0.008 6.793

$weightlist exclude_engine Bip01 1.0 Engine 0.0

$sequence idle "idle" loop ACT_IDLE 10 fps 16.00 weightlist exclude_engine node "idle"
$sequence idle2 "idle2" ACT_IDLE 1 fps 16.00 weightlist exclude_engine node "idle"
$sequence idle3 "idle3" ACT_IDLE 1 fps 16.00 weightlist exclude_engine node "idle"
$sequence flinch1 "flinch1" ACT_BIG_FLINCH 1 fps 16.00 weightlist exclude_engine
$sequence flinch2 "flinch2" ACT_SMALL_FLINCH 1 fps 16.00 weightlist exclude_engine
$sequence hop "hop" LX loop ACT_WALK 1 fps 60.00 weightlist exclude_engine
$sequence swim "swim" LX loop ACT_SWIM 1 fps 30.00 weightlist exclude_engine
$sequence die3 "die3" ACT_DIESIMPLE 1 fps 30.00 weightlist exclude_engine
$sequence pdwiggle "pdwiggle" loop ACT_CHUMTOAD_DEAD_LOOP 1 fps 40.00 weightlist exclude_engine
$sequence playdead1 "playdead1" ACT_CHUMTOAD_DEAD_END 1 fps 16.00 weightlist exclude_engine
$sequence playdead2 "playdead2" ACT_CHUMTOAD_DEAD_BEGIN 1 fps 16.00 weightlist exclude_engine

$weightlist engine Engine 1.0
$animation engine_neutral seq_engine_neutral frame 0 0
$sequence engine "seq_engine" {
	subtract engine_neutral 0
	fps 100
	weightlist engine
	delta
	activity ACT_CHUMTOAD_ENGINE 1
	loop
}
$sequence engine_swim "seq_engine_swim" {
	activity ACT_CHUMTOAD_ENGINE_SWIM 1
	loop
	fps 30.00
	weightlist exclude_engine
	node "engine_swim"
}
$sequence engine_idle_to_swim seq_engine_idle_to_swim {
	fps 20.00
	transition "idle" "engine_swim"
}
$sequence engine_swim_to_idle seq_engine_swim_to_idle {
	fps 20.00
	transition "engine_swim" "idle"
}

$sequence ragdoll "ragdoll_pose" FPS 30 activity ACT_DIERAGDOLL 1
$collisionjoints collision_model.smd {

	$mass 1.0
	$inertia 1.00
	$damping 0.00
	$rotdamping 0.00
	$rootbone "bip01 spine"

	$jointconstrain "bip01 r arm1" x limit -40.00 40.00 0.00
	$jointconstrain "bip01 r arm1" y limit -70.00 50.00 0.00
	$jointconstrain "bip01 r arm1" z limit -50.00 50.00 0.00

	$jointconstrain "bip01 r arm2" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 r arm2" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 r arm2" z limit -50.00 0.00 0.00

	$jointconstrain "bip01 r hand" x limit -35.00 35.00 0.00
	$jointconstrain "bip01 r hand" y limit -30.00 30.00 0.00
	$jointconstrain "bip01 r hand" z limit -60.00 30.00 0.00

	$jointconstrain "bip01 l arm1" x limit -40.00 40.00 0.00
	$jointconstrain "bip01 l arm1" y limit -70.00 50.00 0.00
	$jointconstrain "bip01 l arm1" z limit -50.00 50.00 0.00

	$jointconstrain "bip01 l arm2" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 l arm2" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 l arm2" z limit -50.00 0.00 0.00

	$jointconstrain "bip01 l hand" x limit -35.00 35.00 0.00
	$jointconstrain "bip01 l hand" y limit -30.00 30.00 0.00
	$jointconstrain "bip01 l hand" z limit -60.00 30.00 0.00


	$jointconstrain "bip01 r leg1" x limit -30.00 30.00 0.00
	$jointconstrain "bip01 r leg1" y limit -50.00 60.00 0.00
	$jointconstrain "bip01 r leg1" z limit -30.00 40.00 0.00

	$jointconstrain "bip01 r foot" x limit -30.00 30.00 0.00
	$jointconstrain "bip01 r foot" y limit -30.00 30.00 0.00
	$jointconstrain "bip01 r foot" z limit -30.00 30.00 0.00


	$jointconstrain "bip01 l leg1" x limit -30.00 30.00 0.00
	$jointconstrain "bip01 l leg1" y limit -50.00 60.00 0.00
	$jointconstrain "bip01 l leg1" z limit -30.00 40.00 0.00

	$jointconstrain "bip01 l foot" x limit -30.00 30.00 0.00
	$jointconstrain "bip01 l foot" y limit -30.00 30.00 0.00
	$jointconstrain "bip01 l foot" z limit -30.00 30.00 0.00
}
