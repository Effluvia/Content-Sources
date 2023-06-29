$cd "C:\Programming\heart of evil source\modelsrc\hl1 source\barney"
$modelname "barney.mdl"
$model "studio" "BARNEY-X_Template_Biped1.smd"
$model "gun" "BARNEY-X_Template_Biped(Gun-Hlstr)1.smd"
$model "gun" "BARNEY-X_Template_Biped(Gun)1.smd"
$model "gun" "UnknownModelName.smd"
$poseparameter mouth 0.00 90.00
$poseparameter XR -50.00 50.00
$poseparameter head_yaw -90.00 90.00
$cdmaterials "models\barney\"
$hboxset "default"
$hbox 3 "Bip01 Pelvis" -9.890  -5.660  -8.600  2.940  6.520  6.790
$hbox 6 "Bip01 L Leg" 0.000  -5.920  -4.000  19.370  3.780  3.730
$hbox 6 "Bip01 L Leg1" 0.000  -5.690  -3.620  18.830  3.070  2.940
$hbox 6 "Bip01 L Foot" 0.000  -2.990  -4.040  6.420  8.680  1.610
$hbox 7 "Bip01 R Leg" 0.000  -5.930  -3.870  19.270  3.750  3.790
$hbox 7 "Bip01 R Leg1" -0.330  -5.730  -2.980  18.950  2.960  3.190
$hbox 7 "Bip01 R Foot" 0.000  -3.240  -2.320  6.210  8.440  3.280
$hbox 3 "Bip01 Spine" 0.000  -4.890  -6.970  8.370  8.000  6.730
$hbox 2 "Bip01 Spine2" 0.000  -3.460  -8.320  6.110  8.000  7.530
$hbox 2 "Bip01 Spine3" -1.000  -5.990  -8.220  5.960  8.000  8.480
$hbox 4 "Bip01 L Arm" -0.400  -4.180  0.000  7.200  4.750  4.200
$hbox 4 "Bip01 L Arm1" -2.520  -4.570  -3.920  9.860  2.280  3.430
$hbox 4 "Bip01 L Arm2" 0.000  -2.850  -2.080  11.900  2.850  2.750
$hbox 4 "Bip01 L Hand" 0.000  -1.320  -2.290  4.660  2.600  1.650
$hbox 5 "Bip01 R Arm" 0.000  -4.150  -4.190  7.530  4.120  4.240
$hbox 5 "Bip01 R Arm1" -2.690  -4.650  -3.420  9.470  2.280  3.760
$hbox 5 "Bip01 R Arm2" 0.000  -2.730  -2.560  11.000  3.140  2.190
$hbox 5 "Bip01 R Hand" 0.000  -1.320  -2.910  10.380  3.230  4.000
$hbox 1 "Bip01 Head" -0.660  2.250  -4.240  6.000  7.970  3.630
$hbox 10 "Bip01 Head" -0.660  -4.250  -5.240  6.000  2.250  4.630
$hbox 10 "Bip01 Head" 6.000  -4.250  -5.240  11.190  7.970  4.630
// Model uses material "BX_Front_Mid1.vmt"
// Model uses material "BX_Front_Bot1.vmt"
// Model uses material "BX_Back_Mid1.vmt"
// Model uses material "BX_Back_Bot1.vmt"
// Model uses material "BX_Foot_Btm1.vmt"
// Model uses material "BX_Pits1.vmt"
// Model uses material "BX_Front_Top1.vmt"
// Model uses material "BX_Back_Top1.vmt"
// Model uses material "BX_Holster1.vmt"
// Model uses material "BX_F-Side1.vmt"
// Model uses material "BX_Face1.vmt"
// Model uses material "BX_Tounge.vmt"
// Model uses material "BX_Teeth.vmt"
// Model uses material "BX_Hlm-Sde1.vmt"
// Model uses material "BX_Chrome1.vmt"
// Model uses material "BX_Hlm-Btm1.vmt"
// Model uses material "BX_Arm1.vmt"
// Model uses material "BX_Knuckles.vmt"
// Model uses material "BX_Hand1.vmt"
// Model uses material "BX_Pstl.vmt"
// Model uses material "BX_GunTip1.vmt"
// Model uses material "BX_Palm1.vmt"
// Model uses material "BX_Hsd1.vmt"
// Model uses material "BX_Knuc.vmt"
// Model uses material "BX_GTip1.vmt"
// Model uses material "BX_Gxttra1.vmt"
$attachment "0" "Bip01 R Hand" 15.00 3.50 5.00 rotate -0.00 0.00 -0.00
$attachment "eyes" "Bip01 Head" 0.00 -0.00 -0.00 rotate -0.00 81.00 90.00
$attachment "forward" "Bip01 Spine" 0.00 -0.00 0.00 rotate -0.00 81.00 90.00
$surfaceprop "flesh"
$illumposition -4.379 0.898 37.462
$sequence idle1 "idle1" loop ACT_IDLE 50 fps 15.00
$sequence idle2 "idle2" ACT_IDLE 1 fps 15.00
$sequence idle3 "idle3" ACT_IDLE 1 fps 15.00
$sequence idle4 "idle4" ACT_IDLE 10 fps 12.00
$sequence walk "walk" loop ACT_WALK 1 fps 30.00 {
  { event 1004 0 "NPC.StepLeftWalk" }
  { event 1004 15 "NPC.StepRightWalk" }
}

