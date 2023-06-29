$modelname "Male_sdk.mdl"
$cdmaterials sdk\models\Humans\Male
$model male_06 "male_06_reference.smd"

$sequence idle01 idle01 loop frames 0 358 fixuploop -15 0 activity ACT_IDLE 1 node "standing"
$sequence RunN_smg1 RunN_smg1
$sequence RunN_smg1_Aim RunN_smg1_Aim

$sequence a_RunS RunS loop LX LY startloop 0 rotateto -180 fps 31 {
	alignto idle01
	// ikrule lfoot footstep 0 range .. 9 12 ..
	// ikrule rfoot footstep 1 range .. 0 3 ..
}
$sequence a_RunSE RunSE loop LX LY startloop 0 rotateto -135 fps 29 {
	alignto idle01
	// ikrule lfoot footstep 0 range .. 7 11 ..
	// ikrule rfoot footstep 1 range .. 17 1 ..
}
$sequence a_RunE RunE loop LX LY startloop 0 rotateto -90 fps 31 {
	alignto idle01
	// ikrule lfoot footstep 0 range .. 7 9 ..
	// ikrule rfoot footstep 1 range .. 14 0 ..
}
$sequence a_RunNE RunNE loop LX LY startloop 0 rotateto -45 fps 29 {
	alignto idle01
	// ikrule lfoot footstep 0 range .. 6 10 ..
	// ikrule rfoot footstep 1 range .. 17 19 ..
}
$sequence a_RunN RunN loop LX LY startloop 0 rotateto 0 {
	alignto idle01
	// ikrule lfoot footstep 0 range .. 8 10 ..
	// ikrule rfoot footstep 1 range .. 17 1 ..
}
$sequence a_RunNW RunNW loop LX LY startloop 0 rotateto 45 fps 28 {
	alignto idle01
	// ikrule lfoot footstep 0 range .. 9 11 ..
	// ikrule rfoot footstep 1 range .. 18 1 ..
}
$sequence a_RunW RunW loop LX LY startloop 0 rotateto 90 fps 32 {
	alignto idle01
	// ikrule lfoot footstep 0 range .. 7 9 ..
	// ikrule rfoot footstep 1 range .. 16 0 ..
}
$sequence a_RunSW RunSW loop LX LY startloop 0 rotateto 135 fps 31 {
	alignto idle01
	// ikrule lfoot footstep 0 range .. 8 11 ..
	// ikrule rfoot footstep 1 range .. 16 1 ..
}

$sequence IdleAngryToShoot Idle_Angry_SMG1_to_Aim fps 30 transition "standingSMG" "shooting" ACT_TRANSITION 1

