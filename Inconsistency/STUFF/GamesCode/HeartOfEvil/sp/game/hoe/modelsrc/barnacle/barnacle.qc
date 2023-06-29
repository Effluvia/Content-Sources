$modelname "barnacle.mdl"
$cdmaterials "models\barnacle\"

$attachment "TongueEnd" "Barnacle.tongue8" 4.41 0.00 0.00 rotate -0.00 0.00 0.00

// fake bonemerge so Barnacle.tongueroot isn't removed
$bonemerge "Barnacle.tongueroot"

$bodygroup body
{
    studio body_ref.smd
}
$bodygroup tongue
{
    studio tongue_ref.smd
}

$surfaceprop "alienflesh"

$eyeposition 0.000 0.000 -30.000

// pulling victim towards mouth
$sequence attack1 attack1.smd loop fps 30 activity ACT_BARNACLE_SLURP 1

$sequence death death.smd fps 15 {
    { event AE_BARNACLE_PUKEGIB 13 }
    { event AE_BARNACLE_PUKEGIB 25 }
    { event AE_BARNACLE_PUKEGIB 33 }
    { event AE_BARNACLE_PUKEGIB 44 }
    activity ACT_DIESIMPLE 1
    snap
}

$sequence chew_human chewcycle.smd loop fps 30 activity ACT_BARNACLE_CHEW_HUMAN 1
$sequence chew_small chewcycle.smd loop fps 30 activity ACT_BARNACLE_CHEW_SMALL_THINGS 1

$animation a_bite death.smd frame 0 30
$sequence bite_player a_bite fps 30 {
    { event AE_BARNACLE_BITE 28 }
    activity ACT_BARNACLE_BITE_PLAYER 1
    snap
}
$sequence bite_human a_bite fps 30 {
    { event AE_BARNACLE_BITE 28 }
    activity ACT_BARNACLE_BITE_HUMAN 1
    snap
}
$sequence bite_small a_bite fps 30 {
    { event AE_BARNACLE_BITE 28 }
    activity ACT_BARNACLE_BITE_SMALL_THINGS 1
    snap
}

$sequence idle1 idle1.smd loop fps 15 activity ACT_IDLE 1
//$sequence idle2 idle2.smd fps 15 activity ACT_IDLE 1
//$sequence idle3 idle3.smd fps 15 activity ACT_IDLE 1

$sequence flinch flinch_sm.smd fps 30 {
    activity ACT_SMALL_FLINCH 1
    snap
}