$sequence run "run" loop ACT_RUN 1 fps 25.00 {
  { event 1004 5 "NPC.StepLeftRun" }
  { event 1004 13 "NPC.StepRightRun" }
}

$sequence draw "draw" ACT_ARM 1 fps 20.00 {
  { event 2 7 }
}

$sequence disarm "disarm" ACT_DISARM 1 fps 10.00 {
  { event 4 15 }
}

$sequence reload "reload" ACT_RELOAD 1 fps 15.00
$sequence turnleft "turnleft" ACT_TURN_LEFT 1 fps 15.00
$sequence turnright "turnright" ACT_TURN_RIGHT 1 fps 15.00
$sequence laflinch "laflinch" ACT_FLINCH_LEFTARM 1 fps 16.00
$sequence raflinch "raflinch" ACT_FLINCH_RIGHTARM 1 fps 16.00
$sequence llflinch "llflinch" ACT_FLINCH_LEFTLEG 1 fps 20.00
$sequence rlflinch "rlflinch" ACT_FLINCH_RIGHTLEG 1 fps 20.00
$sequence smlflinch "smlflinch" ACT_SMALL_FLINCH 1 fps 30.00
$sequence cower_stand "cower_stand" fps 14.00
$sequence locked_door "locked_door" loop fps 28.00
$sequence fall_loop "fall_loop" loop fps 28.00
$sequence barn_wave "barn_wave" fps 22.00
$sequence beat_grunt "beat_grunt" fps 14.00
$sequence beat_gruntidle "beat_gruntidle" loop fps 14.00
$sequence flashlight "flashlight" loop fps 14.00 {
  { event 1004 5 "Weapons.BulletHit1" }
}

$sequence shootgun "shootgun" ACT_RANGE_ATTACK1 1 fps 18.00 {
  { event 3 0 }
  { event 5001 0 "21" }
  blendwidth 2
  blend XR -50.000000 50.000000
 shootgun
}
$sequence shootgun2 "shootgun2" ACT_RANGE_ATTACK1 2 fps 25.00 {
  { event 3 0 }
  { event 5001 0 "21" }
  blendwidth 2
  blend XR -50.000000 50.000000
 shootgun2
}
$sequence diesimple "diesimple" ACT_DIE_HEADSHOT 1 fps 15.00 {
  { event 2001 5 }
}

$sequence dieviolent "dieviolent" ACT_DIE_HEADSHOT 1 fps 20.00 {
  { event 2001 10 }
}

$sequence diegutshot "diegutshot" ACT_DIE_GUTSHOT 1 fps 15.00 {
  { event 2001 33 }
}

$sequence dieforward "dieforward" ACT_DIEFORWARD 1 fps 18.00 {
  { event 2001 8 }
}

$sequence diebackward "diebackward" ACT_DIEBACKWARD 1 fps 15.00 {
  { event 2001 19 }
}

$sequence diecrump "diecrump" ACT_DIESIMPLE 2 fps 18.00
$sequence barnaclehit "barnaclehit" ACT_BARNACLE_HIT 1 fps 15.00
$sequence barnaclepull "barnaclepull" loop ACT_BARNACLE_PULL 1 fps 45.00
$sequence barnaclecrunch "barnaclecrunch" ACT_BARNACLE_CHOMP 1 fps 5.00
$sequence barnaclechew "barnaclechew" loop ACT_BARNACLE_CHEW 1 fps 5.00
$sequence lying_on_back "lying_on_back" fps 1.00
$sequence lying_on_side "lying_on_side" fps 1.00
$sequence lying_on_stomach "lying_on_stomach" fps 1.00
$sequence stuffed_in_vent "stuffed_in_vent" fps 1.00
$sequence standing_idle "standing_idle" loop fps 15.00
$sequence cprbarney "cprbarney" loop fps 20.00
$sequence cprbarneyrevive "cprbarneyrevive" fps 20.00
$sequence barney_dragvent "barney_dragvent" fps 15.00 {
  { event 1000 1 }
}

$sequence dying_barney "dying_barney" fps 18.00 {
  { event 1000 1 }
}

$sequence dying_barneyidle "dying_barneyidle" loop fps 12.00
$sequence dying_friend "dying_friend" fps 18.00
$sequence dying_friendidle "dying_friendidle" loop fps 12.00
$sequence c1a3_bidle "c1a3_bidle" fps 3.00
$sequence c1a3_ventb "c1a3_ventb" fps 18.00 {
  { event 1000 1 }
}

$sequence c1a3_emergeidle "c1a3_emergeidle" loop fps 16.00
$sequence c1a3_emerge "c1a3_emerge" fps 16.00 {
  { event 1000 1 }
}

$sequence haulbarney "haulbarney" fps 18.00 {
  { event 1000 1 }
}

$sequence intropush "intropush" fps 16.00 {
  { event 1004 15 "NPC.ButtonBlip1" }
  { event 1004 24 "NPC.ButtonBlip1" }
  { event 1004 34 "NPC.ButtonBlip1" }
  { event 1004 44 "NPC.ButtonBlip1" }
}

$sequence fence "fence" fps 24.00 {
  { event 1000 70 }
  { event 1001 70 }
  { event 1004 72 "NPC.Electro4" }
  { event 1008 100 "NPC_Barney.Pain2" }
}

$sequence sit1 "sit1" loop fps 20.00
$sequence almostidle "almostidle" loop fps 16.00
$sequence almost "almost" fps 26.00 {
  { event 1000 1 }
}

$sequence laseridle "laseridle" fps 6.00
$sequence laser_top "laser_top" fps 6.00 {
  { event 1000 0 }
}

$sequence laser_bottom "laser_bottom" fps 6.00 {
  { event 1000 0 }
}

$sequence barneyfallidle "barneyfallidle" loop fps 33.00
$sequence barneyfall "barneyfall" fps 32.00 {
  { event 1000 1 }
  { event 1008 2 "NPC_Barney.Pain1" }
  { event 1008 24 "NPC_Barney.Pain2" }
  { event 1008 55 "NPC.BustFlesh1" }
}

$sequence c3a2_draw "c3a2_draw" fps 16.00 {
  { event 2 13 }
}

$sequence unlatch "unlatch" fps 12.00
$sequence retina "retina" fps 20.00
$sequence relaxstand "relaxstand" loop fps 10.00
$sequence assassinated "assassinated" fps 18.00 {
  { event 1000 1 }
}

$sequence trackswitch "trackswitch" fps 22.00
$sequence pepsiswing "pepsiswing" fps 25.00
$sequence pepsipush "pepsipush" fps 18.00
$sequence buttonpush "buttonpush" fps 20.00
$animation mdldecompiler_delta.smd "mdldecompiler_delta.smd" fps 30
$animation mouth_close "mouth_close" fps 30.000000 subtract mdldecompiler_delta.smd 0

$animation mouth_open "mouth_open" fps 30.000000 subtract mdldecompiler_delta.smd 0

$sequence barney_mouth "mouth_close" fps 30.00 {
  blendwidth 2
  blend mouth 0.000000 90.000000
  delta
  autoplay
 mouth_open
}
$animation neck_right "neck_right" fps 30.000000 subtract mdldecompiler_delta.smd 0

$animation neck_center "neck_center" fps 30.000000 subtract mdldecompiler_delta.smd 0

$animation neck_left "neck_left" fps 30.000000 subtract mdldecompiler_delta.smd 0

$sequence barney_neckturn "neck_right" fps 30.00 {
  blendwidth 3
  blend head_yaw -90.000000 90.000000
  delta
  autoplay
 neck_center neck_left
}
$sequence ragdoll "ragdoll" ACT_DIERAGDOLL 1 fps 30.00
$collisionjoints "phymodel.smd" {

	$mass 115.0
	$inertia 10.00
	$damping 0.01
	$rotdamping 1.50
	$rootbone "bip01"

	$jointconstrain "bip01 spine2" x limit -60.00 60.00 0.20
	$jointconstrain "bip01 spine2" y limit -25.00 25.00 0.20
	$jointconstrain "bip01 spine2" z limit -25.00 30.00 0.20

	$jointconstrain "bip01 r arm1" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 r arm1" y limit -45.00 64.00 0.20
	$jointconstrain "bip01 r arm1" z limit -73.00 12.00 0.20

	$jointconstrain "bip01 r arm2" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 r arm2" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 r arm2" z limit -120.00 4.00 0.20

	$jointconstrain "bip01 l arm1" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 l arm1" y limit -67.00 30.00 0.20
	$jointconstrain "bip01 l arm1" z limit -73.00 12.00 0.20

	$jointconstrain "bip01 l arm2" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 l arm2" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 l arm2" z limit -120.00 4.00 0.20

	$jointconstrain "bip01 l hand" x limit -60.00 60.00 0.20
	$jointconstrain "bip01 l hand" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 l hand" z limit -57.00 70.00 0.20

	$jointconstrain "bip01 r hand" x limit -60.00 60.00 0.20
	$jointconstrain "bip01 r hand" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 r hand" z limit -57.00 70.00 0.20

	$jointconstrain "bip01 r leg" x limit -25.00 20.00 0.20
	$jointconstrain "bip01 r leg" y limit -40.00 20.00 0.20
	$jointconstrain "bip01 r leg" z limit -37.00 63.00 0.20

	$jointconstrain "bip01 r leg1" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 r leg1" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 r leg1" z limit -95.00 4.00 0.20

	$jointrotdamping "bip01 head" 5.00
	$jointconstrain "bip01 head" x limit -20.00 20.00 0.20
	$jointconstrain "bip01 head" y limit -25.00 25.00 0.20
	$jointconstrain "bip01 head" z limit -13.00 30.00 0.20

	$jointconstrain "bip01 l leg" x limit -25.00 20.00 0.20
	$jointconstrain "bip01 l leg" y limit -20.00 40.00 0.20
	$jointconstrain "bip01 l leg" z limit -37.00 63.00 0.20

	$jointconstrain "bip01 l leg1" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 l leg1" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 l leg1" z limit -95.00 4.00 0.20

	$jointconstrain "bip01 l foot" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 l foot" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 l foot" z limit -15.00 35.00 0.20

	$jointconstrain "bip01 r foot" x limit 0.00 0.00 0.00
	$jointconstrain "bip01 r foot" y limit 0.00 0.00 0.00
	$jointconstrain "bip01 r foot" z limit -15.00 35.00 0.20
}
