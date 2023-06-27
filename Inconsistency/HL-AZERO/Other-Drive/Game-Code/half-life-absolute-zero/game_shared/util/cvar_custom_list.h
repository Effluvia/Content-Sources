

// NOTE TO SELF:
// for release to compile right, something in the HASH list needs to
// have the CLIENTSENDOFF_BROADCAST constants, orrrrrrr else.
// That is, if something is serverside only, it should not be in the hash
// list at all.  No exceptions.

// something that is _CLIENTONLY can use the _AC_ cvar tags for saving.
// everything else uses _A_. I think that's how it works.

// for something shared, it mainly uses server preferences where required
// and uses _CLIENTSENDOFF_BROADCAST for most areas below. Must also be
// part of the HASH list as said above.


// !!! See cvar_custom_info.h, has the CVar DEFAULT_...'s and hash ID's now.


#define EASY_CVAR_HASH_MASS\
	EASY_CVAR_HASH_CLIENTONLY(strobeDurationMin, 0)\
	EASY_CVAR_HASH_CLIENTONLY(strobeDurationMax, 1)\
	EASY_CVAR_HASH_CLIENTONLY(strobeRadiusMin, 2)\
	EASY_CVAR_HASH_CLIENTONLY(strobeRadiusMax, 3)\
	EASY_CVAR_HASH_CLIENTONLY(strobeSpawnDistHori, 4)\
	EASY_CVAR_HASH_CLIENTONLY(strobeSpawnDistVertMin, 5)\
	EASY_CVAR_HASH_CLIENTONLY(strobeSpawnDistVertMax, 6)\
	EASY_CVAR_HASH_CLIENTONLY(strobeMultiColor, 7)\
	EASY_CVAR_HASH_CLIENTONLY(raveLaserEnabled, 8)\
	EASY_CVAR_HASH_CLIENTONLY(raveLaserSpawnFreq, 9)\
	EASY_CVAR_HASH_CLIENTONLY(raveLaserLength, 10)\
	EASY_CVAR_HASH_CLIENTONLY(raveLaserSpawnDistHoriMin, 11)\
	EASY_CVAR_HASH_CLIENTONLY(raveLaserSpawnDistHoriMax, 12)\
	EASY_CVAR_HASH_CLIENTONLY(raveLaserSpawnDistVertMin, 13)\
	EASY_CVAR_HASH_CLIENTONLY(raveLaserSpawnDistVertMax, 14)\
	EASY_CVAR_HASH_CLIENTONLY(raveLaserBrightnessMin, 15)\
	EASY_CVAR_HASH_CLIENTONLY(raveLaserBrightnessMax, 16)\
	EASY_CVAR_HASH_CLIENTONLY(raveLaserDurationMin, 17)\
	EASY_CVAR_HASH_CLIENTONLY(raveLaserDurationMax, 18)\
	EASY_CVAR_HASH_CLIENTONLY(raveLaserThicknessMin, 19)\
	EASY_CVAR_HASH_CLIENTONLY(raveLaserThicknessMax, 20)\
	EASY_CVAR_HASH_CLIENTONLY(raveLaserNoiseMin, 21)\
	EASY_CVAR_HASH_CLIENTONLY(raveLaserNoiseMax, 22)\
	EASY_CVAR_HASH_CLIENTONLY(raveLaserFrameRateMin, 23)\
	EASY_CVAR_HASH_CLIENTONLY(raveLaserFrameRateMax, 24)\
	EASY_CVAR_HASH_CLIENTONLY(raveLaserMultiColor, 25)\
	EASY_CVAR_HASH_CLIENTONLY(cameraPosFixedX, 26)\
	EASY_CVAR_HASH_CLIENTONLY(cameraPosFixedY, 27)\
	EASY_CVAR_HASH_CLIENTONLY(cameraPosFixedZ, 28)\
	EASY_CVAR_HASH_CLIENTONLY(cameraPosOffX, 29)\
	EASY_CVAR_HASH_CLIENTONLY(cameraPosOffY, 30)\
	EASY_CVAR_HASH_CLIENTONLY(cameraPosOffZ, 31)\
	EASY_CVAR_HASH_CLIENTONLY(cameraRotFixedX, 32)\
	EASY_CVAR_HASH_CLIENTONLY(cameraRotFixedY, 33)\
	EASY_CVAR_HASH_CLIENTONLY(cameraRotFixedZ, 34)\
	EASY_CVAR_HASH_CLIENTONLY(cameraRotOffX, 35)\
	EASY_CVAR_HASH_CLIENTONLY(cameraRotOffY, 36)\
	EASY_CVAR_HASH_CLIENTONLY(cameraRotOffZ, 37)\
	EASY_CVAR_HASH_CLIENTONLY(imAllFuckedUp, 38)\
	EASY_CVAR_HASH_CLIENTONLY(thatWasntGrass, 39)\
	EASY_CVAR_HASH(thatWasntPunch, 40)\
	EASY_CVAR_HASH_CLIENTONLY(fogNear, 41)\
	EASY_CVAR_HASH_CLIENTONLY(fogFar, 42)\
	EASY_CVAR_HASH_CLIENTONLY(myCameraSucks, 43)\
	EASY_CVAR_HASH(muteBulletHitSounds, 44)\
	EASY_CVAR_HASH(geigerChannel, 45)\
	EASY_CVAR_HASH_CLIENTONLY(muteTempEntityGroundHitSound, 46)\
	EASY_CVAR_HASH(muteRicochetSound, 47)\
	EASY_CVAR_HASH_CLIENTONLY(mutePlayerWeaponFire, 48)\
	EASY_CVAR_HASH(muteCrowbarSounds, 49)\
	EASY_CVAR_HASH(seeMonsterHealth, 50)\
	EASY_CVAR_HASH_CLIENTONLY(event5011Allowed, 51)\
	EASY_CVAR_HASH_CLIENTONLY(event5021Allowed, 52)\
	EASY_CVAR_HASH_CLIENTONLY(event5031Allowed, 53)\
	EASY_CVAR_HASH_CLIENTONLY(event5002Allowed, 54)\
	EASY_CVAR_HASH_CLIENTONLY(event5004Allowed, 55)\
	EASY_CVAR_HASH_CLIENTONLY(eventsAreFabulous, 56)\
	EASY_CVAR_HASH(glockOldReloadLogic, 57)\
	EASY_CVAR_HASH(testVar, 58)\
	EASY_CVAR_HASH_CLIENTONLY(painArrowColorMode, 59)\
	EASY_CVAR_HASH_CLIENTONLY(painFlashColorMode, 60)\
	EASY_CVAR_HASH_CLIENTONLY(painFlashSuitless, 61)\
	EASY_CVAR_HASH_CLIENTONLY(painFlashDrownMode, 62)\
	EASY_CVAR_HASH(firstPersonIdleDelayMin, 63)\
	EASY_CVAR_HASH(firstPersonIdleDelayMax, 64)\
	EASY_CVAR_HASH_CLIENTONLY(forceDrawBatteryNumber, 65)\
	EASY_CVAR_HASH_CLIENTONLY(canShowWeaponSelectAtDeath, 66)\
	DUMMY\
	EASY_CVAR_HASH_CLIENTONLY(preE3UsesFailColors, 68)\
	EASY_CVAR_HASH_CLIENTONLY(E3UsesFailColors, 69)\
	EASY_CVAR_HASH_CLIENTONLY(preE3ShowsDamageIcons, 70)\
	EASY_CVAR_HASH_CLIENTONLY(E3ShowsDamageIcons, 71)\
	EASY_CVAR_HASH(playerCrossbowMode, 72)\
	EASY_CVAR_HASH(tripmineAnimWaitsForFinish, 73)\
	EASY_CVAR_HASH(revolverLaserScope, 74)\
	DUMMY\
	DUMMY\
	DUMMY\
	DUMMY\
	DUMMY\
	EASY_CVAR_HASH(cheat_infiniteclip, 80)\
	EASY_CVAR_HASH(cheat_infiniteammo, 81)\
	EASY_CVAR_HASH(cheat_minimumfiredelay, 82)\
	EASY_CVAR_HASH(cheat_minimumfiredelaycustom, 83)\
	EASY_CVAR_HASH(cheat_nogaussrecoil, 84)\
	EASY_CVAR_HASH(gaussRecoilSendsUpInSP, 85)\
	EASY_CVAR_HASH(raveEffectSpawnInterval, 86)\
	EASY_CVAR_HASH_CLIENTONLY(fogTest, 87)\
	DUMMY\
	EASY_CVAR_HASH_CLIENTONLY(allowAlphaCrosshairWithoutGuns, 89)\
	EASY_CVAR_HASH_CLIENTONLY(alphaCrosshairBlockedOnFrozen, 90)\
	EASY_CVAR_HASH(mirrorsReflectOnlyNPCs, 91)\
	EASY_CVAR_HASH(mirrorsDoNotReflectPlayer, 92)\
	EASY_CVAR_HASH(sv_germancensorship, 93)\
	DUMMY\
	EASY_CVAR_HASH(egonEffectsMode, 94)\
	EASY_CVAR_HASH(egonHitCloud, 95)\
	DUMMY\
	EASY_CVAR_HASH(handGrenadePickupYieldsOne, 97)\
	DUMMY\
	DUMMY\
	EASY_CVAR_HASH(timedDamageDeathRemoveMode, 100)\
	DUMMY\
	EASY_CVAR_HASH(glockUseLastBulletAnim, 102)\
	EASY_CVAR_HASH(playerBarnacleVictimViewOffset, 103)\
	EASY_CVAR_HASH(trailTypeTest, 104)\
	EASY_CVAR_HASH(hornetTrail, 105)\
	EASY_CVAR_HASH(hornetTrailSolidColor, 106)\
	EASY_CVAR_HASH(hornetDeathModEasy, 107)\
	EASY_CVAR_HASH(hornetDeathModMedium, 108)\
	EASY_CVAR_HASH(hornetDeathModHard, 109)\
	EASY_CVAR_HASH(hornetZoomPuff, 110)\
	EASY_CVAR_HASH(hornetSpiral, 111)\
	EASY_CVAR_HASH(hornetSpeedMulti, 112)\
	EASY_CVAR_HASH(hornetSpeedDartMulti, 113)\
	EASY_CVAR_HASH(rocketTrailAlphaInterval, 114)\
	EASY_CVAR_HASH(rocketTrailAlphaScale, 115)\
	EASY_CVAR_HASH(rocketSkipIgnite, 116)\
	EASY_CVAR_HASH(gauss_mode, gauss_mode_ID)\
	EASY_CVAR_HASH_CLIENTONLY(painFlashDmgMin, 134)\
	EASY_CVAR_HASH_CLIENTONLY(painFlashDmgExMult, 135)\
	EASY_CVAR_HASH_CLIENTONLY(painFlashCumulativeMinDrowning, 136)\
	EASY_CVAR_HASH_CLIENTONLY(painFlashCumulativeMax, 137)\
	EASY_CVAR_HASH_CLIENTONLY(painFlashDrawOpacityMax, 138)\
	EASY_CVAR_HASH_CLIENTONLY(painArrowDrawOpacityMin, 139)\
	EASY_CVAR_HASH_CLIENTONLY(painArrowDrawOpacityMax, 140)\
	EASY_CVAR_HASH_CLIENTONLY(painFlashFadeMult, 141)\
	EASY_CVAR_HASH_CLIENTONLY(painArrowFadeMult, 142)\
	EASY_CVAR_HASH_CLIENTONLY(painFlashArmorBlock, 143)\
	EASY_CVAR_HASH_CLIENTONLY(painFlashDirTolerance, 144)\
	EASY_CVAR_HASH_CLIENTONLY(painArrowCumulativeAppearMin, 145)\
	EASY_CVAR_HASH_CLIENTONLY(painArrowCumulativeDmgJump, 146)\
	EASY_CVAR_HASH_CLIENTONLY(painFlashPrintouts, 147)\
	EASY_CVAR_HASH_CLIENTONLY(itemFlashCumulativeJump, 148)\
	EASY_CVAR_HASH_CLIENTONLY(itemFlashDrawOpacityMin, 149)\
	EASY_CVAR_HASH_CLIENTONLY(itemFlashDrawOpacityMax, 150)\
	EASY_CVAR_HASH_CLIENTONLY(itemFlashFadeMult, 151)\
	EASY_CVAR_HASH(crossbowReloadSoundDelay, 152)\
	EASY_CVAR_HASH_CLIENTONLY(crossbowFirePlaysReloadSound, 153)\
	EASY_CVAR_HASH_CLIENTONLY(iHaveAscended, 154)\
	EASY_CVAR_HASH_CLIENTONLY(drawViewModel, 155)\
	EASY_CVAR_HASH_CLIENTONLY(drawHUD, 156)\
	EASY_CVAR_HASH(playerBulletHitEffectForceServer, 157)\
	EASY_CVAR_HASH(forceAllowServersideTextureSounds, 158)\
	EASY_CVAR_HASH(playerWeaponSpreadMode, 159)\
	EASY_CVAR_HASH(viewModelPrintouts, 160)\
	EASY_CVAR_HASH(viewModelSyncFixPrintouts, 161)\
	EASY_CVAR_HASH(textureHitSoundPrintouts, 162)\
	DUMMY\
	EASY_CVAR_HASH(playerWeaponTracerMode, 164)\
	EASY_CVAR_HASH(decalTracerExclusivity, 165)\
	EASY_CVAR_HASH_CLIENTONLY(healthcolor_fullRedMin, 166)\
	EASY_CVAR_HASH_CLIENTONLY(healthcolor_brightness, 167)\
	EASY_CVAR_HASH_CLIENTONLY(healthcolor_yellowMark, 168)\
	EASY_CVAR_HASH_CLIENTONLY(cl_drawExtraZeros, 169)\
	EASY_CVAR_HASH(hideDamage, 170)\
	EASY_CVAR_HASH_CLIENTONLY(timedDamage_brightnessMax, 171)\
	EASY_CVAR_HASH_CLIENTONLY(timedDamage_brightnessMin, 172)\
	EASY_CVAR_HASH_CLIENTONLY(timedDamage_brightnessCap, 173)\
	EASY_CVAR_HASH_CLIENTONLY(timedDamage_brightnessFloor, 174)\
	EASY_CVAR_HASH_CLIENTONLY(timedDamage_flashSpeed, 175)\
	EASY_CVAR_HASH_CLIENTONLY(timedDamage_debug, 176)\
	EASY_CVAR_HASH(myRocketsAreBarney, 177)\
	EASY_CVAR_HASH(wpn_glocksilencer, wpn_glocksilencer_ID)\
	EASY_CVAR_HASH(sv_rpg_clipless, sv_rpg_clipless_ID)\
	EASY_CVAR_HASH(egonRadiusDamageMode, egonRadiusDamageMode_ID)\
	EASY_CVAR_HASH(egonFireRateMode, egonFireRateMode_ID)\
	DUMMY



#define EASY_CVAR_HIDDEN_LIST\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(gruntsCanHaveMP5Grenade, gruntscanhavemp5grenade)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(strobeDurationMin, strobedurationmin, 0)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(strobeDurationMax, strobedurationmax, 1)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(strobeRadiusMin, stroberadiusmin, 2)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(strobeRadiusMax, stroberadiusmax, 3)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(strobeSpawnDistHori, strobespawndisthori, 4)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(strobeSpawnDistVertMin, strobespawndistvertmin, 5)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(strobeSpawnDistVertMax, strobespawndistvertmax, 6)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(strobeMultiColor, strobemulticolor, 7)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(raveLaserEnabled, ravelaserenabled, 8)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(raveLaserSpawnFreq, ravelaserspawnfreq, 9)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(raveLaserLength, ravelaserlength, 10)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMin, ravelaserspawndisthorimin, 11)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMax, ravelaserspawndisthorimax, 12)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMin, ravelaserspawndistvertmin, 13)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMax, ravelaserspawndistvertmax, 14)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(raveLaserBrightnessMin, ravelaserbrightnessmin, 15)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(raveLaserBrightnessMax, ravelaserbrightnessmax, 16)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(raveLaserDurationMin, ravelaserdurationmin, 17)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(raveLaserDurationMax, ravelaserdurationmax, 18)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(raveLaserThicknessMin, ravelaserthicknessmin, 19)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(raveLaserThicknessMax, ravelaserthicknessmax, 20)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(raveLaserNoiseMin, ravelasernoisemin, 21)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(raveLaserNoiseMax, ravelasernoisemax, 22)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(raveLaserFrameRateMin, ravelaserframeratemin, 23)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(raveLaserFrameRateMax, ravelaserframeratemax, 24)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(raveLaserMultiColor, ravelasermulticolor, 25)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(cameraPosFixedX, cameraposfixedx, 26)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(cameraPosFixedY, cameraposfixedy, 27)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(cameraPosFixedZ, cameraposfixedz, 28)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(cameraPosOffX, cameraposoffx, 29)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(cameraPosOffY, cameraposoffy, 30)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(cameraPosOffZ, cameraposoffz, 31)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(cameraRotFixedX, camerarotfixedx, 32)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(cameraRotFixedY, camerarotfixedy, 33)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(cameraRotFixedZ, camerarotfixedz, 34)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(cameraRotOffX, camerarotoffx, 35)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(cameraRotOffY, camerarotoffy, 36)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(cameraRotOffZ, camerarotoffz, 37)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(imAllFuckedUp, imallfuckedup, 38)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(thatWasntGrass, thatwasntgrass, 39)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch, thatwasntpunch, 40)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(fogNear, fognear, 41)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(fogFar, fogfar, 42)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(NPCsTalkMore, npcstalkmore)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(myCameraSucks, mycamerasucks, 43)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(shutupstuka, shutupstuka)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hassaultSpinMovement, hassaultspinmovement)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hassaultIdleSpinSound, hassaultidlespinsound)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hassaultFireSound, hassaultfiresound)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteBulletHitSounds, mutebullethitsounds, 44)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(mutePlayerPainSounds, muteplayerpainsounds)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hassaultIdleSpinSoundChannel, hassaultidlespinsoundchannel)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hassaultSpinUpDownSoundChannel, hassaultspinupdownsoundchannel)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hassaultFireSoundChannel, hassaultfiresoundchannel)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(geigerChannel, geigerchannel, 45)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hassaultWaitTime, hassaultwaittime)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hassaultSpinupRemainTime, hassaultspinupremaintime)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hassaultResidualAttackTime, hassaultresidualattacktime)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hassaultSpinupStartTime, hassaultspinupstarttime)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hassaultVoicePitchMin, hassaultvoicepitchmin)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hassaultVoicePitchMax, hassaultvoicepitchmax)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hassaultFireSpread, hassaultfirespread)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hassaultAllowGrenades, hassaultallowgrenades)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(muteTempEntityGroundHitSound, mutetempentitygroundhitsound, 46)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(houndeyeAttackMode, houndeyeattackmode)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteRicochetSound, mutericochetsound, 47)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(mutePlayerWeaponFire, muteplayerweaponfire, 48)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteCrowbarSounds, mutecrowbarsounds, 49)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(scientistHealNPC, scientisthealnpc)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(scientistHealNPCDebug, scientisthealnpcdebug)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(bulletholeAlertRange, bulletholealertrange)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(fleshhitmakessound, fleshhitmakessound)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(nothingHurts, nothinghurts)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(seeMonsterHealth, seemonsterhealth, 50)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(scientistHealNPCFract, scientisthealnpcfract)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(bullsquidRangeDisabled, bullsquidrangedisabled)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(applyLKPPathFixToAll, applylkppathfixtoall)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(timedDamageAffectsMonsters, timeddamageaffectsmonsters)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(scientistHealCooldown, scientisthealcooldown)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(crazyMonsterPrintouts, crazymonsterprintouts)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(movementIsCompletePrintout, movementiscompleteprintout)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(bulletHoleAlertPrintout, bulletholealertprintout)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(bulletholeAlertStukaOnly, bulletholealertstukaonly)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(barneyPrintouts, barneyprintouts)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(monsterSpawnPrintout, monsterspawnprintout)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(zombieBulletResistance, zombiebulletresistance)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(zombieBulletPushback, zombiebulletpushback)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(houndeyePrintout, houndeyeprintout)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(quakeExplosionSound, quakeexplosionsound)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(explosionDebrisSoundVolume, explosiondebrissoundvolume)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(noFlinchOnHard, noflinchonhard)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hassaultDrawLKP, hassaultdrawlkp)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(meleeDrawBloodModeA, meleedrawbloodmodea)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(meleeDrawBloodModeB, meleedrawbloodmodeb)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(drawDebugBloodTrace, drawdebugbloodtrace)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(meleeDrawBloodModeBFix, meleedrawbloodmodebfix)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(meleeDrawBloodModeAOffset, meleedrawbloodmodeaoffset)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(meleeDrawBloodModeBOffset, meleedrawbloodmodeboffset)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(panthereyeHasCloakingAbility, panthereyehascloakingability)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hgruntSpeedMulti, hgruntspeedmulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hgruntForceStrafeFireAnim, hgruntforcestrafefireanim)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hgruntLockRunAndGunTime, hgruntlockrunandguntime)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hgruntHeadshotGore, hgruntheadshotgore)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hgruntAllowStrafeFire, hgruntallowstrafefire)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(thoroughHitBoxUpdates, thoroughhitboxupdates)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hgruntTinyClip, hgrunttinyclip)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hgruntStrafeAlwaysHasAmmo, hgruntstrafealwayshasammo)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hgruntBrassEjectForwardOffset, hgruntbrassejectforwardoffset)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(agrunt_muzzleflash, agrunt_muzzleflash)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(event5011Allowed, event5011allowed, 51)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(event5021Allowed, event5021allowed, 52)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(event5031Allowed, event5031allowed, 53)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(event5002Allowed, event5002allowed, 54)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(event5004Allowed, event5004allowed, 55)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(eventsAreFabulous, eventsarefabulous, 56)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(glockOldReloadLogic, glockoldreloadlogic, 57)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(glockOldReloadLogicBarney, glockoldreloadlogicbarney)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(barneyDroppedGlockAmmoCap, barneydroppedglockammocap)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(drawCollisionBoundsAtDeath, drawcollisionboundsatdeath)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(drawHitBoundsAtDeath, drawhitboundsatdeath)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(islaveReviveFriendMode, islaverevivefriendmode)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(islaveReviveFriendChance, islaverevivefriendchance)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(islaveReviveFriendRange, islaverevivefriendrange)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(islaveReviveSelfMinDelay, islavereviveselfmindelay)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(islaveReviveSelfMaxDelay, islavereviveselfmaxdelay)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(islaveReviveSelfChance, islavereviveselfchance)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hgruntRunAndGunDistance, hgruntrunandgundistance)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hgruntPrintout, hgruntprintout)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar, testvar, 58)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(painArrowColorMode, painarrowcolormode, 59)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(painFlashColorMode, painflashcolormode, 60)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(painFlashSuitless, painflashsuitless, 61)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(painFlashDrownMode, painflashdrownmode, 62)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(firstPersonIdleDelayMin, firstpersonidledelaymin, 63)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(firstPersonIdleDelayMax, firstpersonidledelaymax, 64)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(forceDrawBatteryNumber, forcedrawbatterynumber, 65)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(offsetgivedistance, offsetgivedistance)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(offsetgivelookvertical, offsetgivelookvertical)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(canShowWeaponSelectAtDeath, canshowweaponselectatdeath, 66)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(endlessFlashlightBattery, endlessflashlightbattery)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(preE3UsesFailColors, pree3usesfailcolors, 68)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(E3UsesFailColors, e3usesfailcolors, 69)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(preE3ShowsDamageIcons, pree3showsdamageicons, 70)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(E3ShowsDamageIcons, e3showsdamageicons, 71)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerCrossbowMode, playercrossbowmode, 72)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hassassinCrossbowMode, hassassincrossbowmode)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(tripmineAnimWaitsForFinish, tripmineanimwaitsforfinish, 73)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(revolverLaserScope, revolverlaserscope, 74)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip, cheat_infiniteclip, 80)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo, cheat_infiniteammo, 81)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay, cheat_minimumfiredelay, 82)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom, cheat_minimumfiredelaycustom, 83)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_nogaussrecoil, cheat_nogaussrecoil, 84)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(autoSneaky, autosneaky)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(cheat_touchNeverExplodes, cheat_touchneverexplodes)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(gaussRecoilSendsUpInSP, gaussrecoilsendsupinsp, 85)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(drawDebugPathfinding, drawdebugpathfinding)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(STUcheckDistH, stucheckdisth)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(STUcheckDistV, stucheckdistv)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(STUcheckDistD, stucheckdistd)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(STUextraTriangH, stuextratriangh)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(STUextraTriangV, stuextratriangv)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(STUrepelMulti, sturepelmulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(STUSpeedMulti, stuspeedmulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(STUExplodeTest, stuexplodetest)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(STUYawSpeedMulti, stuyawspeedmulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(STUDetection, studetection)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(stukaAdvancedCombat, stukaadvancedcombat)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(drawDebugPathfinding2, drawdebugpathfinding2)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hassaultFriendlyFire, hassaultfriendlyfire)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(myStrobe, mystrobe)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(peopleStrobe, peoplestrobe)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(forceWorldLightOff, forceworldlightoff)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(wildHeads, wildheads)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(raveEffectSpawnInterval, raveeffectspawninterval, 86)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(drawBarnacleDebug, drawbarnacledebug)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(fogTest, fogtest, 87)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(sparksAllMulti, sparksallmulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(sparksEnvMulti, sparksenvmulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(sparksButtonMulti, sparksbuttonmulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(sparksPlayerCrossbowMulti, sparksplayercrossbowmulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(sparksComputerHitMulti, sparkscomputerhitmulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(sparksTurretDeathMulti, sparksturretdeathmulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(sparksOspreyHitMulti, sparksospreyhitmulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(sparksExplosionMulti, sparksexplosionmulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(sparksBeamMulti, sparksbeammulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(sparksAIFailMulti, sparksaifailmulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(normalSpeedMulti, normalspeedmulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(noclipSpeedMulti, noclipspeedmulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(jumpForceMulti, jumpforcemulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(apacheForceCinBounds, apacheforcecinbounds)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(apacheBottomBoundAdj, apachebottomboundadj)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(apacheInfluence, apacheinfluence)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(allowAlphaCrosshairWithoutGuns, allowalphacrosshairwithoutguns, 89)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(alphaCrosshairBlockedOnFrozen, alphacrosshairblockedonfrozen, 90)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hgruntRunAndGunDotMin, hgruntrunandgundotmin)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(panthereyeJumpDotTol, panthereyejumpdottol)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(panthereyePrintout, panthereyeprintout)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(gargantuaPrintout, gargantuaprintout)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(squadmonsterPrintout, squadmonsterprintout)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hassaultPrintout, hassaultprintout)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(barnaclePrintout, barnacleprintout)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(friendlyPrintout, friendlyprintout)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(stukaPrintout, stukaprintout)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(timedDamageEndlessOnHard, timeddamageendlessonhard)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(mirrorsReflectOnlyNPCs, mirrorsreflectonlynpcs, 91)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(mirrorsDoNotReflectPlayer, mirrorsdonotreflectplayer, 92)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(altSquadRulesRuntime, altsquadrulesruntime)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hgruntLockStrafeTime, hgruntlockstrafetime)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(pathfindIgnoreIsolatedNodes, pathfindignoreisolatednodes)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(drawNodeAll, drawnodeall)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(drawNodeSpecial, drawnodespecial)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(drawNodeConnections, drawnodeconnections)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(drawNodeAlternateTime, drawnodealternatetime)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(nodeSearchStartVerticalOffset, nodesearchstartverticaloffset)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(blockChangeLevelTrigger, blockchangeleveltrigger)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(blockMusicTrigger, blockmusictrigger)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(blockMultiTrigger, blockmultitrigger)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(blockTeleportTrigger, blockteleporttrigger)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(blockHurtTrigger, blockhurttrigger)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(blockAutosaveTrigger, blockautosavetrigger)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hideNodeGraphRebuildNotice, hidenodegraphrebuildnotice)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(barnacleTongueRetractDelay, barnacletongueretractdelay)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(allowGermanModels, allowgermanmodels)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(germanRobotGibs, germanrobotgibs)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(germanRobotBleedsOil, germanrobotbleedsoil)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(germanRobotDamageDecal, germanrobotdamagedecal)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(germanRobotGibsDecal, germanrobotgibsdecal)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonEffectsMode, egoneffectsmode, 94)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonHitCloud, egonhitcloud, 95)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(handGrenadePickupYieldsOne, handgrenadepickupyieldsone, 97)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(handGrenadesUseOldBounceSound, handgrenadesuseoldbouncesound)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(timedDamageDeathRemoveMode, timeddamagedeathremovemode, 100)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(barnacleCanGib, barnaclecangib)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(sentryCanGib, sentrycangib)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(miniturretCanGib, miniturretcangib)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(turretCanGib, turretcangib)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(turretBleedsOil, turretbleedsoil)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(turretDamageDecal, turretdamagedecal)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(turretGibDecal, turretgibdecal)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(canDropInSinglePlayer, candropinsingleplayer)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(timedDamageIgnoresArmor, timeddamageignoresarmor)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(itemBatteryPrerequisite, itembatteryprerequisite)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(playerExtraPainSoundsMode, playerextrapainsoundsmode)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(timedDamageDisableViewPunch, timeddamagedisableviewpunch)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(batteryDrainsAtDeath, batterydrainsatdeath)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(batteryDrainsAtAdrenalineMode, batterydrainsatadrenalinemode)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(canTakeLongJump, cantakelongjump)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(printOutCommonTimables, printoutcommontimables)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(playerBrightLight, playerbrightlight)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(disablePainPunchAutomatic, disablepainpunchautomatic)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(gargantuaCorpseDeath, gargantuacorpsedeath)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(gargantuaFallSound, gargantuafallsound)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(gargantuaBleeds, gargantuableeds)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(shrapMode, shrapmode)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(shrapRand, shraprand)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(shrapRandHeightExtra, shraprandheightextra)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(explosionShrapnelMulti, explosionshrapnelmulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(useAlphaSparks, usealphasparks)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(emergencyFix, emergencyfix)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(timedDamageReviveRemoveMode, timeddamagereviveremovemode)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(ospreyIgnoresGruntCount, ospreyignoresgruntcount)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(mp5GrenadeInheritsPlayerVelocity, mp5grenadeinheritsplayervelocity)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(crossbowInheritsPlayerVelocity, crossbowinheritsplayervelocity)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(fastHornetsInheritsPlayerVelocity, fasthornetsinheritsplayervelocity)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(snarkInheritsPlayerVelocity, snarkinheritsplayervelocity)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(chumtoadInheritsPlayerVelocity, chumtoadinheritsplayervelocity)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(weaponPickupPlaysAnyReloadSounds, weaponpickupplaysanyreloadsounds)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(glockUseLastBulletAnim, glockuselastbulletanim, 102)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerBarnacleVictimViewOffset, playerbarnaclevictimviewoffset, 103)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hgruntMovementDeltaCheck, hgruntmovementdeltacheck)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hassaultExtraMuzzleFlashRadius, hassaultextramuzzleflashradius)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hassaultExtraMuzzleFlashBrightness, hassaultextramuzzleflashbrightness)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hassaultExtraMuzzleFlashForward, hassaultextramuzzleflashforward)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(leaderlessSquadAllowed, leaderlesssquadallowed)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(nodeConnectionBreakableCheck, nodeconnectionbreakablecheck)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(playerReviveInvincibilityTime, playerreviveinvincibilitytime)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(playerReviveBuddhaMode, playerrevivebuddhamode)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(playerReviveTimeBlocksTimedDamage, playerrevivetimeblockstimeddamage)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hassaultBulletDamageMulti, hassaultbulletdamagemulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hassaultBulletsPerShot, hassaultbulletspershot)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hassaultFireAnimSpeedMulti, hassaultfireanimspeedmulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hassaultMeleeAnimSpeedMulti, hassaultmeleeanimspeedmulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hassassinCrossbowReloadSoundDelay, hassassincrossbowreloadsounddelay)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hgruntStrafeAnimSpeedMulti, hgruntstrafeanimspeedmulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hgruntRunAndGunAnimSpeedMulti, hgruntrunandgunanimspeedmulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(cheat_iwantguts, cheat_iwantguts)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(nodeDetailPrintout, nodedetailprintout)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(soundAttenuationStuka, soundattenuationstuka)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(soundVolumeStuka, soundvolumestuka)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(cineChangelevelFix, cinechangelevelfix)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(drawDebugCine, drawdebugcine)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(cineAllowSequenceOverwrite, cineallowsequenceoverwrite)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(stukaInflictsBleeding, stukainflictsbleeding)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(animationKilledBoundsRemoval, animationkilledboundsremoval)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(gargantuaKilledBoundsAssist, gargantuakilledboundsassist)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(bullsquidSpitTrajTimeMin, bullsquidspittrajtimemin)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(bullsquidSpitTrajTimeMax, bullsquidspittrajtimemax)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(bullsquidSpitTrajDistMin, bullsquidspittrajdistmin)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(bullsquidSpitTrajDistMax, bullsquidspittrajdistmax)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(bullsquidSpitGravityMulti, bullsquidspitgravitymulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(bullsquidSpitUseAlphaModel, bullsquidspitusealphamodel)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(bullsquidSpitUseAlphaEffect, bullsquidspitusealphaeffect)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(bullsquidSpitEffectSpread, bullsquidspiteffectspread)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(bullsquidSpitEffectMin, bullsquidspiteffectmin)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(bullsquidSpitEffectMax, bullsquidspiteffectmax)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(bullsquidSpitEffectHitMin, bullsquidspiteffecthitmin)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(bullsquidSpitEffectHitMax, bullsquidspiteffecthitmax)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(bullsquidSpitEffectSpawn, bullsquidspiteffectspawn)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(bullsquidSpitEffectHitSpawn, bullsquidspiteffecthitspawn)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(bullsquidSpitSpriteScale, bullsquidspitspritescale)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(bullsquidSpitAlphaScale, bullsquidspitalphascale)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(scientistBravery, scientistbravery)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(barneyUnholsterTime, barneyunholstertime)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(barneyUnholsterAnimChoice, barneyunholsteranimchoice)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(trailTypeTest, trailtypetest, 104)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetTrail, hornettrail, 105)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetTrailSolidColor, hornettrailsolidcolor, 106)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetDeathModEasy, hornetdeathmodeasy, 107)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetDeathModMedium, hornetdeathmodmedium, 108)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetDeathModHard, hornetdeathmodhard, 109)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetZoomPuff, hornetzoompuff, 110)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetSpiral, hornetspiral, 111)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetSpeedMulti, hornetspeedmulti, 112)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetSpeedDartMulti, hornetspeeddartmulti, 113)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketTrailAlphaInterval, rockettrailalphainterval, 114)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketTrailAlphaScale, rockettrailalphascale, 115)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketSkipIgnite, rocketskipignite, 116)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(agruntHornetRandomness, agrunthornetrandomness)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hornetSpiralPeriod, hornetspiralperiod)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hornetSpiralAmplitude, hornetspiralamplitude)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(painFlashDmgMin, painflashdmgmin, 134)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(painFlashDmgExMult, painflashdmgexmult, 135)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(painFlashCumulativeMinDrowning, painflashcumulativemindrowning, 136)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(painFlashCumulativeMax, painflashcumulativemax, 137)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(painFlashDrawOpacityMax, painflashdrawopacitymax, 138)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(painArrowDrawOpacityMin, painarrowdrawopacitymin, 139)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(painArrowDrawOpacityMax, painarrowdrawopacitymax, 140)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(painFlashFadeMult, painflashfademult, 141)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(painArrowFadeMult, painarrowfademult, 142)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(painFlashArmorBlock, painflasharmorblock, 143)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(painFlashDirTolerance, painflashdirtolerance, 144)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(painArrowCumulativeAppearMin, painarrowcumulativeappearmin, 145)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(painArrowCumulativeDmgJump, painarrowcumulativedmgjump, 146)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(painFlashPrintouts, painflashprintouts, 147)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(itemFlashCumulativeJump, itemflashcumulativejump, 148)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(itemFlashDrawOpacityMin, itemflashdrawopacitymin, 149)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(itemFlashDrawOpacityMax, itemflashdrawopacitymax, 150)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(itemFlashFadeMult, itemflashfademult, 151)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(chumtoadPrintout, chumtoadprintout)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(crossbowReloadSoundDelay, crossbowreloadsounddelay, 152)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(crossbowFirePlaysReloadSound, crossbowfireplaysreloadsound, 153)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(pathfindFidgetFailTime, pathfindfidgetfailtime)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(pathfindPrintout, pathfindprintout)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(pathfindTopRampFixDistance, pathfindtoprampfixdistance)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(pathfindTopRampFixDraw, pathfindtoprampfixdraw)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(iHaveAscended, ihaveascended, 154)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(pathfindLooseMapNodes, pathfindloosemapnodes)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(pathfindRampFix, pathfindrampfix)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(chumtoadPlayDeadFoolChance, chumtoadplaydeadfoolchance)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(animationFramerateMulti, animationframeratemulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(pathfindNodeToleranceMulti, pathfindnodetolerancemulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(friendlyPianoFollowVolume, friendlypianofollowvolume)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(friendlyPianoOtherVolume, friendlypianoothervolume)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(showtriggers, showtriggers)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(tentacleAlertSound, tentaclealertsound)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(tentacleSwingSound1, tentacleswingsound1)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(tentacleSwingSound2, tentacleswingsound2)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(playerFollowerMax, playerfollowermax)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(announcerIsAJerk, announcerisajerk)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(playerUseDrawDebug, playerusedrawdebug)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(playerChumtoadThrowDrawDebug, playerchumtoadthrowdrawdebug)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(peaceOut, peaceout)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(drawViewModel, drawviewmodel, 155)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(drawHUD, drawhud, 156)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(disablePauseSinglePlayer, disablepausesingleplayer)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerBulletHitEffectForceServer, playerbullethiteffectforceserver, 157)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(forceAllowServersideTextureSounds, forceallowserversidetexturesounds, 158)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerWeaponSpreadMode, playerweaponspreadmode, 159)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(monsterAIForceFindDistance, monsteraiforcefinddistance)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(baseEntityDamagePushNormalMulti, baseentitydamagepushnormalmulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(baseEntityDamagePushVerticalBoost, baseentitydamagepushverticalboost)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(baseEntityDamagePushVerticalMulti, baseentitydamagepushverticalmulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(baseEntityDamagePushVerticalMinimum, baseentitydamagepushverticalminimum)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts, viewmodelprintouts, 160)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelSyncFixPrintouts, viewmodelsyncfixprintouts, 161)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(textureHitSoundPrintouts, texturehitsoundprintouts, 162)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hgruntAllowGrenades, hgruntallowgrenades)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(scheduleInterruptPrintouts, scheduleinterruptprintouts)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(animationPrintouts, animationprintouts)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hassaultMeleeAttackEnabled, hassaultmeleeattackenabled)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(pathfindStumpedWaitTime, pathfindstumpedwaittime)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(pathfindStumpedMode, pathfindstumpedmode)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(pathfindStumpedForgetEnemy, pathfindstumpedforgetenemy)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(pathfindEdgeCheck, pathfindedgecheck)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(RadiusDamageDrawDebug, radiusdamagedrawdebug)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(AlienRadiationImmunity, alienradiationimmunity)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(customLogoSprayMode, customlogospraymode)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(monsterFadeOutRate, monsterfadeoutrate)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(playerFadeOutRate, playerfadeoutrate)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(drawDebugEnemyLKP, drawdebugenemylkp)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(trackchangePrintouts, trackchangeprintouts)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(trackTrainPrintouts, tracktrainprintouts)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerWeaponTracerMode, playerweapontracermode, 164)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(monsterWeaponTracerMode, monsterweapontracermode)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(decalTracerExclusivity, decaltracerexclusivity, 165)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(monsterToPlayerHitgroupSpecial, monstertoplayerhitgroupspecial)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(multiplayerCrowbarHitSoundMode, multiplayercrowbarhitsoundmode)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(healthcolor_fullRedMin, healthcolor_fullredmin, 166)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(healthcolor_brightness, healthcolor_brightness, 167)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(healthcolor_yellowMark, healthcolor_yellowmark, 168)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(cl_drawExtraZeros, cl_drawextrazeros, 169)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(pathfindLargeBoundFix, pathfindlargeboundfix)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(flyerKilledFallingLoop, flyerkilledfallingloop)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(floaterDummy, floaterdummy)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(barneyDummy, barneydummy)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(ladderCycleMulti, laddercyclemulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(ladderSpeedMulti, ladderspeedmulti)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(barnacleGrabNoInterpolation, barnaclegrabnointerpolation)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hideDamage, hidedamage, 170)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(timedDamage_brightnessMax, timeddamage_brightnessmax, 171)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(timedDamage_brightnessMin, timeddamage_brightnessmin, 172)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(timedDamage_brightnessCap, timeddamage_brightnesscap, 173)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(timedDamage_brightnessFloor, timeddamage_brightnessfloor, 174)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(timedDamage_flashSpeed, timeddamage_flashspeed, 175)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY(timedDamage_debug, timeddamage_debug, 176)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(wallHealthDoor_closeDelay, wallhealthdoor_closedelay)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(houndeye_attack_canGib, houndeye_attack_cangib)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(myRocketsAreBarney, myrocketsarebarney, 177)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(hassassinCrossbowDebug, hassassincrossbowdebug)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(crossbowBoltDirectionAffectedByWater, crossbowboltdirectionaffectedbywater)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(kingpinDebug, kingpindebug)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(minimumRespawnDelay, minimumrespawndelay)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(drawDebugCrowbar, drawdebugcrowbar)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(pathfindIgnoreNearestNodeCache, pathfindignorenearestnodecache)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(pathfindIgnoreStaticRoutes, pathfindignorestaticroutes)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(pathfindForcePointHull, pathfindforcepointhull)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(pathfindNearestNodeExtra, pathfindnearestnodeextra)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonRadiusDamageMode, egonradiusdamagemode, egonRadiusDamageMode_ID)\
	EASY_CVAR_HIDDEN_ACCESS_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonFireRateMode, egonfireratemode, egonFireRateMode_ID)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(sv_explosion_shake_range, sv_explosion_shake_range)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(sv_explosion_shake_amp, sv_explosion_shake_amp)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(sv_explosion_shake_freq, sv_explosion_shake_freq)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(sv_explosion_shake_duration, sv_explosion_shake_duration)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(sv_gargantua_throwattack, sv_gargantua_throwattack)\
	EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(pathfindMonsterclipFreshLogic, pathfindmonsterclipfreshlogic)\
	DUMMY

// IMPORTANT!  Keep HIDDEN_CVAR_INFO_LENGTH (dlls/client.cpp) in check with any changes to the number of entries above



#define EASY_CVAR_HIDDEN_SAVE_MASS\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(gruntsCanHaveMP5Grenade)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(strobeDurationMin)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(strobeDurationMax)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(strobeRadiusMin)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(strobeRadiusMax)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(strobeSpawnDistHori)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(strobeSpawnDistVertMin)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(strobeSpawnDistVertMax)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(strobeMultiColor)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(raveLaserEnabled)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(raveLaserSpawnFreq)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(raveLaserLength)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(raveLaserSpawnDistHoriMin)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(raveLaserSpawnDistHoriMax)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(raveLaserSpawnDistVertMin)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(raveLaserSpawnDistVertMax)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(raveLaserBrightnessMin)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(raveLaserBrightnessMax)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(raveLaserDurationMin)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(raveLaserDurationMax)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(raveLaserThicknessMin)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(raveLaserThicknessMax)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(raveLaserNoiseMin)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(raveLaserNoiseMax)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(raveLaserFrameRateMin)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(raveLaserFrameRateMax)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(raveLaserMultiColor)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(cameraPosFixedX)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(cameraPosFixedY)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(cameraPosFixedZ)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(cameraPosOffX)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(cameraPosOffY)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(cameraPosOffZ)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(cameraRotFixedX)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(cameraRotFixedY)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(cameraRotFixedZ)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(cameraRotOffX)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(cameraRotOffY)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(cameraRotOffZ)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(imAllFuckedUp)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(thatWasntGrass)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(thatWasntPunch)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(fogNear)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(fogFar)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(NPCsTalkMore)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(myCameraSucks)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(shutupstuka)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hassaultSpinMovement)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hassaultIdleSpinSound)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hassaultFireSound)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(muteBulletHitSounds)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(mutePlayerPainSounds)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hassaultIdleSpinSoundChannel)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hassaultSpinUpDownSoundChannel)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hassaultFireSoundChannel)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(geigerChannel)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hassaultWaitTime)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hassaultSpinupRemainTime)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hassaultResidualAttackTime)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hassaultSpinupStartTime)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hassaultVoicePitchMin)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hassaultVoicePitchMax)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hassaultFireSpread)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hassaultAllowGrenades)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(muteTempEntityGroundHitSound)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(houndeyeAttackMode)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(muteRicochetSound)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(mutePlayerWeaponFire)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(muteCrowbarSounds)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(scientistHealNPC)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(scientistHealNPCDebug)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(bulletholeAlertRange)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(fleshhitmakessound)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(nothingHurts)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(seeMonsterHealth)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(scientistHealNPCFract)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(bullsquidRangeDisabled)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(applyLKPPathFixToAll)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(timedDamageAffectsMonsters)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(scientistHealCooldown)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(crazyMonsterPrintouts)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(movementIsCompletePrintout)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(bulletHoleAlertPrintout)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(bulletholeAlertStukaOnly)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(barneyPrintouts)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(monsterSpawnPrintout)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(zombieBulletResistance)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(zombieBulletPushback)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(houndeyePrintout)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(quakeExplosionSound)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(explosionDebrisSoundVolume)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(noFlinchOnHard)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hassaultDrawLKP)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(meleeDrawBloodModeA)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(meleeDrawBloodModeB)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(drawDebugBloodTrace)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(meleeDrawBloodModeBFix)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(meleeDrawBloodModeAOffset)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(meleeDrawBloodModeBOffset)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(panthereyeHasCloakingAbility)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hgruntSpeedMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hgruntForceStrafeFireAnim)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hgruntLockRunAndGunTime)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hgruntHeadshotGore)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hgruntAllowStrafeFire)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(thoroughHitBoxUpdates)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hgruntTinyClip)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hgruntStrafeAlwaysHasAmmo)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hgruntBrassEjectForwardOffset)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(agrunt_muzzleflash)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(event5011Allowed)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(event5021Allowed)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(event5031Allowed)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(event5002Allowed)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(event5004Allowed)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(eventsAreFabulous)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(glockOldReloadLogic)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(glockOldReloadLogicBarney)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(barneyDroppedGlockAmmoCap)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(drawCollisionBoundsAtDeath)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(drawHitBoundsAtDeath)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(islaveReviveFriendMode)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(islaveReviveFriendChance)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(islaveReviveFriendRange)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(islaveReviveSelfMinDelay)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(islaveReviveSelfMaxDelay)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(islaveReviveSelfChance)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hgruntRunAndGunDistance)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hgruntPrintout)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(testVar)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(painArrowColorMode)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(painFlashColorMode)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(painFlashSuitless)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(painFlashDrownMode)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(firstPersonIdleDelayMin)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(firstPersonIdleDelayMax)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(forceDrawBatteryNumber)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(offsetgivedistance)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(offsetgivelookvertical)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(canShowWeaponSelectAtDeath)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(endlessFlashlightBattery)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(preE3UsesFailColors)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(E3UsesFailColors)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(preE3ShowsDamageIcons)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(E3ShowsDamageIcons)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(playerCrossbowMode)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hassassinCrossbowMode)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(tripmineAnimWaitsForFinish)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(revolverLaserScope)\
	DUMMY\
	DUMMY\
	DUMMY\
	DUMMY\
	DUMMY\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(cheat_infiniteclip)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(cheat_infiniteammo)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(cheat_minimumfiredelay)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(cheat_minimumfiredelaycustom)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(cheat_nogaussrecoil)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(autoSneaky)\
	DUMMY\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(cheat_touchNeverExplodes)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(gaussRecoilSendsUpInSP)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(drawDebugPathfinding)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(STUcheckDistH)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(STUcheckDistV)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(STUcheckDistD)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(STUextraTriangH)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(STUextraTriangV)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(STUrepelMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(STUSpeedMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(STUExplodeTest)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(STUYawSpeedMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(STUDetection)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(stukaAdvancedCombat)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(drawDebugPathfinding2)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hassaultFriendlyFire)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(myStrobe)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(peopleStrobe)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(forceWorldLightOff)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(wildHeads)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(raveEffectSpawnInterval)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(drawBarnacleDebug)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(fogTest)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(sparksAllMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(sparksEnvMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(sparksButtonMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(sparksPlayerCrossbowMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(sparksComputerHitMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(sparksTurretDeathMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(sparksOspreyHitMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(sparksExplosionMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(sparksBeamMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(sparksAIFailMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(normalSpeedMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(noclipSpeedMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(jumpForceMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(apacheForceCinBounds)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(apacheBottomBoundAdj)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(apacheInfluence)\
	DUMMY\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(allowAlphaCrosshairWithoutGuns)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(alphaCrosshairBlockedOnFrozen)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hgruntRunAndGunDotMin)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(panthereyeJumpDotTol)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(panthereyePrintout)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(gargantuaPrintout)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(squadmonsterPrintout)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hassaultPrintout)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(barnaclePrintout)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(friendlyPrintout)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(stukaPrintout)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(timedDamageEndlessOnHard)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(mirrorsReflectOnlyNPCs)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(mirrorsDoNotReflectPlayer)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(altSquadRulesRuntime)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hgruntLockStrafeTime)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(pathfindIgnoreIsolatedNodes)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(drawNodeAll)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(drawNodeSpecial)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(drawNodeConnections)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(drawNodeAlternateTime)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(nodeSearchStartVerticalOffset)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(blockChangeLevelTrigger)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(blockMusicTrigger)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(blockMultiTrigger)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(blockTeleportTrigger)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(blockHurtTrigger)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(blockAutosaveTrigger)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hideNodeGraphRebuildNotice)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(barnacleTongueRetractDelay)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(allowGermanModels)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(germanRobotGibs)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(germanRobotBleedsOil)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(germanRobotDamageDecal)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(germanRobotGibsDecal)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(egonEffectsMode)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(egonHitCloud)\
	DUMMY\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(handGrenadePickupYieldsOne)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(handGrenadesUseOldBounceSound)\
	DUMMY\
	DUMMY\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(timedDamageDeathRemoveMode)\
	DUMMY\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(barnacleCanGib)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(sentryCanGib)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(miniturretCanGib)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(turretCanGib)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(turretBleedsOil)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(turretDamageDecal)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(turretGibDecal)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(canDropInSinglePlayer)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(timedDamageIgnoresArmor)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(itemBatteryPrerequisite)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(playerExtraPainSoundsMode)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(timedDamageDisableViewPunch)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(batteryDrainsAtDeath)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(batteryDrainsAtAdrenalineMode)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(canTakeLongJump)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(printOutCommonTimables)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(playerBrightLight)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(disablePainPunchAutomatic)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(gargantuaCorpseDeath)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(gargantuaFallSound)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(gargantuaBleeds)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(shrapMode)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(shrapRand)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(shrapRandHeightExtra)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(explosionShrapnelMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(useAlphaSparks)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(emergencyFix)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(timedDamageReviveRemoveMode)\
	DUMMY\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(ospreyIgnoresGruntCount)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(mp5GrenadeInheritsPlayerVelocity)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(crossbowInheritsPlayerVelocity)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(fastHornetsInheritsPlayerVelocity)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(snarkInheritsPlayerVelocity)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(chumtoadInheritsPlayerVelocity)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(weaponPickupPlaysAnyReloadSounds)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(glockUseLastBulletAnim)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(playerBarnacleVictimViewOffset)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hgruntMovementDeltaCheck)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hassaultExtraMuzzleFlashRadius)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hassaultExtraMuzzleFlashBrightness)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hassaultExtraMuzzleFlashForward)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(leaderlessSquadAllowed)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(nodeConnectionBreakableCheck)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(playerReviveInvincibilityTime)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(playerReviveBuddhaMode)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(playerReviveTimeBlocksTimedDamage)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hassaultBulletDamageMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hassaultBulletsPerShot)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hassaultFireAnimSpeedMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hassaultMeleeAnimSpeedMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hassassinCrossbowReloadSoundDelay)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hgruntStrafeAnimSpeedMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hgruntRunAndGunAnimSpeedMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(cheat_iwantguts)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(nodeDetailPrintout)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(soundAttenuationStuka)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(soundVolumeStuka)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(cineChangelevelFix)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(drawDebugCine)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(cineAllowSequenceOverwrite)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(stukaInflictsBleeding)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(animationKilledBoundsRemoval)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(gargantuaKilledBoundsAssist)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(bullsquidSpitTrajTimeMin)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(bullsquidSpitTrajTimeMax)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(bullsquidSpitTrajDistMin)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(bullsquidSpitTrajDistMax)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(bullsquidSpitGravityMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(bullsquidSpitUseAlphaModel)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(bullsquidSpitUseAlphaEffect)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(bullsquidSpitEffectSpread)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(bullsquidSpitEffectMin)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(bullsquidSpitEffectMax)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(bullsquidSpitEffectHitMin)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(bullsquidSpitEffectHitMax)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(bullsquidSpitEffectSpawn)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(bullsquidSpitEffectHitSpawn)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(bullsquidSpitSpriteScale)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(bullsquidSpitAlphaScale)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(scientistBravery)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(barneyUnholsterTime)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(barneyUnholsterAnimChoice)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(trailTypeTest)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hornetTrail)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hornetTrailSolidColor)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hornetDeathModEasy)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hornetDeathModMedium)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hornetDeathModHard)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hornetZoomPuff)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hornetSpiral)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hornetSpeedMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hornetSpeedDartMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(rocketTrailAlphaInterval)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(rocketTrailAlphaScale)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(rocketSkipIgnite)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(agruntHornetRandomness)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hornetSpiralPeriod)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hornetSpiralAmplitude)\
	DUMMY\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(painFlashDmgMin)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(painFlashDmgExMult)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(painFlashCumulativeMinDrowning)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(painFlashCumulativeMax)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(painFlashDrawOpacityMax)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(painArrowDrawOpacityMin)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(painArrowDrawOpacityMax)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(painFlashFadeMult)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(painArrowFadeMult)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(painFlashArmorBlock)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(painFlashDirTolerance)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(painArrowCumulativeAppearMin)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(painArrowCumulativeDmgJump)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(painFlashPrintouts)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(itemFlashCumulativeJump)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(itemFlashDrawOpacityMin)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(itemFlashDrawOpacityMax)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(itemFlashFadeMult)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(chumtoadPrintout)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(crossbowReloadSoundDelay)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(crossbowFirePlaysReloadSound)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(pathfindFidgetFailTime)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(pathfindPrintout)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(pathfindTopRampFixDistance)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(pathfindTopRampFixDraw)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(iHaveAscended)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(pathfindLooseMapNodes)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(pathfindRampFix)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(chumtoadPlayDeadFoolChance)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(animationFramerateMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(pathfindNodeToleranceMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(friendlyPianoFollowVolume)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(friendlyPianoOtherVolume)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(showtriggers)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(tentacleAlertSound)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(tentacleSwingSound1)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(tentacleSwingSound2)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(playerFollowerMax)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(announcerIsAJerk)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(playerUseDrawDebug)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(playerChumtoadThrowDrawDebug)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(peaceOut)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(drawViewModel)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(drawHUD)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(disablePauseSinglePlayer)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(playerBulletHitEffectForceServer)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(forceAllowServersideTextureSounds)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(playerWeaponSpreadMode)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(monsterAIForceFindDistance)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(baseEntityDamagePushNormalMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(baseEntityDamagePushVerticalBoost)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(baseEntityDamagePushVerticalMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(baseEntityDamagePushVerticalMinimum)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(viewModelPrintouts)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(viewModelSyncFixPrintouts)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(textureHitSoundPrintouts)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hgruntAllowGrenades)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(scheduleInterruptPrintouts)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(animationPrintouts)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hassaultMeleeAttackEnabled)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(pathfindStumpedWaitTime)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(pathfindStumpedMode)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(pathfindStumpedForgetEnemy)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(pathfindEdgeCheck)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(RadiusDamageDrawDebug)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(AlienRadiationImmunity)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(customLogoSprayMode)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(monsterFadeOutRate)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(playerFadeOutRate)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(drawDebugEnemyLKP)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(trackchangePrintouts)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(trackTrainPrintouts)\
	DUMMY\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(playerWeaponTracerMode)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(monsterWeaponTracerMode)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(decalTracerExclusivity)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(monsterToPlayerHitgroupSpecial)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(multiplayerCrowbarHitSoundMode)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(healthcolor_fullRedMin)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(healthcolor_brightness)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(healthcolor_yellowMark)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(cl_drawExtraZeros)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(pathfindLargeBoundFix)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(flyerKilledFallingLoop)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(floaterDummy)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(barneyDummy)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(ladderCycleMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(ladderSpeedMulti)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(barnacleGrabNoInterpolation)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hideDamage)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(timedDamage_brightnessMax)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(timedDamage_brightnessMin)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(timedDamage_brightnessCap)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(timedDamage_brightnessFloor)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(timedDamage_flashSpeed)\
	EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(timedDamage_debug)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(wallHealthDoor_closeDelay)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(houndeye_attack_canGib)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(myRocketsAreBarney)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(hassassinCrossbowDebug)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(crossbowBoltDirectionAffectedByWater)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(kingpinDebug)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(minimumRespawnDelay)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(drawDebugCrowbar)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(pathfindIgnoreNearestNodeCache)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(pathfindIgnoreStaticRoutes)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(pathfindForcePointHull)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(pathfindNearestNodeExtra)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(egonRadiusDamageMode)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(egonFireRateMode)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(sv_explosion_shake_range)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(sv_explosion_shake_amp)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(sv_explosion_shake_freq)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(sv_explosion_shake_duration)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(sv_gargantua_throwattack)\
	EASY_CVAR_HIDDEN_SAVE_SERVERONLY(pathfindMonsterclipFreshLogic)\
	DUMMY



#define EASY_CVAR_HIDDEN_LOAD_MASS\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(gruntsCanHaveMP5Grenade, gruntscanhavemp5grenade)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(strobeDurationMin, strobedurationmin)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(strobeDurationMax, strobedurationmax)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(strobeRadiusMin, stroberadiusmin)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(strobeRadiusMax, stroberadiusmax)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(strobeSpawnDistHori, strobespawndisthori)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(strobeSpawnDistVertMin, strobespawndistvertmin)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(strobeSpawnDistVertMax, strobespawndistvertmax)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(strobeMultiColor, strobemulticolor)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(raveLaserEnabled, ravelaserenabled)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(raveLaserSpawnFreq, ravelaserspawnfreq)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(raveLaserLength, ravelaserlength)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(raveLaserSpawnDistHoriMin, ravelaserspawndisthorimin)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(raveLaserSpawnDistHoriMax, ravelaserspawndisthorimax)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(raveLaserSpawnDistVertMin, ravelaserspawndistvertmin)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(raveLaserSpawnDistVertMax, ravelaserspawndistvertmax)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(raveLaserBrightnessMin, ravelaserbrightnessmin)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(raveLaserBrightnessMax, ravelaserbrightnessmax)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(raveLaserDurationMin, ravelaserdurationmin)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(raveLaserDurationMax, ravelaserdurationmax)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(raveLaserThicknessMin, ravelaserthicknessmin)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(raveLaserThicknessMax, ravelaserthicknessmax)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(raveLaserNoiseMin, ravelasernoisemin)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(raveLaserNoiseMax, ravelasernoisemax)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(raveLaserFrameRateMin, ravelaserframeratemin)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(raveLaserFrameRateMax, ravelaserframeratemax)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(raveLaserMultiColor, ravelasermulticolor)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(cameraPosFixedX, cameraposfixedx)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(cameraPosFixedY, cameraposfixedy)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(cameraPosFixedZ, cameraposfixedz)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(cameraPosOffX, cameraposoffx)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(cameraPosOffY, cameraposoffy)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(cameraPosOffZ, cameraposoffz)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(cameraRotFixedX, camerarotfixedx)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(cameraRotFixedY, camerarotfixedy)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(cameraRotFixedZ, camerarotfixedz)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(cameraRotOffX, camerarotoffx)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(cameraRotOffY, camerarotoffy)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(cameraRotOffZ, camerarotoffz)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(imAllFuckedUp, imallfuckedup)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(thatWasntGrass, thatwasntgrass)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(thatWasntPunch, thatwasntpunch)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(fogNear, fognear)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(fogFar, fogfar)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(NPCsTalkMore, npcstalkmore)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(myCameraSucks, mycamerasucks)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(shutupstuka, shutupstuka)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hassaultSpinMovement, hassaultspinmovement)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hassaultIdleSpinSound, hassaultidlespinsound)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hassaultFireSound, hassaultfiresound)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(muteBulletHitSounds, mutebullethitsounds)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(mutePlayerPainSounds, muteplayerpainsounds)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hassaultIdleSpinSoundChannel, hassaultidlespinsoundchannel)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hassaultSpinUpDownSoundChannel, hassaultspinupdownsoundchannel)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hassaultFireSoundChannel, hassaultfiresoundchannel)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(geigerChannel, geigerchannel)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hassaultWaitTime, hassaultwaittime)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hassaultSpinupRemainTime, hassaultspinupremaintime)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hassaultResidualAttackTime, hassaultresidualattacktime)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hassaultSpinupStartTime, hassaultspinupstarttime)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hassaultVoicePitchMin, hassaultvoicepitchmin)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hassaultVoicePitchMax, hassaultvoicepitchmax)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hassaultFireSpread, hassaultfirespread)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hassaultAllowGrenades, hassaultallowgrenades)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(muteTempEntityGroundHitSound, mutetempentitygroundhitsound)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(houndeyeAttackMode, houndeyeattackmode)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(muteRicochetSound, mutericochetsound)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(mutePlayerWeaponFire, muteplayerweaponfire)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(muteCrowbarSounds, mutecrowbarsounds)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(scientistHealNPC, scientisthealnpc)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(scientistHealNPCDebug, scientisthealnpcdebug)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(bulletholeAlertRange, bulletholealertrange)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(fleshhitmakessound, fleshhitmakessound)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(nothingHurts, nothinghurts)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(seeMonsterHealth, seemonsterhealth)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(scientistHealNPCFract, scientisthealnpcfract)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(bullsquidRangeDisabled, bullsquidrangedisabled)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(applyLKPPathFixToAll, applylkppathfixtoall)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(timedDamageAffectsMonsters, timeddamageaffectsmonsters)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(scientistHealCooldown, scientisthealcooldown)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(crazyMonsterPrintouts, crazymonsterprintouts)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(movementIsCompletePrintout, movementiscompleteprintout)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(bulletHoleAlertPrintout, bulletholealertprintout)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(bulletholeAlertStukaOnly, bulletholealertstukaonly)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(barneyPrintouts, barneyprintouts)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(monsterSpawnPrintout, monsterspawnprintout)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(zombieBulletResistance, zombiebulletresistance)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(zombieBulletPushback, zombiebulletpushback)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(houndeyePrintout, houndeyeprintout)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(quakeExplosionSound, quakeexplosionsound)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(explosionDebrisSoundVolume, explosiondebrissoundvolume)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(noFlinchOnHard, noflinchonhard)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hassaultDrawLKP, hassaultdrawlkp)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(meleeDrawBloodModeA, meleedrawbloodmodea)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(meleeDrawBloodModeB, meleedrawbloodmodeb)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(drawDebugBloodTrace, drawdebugbloodtrace)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(meleeDrawBloodModeBFix, meleedrawbloodmodebfix)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(meleeDrawBloodModeAOffset, meleedrawbloodmodeaoffset)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(meleeDrawBloodModeBOffset, meleedrawbloodmodeboffset)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(panthereyeHasCloakingAbility, panthereyehascloakingability)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hgruntSpeedMulti, hgruntspeedmulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hgruntForceStrafeFireAnim, hgruntforcestrafefireanim)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hgruntLockRunAndGunTime, hgruntlockrunandguntime)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hgruntHeadshotGore, hgruntheadshotgore)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hgruntAllowStrafeFire, hgruntallowstrafefire)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(thoroughHitBoxUpdates, thoroughhitboxupdates)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hgruntTinyClip, hgrunttinyclip)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hgruntStrafeAlwaysHasAmmo, hgruntstrafealwayshasammo)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hgruntBrassEjectForwardOffset, hgruntbrassejectforwardoffset)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(agrunt_muzzleflash, agrunt_muzzleflash)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(event5011Allowed, event5011allowed)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(event5021Allowed, event5021allowed)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(event5031Allowed, event5031allowed)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(event5002Allowed, event5002allowed)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(event5004Allowed, event5004allowed)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(eventsAreFabulous, eventsarefabulous)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(glockOldReloadLogic, glockoldreloadlogic)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(glockOldReloadLogicBarney, glockoldreloadlogicbarney)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(barneyDroppedGlockAmmoCap, barneydroppedglockammocap)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(drawCollisionBoundsAtDeath, drawcollisionboundsatdeath)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(drawHitBoundsAtDeath, drawhitboundsatdeath)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(islaveReviveFriendMode, islaverevivefriendmode)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(islaveReviveFriendChance, islaverevivefriendchance)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(islaveReviveFriendRange, islaverevivefriendrange)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(islaveReviveSelfMinDelay, islavereviveselfmindelay)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(islaveReviveSelfMaxDelay, islavereviveselfmaxdelay)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(islaveReviveSelfChance, islavereviveselfchance)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hgruntRunAndGunDistance, hgruntrunandgundistance)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hgruntPrintout, hgruntprintout)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(testVar, testvar)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(painArrowColorMode, painarrowcolormode)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(painFlashColorMode, painflashcolormode)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(painFlashSuitless, painflashsuitless)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(painFlashDrownMode, painflashdrownmode)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(firstPersonIdleDelayMin, firstpersonidledelaymin)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(firstPersonIdleDelayMax, firstpersonidledelaymax)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(forceDrawBatteryNumber, forcedrawbatterynumber)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(offsetgivedistance, offsetgivedistance)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(offsetgivelookvertical, offsetgivelookvertical)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(canShowWeaponSelectAtDeath, canshowweaponselectatdeath)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(endlessFlashlightBattery, endlessflashlightbattery)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(preE3UsesFailColors, pree3usesfailcolors)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(E3UsesFailColors, e3usesfailcolors)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(preE3ShowsDamageIcons, pree3showsdamageicons)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(E3ShowsDamageIcons, e3showsdamageicons)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(playerCrossbowMode, playercrossbowmode)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hassassinCrossbowMode, hassassincrossbowmode)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(tripmineAnimWaitsForFinish, tripmineanimwaitsforfinish)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(revolverLaserScope, revolverlaserscope)\
	DUMMY\
	DUMMY\
	DUMMY\
	DUMMY\
	DUMMY\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(cheat_infiniteclip, cheat_infiniteclip)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(cheat_infiniteammo, cheat_infiniteammo)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(cheat_minimumfiredelay, cheat_minimumfiredelay)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(cheat_minimumfiredelaycustom, cheat_minimumfiredelaycustom)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(cheat_nogaussrecoil, cheat_nogaussrecoil)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(autoSneaky, autosneaky)\
	DUMMY\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(cheat_touchNeverExplodes, cheat_touchneverexplodes)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(gaussRecoilSendsUpInSP, gaussrecoilsendsupinsp)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(drawDebugPathfinding, drawdebugpathfinding)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(STUcheckDistH, stucheckdisth)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(STUcheckDistV, stucheckdistv)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(STUcheckDistD, stucheckdistd)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(STUextraTriangH, stuextratriangh)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(STUextraTriangV, stuextratriangv)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(STUrepelMulti, sturepelmulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(STUSpeedMulti, stuspeedmulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(STUExplodeTest, stuexplodetest)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(STUYawSpeedMulti, stuyawspeedmulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(STUDetection, studetection)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(stukaAdvancedCombat, stukaadvancedcombat)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(drawDebugPathfinding2, drawdebugpathfinding2)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hassaultFriendlyFire, hassaultfriendlyfire)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(myStrobe, mystrobe)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(peopleStrobe, peoplestrobe)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(forceWorldLightOff, forceworldlightoff)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(wildHeads, wildheads)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(raveEffectSpawnInterval, raveeffectspawninterval)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(drawBarnacleDebug, drawbarnacledebug)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(fogTest, fogtest)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(sparksAllMulti, sparksallmulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(sparksEnvMulti, sparksenvmulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(sparksButtonMulti, sparksbuttonmulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(sparksPlayerCrossbowMulti, sparksplayercrossbowmulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(sparksComputerHitMulti, sparkscomputerhitmulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(sparksTurretDeathMulti, sparksturretdeathmulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(sparksOspreyHitMulti, sparksospreyhitmulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(sparksExplosionMulti, sparksexplosionmulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(sparksBeamMulti, sparksbeammulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(sparksAIFailMulti, sparksaifailmulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(normalSpeedMulti, normalspeedmulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(noclipSpeedMulti, noclipspeedmulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(jumpForceMulti, jumpforcemulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(apacheForceCinBounds, apacheforcecinbounds)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(apacheBottomBoundAdj, apachebottomboundadj)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(apacheInfluence, apacheinfluence)\
	DUMMY\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(allowAlphaCrosshairWithoutGuns, allowalphacrosshairwithoutguns)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(alphaCrosshairBlockedOnFrozen, alphacrosshairblockedonfrozen)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hgruntRunAndGunDotMin, hgruntrunandgundotmin)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(panthereyeJumpDotTol, panthereyejumpdottol)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(panthereyePrintout, panthereyeprintout)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(gargantuaPrintout, gargantuaprintout)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(squadmonsterPrintout, squadmonsterprintout)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hassaultPrintout, hassaultprintout)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(barnaclePrintout, barnacleprintout)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(friendlyPrintout, friendlyprintout)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(stukaPrintout, stukaprintout)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(timedDamageEndlessOnHard, timeddamageendlessonhard)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(mirrorsReflectOnlyNPCs, mirrorsreflectonlynpcs)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(mirrorsDoNotReflectPlayer, mirrorsdonotreflectplayer)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(altSquadRulesRuntime, altsquadrulesruntime)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hgruntLockStrafeTime, hgruntlockstrafetime)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(pathfindIgnoreIsolatedNodes, pathfindignoreisolatednodes)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(drawNodeAll, drawnodeall)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(drawNodeSpecial, drawnodespecial)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(drawNodeConnections, drawnodeconnections)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(drawNodeAlternateTime, drawnodealternatetime)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(nodeSearchStartVerticalOffset, nodesearchstartverticaloffset)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(blockChangeLevelTrigger, blockchangeleveltrigger)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(blockMusicTrigger, blockmusictrigger)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(blockMultiTrigger, blockmultitrigger)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(blockTeleportTrigger, blockteleporttrigger)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(blockHurtTrigger, blockhurttrigger)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(blockAutosaveTrigger, blockautosavetrigger)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hideNodeGraphRebuildNotice, hidenodegraphrebuildnotice)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(barnacleTongueRetractDelay, barnacletongueretractdelay)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(allowGermanModels, allowgermanmodels)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(germanRobotGibs, germanrobotgibs)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(germanRobotBleedsOil, germanrobotbleedsoil)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(germanRobotDamageDecal, germanrobotdamagedecal)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(germanRobotGibsDecal, germanrobotgibsdecal)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(egonEffectsMode, egoneffectsmode)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(egonHitCloud, egonhitcloud)\
	DUMMY\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(handGrenadePickupYieldsOne, handgrenadepickupyieldsone)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(handGrenadesUseOldBounceSound, handgrenadesuseoldbouncesound)\
	DUMMY\
	DUMMY\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(timedDamageDeathRemoveMode, timeddamagedeathremovemode)\
	DUMMY\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(barnacleCanGib, barnaclecangib)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(sentryCanGib, sentrycangib)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(miniturretCanGib, miniturretcangib)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(turretCanGib, turretcangib)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(turretBleedsOil, turretbleedsoil)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(turretDamageDecal, turretdamagedecal)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(turretGibDecal, turretgibdecal)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(canDropInSinglePlayer, candropinsingleplayer)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(timedDamageIgnoresArmor, timeddamageignoresarmor)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(itemBatteryPrerequisite, itembatteryprerequisite)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(playerExtraPainSoundsMode, playerextrapainsoundsmode)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(timedDamageDisableViewPunch, timeddamagedisableviewpunch)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(batteryDrainsAtDeath, batterydrainsatdeath)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(batteryDrainsAtAdrenalineMode, batterydrainsatadrenalinemode)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(canTakeLongJump, cantakelongjump)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(printOutCommonTimables, printoutcommontimables)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(playerBrightLight, playerbrightlight)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(disablePainPunchAutomatic, disablepainpunchautomatic)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(gargantuaCorpseDeath, gargantuacorpsedeath)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(gargantuaFallSound, gargantuafallsound)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(gargantuaBleeds, gargantuableeds)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(shrapMode, shrapmode)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(shrapRand, shraprand)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(shrapRandHeightExtra, shraprandheightextra)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(explosionShrapnelMulti, explosionshrapnelmulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(useAlphaSparks, usealphasparks)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(emergencyFix, emergencyfix)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(timedDamageReviveRemoveMode, timeddamagereviveremovemode)\
	DUMMY\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(ospreyIgnoresGruntCount, ospreyignoresgruntcount)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(mp5GrenadeInheritsPlayerVelocity, mp5grenadeinheritsplayervelocity)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(crossbowInheritsPlayerVelocity, crossbowinheritsplayervelocity)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(fastHornetsInheritsPlayerVelocity, fasthornetsinheritsplayervelocity)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(snarkInheritsPlayerVelocity, snarkinheritsplayervelocity)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(chumtoadInheritsPlayerVelocity, chumtoadinheritsplayervelocity)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(weaponPickupPlaysAnyReloadSounds, weaponpickupplaysanyreloadsounds)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(glockUseLastBulletAnim, glockuselastbulletanim)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(playerBarnacleVictimViewOffset, playerbarnaclevictimviewoffset)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hgruntMovementDeltaCheck, hgruntmovementdeltacheck)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hassaultExtraMuzzleFlashRadius, hassaultextramuzzleflashradius)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hassaultExtraMuzzleFlashBrightness, hassaultextramuzzleflashbrightness)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hassaultExtraMuzzleFlashForward, hassaultextramuzzleflashforward)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(leaderlessSquadAllowed, leaderlesssquadallowed)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(nodeConnectionBreakableCheck, nodeconnectionbreakablecheck)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(playerReviveInvincibilityTime, playerreviveinvincibilitytime)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(playerReviveBuddhaMode, playerrevivebuddhamode)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(playerReviveTimeBlocksTimedDamage, playerrevivetimeblockstimeddamage)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hassaultBulletDamageMulti, hassaultbulletdamagemulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hassaultBulletsPerShot, hassaultbulletspershot)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hassaultFireAnimSpeedMulti, hassaultfireanimspeedmulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hassaultMeleeAnimSpeedMulti, hassaultmeleeanimspeedmulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hassassinCrossbowReloadSoundDelay, hassassincrossbowreloadsounddelay)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hgruntStrafeAnimSpeedMulti, hgruntstrafeanimspeedmulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hgruntRunAndGunAnimSpeedMulti, hgruntrunandgunanimspeedmulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(cheat_iwantguts, cheat_iwantguts)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(nodeDetailPrintout, nodedetailprintout)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(soundAttenuationStuka, soundattenuationstuka)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(soundVolumeStuka, soundvolumestuka)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(cineChangelevelFix, cinechangelevelfix)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(drawDebugCine, drawdebugcine)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(cineAllowSequenceOverwrite, cineallowsequenceoverwrite)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(stukaInflictsBleeding, stukainflictsbleeding)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(animationKilledBoundsRemoval, animationkilledboundsremoval)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(gargantuaKilledBoundsAssist, gargantuakilledboundsassist)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(bullsquidSpitTrajTimeMin, bullsquidspittrajtimemin)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(bullsquidSpitTrajTimeMax, bullsquidspittrajtimemax)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(bullsquidSpitTrajDistMin, bullsquidspittrajdistmin)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(bullsquidSpitTrajDistMax, bullsquidspittrajdistmax)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(bullsquidSpitGravityMulti, bullsquidspitgravitymulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(bullsquidSpitUseAlphaModel, bullsquidspitusealphamodel)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(bullsquidSpitUseAlphaEffect, bullsquidspitusealphaeffect)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(bullsquidSpitEffectSpread, bullsquidspiteffectspread)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(bullsquidSpitEffectMin, bullsquidspiteffectmin)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(bullsquidSpitEffectMax, bullsquidspiteffectmax)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(bullsquidSpitEffectHitMin, bullsquidspiteffecthitmin)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(bullsquidSpitEffectHitMax, bullsquidspiteffecthitmax)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(bullsquidSpitEffectSpawn, bullsquidspiteffectspawn)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(bullsquidSpitEffectHitSpawn, bullsquidspiteffecthitspawn)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(bullsquidSpitSpriteScale, bullsquidspitspritescale)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(bullsquidSpitAlphaScale, bullsquidspitalphascale)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(scientistBravery, scientistbravery)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(barneyUnholsterTime, barneyunholstertime)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(barneyUnholsterAnimChoice, barneyunholsteranimchoice)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(trailTypeTest, trailtypetest)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hornetTrail, hornettrail)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hornetTrailSolidColor, hornettrailsolidcolor)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hornetDeathModEasy, hornetdeathmodeasy)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hornetDeathModMedium, hornetdeathmodmedium)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hornetDeathModHard, hornetdeathmodhard)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hornetZoomPuff, hornetzoompuff)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hornetSpiral, hornetspiral)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hornetSpeedMulti, hornetspeedmulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hornetSpeedDartMulti, hornetspeeddartmulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(rocketTrailAlphaInterval, rockettrailalphainterval)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(rocketTrailAlphaScale, rockettrailalphascale)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(rocketSkipIgnite, rocketskipignite)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(agruntHornetRandomness, agrunthornetrandomness)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hornetSpiralPeriod, hornetspiralperiod)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hornetSpiralAmplitude, hornetspiralamplitude)\
	DUMMY\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(painFlashDmgMin, painflashdmgmin)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(painFlashDmgExMult, painflashdmgexmult)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(painFlashCumulativeMinDrowning, painflashcumulativemindrowning)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(painFlashCumulativeMax, painflashcumulativemax)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(painFlashDrawOpacityMax, painflashdrawopacitymax)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(painArrowDrawOpacityMin, painarrowdrawopacitymin)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(painArrowDrawOpacityMax, painarrowdrawopacitymax)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(painFlashFadeMult, painflashfademult)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(painArrowFadeMult, painarrowfademult)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(painFlashArmorBlock, painflasharmorblock)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(painFlashDirTolerance, painflashdirtolerance)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(painArrowCumulativeAppearMin, painarrowcumulativeappearmin)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(painArrowCumulativeDmgJump, painarrowcumulativedmgjump)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(painFlashPrintouts, painflashprintouts)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(itemFlashCumulativeJump, itemflashcumulativejump)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(itemFlashDrawOpacityMin, itemflashdrawopacitymin)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(itemFlashDrawOpacityMax, itemflashdrawopacitymax)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(itemFlashFadeMult, itemflashfademult)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(chumtoadPrintout, chumtoadprintout)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(crossbowReloadSoundDelay, crossbowreloadsounddelay)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(crossbowFirePlaysReloadSound, crossbowfireplaysreloadsound)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(pathfindFidgetFailTime, pathfindfidgetfailtime)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(pathfindPrintout, pathfindprintout)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(pathfindTopRampFixDistance, pathfindtoprampfixdistance)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(pathfindTopRampFixDraw, pathfindtoprampfixdraw)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(iHaveAscended, ihaveascended)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(pathfindLooseMapNodes, pathfindloosemapnodes)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(pathfindRampFix, pathfindrampfix)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(chumtoadPlayDeadFoolChance, chumtoadplaydeadfoolchance)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(animationFramerateMulti, animationframeratemulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(pathfindNodeToleranceMulti, pathfindnodetolerancemulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(friendlyPianoFollowVolume, friendlypianofollowvolume)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(friendlyPianoOtherVolume, friendlypianoothervolume)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(showtriggers, showtriggers)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(tentacleAlertSound, tentaclealertsound)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(tentacleSwingSound1, tentacleswingsound1)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(tentacleSwingSound2, tentacleswingsound2)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(playerFollowerMax, playerfollowermax)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(announcerIsAJerk, announcerisajerk)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(playerUseDrawDebug, playerusedrawdebug)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(playerChumtoadThrowDrawDebug, playerchumtoadthrowdrawdebug)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(peaceOut, peaceout)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(drawViewModel, drawviewmodel)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(drawHUD, drawhud)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(disablePauseSinglePlayer, disablepausesingleplayer)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(playerBulletHitEffectForceServer, playerbullethiteffectforceserver)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(forceAllowServersideTextureSounds, forceallowserversidetexturesounds)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(playerWeaponSpreadMode, playerweaponspreadmode)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(monsterAIForceFindDistance, monsteraiforcefinddistance)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(baseEntityDamagePushNormalMulti, baseentitydamagepushnormalmulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(baseEntityDamagePushVerticalBoost, baseentitydamagepushverticalboost)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(baseEntityDamagePushVerticalMulti, baseentitydamagepushverticalmulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(baseEntityDamagePushVerticalMinimum, baseentitydamagepushverticalminimum)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(viewModelPrintouts, viewmodelprintouts)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(viewModelSyncFixPrintouts, viewmodelsyncfixprintouts)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(textureHitSoundPrintouts, texturehitsoundprintouts)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hgruntAllowGrenades, hgruntallowgrenades)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(scheduleInterruptPrintouts, scheduleinterruptprintouts)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(animationPrintouts, animationprintouts)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hassaultMeleeAttackEnabled, hassaultmeleeattackenabled)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(pathfindStumpedWaitTime, pathfindstumpedwaittime)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(pathfindStumpedMode, pathfindstumpedmode)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(pathfindStumpedForgetEnemy, pathfindstumpedforgetenemy)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(pathfindEdgeCheck, pathfindedgecheck)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(RadiusDamageDrawDebug, radiusdamagedrawdebug)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(AlienRadiationImmunity, alienradiationimmunity)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(customLogoSprayMode, customlogospraymode)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(monsterFadeOutRate, monsterfadeoutrate)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(playerFadeOutRate, playerfadeoutrate)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(drawDebugEnemyLKP, drawdebugenemylkp)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(trackchangePrintouts, trackchangeprintouts)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(trackTrainPrintouts, tracktrainprintouts)\
	DUMMY\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(playerWeaponTracerMode, playerweapontracermode)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(monsterWeaponTracerMode, monsterweapontracermode)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(decalTracerExclusivity, decaltracerexclusivity)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(monsterToPlayerHitgroupSpecial, monstertoplayerhitgroupspecial)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(multiplayerCrowbarHitSoundMode, multiplayercrowbarhitsoundmode)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(healthcolor_fullRedMin, healthcolor_fullredmin)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(healthcolor_brightness, healthcolor_brightness)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(healthcolor_yellowMark, healthcolor_yellowmark)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(cl_drawExtraZeros, cl_drawextrazeros)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(pathfindLargeBoundFix, pathfindlargeboundfix)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(flyerKilledFallingLoop, flyerkilledfallingloop)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(floaterDummy, floaterdummy)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(barneyDummy, barneydummy)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(ladderCycleMulti, laddercyclemulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(ladderSpeedMulti, ladderspeedmulti)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(barnacleGrabNoInterpolation, barnaclegrabnointerpolation)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hideDamage, hidedamage)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(timedDamage_brightnessMax, timeddamage_brightnessmax)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(timedDamage_brightnessMin, timeddamage_brightnessmin)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(timedDamage_brightnessCap, timeddamage_brightnesscap)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(timedDamage_brightnessFloor, timeddamage_brightnessfloor)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(timedDamage_flashSpeed, timeddamage_flashspeed)\
	EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(timedDamage_debug, timeddamage_debug)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(wallHealthDoor_closeDelay, wallhealthdoor_closedelay)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(houndeye_attack_canGib, houndeye_attack_cangib)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(myRocketsAreBarney, myrocketsarebarney)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(hassassinCrossbowDebug, hassassincrossbowdebug)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(crossbowBoltDirectionAffectedByWater, crossbowboltdirectionaffectedbywater)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(kingpinDebug, kingpindebug)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(minimumRespawnDelay, minimumrespawndelay)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(drawDebugCrowbar, drawdebugcrowbar)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(pathfindIgnoreNearestNodeCache, pathfindignorenearestnodecache)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(pathfindIgnoreStaticRoutes, pathfindignorestaticroutes)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(pathfindForcePointHull, pathfindforcepointhull)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(pathfindNearestNodeExtra, pathfindnearestnodeextra)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(egonRadiusDamageMode, egonradiusdamagemode)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(egonFireRateMode, egonfireratemode)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(sv_explosion_shake_range, sv_explosion_shake_range)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(sv_explosion_shake_amp, sv_explosion_shake_amp)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(sv_explosion_shake_freq, sv_explosion_shake_freq)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(sv_explosion_shake_duration, sv_explosion_shake_duration)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(sv_gargantua_throwattack, sv_gargantua_throwattack)\
	EASY_CVAR_HIDDEN_LOAD_SERVERONLY(pathfindMonsterclipFreshLogic, pathfindmonsterclipfreshlogic)\
	DUMMY





#define EASY_CVAR_DECLARATION_SERVER_MASS\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(gruntsCanHaveMP5Grenade)\
	EASY_CVAR_DECLARATION_SERVER(hud_version)\
	EASY_CVAR_DECLARATION_SERVER(hud_batterydraw)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(strobeDurationMin)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(strobeDurationMax)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(strobeRadiusMin)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(strobeRadiusMax)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(strobeSpawnDistHori)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(strobeSpawnDistVertMin)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(strobeSpawnDistVertMax)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(strobeMultiColor)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(raveLaserEnabled)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(raveLaserSpawnFreq)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(raveLaserLength)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMin)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMax)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMin)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMax)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(raveLaserBrightnessMin)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(raveLaserBrightnessMax)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(raveLaserDurationMin)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(raveLaserDurationMax)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(raveLaserThicknessMin)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(raveLaserThicknessMax)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(raveLaserNoiseMin)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(raveLaserNoiseMax)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(raveLaserFrameRateMin)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(raveLaserFrameRateMax)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(raveLaserMultiColor)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(cameraPosFixedX)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(cameraPosFixedY)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(cameraPosFixedZ)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(cameraPosOffX)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(cameraPosOffY)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(cameraPosOffZ)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(cameraRotFixedX)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(cameraRotFixedY)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(cameraRotFixedZ)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(cameraRotOffX)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(cameraRotOffY)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(cameraRotOffZ)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(imAllFuckedUp)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(thatWasntGrass)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(fogNear)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(fogFar)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(NPCsTalkMore)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(myCameraSucks)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(shutupstuka)\
	EASY_CVAR_DECLARATION_SERVER(chromeEffect)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hassaultSpinMovement)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hassaultIdleSpinSound)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hassaultFireSound)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteBulletHitSounds)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(mutePlayerPainSounds)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hassaultIdleSpinSoundChannel)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hassaultSpinUpDownSoundChannel)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hassaultFireSoundChannel)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(geigerChannel)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hassaultWaitTime)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hassaultSpinupRemainTime)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hassaultResidualAttackTime)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hassaultSpinupStartTime)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hassaultVoicePitchMin)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hassaultVoicePitchMax)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hassaultFireSpread)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hassaultAllowGrenades)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(muteTempEntityGroundHitSound)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(houndeyeAttackMode)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteRicochetSound)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(mutePlayerWeaponFire)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteCrowbarSounds)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(scientistHealNPC)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(scientistHealNPCDebug)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(bulletholeAlertRange)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(fleshhitmakessound)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(nothingHurts)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(seeMonsterHealth)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(scientistHealNPCFract)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(bullsquidRangeDisabled)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(applyLKPPathFixToAll)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(timedDamageAffectsMonsters)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(scientistHealCooldown)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(crazyMonsterPrintouts)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(movementIsCompletePrintout)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(bulletHoleAlertPrintout)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(bulletholeAlertStukaOnly)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(barneyPrintouts)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(monsterSpawnPrintout)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(zombieBulletResistance)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(zombieBulletPushback)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(houndeyePrintout)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(quakeExplosionSound)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(explosionDebrisSoundVolume)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(noFlinchOnHard)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hassaultDrawLKP)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(meleeDrawBloodModeA)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(meleeDrawBloodModeB)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(drawDebugBloodTrace)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(meleeDrawBloodModeBFix)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(meleeDrawBloodModeAOffset)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(meleeDrawBloodModeBOffset)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST(wpn_glocksilencer)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(panthereyeHasCloakingAbility)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hgruntSpeedMulti)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hgruntForceStrafeFireAnim)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hgruntLockRunAndGunTime)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hgruntHeadshotGore)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hgruntAllowStrafeFire)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(thoroughHitBoxUpdates)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hgruntTinyClip)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hgruntStrafeAlwaysHasAmmo)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hgruntBrassEjectForwardOffset)\
	EASY_CVAR_DECLARATION_SERVER(cl_muzzleflash)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(agrunt_muzzleflash)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(event5011Allowed)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(event5021Allowed)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(event5031Allowed)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(event5002Allowed)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(event5004Allowed)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(eventsAreFabulous)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(glockOldReloadLogic)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(glockOldReloadLogicBarney)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(barneyDroppedGlockAmmoCap)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(drawCollisionBoundsAtDeath)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(drawHitBoundsAtDeath)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(islaveReviveFriendMode)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(islaveReviveFriendChance)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(islaveReviveFriendRange)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(islaveReviveSelfMinDelay)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(islaveReviveSelfMaxDelay)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(islaveReviveSelfChance)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hgruntRunAndGunDistance)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hgruntPrintout)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(painArrowColorMode)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(painFlashColorMode)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(painFlashSuitless)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(painFlashDrownMode)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(firstPersonIdleDelayMin)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(firstPersonIdleDelayMax)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(forceDrawBatteryNumber)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(offsetgivedistance)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(offsetgivelookvertical)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(canShowWeaponSelectAtDeath)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(endlessFlashlightBattery)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(preE3UsesFailColors)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(E3UsesFailColors)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(preE3ShowsDamageIcons)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(E3ShowsDamageIcons)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerCrossbowMode)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hassassinCrossbowMode)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(tripmineAnimWaitsForFinish)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(revolverLaserScope)\
	DUMMY\
	DUMMY\
	DUMMY\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY(auto_adjust_fov)\
	DUMMY\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_nogaussrecoil)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(autoSneaky)\
	EASY_CVAR_DECLARATION_SERVER(sv_longjump_chargemode)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(cheat_touchNeverExplodes)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(gaussRecoilSendsUpInSP)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(drawDebugPathfinding)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(STUcheckDistH)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(STUcheckDistV)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(STUcheckDistD)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(STUextraTriangH)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(STUextraTriangV)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(STUrepelMulti)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(STUSpeedMulti)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(STUExplodeTest)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(STUYawSpeedMulti)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(STUDetection)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(stukaAdvancedCombat)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(drawDebugPathfinding2)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hassaultFriendlyFire)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(myStrobe)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(peopleStrobe)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(forceWorldLightOff)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(wildHeads)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(raveEffectSpawnInterval)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(drawBarnacleDebug)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(fogTest)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(sparksAllMulti)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(sparksEnvMulti)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(sparksButtonMulti)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(sparksPlayerCrossbowMulti)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(sparksComputerHitMulti)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(sparksTurretDeathMulti)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(sparksOspreyHitMulti)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(sparksExplosionMulti)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(sparksBeamMulti)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(sparksAIFailMulti)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(normalSpeedMulti)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(noclipSpeedMulti)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(jumpForceMulti)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(apacheForceCinBounds)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(apacheBottomBoundAdj)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(apacheInfluence)\
	DUMMY\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(allowAlphaCrosshairWithoutGuns)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(alphaCrosshairBlockedOnFrozen)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hgruntRunAndGunDotMin)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(panthereyeJumpDotTol)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(panthereyePrintout)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(gargantuaPrintout)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(squadmonsterPrintout)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hassaultPrintout)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(barnaclePrintout)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(friendlyPrintout)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(stukaPrintout)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(timedDamageEndlessOnHard)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(mirrorsReflectOnlyNPCs)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(mirrorsDoNotReflectPlayer)\
	EASY_CVAR_DECLARATION_SERVER(r_shadows)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(altSquadRulesRuntime)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hgruntLockStrafeTime)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(pathfindIgnoreIsolatedNodes)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(drawNodeAll)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(drawNodeSpecial)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(drawNodeConnections)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(drawNodeAlternateTime)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(nodeSearchStartVerticalOffset)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(blockChangeLevelTrigger)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(blockMusicTrigger)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(blockMultiTrigger)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(blockTeleportTrigger)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(blockHurtTrigger)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(blockAutosaveTrigger)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hideNodeGraphRebuildNotice)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(barnacleTongueRetractDelay)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST(sv_germancensorship)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(allowGermanModels)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(germanRobotGibs)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(germanRobotBleedsOil)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(germanRobotDamageDecal)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(germanRobotGibsDecal)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonEffectsMode)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonHitCloud)\
	DUMMY\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(handGrenadePickupYieldsOne)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(handGrenadesUseOldBounceSound)\
	DUMMY\
	EASY_CVAR_DECLARATION_SERVER(hud_moveselect_mousewheelsound)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(timedDamageDeathRemoveMode)\
	EASY_CVAR_DECLARATION_SERVER(hud_moveselect_sound)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(barnacleCanGib)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(sentryCanGib)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(miniturretCanGib)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(turretCanGib)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(turretBleedsOil)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(turretDamageDecal)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(turretGibDecal)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(canDropInSinglePlayer)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(timedDamageIgnoresArmor)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(itemBatteryPrerequisite)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(playerExtraPainSoundsMode)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(timedDamageDisableViewPunch)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(batteryDrainsAtDeath)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(batteryDrainsAtAdrenalineMode)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(canTakeLongJump)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(printOutCommonTimables)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(playerBrightLight)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(disablePainPunchAutomatic)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(gargantuaCorpseDeath)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(gargantuaFallSound)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(gargantuaBleeds)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(shrapMode)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(shrapRand)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(shrapRandHeightExtra)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(explosionShrapnelMulti)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(useAlphaSparks)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(emergencyFix)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(timedDamageReviveRemoveMode)\
	DUMMY\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(ospreyIgnoresGruntCount)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(mp5GrenadeInheritsPlayerVelocity)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(crossbowInheritsPlayerVelocity)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(fastHornetsInheritsPlayerVelocity)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(snarkInheritsPlayerVelocity)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(chumtoadInheritsPlayerVelocity)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(weaponPickupPlaysAnyReloadSounds)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(glockUseLastBulletAnim)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerBarnacleVictimViewOffset)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hgruntMovementDeltaCheck)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hiddenMemPrintout)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hassaultExtraMuzzleFlashRadius)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hassaultExtraMuzzleFlashBrightness)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hassaultExtraMuzzleFlashForward)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(leaderlessSquadAllowed)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(nodeConnectionBreakableCheck)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(playerReviveInvincibilityTime)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(playerReviveBuddhaMode)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(playerReviveTimeBlocksTimedDamage)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hassaultBulletDamageMulti)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hassaultBulletsPerShot)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hassaultFireAnimSpeedMulti)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hassaultMeleeAnimSpeedMulti)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hassassinCrossbowReloadSoundDelay)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hgruntStrafeAnimSpeedMulti)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hgruntRunAndGunAnimSpeedMulti)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(cheat_iwantguts)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(nodeDetailPrintout)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(soundAttenuationStuka)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(soundVolumeStuka)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(cineChangelevelFix)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(drawDebugCine)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(cineAllowSequenceOverwrite)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(stukaInflictsBleeding)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(animationKilledBoundsRemoval)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(gargantuaKilledBoundsAssist)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(bullsquidSpitTrajTimeMin)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(bullsquidSpitTrajTimeMax)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(bullsquidSpitTrajDistMin)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(bullsquidSpitTrajDistMax)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(bullsquidSpitGravityMulti)\
	EASY_CVAR_DECLARATION_SERVER(cl_bullsquidspit)\
	EASY_CVAR_DECLARATION_SERVER(cl_bullsquidspitarc)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(bullsquidSpitUseAlphaModel)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(bullsquidSpitUseAlphaEffect)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(bullsquidSpitEffectSpread)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(bullsquidSpitEffectMin)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(bullsquidSpitEffectMax)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(bullsquidSpitEffectHitMin)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(bullsquidSpitEffectHitMax)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(bullsquidSpitEffectSpawn)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(bullsquidSpitEffectHitSpawn)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(bullsquidSpitSpriteScale)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(bullsquidSpitAlphaScale)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(scientistBravery)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(barneyUnholsterTime)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(barneyUnholsterAnimChoice)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(trailTypeTest)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetTrail)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetTrailSolidColor)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetDeathModEasy)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetDeathModMedium)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetDeathModHard)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetZoomPuff)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetSpiral)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetSpeedMulti)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetSpeedDartMulti)\
	EASY_CVAR_DECLARATION_SERVER(cl_rockettrail)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketTrailAlphaInterval)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketTrailAlphaScale)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketSkipIgnite)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(agruntHornetRandomness)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hornetSpiralPeriod)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hornetSpiralAmplitude)\
	EASY_CVAR_DECLARATION_SERVER(cl_hornetspiral)\
	EASY_CVAR_DECLARATION_SERVER(cl_hornettrail)\
	EASY_CVAR_DECLARATION_SERVER(hud_drawammobar)\
	EASY_CVAR_DECLARATION_SERVER(hud_weaponselecthideslower)\
	EASY_CVAR_DECLARATION_SERVER(hud_drawsidebarmode)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST(gauss_mode)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(painFlashDmgMin)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(painFlashDmgExMult)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(painFlashCumulativeMinDrowning)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(painFlashCumulativeMax)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(painFlashDrawOpacityMax)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(painArrowDrawOpacityMin)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(painArrowDrawOpacityMax)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(painFlashFadeMult)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(painArrowFadeMult)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(painFlashArmorBlock)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(painFlashDirTolerance)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(painArrowCumulativeAppearMin)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(painArrowCumulativeDmgJump)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(painFlashPrintouts)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(itemFlashCumulativeJump)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(itemFlashDrawOpacityMin)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(itemFlashDrawOpacityMax)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(itemFlashFadeMult)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(chumtoadPrintout)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(crossbowReloadSoundDelay)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(crossbowFirePlaysReloadSound)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(pathfindFidgetFailTime)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(pathfindPrintout)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(pathfindTopRampFixDistance)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(pathfindTopRampFixDraw)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(iHaveAscended)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(pathfindLooseMapNodes)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(pathfindRampFix)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(chumtoadPlayDeadFoolChance)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(animationFramerateMulti)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(pathfindNodeToleranceMulti)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(friendlyPianoFollowVolume)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(friendlyPianoOtherVolume)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(showtriggers)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(tentacleAlertSound)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(tentacleSwingSound1)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(tentacleSwingSound2)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(playerFollowerMax)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(announcerIsAJerk)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(playerUseDrawDebug)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(playerChumtoadThrowDrawDebug)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(peaceOut)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(drawViewModel)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(drawHUD)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(disablePauseSinglePlayer)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerBulletHitEffectForceServer)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(forceAllowServersideTextureSounds)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerWeaponSpreadMode)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(monsterAIForceFindDistance)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(baseEntityDamagePushNormalMulti)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(baseEntityDamagePushVerticalBoost)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(baseEntityDamagePushVerticalMulti)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(baseEntityDamagePushVerticalMinimum)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelSyncFixPrintouts)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(textureHitSoundPrintouts)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hgruntAllowGrenades)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(scheduleInterruptPrintouts)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(animationPrintouts)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hassaultMeleeAttackEnabled)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(pathfindStumpedWaitTime)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(pathfindStumpedMode)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(pathfindStumpedForgetEnemy)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(pathfindEdgeCheck)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(RadiusDamageDrawDebug)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(AlienRadiationImmunity)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(customLogoSprayMode)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(monsterFadeOutRate)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(playerFadeOutRate)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(drawDebugEnemyLKP)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(trackchangePrintouts)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(trackTrainPrintouts)\
	EASY_CVAR_DECLARATION_SERVER(cl_holster)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerWeaponTracerMode)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(monsterWeaponTracerMode)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(decalTracerExclusivity)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(monsterToPlayerHitgroupSpecial)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(multiplayerCrowbarHitSoundMode)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(healthcolor_fullRedMin)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(healthcolor_brightness)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(healthcolor_yellowMark)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(cl_drawExtraZeros)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(pathfindLargeBoundFix)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(flyerKilledFallingLoop)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(floaterDummy)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(barneyDummy)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(ladderCycleMulti)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(ladderSpeedMulti)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(barnacleGrabNoInterpolation)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hideDamage)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(timedDamage_brightnessMax)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(timedDamage_brightnessMin)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(timedDamage_brightnessCap)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(timedDamage_brightnessFloor)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(timedDamage_flashSpeed)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY_DEBUGONLY(timedDamage_debug)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(wallHealthDoor_closeDelay)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(houndeye_attack_canGib)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(myRocketsAreBarney)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hassassinCrossbowDebug)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(crossbowBoltDirectionAffectedByWater)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(kingpinDebug)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(minimumRespawnDelay)\
	EASY_CVAR_DECLARATION_SERVER(r_glowshell_debug)\
	EASY_CVAR_DECLARATION_SERVER(cl_viewpunch)\
	EASY_CVAR_DECLARATION_SERVER(cl_explosion)\
	EASY_CVAR_DECLARATION_SERVER(soundSentenceSave)\
	EASY_CVAR_DECLARATION_SERVER(pissedNPCs)\
	EASY_CVAR_DECLARATION_SERVER(hud_logo)\
	EASY_CVAR_DECLARATION_SERVER(hud_brokentrans)\
	EASY_CVAR_DECLARATION_SERVER(cl_fvox)\
	EASY_CVAR_DECLARATION_SERVER(cl_ladder)\
	EASY_CVAR_DECLARATION_SERVER(precacheAll)\
	EASY_CVAR_DECLARATION_SERVER(cl_interp_entity)\
	EASY_CVAR_DECLARATION_SERVER(hud_swapFirstTwoBuckets)\
	EASY_CVAR_DECLARATION_SERVER(ignoreMultiplayerSkillOverride)\
	EASY_CVAR_DECLARATION_SERVER(m_rawinput)\
	EASY_CVAR_DECLARATION_SERVER(cl_earlyaccess)\
	EASY_CVAR_DECLARATION_SERVER(cl_viewroll)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(drawDebugCrowbar)\
	EASY_CVAR_DECLARATION_SERVER(blastExtraArmorDamageMode)\
	EASY_CVAR_DECLARATION_SERVER(hud_batteryhiddendead)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(hyperBarney)\
	EASY_CVAR_DECLARATION_SERVER(monsterKilledToss)\
	EASY_CVAR_DECLARATION_SERVER(interpolation_movetypestep_mindelta)\
	EASY_CVAR_DECLARATION_SERVER(sv_bloodparticlemode)\
	EASY_CVAR_DECLARATION_SERVER(cl_interp_view_extra)\
	EASY_CVAR_DECLARATION_SERVER(cl_interp_view_standard)\
	EASY_CVAR_DECLARATION_SERVER(cl_interp_viewmodel)\
	EASY_CVAR_DECLARATION_SERVER(sv_explosionknockback)\
	EASY_CVAR_DECLARATION_SERVER(cl_gaussfollowattachment)\
	EASY_CVAR_DECLARATION_SERVER(cl_mp5_viewpunch_mod)\
	EASY_CVAR_DECLARATION_SERVER(cl_breakholster)\
	EASY_CVAR_DECLARATION_SERVER(pausecorrection1)\
	EASY_CVAR_DECLARATION_SERVER(pausecorrection2)\
	EASY_CVAR_DECLARATION_SERVER(hud_rpg_clipless)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST(sv_rpg_clipless)\
	EASY_CVAR_DECLARATION_SERVER(cl_viewmodel_fidget)\
	EASY_CVAR_DECLARATION_SERVER(sv_player_midair_fix)\
	EASY_CVAR_DECLARATION_SERVER(sv_player_midair_accel)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(pathfindIgnoreNearestNodeCache)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(pathfindIgnoreStaticRoutes)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(pathfindForcePointHull)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(pathfindNearestNodeExtra)\
	EASY_CVAR_DECLARATION_SERVER(sv_turret_postdeath)\
	EASY_CVAR_DECLARATION_SERVER(cl_mp5_evil_skip)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonRadiusDamageMode)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonFireRateMode)\
	EASY_CVAR_DECLARATION_SERVER(sv_explosion_shake)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(sv_explosion_shake_range)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(sv_explosion_shake_amp)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(sv_explosion_shake_freq)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(sv_explosion_shake_duration)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(sv_gargantua_throwattack)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY(cl_viewpunch_mod)\
	EASY_CVAR_DECLARATION_SERVER_CLIENTONLY(cl_gauss_viewpunch_mod)\
	EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(pathfindMonsterclipFreshLogic)\
	EASY_CVAR_DECLARATION_SERVER(sv_rpg_projectile_model)\
	EASY_CVAR_DECLARATION_SERVER(hmilitaryDeadInvestigate)\
	EASY_CVAR_DECLARATION_SERVER(playerDeadTruce)\
	EASY_CVAR_DECLARATION_SERVER(playerDeadTalkerBehavior)\
	DUMMY






#define EASY_CVAR_DECLARATION_CLIENT_MASS\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(gruntsCanHaveMP5Grenade)\
	EASY_CVAR_DECLARATION_CLIENT(hud_version)\
	EASY_CVAR_DECLARATION_CLIENT(hud_batterydraw)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(strobeDurationMin)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(strobeDurationMax)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(strobeRadiusMin)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(strobeRadiusMax)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(strobeSpawnDistHori)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(strobeSpawnDistVertMin)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(strobeSpawnDistVertMax)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(strobeMultiColor)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserEnabled)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserSpawnFreq)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserLength)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMin)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMax)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMin)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMax)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserBrightnessMin)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserBrightnessMax)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserDurationMin)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserDurationMax)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserThicknessMin)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserThicknessMax)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserNoiseMin)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserNoiseMax)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserFrameRateMin)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserFrameRateMax)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserMultiColor)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(cameraPosFixedX)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(cameraPosFixedY)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(cameraPosFixedZ)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(cameraPosOffX)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(cameraPosOffY)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(cameraPosOffZ)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(cameraRotFixedX)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(cameraRotFixedY)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(cameraRotFixedZ)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(cameraRotOffX)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(cameraRotOffY)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(cameraRotOffZ)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(imAllFuckedUp)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(thatWasntGrass)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(fogNear)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(fogFar)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(NPCsTalkMore)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(myCameraSucks)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(shutupstuka)\
	EASY_CVAR_DECLARATION_CLIENT(chromeEffect)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hassaultSpinMovement)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hassaultIdleSpinSound)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hassaultFireSound)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteBulletHitSounds)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(mutePlayerPainSounds)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hassaultIdleSpinSoundChannel)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hassaultSpinUpDownSoundChannel)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hassaultFireSoundChannel)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(geigerChannel)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hassaultWaitTime)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hassaultSpinupRemainTime)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hassaultResidualAttackTime)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hassaultSpinupStartTime)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hassaultVoicePitchMin)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hassaultVoicePitchMax)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hassaultFireSpread)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hassaultAllowGrenades)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(muteTempEntityGroundHitSound)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(houndeyeAttackMode)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteRicochetSound)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(mutePlayerWeaponFire)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteCrowbarSounds)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(scientistHealNPC)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(scientistHealNPCDebug)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(bulletholeAlertRange)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(fleshhitmakessound)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(nothingHurts)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(seeMonsterHealth)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(scientistHealNPCFract)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(bullsquidRangeDisabled)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(applyLKPPathFixToAll)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(timedDamageAffectsMonsters)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(scientistHealCooldown)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(crazyMonsterPrintouts)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(movementIsCompletePrintout)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(bulletHoleAlertPrintout)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(bulletholeAlertStukaOnly)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(barneyPrintouts)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(monsterSpawnPrintout)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(zombieBulletResistance)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(zombieBulletPushback)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(houndeyePrintout)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(quakeExplosionSound)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(explosionDebrisSoundVolume)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(noFlinchOnHard)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hassaultDrawLKP)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(meleeDrawBloodModeA)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(meleeDrawBloodModeB)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(drawDebugBloodTrace)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(meleeDrawBloodModeBFix)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(meleeDrawBloodModeAOffset)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(meleeDrawBloodModeBOffset)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST(wpn_glocksilencer)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(panthereyeHasCloakingAbility)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hgruntSpeedMulti)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hgruntForceStrafeFireAnim)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hgruntLockRunAndGunTime)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hgruntHeadshotGore)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hgruntAllowStrafeFire)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(thoroughHitBoxUpdates)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hgruntTinyClip)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hgruntStrafeAlwaysHasAmmo)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hgruntBrassEjectForwardOffset)\
	EASY_CVAR_DECLARATION_CLIENT(cl_muzzleflash)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(agrunt_muzzleflash)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(event5011Allowed)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(event5021Allowed)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(event5031Allowed)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(event5002Allowed)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(event5004Allowed)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(eventsAreFabulous)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(glockOldReloadLogic)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(glockOldReloadLogicBarney)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(barneyDroppedGlockAmmoCap)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(drawCollisionBoundsAtDeath)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(drawHitBoundsAtDeath)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(islaveReviveFriendMode)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(islaveReviveFriendChance)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(islaveReviveFriendRange)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(islaveReviveSelfMinDelay)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(islaveReviveSelfMaxDelay)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(islaveReviveSelfChance)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hgruntRunAndGunDistance)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hgruntPrintout)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(painArrowColorMode)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(painFlashColorMode)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(painFlashSuitless)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(painFlashDrownMode)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(firstPersonIdleDelayMin)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(firstPersonIdleDelayMax)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(forceDrawBatteryNumber)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(offsetgivedistance)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(offsetgivelookvertical)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(canShowWeaponSelectAtDeath)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(endlessFlashlightBattery)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(preE3UsesFailColors)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(E3UsesFailColors)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(preE3ShowsDamageIcons)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(E3ShowsDamageIcons)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerCrossbowMode)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hassassinCrossbowMode)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(tripmineAnimWaitsForFinish)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(revolverLaserScope)\
	DUMMY\
	DUMMY\
	DUMMY\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY(auto_adjust_fov)\
	DUMMY\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_nogaussrecoil)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(autoSneaky)\
	EASY_CVAR_DECLARATION_CLIENT(sv_longjump_chargemode)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(cheat_touchNeverExplodes)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(gaussRecoilSendsUpInSP)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(drawDebugPathfinding)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(STUcheckDistH)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(STUcheckDistV)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(STUcheckDistD)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(STUextraTriangH)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(STUextraTriangV)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(STUrepelMulti)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(STUSpeedMulti)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(STUExplodeTest)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(STUYawSpeedMulti)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(STUDetection)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(stukaAdvancedCombat)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(drawDebugPathfinding2)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hassaultFriendlyFire)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(myStrobe)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(peopleStrobe)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(forceWorldLightOff)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(wildHeads)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(raveEffectSpawnInterval)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(drawBarnacleDebug)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(fogTest)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(sparksAllMulti)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(sparksEnvMulti)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(sparksButtonMulti)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(sparksPlayerCrossbowMulti)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(sparksComputerHitMulti)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(sparksTurretDeathMulti)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(sparksOspreyHitMulti)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(sparksExplosionMulti)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(sparksBeamMulti)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(sparksAIFailMulti)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(normalSpeedMulti)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(noclipSpeedMulti)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(jumpForceMulti)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(apacheForceCinBounds)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(apacheBottomBoundAdj)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(apacheInfluence)\
	DUMMY\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(allowAlphaCrosshairWithoutGuns)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(alphaCrosshairBlockedOnFrozen)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hgruntRunAndGunDotMin)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(panthereyeJumpDotTol)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(panthereyePrintout)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(gargantuaPrintout)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(squadmonsterPrintout)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hassaultPrintout)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(barnaclePrintout)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(friendlyPrintout)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(stukaPrintout)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(timedDamageEndlessOnHard)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(mirrorsReflectOnlyNPCs)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(mirrorsDoNotReflectPlayer)\
	EASY_CVAR_DECLARATION_CLIENT(r_shadows)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(altSquadRulesRuntime)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hgruntLockStrafeTime)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(pathfindIgnoreIsolatedNodes)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(drawNodeAll)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(drawNodeSpecial)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(drawNodeConnections)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(drawNodeAlternateTime)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(nodeSearchStartVerticalOffset)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(blockChangeLevelTrigger)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(blockMusicTrigger)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(blockMultiTrigger)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(blockTeleportTrigger)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(blockHurtTrigger)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(blockAutosaveTrigger)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hideNodeGraphRebuildNotice)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(barnacleTongueRetractDelay)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST(sv_germancensorship)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(allowGermanModels)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(germanRobotGibs)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(germanRobotBleedsOil)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(germanRobotDamageDecal)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(germanRobotGibsDecal)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonEffectsMode)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonHitCloud)\
	DUMMY\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(handGrenadePickupYieldsOne)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(handGrenadesUseOldBounceSound)\
	DUMMY\
	EASY_CVAR_DECLARATION_CLIENT(hud_moveselect_mousewheelsound)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(timedDamageDeathRemoveMode)\
	EASY_CVAR_DECLARATION_CLIENT(hud_moveselect_sound)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(barnacleCanGib)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(sentryCanGib)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(miniturretCanGib)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(turretCanGib)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(turretBleedsOil)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(turretDamageDecal)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(turretGibDecal)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(canDropInSinglePlayer)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(timedDamageIgnoresArmor)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(itemBatteryPrerequisite)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(playerExtraPainSoundsMode)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(timedDamageDisableViewPunch)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(batteryDrainsAtDeath)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(batteryDrainsAtAdrenalineMode)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(canTakeLongJump)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(printOutCommonTimables)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(playerBrightLight)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(disablePainPunchAutomatic)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(gargantuaCorpseDeath)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(gargantuaFallSound)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(gargantuaBleeds)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(shrapMode)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(shrapRand)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(shrapRandHeightExtra)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(explosionShrapnelMulti)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(useAlphaSparks)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(emergencyFix)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(timedDamageReviveRemoveMode)\
	DUMMY\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(ospreyIgnoresGruntCount)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(mp5GrenadeInheritsPlayerVelocity)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(crossbowInheritsPlayerVelocity)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(fastHornetsInheritsPlayerVelocity)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(snarkInheritsPlayerVelocity)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(chumtoadInheritsPlayerVelocity)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(weaponPickupPlaysAnyReloadSounds)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(glockUseLastBulletAnim)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerBarnacleVictimViewOffset)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hgruntMovementDeltaCheck)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hiddenMemPrintout)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hassaultExtraMuzzleFlashRadius)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hassaultExtraMuzzleFlashBrightness)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hassaultExtraMuzzleFlashForward)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(leaderlessSquadAllowed)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(nodeConnectionBreakableCheck)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(playerReviveInvincibilityTime)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(playerReviveBuddhaMode)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(playerReviveTimeBlocksTimedDamage)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hassaultBulletDamageMulti)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hassaultBulletsPerShot)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hassaultFireAnimSpeedMulti)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hassaultMeleeAnimSpeedMulti)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hassassinCrossbowReloadSoundDelay)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hgruntStrafeAnimSpeedMulti)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hgruntRunAndGunAnimSpeedMulti)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(cheat_iwantguts)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(nodeDetailPrintout)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(soundAttenuationStuka)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(soundVolumeStuka)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(cineChangelevelFix)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(drawDebugCine)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(cineAllowSequenceOverwrite)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(stukaInflictsBleeding)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(animationKilledBoundsRemoval)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(gargantuaKilledBoundsAssist)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(bullsquidSpitTrajTimeMin)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(bullsquidSpitTrajTimeMax)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(bullsquidSpitTrajDistMin)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(bullsquidSpitTrajDistMax)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(bullsquidSpitGravityMulti)\
	EASY_CVAR_DECLARATION_CLIENT(cl_bullsquidspit)\
	EASY_CVAR_DECLARATION_CLIENT(cl_bullsquidspitarc)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(bullsquidSpitUseAlphaModel)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(bullsquidSpitUseAlphaEffect)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(bullsquidSpitEffectSpread)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(bullsquidSpitEffectMin)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(bullsquidSpitEffectMax)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(bullsquidSpitEffectHitMin)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(bullsquidSpitEffectHitMax)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(bullsquidSpitEffectSpawn)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(bullsquidSpitEffectHitSpawn)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(bullsquidSpitSpriteScale)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(bullsquidSpitAlphaScale)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(scientistBravery)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(barneyUnholsterTime)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(barneyUnholsterAnimChoice)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(trailTypeTest)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetTrail)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetTrailSolidColor)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetDeathModEasy)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetDeathModMedium)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetDeathModHard)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetZoomPuff)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetSpiral)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetSpeedMulti)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetSpeedDartMulti)\
	EASY_CVAR_DECLARATION_CLIENT(cl_rockettrail)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketTrailAlphaInterval)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketTrailAlphaScale)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketSkipIgnite)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(agruntHornetRandomness)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hornetSpiralPeriod)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hornetSpiralAmplitude)\
	EASY_CVAR_DECLARATION_CLIENT(cl_hornetspiral)\
	EASY_CVAR_DECLARATION_CLIENT(cl_hornettrail)\
	EASY_CVAR_DECLARATION_CLIENT(hud_drawammobar)\
	EASY_CVAR_DECLARATION_CLIENT(hud_weaponselecthideslower)\
	EASY_CVAR_DECLARATION_CLIENT(hud_drawsidebarmode)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST(gauss_mode)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(painFlashDmgMin)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(painFlashDmgExMult)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(painFlashCumulativeMinDrowning)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(painFlashCumulativeMax)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(painFlashDrawOpacityMax)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(painArrowDrawOpacityMin)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(painArrowDrawOpacityMax)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(painFlashFadeMult)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(painArrowFadeMult)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(painFlashArmorBlock)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(painFlashDirTolerance)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(painArrowCumulativeAppearMin)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(painArrowCumulativeDmgJump)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(painFlashPrintouts)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(itemFlashCumulativeJump)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(itemFlashDrawOpacityMin)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(itemFlashDrawOpacityMax)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(itemFlashFadeMult)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(chumtoadPrintout)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(crossbowReloadSoundDelay)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(crossbowFirePlaysReloadSound)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(pathfindFidgetFailTime)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(pathfindPrintout)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(pathfindTopRampFixDistance)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(pathfindTopRampFixDraw)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(iHaveAscended)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(pathfindLooseMapNodes)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(pathfindRampFix)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(chumtoadPlayDeadFoolChance)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(animationFramerateMulti)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(pathfindNodeToleranceMulti)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(friendlyPianoFollowVolume)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(friendlyPianoOtherVolume)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(showtriggers)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(tentacleAlertSound)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(tentacleSwingSound1)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(tentacleSwingSound2)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(playerFollowerMax)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(announcerIsAJerk)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(playerUseDrawDebug)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(playerChumtoadThrowDrawDebug)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(peaceOut)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(drawViewModel)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(drawHUD)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(disablePauseSinglePlayer)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerBulletHitEffectForceServer)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(forceAllowServersideTextureSounds)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerWeaponSpreadMode)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(monsterAIForceFindDistance)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(baseEntityDamagePushNormalMulti)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(baseEntityDamagePushVerticalBoost)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(baseEntityDamagePushVerticalMulti)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(baseEntityDamagePushVerticalMinimum)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelSyncFixPrintouts)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(textureHitSoundPrintouts)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hgruntAllowGrenades)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(scheduleInterruptPrintouts)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(animationPrintouts)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hassaultMeleeAttackEnabled)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(pathfindStumpedWaitTime)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(pathfindStumpedMode)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(pathfindStumpedForgetEnemy)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(pathfindEdgeCheck)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(RadiusDamageDrawDebug)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(AlienRadiationImmunity)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(customLogoSprayMode)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(monsterFadeOutRate)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(playerFadeOutRate)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(drawDebugEnemyLKP)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(trackchangePrintouts)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(trackTrainPrintouts)\
	EASY_CVAR_DECLARATION_CLIENT(cl_holster)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerWeaponTracerMode)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(monsterWeaponTracerMode)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(decalTracerExclusivity)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(monsterToPlayerHitgroupSpecial)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(multiplayerCrowbarHitSoundMode)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(healthcolor_fullRedMin)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(healthcolor_brightness)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(healthcolor_yellowMark)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(cl_drawExtraZeros)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(pathfindLargeBoundFix)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(flyerKilledFallingLoop)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(floaterDummy)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(barneyDummy)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(ladderCycleMulti)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(ladderSpeedMulti)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(barnacleGrabNoInterpolation)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hideDamage)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(timedDamage_brightnessMax)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(timedDamage_brightnessMin)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(timedDamage_brightnessCap)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(timedDamage_brightnessFloor)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(timedDamage_flashSpeed)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY_DEBUGONLY(timedDamage_debug)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(wallHealthDoor_closeDelay)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(houndeye_attack_canGib)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(myRocketsAreBarney)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hassassinCrossbowDebug)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(crossbowBoltDirectionAffectedByWater)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(kingpinDebug)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(minimumRespawnDelay)\
	EASY_CVAR_DECLARATION_CLIENT(r_glowshell_debug)\
	EASY_CVAR_DECLARATION_CLIENT(cl_viewpunch)\
	EASY_CVAR_DECLARATION_CLIENT(cl_explosion)\
	EASY_CVAR_DECLARATION_CLIENT(soundSentenceSave)\
	EASY_CVAR_DECLARATION_CLIENT(pissedNPCs)\
	EASY_CVAR_DECLARATION_CLIENT(hud_logo)\
	EASY_CVAR_DECLARATION_CLIENT(hud_brokentrans)\
	EASY_CVAR_DECLARATION_CLIENT(cl_fvox)\
	EASY_CVAR_DECLARATION_CLIENT(cl_ladder)\
	EASY_CVAR_DECLARATION_CLIENT(precacheAll)\
	EASY_CVAR_DECLARATION_CLIENT(cl_interp_entity)\
	EASY_CVAR_DECLARATION_CLIENT(hud_swapFirstTwoBuckets)\
	EASY_CVAR_DECLARATION_CLIENT(ignoreMultiplayerSkillOverride)\
	EASY_CVAR_DECLARATION_CLIENT(m_rawinput)\
	EASY_CVAR_DECLARATION_CLIENT(cl_earlyaccess)\
	EASY_CVAR_DECLARATION_CLIENT(cl_viewroll)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(drawDebugCrowbar)\
	EASY_CVAR_DECLARATION_CLIENT(blastExtraArmorDamageMode)\
	EASY_CVAR_DECLARATION_CLIENT(hud_batteryhiddendead)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(hyperBarney)\
	EASY_CVAR_DECLARATION_CLIENT(monsterKilledToss)\
	EASY_CVAR_DECLARATION_CLIENT(interpolation_movetypestep_mindelta)\
	EASY_CVAR_DECLARATION_CLIENT(sv_bloodparticlemode)\
	EASY_CVAR_DECLARATION_CLIENT(cl_interp_view_extra)\
	EASY_CVAR_DECLARATION_CLIENT(cl_interp_view_standard)\
	EASY_CVAR_DECLARATION_CLIENT(cl_interp_viewmodel)\
	EASY_CVAR_DECLARATION_CLIENT(sv_explosionknockback)\
	EASY_CVAR_DECLARATION_CLIENT(cl_gaussfollowattachment)\
	EASY_CVAR_DECLARATION_CLIENT(cl_mp5_viewpunch_mod)\
	EASY_CVAR_DECLARATION_CLIENT(cl_breakholster)\
	EASY_CVAR_DECLARATION_CLIENT(pausecorrection1)\
	EASY_CVAR_DECLARATION_CLIENT(pausecorrection2)\
	EASY_CVAR_DECLARATION_CLIENT(hud_rpg_clipless)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST(sv_rpg_clipless)\
	EASY_CVAR_DECLARATION_CLIENT(cl_viewmodel_fidget)\
	EASY_CVAR_DECLARATION_CLIENT(sv_player_midair_fix)\
	EASY_CVAR_DECLARATION_CLIENT(sv_player_midair_accel)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(pathfindIgnoreNearestNodeCache)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(pathfindIgnoreStaticRoutes)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(pathfindForcePointHull)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(pathfindNearestNodeExtra)\
	EASY_CVAR_DECLARATION_CLIENT(sv_turret_postdeath)\
	EASY_CVAR_DECLARATION_CLIENT(cl_mp5_evil_skip)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonRadiusDamageMode)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonFireRateMode)\
	EASY_CVAR_DECLARATION_CLIENT(sv_explosion_shake)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(sv_explosion_shake_range)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(sv_explosion_shake_amp)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(sv_explosion_shake_freq)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(sv_explosion_shake_duration)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(sv_gargantua_throwattack)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY(cl_viewpunch_mod)\
	EASY_CVAR_DECLARATION_CLIENT_CLIENTONLY(cl_gauss_viewpunch_mod)\
	EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(pathfindMonsterclipFreshLogic)\
	EASY_CVAR_DECLARATION_CLIENT(sv_rpg_projectile_model)\
	EASY_CVAR_DECLARATION_CLIENT(hmilitaryDeadInvestigate)\
	EASY_CVAR_DECLARATION_CLIENT(playerDeadTruce)\
	EASY_CVAR_DECLARATION_CLIENT(playerDeadTalkerBehavior)\
	DUMMY



#define EASY_CVAR_UPDATE_SERVER_MASS\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(gruntsCanHaveMP5Grenade)\
	EASY_CVAR_UPDATE_SERVER(hud_version)\
	EASY_CVAR_UPDATE_SERVER(hud_batterydraw)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(strobeDurationMin)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(strobeDurationMax)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(strobeRadiusMin)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(strobeRadiusMax)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(strobeSpawnDistHori)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(strobeSpawnDistVertMin)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(strobeSpawnDistVertMax)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(strobeMultiColor)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserEnabled)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserSpawnFreq)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserLength)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMin)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMax)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMin)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMax)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserBrightnessMin)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserBrightnessMax)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserDurationMin)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserDurationMax)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserThicknessMin)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserThicknessMax)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserNoiseMin)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserNoiseMax)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserFrameRateMin)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserFrameRateMax)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserMultiColor)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(cameraPosFixedX)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(cameraPosFixedY)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(cameraPosFixedZ)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(cameraPosOffX)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(cameraPosOffY)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(cameraPosOffZ)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(cameraRotFixedX)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(cameraRotFixedY)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(cameraRotFixedZ)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(cameraRotOffX)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(cameraRotOffY)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(cameraRotOffZ)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(imAllFuckedUp)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(thatWasntGrass)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(fogNear)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(fogFar)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(NPCsTalkMore)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(myCameraSucks)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(shutupstuka)\
	EASY_CVAR_UPDATE_SERVER(chromeEffect)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hassaultSpinMovement)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hassaultIdleSpinSound)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hassaultFireSound)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteBulletHitSounds)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(mutePlayerPainSounds)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hassaultIdleSpinSoundChannel)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hassaultSpinUpDownSoundChannel)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hassaultFireSoundChannel)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(geigerChannel)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hassaultWaitTime)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hassaultSpinupRemainTime)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hassaultResidualAttackTime)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hassaultSpinupStartTime)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hassaultVoicePitchMin)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hassaultVoicePitchMax)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hassaultFireSpread)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hassaultAllowGrenades)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(muteTempEntityGroundHitSound)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(houndeyeAttackMode)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteRicochetSound)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(mutePlayerWeaponFire)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteCrowbarSounds)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(scientistHealNPC)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(scientistHealNPCDebug)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(bulletholeAlertRange)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(fleshhitmakessound)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(nothingHurts)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(seeMonsterHealth)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(scientistHealNPCFract)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(bullsquidRangeDisabled)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(applyLKPPathFixToAll)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(timedDamageAffectsMonsters)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(scientistHealCooldown)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(crazyMonsterPrintouts)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(movementIsCompletePrintout)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(bulletHoleAlertPrintout)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(bulletholeAlertStukaOnly)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(barneyPrintouts)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(monsterSpawnPrintout)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(zombieBulletResistance)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(zombieBulletPushback)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(houndeyePrintout)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(quakeExplosionSound)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(explosionDebrisSoundVolume)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(noFlinchOnHard)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hassaultDrawLKP)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(meleeDrawBloodModeA)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(meleeDrawBloodModeB)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(drawDebugBloodTrace)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(meleeDrawBloodModeBFix)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(meleeDrawBloodModeAOffset)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(meleeDrawBloodModeBOffset)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST(wpn_glocksilencer)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(panthereyeHasCloakingAbility)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hgruntSpeedMulti)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hgruntForceStrafeFireAnim)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hgruntLockRunAndGunTime)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hgruntHeadshotGore)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hgruntAllowStrafeFire)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(thoroughHitBoxUpdates)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hgruntTinyClip)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hgruntStrafeAlwaysHasAmmo)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hgruntBrassEjectForwardOffset)\
	EASY_CVAR_UPDATE_SERVER(cl_muzzleflash)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(agrunt_muzzleflash)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(event5011Allowed)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(event5021Allowed)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(event5031Allowed)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(event5002Allowed)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(event5004Allowed)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(eventsAreFabulous)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(glockOldReloadLogic)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(glockOldReloadLogicBarney)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(barneyDroppedGlockAmmoCap)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(drawCollisionBoundsAtDeath)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(drawHitBoundsAtDeath)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(islaveReviveFriendMode)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(islaveReviveFriendChance)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(islaveReviveFriendRange)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(islaveReviveSelfMinDelay)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(islaveReviveSelfMaxDelay)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(islaveReviveSelfChance)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hgruntRunAndGunDistance)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hgruntPrintout)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(painArrowColorMode)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(painFlashColorMode)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(painFlashSuitless)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(painFlashDrownMode)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(firstPersonIdleDelayMin)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(firstPersonIdleDelayMax)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(forceDrawBatteryNumber)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(offsetgivedistance)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(offsetgivelookvertical)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(canShowWeaponSelectAtDeath)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(endlessFlashlightBattery)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(preE3UsesFailColors)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(E3UsesFailColors)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(preE3ShowsDamageIcons)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(E3ShowsDamageIcons)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerCrossbowMode)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hassassinCrossbowMode)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(tripmineAnimWaitsForFinish)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(revolverLaserScope)\
	DUMMY\
	DUMMY\
	DUMMY\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY(auto_adjust_fov)\
	DUMMY\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_nogaussrecoil)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(autoSneaky)\
	EASY_CVAR_UPDATE_SERVER(sv_longjump_chargemode)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(cheat_touchNeverExplodes)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(gaussRecoilSendsUpInSP)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(drawDebugPathfinding)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(STUcheckDistH)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(STUcheckDistV)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(STUcheckDistD)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(STUextraTriangH)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(STUextraTriangV)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(STUrepelMulti)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(STUSpeedMulti)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(STUExplodeTest)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(STUYawSpeedMulti)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(STUDetection)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(stukaAdvancedCombat)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(drawDebugPathfinding2)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hassaultFriendlyFire)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(myStrobe)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(peopleStrobe)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(forceWorldLightOff)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(wildHeads)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(raveEffectSpawnInterval)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(drawBarnacleDebug)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(fogTest)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(sparksAllMulti)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(sparksEnvMulti)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(sparksButtonMulti)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(sparksPlayerCrossbowMulti)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(sparksComputerHitMulti)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(sparksTurretDeathMulti)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(sparksOspreyHitMulti)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(sparksExplosionMulti)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(sparksBeamMulti)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(sparksAIFailMulti)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(normalSpeedMulti)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(noclipSpeedMulti)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(jumpForceMulti)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(apacheForceCinBounds)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(apacheBottomBoundAdj)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(apacheInfluence)\
	DUMMY\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(allowAlphaCrosshairWithoutGuns)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(alphaCrosshairBlockedOnFrozen)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hgruntRunAndGunDotMin)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(panthereyeJumpDotTol)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(panthereyePrintout)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(gargantuaPrintout)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(squadmonsterPrintout)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hassaultPrintout)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(barnaclePrintout)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(friendlyPrintout)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(stukaPrintout)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(timedDamageEndlessOnHard)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(mirrorsReflectOnlyNPCs)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(mirrorsDoNotReflectPlayer)\
	EASY_CVAR_UPDATE_SERVER(r_shadows)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(altSquadRulesRuntime)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hgruntLockStrafeTime)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(pathfindIgnoreIsolatedNodes)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(drawNodeAll)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(drawNodeSpecial)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(drawNodeConnections)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(drawNodeAlternateTime)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(nodeSearchStartVerticalOffset)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(blockChangeLevelTrigger)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(blockMusicTrigger)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(blockMultiTrigger)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(blockTeleportTrigger)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(blockHurtTrigger)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(blockAutosaveTrigger)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hideNodeGraphRebuildNotice)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(barnacleTongueRetractDelay)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST(sv_germancensorship)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(allowGermanModels)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(germanRobotGibs)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(germanRobotBleedsOil)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(germanRobotDamageDecal)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(germanRobotGibsDecal)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonEffectsMode)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonHitCloud)\
	DUMMY\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(handGrenadePickupYieldsOne)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(handGrenadesUseOldBounceSound)\
	DUMMY\
	EASY_CVAR_UPDATE_SERVER(hud_moveselect_mousewheelsound)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(timedDamageDeathRemoveMode)\
	EASY_CVAR_UPDATE_SERVER(hud_moveselect_sound)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(barnacleCanGib)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(sentryCanGib)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(miniturretCanGib)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(turretCanGib)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(turretBleedsOil)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(turretDamageDecal)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(turretGibDecal)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(canDropInSinglePlayer)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(timedDamageIgnoresArmor)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(itemBatteryPrerequisite)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(playerExtraPainSoundsMode)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(timedDamageDisableViewPunch)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(batteryDrainsAtDeath)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(batteryDrainsAtAdrenalineMode)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(canTakeLongJump)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(printOutCommonTimables)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(playerBrightLight)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(disablePainPunchAutomatic)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(gargantuaCorpseDeath)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(gargantuaFallSound)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(gargantuaBleeds)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(shrapMode)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(shrapRand)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(shrapRandHeightExtra)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(explosionShrapnelMulti)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(useAlphaSparks)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(emergencyFix)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(timedDamageReviveRemoveMode)\
	DUMMY\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(ospreyIgnoresGruntCount)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(mp5GrenadeInheritsPlayerVelocity)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(crossbowInheritsPlayerVelocity)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(fastHornetsInheritsPlayerVelocity)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(snarkInheritsPlayerVelocity)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(chumtoadInheritsPlayerVelocity)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(weaponPickupPlaysAnyReloadSounds)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(glockUseLastBulletAnim)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerBarnacleVictimViewOffset)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hgruntMovementDeltaCheck)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hiddenMemPrintout)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hassaultExtraMuzzleFlashRadius)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hassaultExtraMuzzleFlashBrightness)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hassaultExtraMuzzleFlashForward)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(leaderlessSquadAllowed)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(nodeConnectionBreakableCheck)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(playerReviveInvincibilityTime)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(playerReviveBuddhaMode)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(playerReviveTimeBlocksTimedDamage)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hassaultBulletDamageMulti)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hassaultBulletsPerShot)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hassaultFireAnimSpeedMulti)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hassaultMeleeAnimSpeedMulti)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hassassinCrossbowReloadSoundDelay)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hgruntStrafeAnimSpeedMulti)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hgruntRunAndGunAnimSpeedMulti)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(cheat_iwantguts)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(nodeDetailPrintout)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(soundAttenuationStuka)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(soundVolumeStuka)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(cineChangelevelFix)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(drawDebugCine)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(cineAllowSequenceOverwrite)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(stukaInflictsBleeding)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(animationKilledBoundsRemoval)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(gargantuaKilledBoundsAssist)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(bullsquidSpitTrajTimeMin)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(bullsquidSpitTrajTimeMax)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(bullsquidSpitTrajDistMin)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(bullsquidSpitTrajDistMax)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(bullsquidSpitGravityMulti)\
	EASY_CVAR_UPDATE_SERVER(cl_bullsquidspit)\
	EASY_CVAR_UPDATE_SERVER(cl_bullsquidspitarc)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(bullsquidSpitUseAlphaModel)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(bullsquidSpitUseAlphaEffect)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(bullsquidSpitEffectSpread)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(bullsquidSpitEffectMin)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(bullsquidSpitEffectMax)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(bullsquidSpitEffectHitMin)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(bullsquidSpitEffectHitMax)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(bullsquidSpitEffectSpawn)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(bullsquidSpitEffectHitSpawn)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(bullsquidSpitSpriteScale)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(bullsquidSpitAlphaScale)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(scientistBravery)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(barneyUnholsterTime)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(barneyUnholsterAnimChoice)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(trailTypeTest)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetTrail)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetTrailSolidColor)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetDeathModEasy)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetDeathModMedium)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetDeathModHard)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetZoomPuff)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetSpiral)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetSpeedMulti)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetSpeedDartMulti)\
	EASY_CVAR_UPDATE_SERVER(cl_rockettrail)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketTrailAlphaInterval)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketTrailAlphaScale)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketSkipIgnite)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(agruntHornetRandomness)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hornetSpiralPeriod)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hornetSpiralAmplitude)\
	EASY_CVAR_UPDATE_SERVER(cl_hornetspiral)\
	EASY_CVAR_UPDATE_SERVER(cl_hornettrail)\
	EASY_CVAR_UPDATE_SERVER(hud_drawammobar)\
	EASY_CVAR_UPDATE_SERVER(hud_weaponselecthideslower)\
	EASY_CVAR_UPDATE_SERVER(hud_drawsidebarmode)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST(gauss_mode)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(painFlashDmgMin)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(painFlashDmgExMult)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(painFlashCumulativeMinDrowning)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(painFlashCumulativeMax)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(painFlashDrawOpacityMax)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(painArrowDrawOpacityMin)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(painArrowDrawOpacityMax)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(painFlashFadeMult)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(painArrowFadeMult)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(painFlashArmorBlock)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(painFlashDirTolerance)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(painArrowCumulativeAppearMin)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(painArrowCumulativeDmgJump)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(painFlashPrintouts)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(itemFlashCumulativeJump)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(itemFlashDrawOpacityMin)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(itemFlashDrawOpacityMax)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(itemFlashFadeMult)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(chumtoadPrintout)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(crossbowReloadSoundDelay)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(crossbowFirePlaysReloadSound)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(pathfindFidgetFailTime)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(pathfindPrintout)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(pathfindTopRampFixDistance)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(pathfindTopRampFixDraw)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(iHaveAscended)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(pathfindLooseMapNodes)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(pathfindRampFix)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(chumtoadPlayDeadFoolChance)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(animationFramerateMulti)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(pathfindNodeToleranceMulti)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(friendlyPianoFollowVolume)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(friendlyPianoOtherVolume)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(showtriggers)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(tentacleAlertSound)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(tentacleSwingSound1)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(tentacleSwingSound2)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(playerFollowerMax)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(announcerIsAJerk)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(playerUseDrawDebug)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(playerChumtoadThrowDrawDebug)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(peaceOut)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(drawViewModel)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(drawHUD)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(disablePauseSinglePlayer)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerBulletHitEffectForceServer)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(forceAllowServersideTextureSounds)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerWeaponSpreadMode)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(monsterAIForceFindDistance)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(baseEntityDamagePushNormalMulti)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(baseEntityDamagePushVerticalBoost)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(baseEntityDamagePushVerticalMulti)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(baseEntityDamagePushVerticalMinimum)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelSyncFixPrintouts)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(textureHitSoundPrintouts)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hgruntAllowGrenades)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(scheduleInterruptPrintouts)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(animationPrintouts)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hassaultMeleeAttackEnabled)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(pathfindStumpedWaitTime)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(pathfindStumpedMode)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(pathfindStumpedForgetEnemy)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(pathfindEdgeCheck)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(RadiusDamageDrawDebug)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(AlienRadiationImmunity)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(customLogoSprayMode)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(monsterFadeOutRate)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(playerFadeOutRate)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(drawDebugEnemyLKP)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(trackchangePrintouts)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(trackTrainPrintouts)\
	EASY_CVAR_UPDATE_SERVER(cl_holster)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerWeaponTracerMode)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(monsterWeaponTracerMode)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(decalTracerExclusivity)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(monsterToPlayerHitgroupSpecial)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(multiplayerCrowbarHitSoundMode)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(healthcolor_fullRedMin)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(healthcolor_brightness)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(healthcolor_yellowMark)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(cl_drawExtraZeros)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(pathfindLargeBoundFix)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(flyerKilledFallingLoop)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(floaterDummy)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(barneyDummy)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(ladderCycleMulti)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(ladderSpeedMulti)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(barnacleGrabNoInterpolation)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hideDamage)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(timedDamage_brightnessMax)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(timedDamage_brightnessMin)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(timedDamage_brightnessCap)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(timedDamage_brightnessFloor)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(timedDamage_flashSpeed)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY(timedDamage_debug)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(wallHealthDoor_closeDelay)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(houndeye_attack_canGib)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(myRocketsAreBarney)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hassassinCrossbowDebug)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(crossbowBoltDirectionAffectedByWater)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(kingpinDebug)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(minimumRespawnDelay)\
	EASY_CVAR_UPDATE_SERVER(r_glowshell_debug)\
	EASY_CVAR_UPDATE_SERVER(cl_viewpunch)\
	EASY_CVAR_UPDATE_SERVER(cl_explosion)\
	EASY_CVAR_UPDATE_SERVER(soundSentenceSave)\
	EASY_CVAR_UPDATE_SERVER(pissedNPCs)\
	EASY_CVAR_UPDATE_SERVER(hud_logo)\
	EASY_CVAR_UPDATE_SERVER(hud_brokentrans)\
	EASY_CVAR_UPDATE_SERVER(cl_fvox)\
	EASY_CVAR_UPDATE_SERVER(cl_ladder)\
	EASY_CVAR_UPDATE_SERVER(precacheAll)\
	EASY_CVAR_UPDATE_SERVER(cl_interp_entity)\
	EASY_CVAR_UPDATE_SERVER(hud_swapFirstTwoBuckets)\
	EASY_CVAR_UPDATE_SERVER(ignoreMultiplayerSkillOverride)\
	EASY_CVAR_UPDATE_SERVER(m_rawinput)\
	EASY_CVAR_UPDATE_SERVER(cl_earlyaccess)\
	EASY_CVAR_UPDATE_SERVER(cl_viewroll)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(drawDebugCrowbar)\
	EASY_CVAR_UPDATE_SERVER(blastExtraArmorDamageMode)\
	EASY_CVAR_UPDATE_SERVER(hud_batteryhiddendead)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(hyperBarney)\
	EASY_CVAR_UPDATE_SERVER(monsterKilledToss)\
	EASY_CVAR_UPDATE_SERVER(interpolation_movetypestep_mindelta)\
	EASY_CVAR_UPDATE_SERVER(sv_bloodparticlemode)\
	EASY_CVAR_UPDATE_SERVER(cl_interp_view_extra)\
	EASY_CVAR_UPDATE_SERVER(cl_interp_view_standard)\
	EASY_CVAR_UPDATE_SERVER(cl_interp_viewmodel)\
	EASY_CVAR_UPDATE_SERVER(sv_explosionknockback)\
	EASY_CVAR_UPDATE_SERVER(cl_gaussfollowattachment)\
	EASY_CVAR_UPDATE_SERVER(cl_mp5_viewpunch_mod)\
	EASY_CVAR_UPDATE_SERVER(cl_breakholster)\
	EASY_CVAR_UPDATE_SERVER(pausecorrection1)\
	EASY_CVAR_UPDATE_SERVER(pausecorrection2)\
	EASY_CVAR_UPDATE_SERVER(hud_rpg_clipless)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST(sv_rpg_clipless)\
	EASY_CVAR_UPDATE_SERVER(cl_viewmodel_fidget)\
	EASY_CVAR_UPDATE_SERVER(sv_player_midair_fix)\
	EASY_CVAR_UPDATE_SERVER(sv_player_midair_accel)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(pathfindIgnoreNearestNodeCache)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(pathfindIgnoreStaticRoutes)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(pathfindForcePointHull)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(pathfindNearestNodeExtra)\
	EASY_CVAR_UPDATE_SERVER(sv_turret_postdeath)\
	EASY_CVAR_UPDATE_SERVER(cl_mp5_evil_skip)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonRadiusDamageMode)\
	EASY_CVAR_UPDATE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonFireRateMode)\
	EASY_CVAR_UPDATE_SERVER(sv_explosion_shake)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(sv_explosion_shake_range)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(sv_explosion_shake_amp)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(sv_explosion_shake_freq)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(sv_explosion_shake_duration)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(sv_gargantua_throwattack)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY(cl_viewpunch_mod)\
	EASY_CVAR_UPDATE_SERVER_CLIENTONLY(cl_gauss_viewpunch_mod)\
	EASY_CVAR_UPDATE_SERVER_DEBUGONLY(pathfindMonsterclipFreshLogic)\
	EASY_CVAR_UPDATE_SERVER(sv_rpg_projectile_model)\
	EASY_CVAR_UPDATE_SERVER(hmilitaryDeadInvestigate)\
	EASY_CVAR_UPDATE_SERVER(playerDeadTruce)\
	EASY_CVAR_UPDATE_SERVER(playerDeadTalkerBehavior)\
	DUMMY






#define EASY_CVAR_UPDATE_SERVER_DEDICATED_MASS\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(gruntsCanHaveMP5Grenade)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(hud_version)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(hud_batterydraw)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(strobeDurationMin)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(strobeDurationMax)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(strobeRadiusMin)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(strobeRadiusMax)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(strobeSpawnDistHori)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(strobeSpawnDistVertMin)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(strobeSpawnDistVertMax)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(strobeMultiColor)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(raveLaserEnabled)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(raveLaserSpawnFreq)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(raveLaserLength)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMin)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMax)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMin)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMax)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(raveLaserBrightnessMin)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(raveLaserBrightnessMax)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(raveLaserDurationMin)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(raveLaserDurationMax)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(raveLaserThicknessMin)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(raveLaserThicknessMax)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(raveLaserNoiseMin)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(raveLaserNoiseMax)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(raveLaserFrameRateMin)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(raveLaserFrameRateMax)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(raveLaserMultiColor)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(cameraPosFixedX)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(cameraPosFixedY)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(cameraPosFixedZ)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(cameraPosOffX)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(cameraPosOffY)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(cameraPosOffZ)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(cameraRotFixedX)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(cameraRotFixedY)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(cameraRotFixedZ)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(cameraRotOffX)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(cameraRotOffY)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(cameraRotOffZ)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(imAllFuckedUp)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(thatWasntGrass)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(fogNear)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(fogFar)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(NPCsTalkMore)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(myCameraSucks)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(shutupstuka)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(chromeEffect)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hassaultSpinMovement)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hassaultIdleSpinSound)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hassaultFireSound)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteBulletHitSounds)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(mutePlayerPainSounds)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hassaultIdleSpinSoundChannel)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hassaultSpinUpDownSoundChannel)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hassaultFireSoundChannel)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(geigerChannel)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hassaultWaitTime)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hassaultSpinupRemainTime)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hassaultResidualAttackTime)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hassaultSpinupStartTime)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hassaultVoicePitchMin)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hassaultVoicePitchMax)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hassaultFireSpread)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hassaultAllowGrenades)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(muteTempEntityGroundHitSound)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(houndeyeAttackMode)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteRicochetSound)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(mutePlayerWeaponFire)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteCrowbarSounds)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(scientistHealNPC)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(scientistHealNPCDebug)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(bulletholeAlertRange)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(fleshhitmakessound)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(nothingHurts)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(seeMonsterHealth)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(scientistHealNPCFract)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(bullsquidRangeDisabled)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(applyLKPPathFixToAll)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(timedDamageAffectsMonsters)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(scientistHealCooldown)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(crazyMonsterPrintouts)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(movementIsCompletePrintout)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(bulletHoleAlertPrintout)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(bulletholeAlertStukaOnly)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(barneyPrintouts)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(monsterSpawnPrintout)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(zombieBulletResistance)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(zombieBulletPushback)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(houndeyePrintout)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(quakeExplosionSound)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(explosionDebrisSoundVolume)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(noFlinchOnHard)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hassaultDrawLKP)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(meleeDrawBloodModeA)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(meleeDrawBloodModeB)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(drawDebugBloodTrace)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(meleeDrawBloodModeBFix)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(meleeDrawBloodModeAOffset)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(meleeDrawBloodModeBOffset)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST(wpn_glocksilencer)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(panthereyeHasCloakingAbility)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hgruntSpeedMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hgruntForceStrafeFireAnim)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hgruntLockRunAndGunTime)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hgruntHeadshotGore)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hgruntAllowStrafeFire)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(thoroughHitBoxUpdates)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hgruntTinyClip)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hgruntStrafeAlwaysHasAmmo)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hgruntBrassEjectForwardOffset)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(cl_muzzleflash)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(agrunt_muzzleflash)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(event5011Allowed)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(event5021Allowed)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(event5031Allowed)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(event5002Allowed)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(event5004Allowed)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(eventsAreFabulous)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(glockOldReloadLogic)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(glockOldReloadLogicBarney)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(barneyDroppedGlockAmmoCap)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(drawCollisionBoundsAtDeath)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(drawHitBoundsAtDeath)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(islaveReviveFriendMode)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(islaveReviveFriendChance)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(islaveReviveFriendRange)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(islaveReviveSelfMinDelay)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(islaveReviveSelfMaxDelay)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(islaveReviveSelfChance)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hgruntRunAndGunDistance)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hgruntPrintout)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(painArrowColorMode)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(painFlashColorMode)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(painFlashSuitless)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(painFlashDrownMode)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(firstPersonIdleDelayMin)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(firstPersonIdleDelayMax)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(forceDrawBatteryNumber)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(offsetgivedistance)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(offsetgivelookvertical)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(canShowWeaponSelectAtDeath)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(endlessFlashlightBattery)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(preE3UsesFailColors)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(E3UsesFailColors)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(preE3ShowsDamageIcons)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(E3ShowsDamageIcons)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerCrossbowMode)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hassassinCrossbowMode)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(tripmineAnimWaitsForFinish)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(revolverLaserScope)\
	DUMMY\
	DUMMY\
	DUMMY\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY(auto_adjust_fov)\
	DUMMY\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_nogaussrecoil)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(autoSneaky)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(sv_longjump_chargemode)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(cheat_touchNeverExplodes)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(gaussRecoilSendsUpInSP)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(drawDebugPathfinding)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(STUcheckDistH)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(STUcheckDistV)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(STUcheckDistD)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(STUextraTriangH)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(STUextraTriangV)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(STUrepelMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(STUSpeedMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(STUExplodeTest)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(STUYawSpeedMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(STUDetection)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(stukaAdvancedCombat)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(drawDebugPathfinding2)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hassaultFriendlyFire)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(myStrobe)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(peopleStrobe)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(forceWorldLightOff)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(wildHeads)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(raveEffectSpawnInterval)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(drawBarnacleDebug)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(fogTest)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(sparksAllMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(sparksEnvMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(sparksButtonMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(sparksPlayerCrossbowMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(sparksComputerHitMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(sparksTurretDeathMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(sparksOspreyHitMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(sparksExplosionMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(sparksBeamMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(sparksAIFailMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(normalSpeedMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(noclipSpeedMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(jumpForceMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(apacheForceCinBounds)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(apacheBottomBoundAdj)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(apacheInfluence)\
	DUMMY\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(allowAlphaCrosshairWithoutGuns)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(alphaCrosshairBlockedOnFrozen)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hgruntRunAndGunDotMin)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(panthereyeJumpDotTol)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(panthereyePrintout)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(gargantuaPrintout)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(squadmonsterPrintout)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hassaultPrintout)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(barnaclePrintout)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(friendlyPrintout)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(stukaPrintout)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(timedDamageEndlessOnHard)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(mirrorsReflectOnlyNPCs)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(mirrorsDoNotReflectPlayer)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(r_shadows)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(altSquadRulesRuntime)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hgruntLockStrafeTime)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(pathfindIgnoreIsolatedNodes)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(drawNodeAll)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(drawNodeSpecial)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(drawNodeConnections)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(drawNodeAlternateTime)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(nodeSearchStartVerticalOffset)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(blockChangeLevelTrigger)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(blockMusicTrigger)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(blockMultiTrigger)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(blockTeleportTrigger)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(blockHurtTrigger)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(blockAutosaveTrigger)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hideNodeGraphRebuildNotice)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(barnacleTongueRetractDelay)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST(sv_germancensorship)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(allowGermanModels)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(germanRobotGibs)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(germanRobotBleedsOil)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(germanRobotDamageDecal)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(germanRobotGibsDecal)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonEffectsMode)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonHitCloud)\
	DUMMY\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(handGrenadePickupYieldsOne)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(handGrenadesUseOldBounceSound)\
	DUMMY\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(hud_moveselect_mousewheelsound)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(timedDamageDeathRemoveMode)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(hud_moveselect_sound)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(barnacleCanGib)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(sentryCanGib)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(miniturretCanGib)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(turretCanGib)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(turretBleedsOil)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(turretDamageDecal)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(turretGibDecal)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(canDropInSinglePlayer)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(timedDamageIgnoresArmor)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(itemBatteryPrerequisite)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(playerExtraPainSoundsMode)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(timedDamageDisableViewPunch)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(batteryDrainsAtDeath)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(batteryDrainsAtAdrenalineMode)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(canTakeLongJump)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(printOutCommonTimables)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(playerBrightLight)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(disablePainPunchAutomatic)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(gargantuaCorpseDeath)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(gargantuaFallSound)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(gargantuaBleeds)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(shrapMode)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(shrapRand)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(shrapRandHeightExtra)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(explosionShrapnelMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(useAlphaSparks)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(emergencyFix)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(timedDamageReviveRemoveMode)\
	DUMMY\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(ospreyIgnoresGruntCount)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(mp5GrenadeInheritsPlayerVelocity)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(crossbowInheritsPlayerVelocity)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(fastHornetsInheritsPlayerVelocity)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(snarkInheritsPlayerVelocity)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(chumtoadInheritsPlayerVelocity)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(weaponPickupPlaysAnyReloadSounds)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(glockUseLastBulletAnim)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerBarnacleVictimViewOffset)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hgruntMovementDeltaCheck)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hiddenMemPrintout)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hassaultExtraMuzzleFlashRadius)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hassaultExtraMuzzleFlashBrightness)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hassaultExtraMuzzleFlashForward)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(leaderlessSquadAllowed)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(nodeConnectionBreakableCheck)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(playerReviveInvincibilityTime)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(playerReviveBuddhaMode)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(playerReviveTimeBlocksTimedDamage)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hassaultBulletDamageMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hassaultBulletsPerShot)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hassaultFireAnimSpeedMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hassaultMeleeAnimSpeedMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hassassinCrossbowReloadSoundDelay)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hgruntStrafeAnimSpeedMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hgruntRunAndGunAnimSpeedMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(cheat_iwantguts)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(nodeDetailPrintout)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(soundAttenuationStuka)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(soundVolumeStuka)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(cineChangelevelFix)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(drawDebugCine)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(cineAllowSequenceOverwrite)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(stukaInflictsBleeding)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(animationKilledBoundsRemoval)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(gargantuaKilledBoundsAssist)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(bullsquidSpitTrajTimeMin)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(bullsquidSpitTrajTimeMax)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(bullsquidSpitTrajDistMin)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(bullsquidSpitTrajDistMax)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(bullsquidSpitGravityMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(cl_bullsquidspit)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(cl_bullsquidspitarc)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(bullsquidSpitUseAlphaModel)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(bullsquidSpitUseAlphaEffect)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(bullsquidSpitEffectSpread)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(bullsquidSpitEffectMin)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(bullsquidSpitEffectMax)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(bullsquidSpitEffectHitMin)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(bullsquidSpitEffectHitMax)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(bullsquidSpitEffectSpawn)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(bullsquidSpitEffectHitSpawn)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(bullsquidSpitSpriteScale)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(bullsquidSpitAlphaScale)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(scientistBravery)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(barneyUnholsterTime)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(barneyUnholsterAnimChoice)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(trailTypeTest)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetTrail)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetTrailSolidColor)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetDeathModEasy)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetDeathModMedium)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetDeathModHard)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetZoomPuff)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetSpiral)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetSpeedMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetSpeedDartMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(cl_rockettrail)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketTrailAlphaInterval)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketTrailAlphaScale)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketSkipIgnite)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(agruntHornetRandomness)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hornetSpiralPeriod)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hornetSpiralAmplitude)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(cl_hornetspiral)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(cl_hornettrail)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(hud_drawammobar)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(hud_weaponselecthideslower)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(hud_drawsidebarmode)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST(gauss_mode)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(painFlashDmgMin)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(painFlashDmgExMult)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(painFlashCumulativeMinDrowning)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(painFlashCumulativeMax)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(painFlashDrawOpacityMax)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(painArrowDrawOpacityMin)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(painArrowDrawOpacityMax)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(painFlashFadeMult)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(painArrowFadeMult)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(painFlashArmorBlock)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(painFlashDirTolerance)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(painArrowCumulativeAppearMin)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(painArrowCumulativeDmgJump)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(painFlashPrintouts)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(itemFlashCumulativeJump)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(itemFlashDrawOpacityMin)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(itemFlashDrawOpacityMax)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(itemFlashFadeMult)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(chumtoadPrintout)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(crossbowReloadSoundDelay)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(crossbowFirePlaysReloadSound)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(pathfindFidgetFailTime)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(pathfindPrintout)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(pathfindTopRampFixDistance)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(pathfindTopRampFixDraw)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(iHaveAscended)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(pathfindLooseMapNodes)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(pathfindRampFix)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(chumtoadPlayDeadFoolChance)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(animationFramerateMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(pathfindNodeToleranceMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(friendlyPianoFollowVolume)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(friendlyPianoOtherVolume)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(showtriggers)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(tentacleAlertSound)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(tentacleSwingSound1)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(tentacleSwingSound2)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(playerFollowerMax)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(announcerIsAJerk)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(playerUseDrawDebug)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(playerChumtoadThrowDrawDebug)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(peaceOut)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(drawViewModel)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(drawHUD)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(disablePauseSinglePlayer)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerBulletHitEffectForceServer)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(forceAllowServersideTextureSounds)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerWeaponSpreadMode)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(monsterAIForceFindDistance)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(baseEntityDamagePushNormalMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(baseEntityDamagePushVerticalBoost)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(baseEntityDamagePushVerticalMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(baseEntityDamagePushVerticalMinimum)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelSyncFixPrintouts)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(textureHitSoundPrintouts)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hgruntAllowGrenades)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(scheduleInterruptPrintouts)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(animationPrintouts)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hassaultMeleeAttackEnabled)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(pathfindStumpedWaitTime)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(pathfindStumpedMode)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(pathfindStumpedForgetEnemy)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(pathfindEdgeCheck)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(RadiusDamageDrawDebug)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(AlienRadiationImmunity)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(customLogoSprayMode)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(monsterFadeOutRate)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(playerFadeOutRate)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(drawDebugEnemyLKP)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(trackchangePrintouts)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(trackTrainPrintouts)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(cl_holster)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerWeaponTracerMode)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(monsterWeaponTracerMode)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(decalTracerExclusivity)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(monsterToPlayerHitgroupSpecial)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(multiplayerCrowbarHitSoundMode)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(healthcolor_fullRedMin)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(healthcolor_brightness)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(healthcolor_yellowMark)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(cl_drawExtraZeros)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(pathfindLargeBoundFix)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(flyerKilledFallingLoop)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(floaterDummy)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(barneyDummy)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(ladderCycleMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(ladderSpeedMulti)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(barnacleGrabNoInterpolation)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hideDamage)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(timedDamage_brightnessMax)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(timedDamage_brightnessMin)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(timedDamage_brightnessCap)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(timedDamage_brightnessFloor)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(timedDamage_flashSpeed)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY_DEBUGONLY(timedDamage_debug)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(wallHealthDoor_closeDelay)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(houndeye_attack_canGib)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(myRocketsAreBarney)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hassassinCrossbowDebug)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(crossbowBoltDirectionAffectedByWater)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(kingpinDebug)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(minimumRespawnDelay)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(r_glowshell_debug)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(cl_viewpunch)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(cl_explosion)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(soundSentenceSave)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(pissedNPCs)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(hud_logo)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(hud_brokentrans)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(cl_fvox)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(cl_ladder)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(precacheAll)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(cl_interp_entity)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(hud_swapFirstTwoBuckets)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(ignoreMultiplayerSkillOverride)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(m_rawinput)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(cl_earlyaccess)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(cl_viewroll)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(drawDebugCrowbar)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(blastExtraArmorDamageMode)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(hud_batteryhiddendead)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(hyperBarney)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(monsterKilledToss)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(interpolation_movetypestep_mindelta)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(sv_bloodparticlemode)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(cl_interp_view_extra)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(cl_interp_view_standard)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(cl_interp_viewmodel)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(sv_explosionknockback)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(cl_gaussfollowattachment)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(cl_mp5_viewpunch_mod)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(cl_breakholster)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(pausecorrection1)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(pausecorrection2)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(hud_rpg_clipless)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST(sv_rpg_clipless)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(cl_viewmodel_fidget)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(sv_player_midair_fix)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(sv_player_midair_accel)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(pathfindIgnoreNearestNodeCache)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(pathfindIgnoreStaticRoutes)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(pathfindForcePointHull)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(pathfindNearestNodeExtra)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(sv_turret_postdeath)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(cl_mp5_evil_skip)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonRadiusDamageMode)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonFireRateMode)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(sv_explosion_shake)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(sv_explosion_shake_range)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(sv_explosion_shake_amp)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(sv_explosion_shake_freq)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(sv_explosion_shake_duration)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(sv_gargantua_throwattack)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY(cl_viewpunch_mod)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_CLIENTONLY(cl_gauss_viewpunch_mod)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED_DEBUGONLY(pathfindMonsterclipFreshLogic)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(sv_rpg_projectile_model)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(hmilitaryDeadInvestigate)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(playerDeadTruce)\
	EASY_CVAR_UPDATE_SERVER_DEDICATED(playerDeadTalkerBehavior)\
	DUMMY





#define EASY_CVAR_UPDATE_CLIENT_MASS\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(gruntsCanHaveMP5Grenade)\
	EASY_CVAR_UPDATE_CLIENT(hud_version)\
	EASY_CVAR_UPDATE_CLIENT(hud_batterydraw)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(strobeDurationMin)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(strobeDurationMax)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(strobeRadiusMin)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(strobeRadiusMax)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(strobeSpawnDistHori)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(strobeSpawnDistVertMin)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(strobeSpawnDistVertMax)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(strobeMultiColor)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserEnabled)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserSpawnFreq)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserLength)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMin)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMax)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMin)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMax)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserBrightnessMin)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserBrightnessMax)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserDurationMin)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserDurationMax)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserThicknessMin)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserThicknessMax)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserNoiseMin)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserNoiseMax)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserFrameRateMin)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserFrameRateMax)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(raveLaserMultiColor)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(cameraPosFixedX)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(cameraPosFixedY)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(cameraPosFixedZ)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(cameraPosOffX)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(cameraPosOffY)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(cameraPosOffZ)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(cameraRotFixedX)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(cameraRotFixedY)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(cameraRotFixedZ)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(cameraRotOffX)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(cameraRotOffY)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(cameraRotOffZ)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(imAllFuckedUp)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(thatWasntGrass)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(fogNear)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(fogFar)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(NPCsTalkMore)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(myCameraSucks)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(shutupstuka)\
	EASY_CVAR_UPDATE_CLIENT(chromeEffect)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hassaultSpinMovement)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hassaultIdleSpinSound)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hassaultFireSound)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteBulletHitSounds)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(mutePlayerPainSounds)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hassaultIdleSpinSoundChannel)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hassaultSpinUpDownSoundChannel)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hassaultFireSoundChannel)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(geigerChannel)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hassaultWaitTime)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hassaultSpinupRemainTime)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hassaultResidualAttackTime)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hassaultSpinupStartTime)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hassaultVoicePitchMin)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hassaultVoicePitchMax)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hassaultFireSpread)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hassaultAllowGrenades)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(muteTempEntityGroundHitSound)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(houndeyeAttackMode)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteRicochetSound)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(mutePlayerWeaponFire)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteCrowbarSounds)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(scientistHealNPC)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(scientistHealNPCDebug)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(bulletholeAlertRange)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(fleshhitmakessound)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(nothingHurts)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(seeMonsterHealth)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(scientistHealNPCFract)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(bullsquidRangeDisabled)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(applyLKPPathFixToAll)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(timedDamageAffectsMonsters)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(scientistHealCooldown)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(crazyMonsterPrintouts)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(movementIsCompletePrintout)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(bulletHoleAlertPrintout)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(bulletholeAlertStukaOnly)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(barneyPrintouts)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(monsterSpawnPrintout)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(zombieBulletResistance)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(zombieBulletPushback)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(houndeyePrintout)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(quakeExplosionSound)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(explosionDebrisSoundVolume)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(noFlinchOnHard)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hassaultDrawLKP)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(meleeDrawBloodModeA)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(meleeDrawBloodModeB)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(drawDebugBloodTrace)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(meleeDrawBloodModeBFix)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(meleeDrawBloodModeAOffset)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(meleeDrawBloodModeBOffset)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST(wpn_glocksilencer)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(panthereyeHasCloakingAbility)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hgruntSpeedMulti)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hgruntForceStrafeFireAnim)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hgruntLockRunAndGunTime)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hgruntHeadshotGore)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hgruntAllowStrafeFire)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(thoroughHitBoxUpdates)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hgruntTinyClip)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hgruntStrafeAlwaysHasAmmo)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hgruntBrassEjectForwardOffset)\
	EASY_CVAR_UPDATE_CLIENT(cl_muzzleflash)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(agrunt_muzzleflash)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(event5011Allowed)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(event5021Allowed)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(event5031Allowed)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(event5002Allowed)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(event5004Allowed)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(eventsAreFabulous)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(glockOldReloadLogic)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(glockOldReloadLogicBarney)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(barneyDroppedGlockAmmoCap)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(drawCollisionBoundsAtDeath)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(drawHitBoundsAtDeath)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(islaveReviveFriendMode)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(islaveReviveFriendChance)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(islaveReviveFriendRange)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(islaveReviveSelfMinDelay)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(islaveReviveSelfMaxDelay)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(islaveReviveSelfChance)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hgruntRunAndGunDistance)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hgruntPrintout)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(painArrowColorMode)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(painFlashColorMode)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(painFlashSuitless)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(painFlashDrownMode)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(firstPersonIdleDelayMin)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(firstPersonIdleDelayMax)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(forceDrawBatteryNumber)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(offsetgivedistance)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(offsetgivelookvertical)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(canShowWeaponSelectAtDeath)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(endlessFlashlightBattery)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(preE3UsesFailColors)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(E3UsesFailColors)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(preE3ShowsDamageIcons)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(E3ShowsDamageIcons)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerCrossbowMode)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hassassinCrossbowMode)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(tripmineAnimWaitsForFinish)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(revolverLaserScope)\
	DUMMY\
	DUMMY\
	DUMMY\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY(auto_adjust_fov)\
	DUMMY\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_nogaussrecoil)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(autoSneaky)\
	EASY_CVAR_UPDATE_CLIENT(sv_longjump_chargemode)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(cheat_touchNeverExplodes)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(gaussRecoilSendsUpInSP)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(drawDebugPathfinding)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(STUcheckDistH)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(STUcheckDistV)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(STUcheckDistD)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(STUextraTriangH)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(STUextraTriangV)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(STUrepelMulti)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(STUSpeedMulti)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(STUExplodeTest)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(STUYawSpeedMulti)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(STUDetection)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(stukaAdvancedCombat)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(drawDebugPathfinding2)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hassaultFriendlyFire)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(myStrobe)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(peopleStrobe)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(forceWorldLightOff)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(wildHeads)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(raveEffectSpawnInterval)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(drawBarnacleDebug)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(fogTest)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(sparksAllMulti)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(sparksEnvMulti)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(sparksButtonMulti)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(sparksPlayerCrossbowMulti)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(sparksComputerHitMulti)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(sparksTurretDeathMulti)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(sparksOspreyHitMulti)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(sparksExplosionMulti)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(sparksBeamMulti)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(sparksAIFailMulti)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(normalSpeedMulti)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(noclipSpeedMulti)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(jumpForceMulti)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(apacheForceCinBounds)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(apacheBottomBoundAdj)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(apacheInfluence)\
	DUMMY\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(allowAlphaCrosshairWithoutGuns)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(alphaCrosshairBlockedOnFrozen)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hgruntRunAndGunDotMin)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(panthereyeJumpDotTol)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(panthereyePrintout)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(gargantuaPrintout)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(squadmonsterPrintout)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hassaultPrintout)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(barnaclePrintout)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(friendlyPrintout)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(stukaPrintout)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(timedDamageEndlessOnHard)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(mirrorsReflectOnlyNPCs)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(mirrorsDoNotReflectPlayer)\
	EASY_CVAR_UPDATE_CLIENT(r_shadows)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(altSquadRulesRuntime)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hgruntLockStrafeTime)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(pathfindIgnoreIsolatedNodes)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(drawNodeAll)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(drawNodeSpecial)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(drawNodeConnections)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(drawNodeAlternateTime)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(nodeSearchStartVerticalOffset)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(blockChangeLevelTrigger)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(blockMusicTrigger)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(blockMultiTrigger)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(blockTeleportTrigger)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(blockHurtTrigger)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(blockAutosaveTrigger)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hideNodeGraphRebuildNotice)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(barnacleTongueRetractDelay)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST(sv_germancensorship)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(allowGermanModels)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(germanRobotGibs)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(germanRobotBleedsOil)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(germanRobotDamageDecal)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(germanRobotGibsDecal)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonEffectsMode)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonHitCloud)\
	DUMMY\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(handGrenadePickupYieldsOne)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(handGrenadesUseOldBounceSound)\
	DUMMY\
	EASY_CVAR_UPDATE_CLIENT(hud_moveselect_mousewheelsound)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(timedDamageDeathRemoveMode)\
	EASY_CVAR_UPDATE_CLIENT(hud_moveselect_sound)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(barnacleCanGib)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(sentryCanGib)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(miniturretCanGib)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(turretCanGib)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(turretBleedsOil)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(turretDamageDecal)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(turretGibDecal)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(canDropInSinglePlayer)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(timedDamageIgnoresArmor)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(itemBatteryPrerequisite)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(playerExtraPainSoundsMode)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(timedDamageDisableViewPunch)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(batteryDrainsAtDeath)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(batteryDrainsAtAdrenalineMode)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(canTakeLongJump)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(printOutCommonTimables)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(playerBrightLight)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(disablePainPunchAutomatic)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(gargantuaCorpseDeath)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(gargantuaFallSound)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(gargantuaBleeds)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(shrapMode)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(shrapRand)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(shrapRandHeightExtra)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(explosionShrapnelMulti)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(useAlphaSparks)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(emergencyFix)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(timedDamageReviveRemoveMode)\
	DUMMY\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(ospreyIgnoresGruntCount)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(mp5GrenadeInheritsPlayerVelocity)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(crossbowInheritsPlayerVelocity)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(fastHornetsInheritsPlayerVelocity)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(snarkInheritsPlayerVelocity)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(chumtoadInheritsPlayerVelocity)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(weaponPickupPlaysAnyReloadSounds)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(glockUseLastBulletAnim)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerBarnacleVictimViewOffset)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hgruntMovementDeltaCheck)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hiddenMemPrintout)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hassaultExtraMuzzleFlashRadius)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hassaultExtraMuzzleFlashBrightness)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hassaultExtraMuzzleFlashForward)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(leaderlessSquadAllowed)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(nodeConnectionBreakableCheck)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(playerReviveInvincibilityTime)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(playerReviveBuddhaMode)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(playerReviveTimeBlocksTimedDamage)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hassaultBulletDamageMulti)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hassaultBulletsPerShot)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hassaultFireAnimSpeedMulti)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hassaultMeleeAnimSpeedMulti)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hassassinCrossbowReloadSoundDelay)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hgruntStrafeAnimSpeedMulti)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hgruntRunAndGunAnimSpeedMulti)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(cheat_iwantguts)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(nodeDetailPrintout)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(soundAttenuationStuka)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(soundVolumeStuka)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(cineChangelevelFix)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(drawDebugCine)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(cineAllowSequenceOverwrite)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(stukaInflictsBleeding)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(animationKilledBoundsRemoval)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(gargantuaKilledBoundsAssist)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(bullsquidSpitTrajTimeMin)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(bullsquidSpitTrajTimeMax)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(bullsquidSpitTrajDistMin)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(bullsquidSpitTrajDistMax)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(bullsquidSpitGravityMulti)\
	EASY_CVAR_UPDATE_CLIENT(cl_bullsquidspit)\
	EASY_CVAR_UPDATE_CLIENT(cl_bullsquidspitarc)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(bullsquidSpitUseAlphaModel)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(bullsquidSpitUseAlphaEffect)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(bullsquidSpitEffectSpread)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(bullsquidSpitEffectMin)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(bullsquidSpitEffectMax)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(bullsquidSpitEffectHitMin)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(bullsquidSpitEffectHitMax)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(bullsquidSpitEffectSpawn)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(bullsquidSpitEffectHitSpawn)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(bullsquidSpitSpriteScale)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(bullsquidSpitAlphaScale)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(scientistBravery)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(barneyUnholsterTime)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(barneyUnholsterAnimChoice)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(trailTypeTest)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetTrail)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetTrailSolidColor)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetDeathModEasy)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetDeathModMedium)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetDeathModHard)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetZoomPuff)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetSpiral)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetSpeedMulti)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetSpeedDartMulti)\
	EASY_CVAR_UPDATE_CLIENT(cl_rockettrail)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketTrailAlphaInterval)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketTrailAlphaScale)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketSkipIgnite)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(agruntHornetRandomness)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hornetSpiralPeriod)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hornetSpiralAmplitude)\
	EASY_CVAR_UPDATE_CLIENT(cl_hornetspiral)\
	EASY_CVAR_UPDATE_CLIENT(cl_hornettrail)\
	EASY_CVAR_UPDATE_CLIENT(hud_drawammobar)\
	EASY_CVAR_UPDATE_CLIENT(hud_weaponselecthideslower)\
	EASY_CVAR_UPDATE_CLIENT(hud_drawsidebarmode)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST(gauss_mode)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(painFlashDmgMin)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(painFlashDmgExMult)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(painFlashCumulativeMinDrowning)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(painFlashCumulativeMax)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(painFlashDrawOpacityMax)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(painArrowDrawOpacityMin)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(painArrowDrawOpacityMax)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(painFlashFadeMult)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(painArrowFadeMult)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(painFlashArmorBlock)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(painFlashDirTolerance)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(painArrowCumulativeAppearMin)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(painArrowCumulativeDmgJump)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(painFlashPrintouts)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(itemFlashCumulativeJump)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(itemFlashDrawOpacityMin)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(itemFlashDrawOpacityMax)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(itemFlashFadeMult)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(chumtoadPrintout)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(crossbowReloadSoundDelay)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(crossbowFirePlaysReloadSound)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(pathfindFidgetFailTime)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(pathfindPrintout)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(pathfindTopRampFixDistance)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(pathfindTopRampFixDraw)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(iHaveAscended)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(pathfindLooseMapNodes)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(pathfindRampFix)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(chumtoadPlayDeadFoolChance)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(animationFramerateMulti)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(pathfindNodeToleranceMulti)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(friendlyPianoFollowVolume)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(friendlyPianoOtherVolume)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(showtriggers)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(tentacleAlertSound)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(tentacleSwingSound1)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(tentacleSwingSound2)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(playerFollowerMax)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(announcerIsAJerk)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(playerUseDrawDebug)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(playerChumtoadThrowDrawDebug)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(peaceOut)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(drawViewModel)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(drawHUD)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(disablePauseSinglePlayer)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerBulletHitEffectForceServer)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(forceAllowServersideTextureSounds)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerWeaponSpreadMode)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(monsterAIForceFindDistance)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(baseEntityDamagePushNormalMulti)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(baseEntityDamagePushVerticalBoost)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(baseEntityDamagePushVerticalMulti)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(baseEntityDamagePushVerticalMinimum)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelSyncFixPrintouts)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(textureHitSoundPrintouts)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hgruntAllowGrenades)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(scheduleInterruptPrintouts)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(animationPrintouts)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hassaultMeleeAttackEnabled)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(pathfindStumpedWaitTime)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(pathfindStumpedMode)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(pathfindStumpedForgetEnemy)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(pathfindEdgeCheck)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(RadiusDamageDrawDebug)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(AlienRadiationImmunity)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(customLogoSprayMode)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(monsterFadeOutRate)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(playerFadeOutRate)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(drawDebugEnemyLKP)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(trackchangePrintouts)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(trackTrainPrintouts)\
	EASY_CVAR_UPDATE_CLIENT(cl_holster)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerWeaponTracerMode)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(monsterWeaponTracerMode)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(decalTracerExclusivity)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(monsterToPlayerHitgroupSpecial)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(multiplayerCrowbarHitSoundMode)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(healthcolor_fullRedMin)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(healthcolor_brightness)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(healthcolor_yellowMark)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(cl_drawExtraZeros)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(pathfindLargeBoundFix)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(flyerKilledFallingLoop)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(floaterDummy)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(barneyDummy)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(ladderCycleMulti)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(ladderSpeedMulti)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(barnacleGrabNoInterpolation)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hideDamage)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(timedDamage_brightnessMax)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(timedDamage_brightnessMin)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(timedDamage_brightnessCap)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(timedDamage_brightnessFloor)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(timedDamage_flashSpeed)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY_DEBUGONLY(timedDamage_debug)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(wallHealthDoor_closeDelay)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(houndeye_attack_canGib)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(myRocketsAreBarney)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hassassinCrossbowDebug)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(crossbowBoltDirectionAffectedByWater)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(kingpinDebug)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(minimumRespawnDelay)\
	EASY_CVAR_UPDATE_CLIENT(r_glowshell_debug)\
	EASY_CVAR_UPDATE_CLIENT(cl_viewpunch)\
	EASY_CVAR_UPDATE_CLIENT(cl_explosion)\
	EASY_CVAR_UPDATE_CLIENT(soundSentenceSave)\
	EASY_CVAR_UPDATE_CLIENT(pissedNPCs)\
	EASY_CVAR_UPDATE_CLIENT(hud_logo)\
	EASY_CVAR_UPDATE_CLIENT(hud_brokentrans)\
	EASY_CVAR_UPDATE_CLIENT(cl_fvox)\
	EASY_CVAR_UPDATE_CLIENT(cl_ladder)\
	EASY_CVAR_UPDATE_CLIENT(precacheAll)\
	EASY_CVAR_UPDATE_CLIENT(cl_interp_entity)\
	EASY_CVAR_UPDATE_CLIENT(hud_swapFirstTwoBuckets)\
	EASY_CVAR_UPDATE_CLIENT(ignoreMultiplayerSkillOverride)\
	EASY_CVAR_UPDATE_CLIENT(m_rawinput)\
	EASY_CVAR_UPDATE_CLIENT(cl_earlyaccess)\
	EASY_CVAR_UPDATE_CLIENT(cl_viewroll)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(drawDebugCrowbar)\
	EASY_CVAR_UPDATE_CLIENT(blastExtraArmorDamageMode)\
	EASY_CVAR_UPDATE_CLIENT(hud_batteryhiddendead)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(hyperBarney)\
	EASY_CVAR_UPDATE_CLIENT(monsterKilledToss)\
	EASY_CVAR_UPDATE_CLIENT(interpolation_movetypestep_mindelta)\
	EASY_CVAR_UPDATE_CLIENT(sv_bloodparticlemode)\
	EASY_CVAR_UPDATE_CLIENT(cl_interp_view_extra)\
	EASY_CVAR_UPDATE_CLIENT(cl_interp_view_standard)\
	EASY_CVAR_UPDATE_CLIENT(cl_interp_viewmodel)\
	EASY_CVAR_UPDATE_CLIENT(sv_explosionknockback)\
	EASY_CVAR_UPDATE_CLIENT(cl_gaussfollowattachment)\
	EASY_CVAR_UPDATE_CLIENT(cl_mp5_viewpunch_mod)\
	EASY_CVAR_UPDATE_CLIENT(cl_breakholster)\
	EASY_CVAR_UPDATE_CLIENT(pausecorrection1)\
	EASY_CVAR_UPDATE_CLIENT(pausecorrection2)\
	EASY_CVAR_UPDATE_CLIENT(hud_rpg_clipless)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST(sv_rpg_clipless)\
	EASY_CVAR_UPDATE_CLIENT(cl_viewmodel_fidget)\
	EASY_CVAR_UPDATE_CLIENT(sv_player_midair_fix)\
	EASY_CVAR_UPDATE_CLIENT(sv_player_midair_accel)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(pathfindIgnoreNearestNodeCache)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(pathfindIgnoreStaticRoutes)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(pathfindForcePointHull)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(pathfindNearestNodeExtra)\
	EASY_CVAR_UPDATE_CLIENT(sv_turret_postdeath)\
	EASY_CVAR_UPDATE_CLIENT(cl_mp5_evil_skip)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonRadiusDamageMode)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonFireRateMode)\
	EASY_CVAR_UPDATE_CLIENT(sv_explosion_shake)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(sv_explosion_shake_range)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(sv_explosion_shake_amp)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(sv_explosion_shake_freq)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(sv_explosion_shake_duration)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(sv_gargantua_throwattack)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY(cl_viewpunch_mod)\
	EASY_CVAR_UPDATE_CLIENT_CLIENTONLY(cl_gauss_viewpunch_mod)\
	EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(pathfindMonsterclipFreshLogic)\
	EASY_CVAR_UPDATE_CLIENT(sv_rpg_projectile_model)\
	EASY_CVAR_UPDATE_CLIENT(hmilitaryDeadInvestigate)\
	EASY_CVAR_UPDATE_CLIENT(playerDeadTruce)\
	EASY_CVAR_UPDATE_CLIENT(playerDeadTalkerBehavior)\
	DUMMY




#define EASY_CVAR_CREATE_SERVER_SETUP_MASS\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(gruntsCanHaveMP5Grenade)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(hud_version)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(hud_batterydraw)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(strobeDurationMin)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(strobeDurationMax)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(strobeRadiusMin)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(strobeRadiusMax)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(strobeSpawnDistHori)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(strobeSpawnDistVertMin)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(strobeSpawnDistVertMax)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(strobeMultiColor)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(raveLaserEnabled)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(raveLaserSpawnFreq)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(raveLaserLength)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMin)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMax)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMin)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMax)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(raveLaserBrightnessMin)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(raveLaserBrightnessMax)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(raveLaserDurationMin)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(raveLaserDurationMax)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(raveLaserThicknessMin)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(raveLaserThicknessMax)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(raveLaserNoiseMin)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(raveLaserNoiseMax)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(raveLaserFrameRateMin)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(raveLaserFrameRateMax)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(raveLaserMultiColor)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(cameraPosFixedX)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(cameraPosFixedY)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(cameraPosFixedZ)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(cameraPosOffX)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(cameraPosOffY)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(cameraPosOffZ)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(cameraRotFixedX)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(cameraRotFixedY)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(cameraRotFixedZ)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(cameraRotOffX)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(cameraRotOffY)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(cameraRotOffZ)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(imAllFuckedUp)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(thatWasntGrass)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(thatWasntPunch)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(fogNear)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(fogFar)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(NPCsTalkMore)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(myCameraSucks)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(shutupstuka)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(chromeEffect)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hassaultSpinMovement)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hassaultIdleSpinSound)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hassaultFireSound)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(muteBulletHitSounds)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(mutePlayerPainSounds)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hassaultIdleSpinSoundChannel)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hassaultSpinUpDownSoundChannel)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hassaultFireSoundChannel)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(geigerChannel)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hassaultWaitTime)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hassaultSpinupRemainTime)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hassaultResidualAttackTime)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hassaultSpinupStartTime)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hassaultVoicePitchMin)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hassaultVoicePitchMax)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hassaultFireSpread)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hassaultAllowGrenades)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(muteTempEntityGroundHitSound)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(houndeyeAttackMode)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(muteRicochetSound)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(mutePlayerWeaponFire)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(muteCrowbarSounds)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(scientistHealNPC)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(scientistHealNPCDebug)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(bulletholeAlertRange)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(fleshhitmakessound)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(nothingHurts)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(seeMonsterHealth)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(scientistHealNPCFract)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(bullsquidRangeDisabled)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(applyLKPPathFixToAll)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(timedDamageAffectsMonsters)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(scientistHealCooldown)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(crazyMonsterPrintouts)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(movementIsCompletePrintout)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(bulletHoleAlertPrintout)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(bulletholeAlertStukaOnly)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(barneyPrintouts)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(monsterSpawnPrintout)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(zombieBulletResistance)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(zombieBulletPushback)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(houndeyePrintout)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(quakeExplosionSound)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(explosionDebrisSoundVolume)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(noFlinchOnHard)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hassaultDrawLKP)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(meleeDrawBloodModeA)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(meleeDrawBloodModeB)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(drawDebugBloodTrace)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(meleeDrawBloodModeBFix)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(meleeDrawBloodModeAOffset)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(meleeDrawBloodModeBOffset)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY(wpn_glocksilencer)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(panthereyeHasCloakingAbility)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hgruntSpeedMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hgruntForceStrafeFireAnim)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hgruntLockRunAndGunTime)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hgruntHeadshotGore)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hgruntAllowStrafeFire)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(thoroughHitBoxUpdates)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hgruntTinyClip)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hgruntStrafeAlwaysHasAmmo)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hgruntBrassEjectForwardOffset)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(cl_muzzleflash)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(agrunt_muzzleflash)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(event5011Allowed)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(event5021Allowed)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(event5031Allowed)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(event5002Allowed)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(event5004Allowed)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(eventsAreFabulous)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(glockOldReloadLogic)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(glockOldReloadLogicBarney)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(barneyDroppedGlockAmmoCap)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(drawCollisionBoundsAtDeath)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(drawHitBoundsAtDeath)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(islaveReviveFriendMode)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(islaveReviveFriendChance)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(islaveReviveFriendRange)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(islaveReviveSelfMinDelay)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(islaveReviveSelfMaxDelay)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(islaveReviveSelfChance)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hgruntRunAndGunDistance)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hgruntPrintout)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(testVar)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(painArrowColorMode)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(painFlashColorMode)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(painFlashSuitless)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(painFlashDrownMode)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(firstPersonIdleDelayMin)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(firstPersonIdleDelayMax)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(forceDrawBatteryNumber)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(offsetgivedistance)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(offsetgivelookvertical)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(canShowWeaponSelectAtDeath)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(endlessFlashlightBattery)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(preE3UsesFailColors)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(E3UsesFailColors)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(preE3ShowsDamageIcons)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(E3ShowsDamageIcons)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(playerCrossbowMode)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hassassinCrossbowMode)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(tripmineAnimWaitsForFinish)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(revolverLaserScope)\
	DUMMY\
	DUMMY\
	DUMMY\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(auto_adjust_fov)\
	DUMMY\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(cheat_infiniteclip)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(cheat_infiniteammo)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(cheat_minimumfiredelay)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(cheat_minimumfiredelaycustom)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(cheat_nogaussrecoil)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(autoSneaky)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY(sv_longjump_chargemode)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(cheat_touchNeverExplodes)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(gaussRecoilSendsUpInSP)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(drawDebugPathfinding)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(STUcheckDistH)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(STUcheckDistV)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(STUcheckDistD)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(STUextraTriangH)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(STUextraTriangV)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(STUrepelMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(STUSpeedMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(STUExplodeTest)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(STUYawSpeedMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(STUDetection)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(stukaAdvancedCombat)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(drawDebugPathfinding2)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hassaultFriendlyFire)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(myStrobe)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(peopleStrobe)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(forceWorldLightOff)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(wildHeads)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(raveEffectSpawnInterval)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(drawBarnacleDebug)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(fogTest)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(sparksAllMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(sparksEnvMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(sparksButtonMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(sparksPlayerCrossbowMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(sparksComputerHitMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(sparksTurretDeathMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(sparksOspreyHitMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(sparksExplosionMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(sparksBeamMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(sparksAIFailMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(normalSpeedMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(noclipSpeedMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(jumpForceMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(apacheForceCinBounds)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(apacheBottomBoundAdj)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(apacheInfluence)\
	DUMMY\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(allowAlphaCrosshairWithoutGuns)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(alphaCrosshairBlockedOnFrozen)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hgruntRunAndGunDotMin)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(panthereyeJumpDotTol)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(panthereyePrintout)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(gargantuaPrintout)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(squadmonsterPrintout)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hassaultPrintout)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(barnaclePrintout)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(friendlyPrintout)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(stukaPrintout)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(timedDamageEndlessOnHard)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(mirrorsReflectOnlyNPCs)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(mirrorsDoNotReflectPlayer)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(r_shadows)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(altSquadRulesRuntime)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hgruntLockStrafeTime)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(pathfindIgnoreIsolatedNodes)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(drawNodeAll)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(drawNodeSpecial)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(drawNodeConnections)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(drawNodeAlternateTime)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(nodeSearchStartVerticalOffset)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(blockChangeLevelTrigger)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(blockMusicTrigger)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(blockMultiTrigger)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(blockTeleportTrigger)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(blockHurtTrigger)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(blockAutosaveTrigger)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hideNodeGraphRebuildNotice)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(barnacleTongueRetractDelay)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY(sv_germancensorship)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(allowGermanModels)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(germanRobotGibs)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(germanRobotBleedsOil)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(germanRobotDamageDecal)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(germanRobotGibsDecal)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(egonEffectsMode)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(egonHitCloud)\
	DUMMY\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(handGrenadePickupYieldsOne)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(handGrenadesUseOldBounceSound)\
	DUMMY\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(hud_moveselect_mousewheelsound)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(timedDamageDeathRemoveMode)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(hud_moveselect_sound)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(barnacleCanGib)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(sentryCanGib)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(miniturretCanGib)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(turretCanGib)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(turretBleedsOil)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(turretDamageDecal)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(turretGibDecal)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(canDropInSinglePlayer)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(timedDamageIgnoresArmor)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(itemBatteryPrerequisite)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(playerExtraPainSoundsMode)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(timedDamageDisableViewPunch)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(batteryDrainsAtDeath)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(batteryDrainsAtAdrenalineMode)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(canTakeLongJump)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(printOutCommonTimables)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(playerBrightLight)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(disablePainPunchAutomatic)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(gargantuaCorpseDeath)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(gargantuaFallSound)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(gargantuaBleeds)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(shrapMode)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(shrapRand)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(shrapRandHeightExtra)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(explosionShrapnelMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(useAlphaSparks)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(emergencyFix)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(timedDamageReviveRemoveMode)\
	DUMMY\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(ospreyIgnoresGruntCount)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(mp5GrenadeInheritsPlayerVelocity)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(crossbowInheritsPlayerVelocity)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(fastHornetsInheritsPlayerVelocity)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(snarkInheritsPlayerVelocity)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(chumtoadInheritsPlayerVelocity)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(weaponPickupPlaysAnyReloadSounds)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(glockUseLastBulletAnim)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(playerBarnacleVictimViewOffset)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hgruntMovementDeltaCheck)\
	EASY_CVAR_CREATE_SERVER_SETUP_SERVERONLY_DEBUGONLY(hiddenMemPrintout)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hassaultExtraMuzzleFlashRadius)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hassaultExtraMuzzleFlashBrightness)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hassaultExtraMuzzleFlashForward)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(leaderlessSquadAllowed)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(nodeConnectionBreakableCheck)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(playerReviveInvincibilityTime)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(playerReviveBuddhaMode)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(playerReviveTimeBlocksTimedDamage)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hassaultBulletDamageMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hassaultBulletsPerShot)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hassaultFireAnimSpeedMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hassaultMeleeAnimSpeedMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hassassinCrossbowReloadSoundDelay)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hgruntStrafeAnimSpeedMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hgruntRunAndGunAnimSpeedMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(cheat_iwantguts)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(nodeDetailPrintout)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(soundAttenuationStuka)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(soundVolumeStuka)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(cineChangelevelFix)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(drawDebugCine)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(cineAllowSequenceOverwrite)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(stukaInflictsBleeding)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(animationKilledBoundsRemoval)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(gargantuaKilledBoundsAssist)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(bullsquidSpitTrajTimeMin)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(bullsquidSpitTrajTimeMax)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(bullsquidSpitTrajDistMin)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(bullsquidSpitTrajDistMax)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(bullsquidSpitGravityMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY(cl_bullsquidspit)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY(cl_bullsquidspitarc)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(bullsquidSpitUseAlphaModel)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(bullsquidSpitUseAlphaEffect)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(bullsquidSpitEffectSpread)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(bullsquidSpitEffectMin)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(bullsquidSpitEffectMax)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(bullsquidSpitEffectHitMin)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(bullsquidSpitEffectHitMax)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(bullsquidSpitEffectSpawn)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(bullsquidSpitEffectHitSpawn)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(bullsquidSpitSpriteScale)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(bullsquidSpitAlphaScale)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(scientistBravery)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(barneyUnholsterTime)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(barneyUnholsterAnimChoice)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(trailTypeTest)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hornetTrail)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hornetTrailSolidColor)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hornetDeathModEasy)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hornetDeathModMedium)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hornetDeathModHard)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hornetZoomPuff)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hornetSpiral)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hornetSpeedMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hornetSpeedDartMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY(cl_rockettrail)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(rocketTrailAlphaInterval)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(rocketTrailAlphaScale)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(rocketSkipIgnite)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(agruntHornetRandomness)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hornetSpiralPeriod)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hornetSpiralAmplitude)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY(cl_hornetspiral)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY(cl_hornettrail)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(hud_drawammobar)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(hud_weaponselecthideslower)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(hud_drawsidebarmode)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY(gauss_mode)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(painFlashDmgMin)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(painFlashDmgExMult)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(painFlashCumulativeMinDrowning)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(painFlashCumulativeMax)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(painFlashDrawOpacityMax)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(painArrowDrawOpacityMin)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(painArrowDrawOpacityMax)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(painFlashFadeMult)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(painArrowFadeMult)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(painFlashArmorBlock)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(painFlashDirTolerance)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(painArrowCumulativeAppearMin)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(painArrowCumulativeDmgJump)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(painFlashPrintouts)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(itemFlashCumulativeJump)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(itemFlashDrawOpacityMin)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(itemFlashDrawOpacityMax)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(itemFlashFadeMult)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(chumtoadPrintout)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(crossbowReloadSoundDelay)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(crossbowFirePlaysReloadSound)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(pathfindFidgetFailTime)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(pathfindPrintout)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(pathfindTopRampFixDistance)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(pathfindTopRampFixDraw)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(iHaveAscended)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(pathfindLooseMapNodes)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(pathfindRampFix)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(chumtoadPlayDeadFoolChance)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(animationFramerateMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(pathfindNodeToleranceMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(friendlyPianoFollowVolume)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(friendlyPianoOtherVolume)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(showtriggers)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(tentacleAlertSound)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(tentacleSwingSound1)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(tentacleSwingSound2)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(playerFollowerMax)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(announcerIsAJerk)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(playerUseDrawDebug)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(playerChumtoadThrowDrawDebug)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(peaceOut)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(drawViewModel)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(drawHUD)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(disablePauseSinglePlayer)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(playerBulletHitEffectForceServer)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(forceAllowServersideTextureSounds)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(playerWeaponSpreadMode)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(monsterAIForceFindDistance)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(baseEntityDamagePushNormalMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(baseEntityDamagePushVerticalBoost)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(baseEntityDamagePushVerticalMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(baseEntityDamagePushVerticalMinimum)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(viewModelPrintouts)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(viewModelSyncFixPrintouts)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(textureHitSoundPrintouts)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hgruntAllowGrenades)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(scheduleInterruptPrintouts)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(animationPrintouts)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hassaultMeleeAttackEnabled)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(pathfindStumpedWaitTime)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(pathfindStumpedMode)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(pathfindStumpedForgetEnemy)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(pathfindEdgeCheck)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(RadiusDamageDrawDebug)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(AlienRadiationImmunity)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(customLogoSprayMode)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(monsterFadeOutRate)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(playerFadeOutRate)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(drawDebugEnemyLKP)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(trackchangePrintouts)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(trackTrainPrintouts)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(cl_holster)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(playerWeaponTracerMode)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(monsterWeaponTracerMode)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(decalTracerExclusivity)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(monsterToPlayerHitgroupSpecial)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(multiplayerCrowbarHitSoundMode)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(healthcolor_fullRedMin)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(healthcolor_brightness)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(healthcolor_yellowMark)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(cl_drawExtraZeros)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(pathfindLargeBoundFix)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(flyerKilledFallingLoop)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(floaterDummy)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(barneyDummy)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(ladderCycleMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(ladderSpeedMulti)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(barnacleGrabNoInterpolation)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hideDamage)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(timedDamage_brightnessMax)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(timedDamage_brightnessMin)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(timedDamage_brightnessCap)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(timedDamage_brightnessFloor)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(timedDamage_flashSpeed)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(timedDamage_debug)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(wallHealthDoor_closeDelay)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(houndeye_attack_canGib)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(myRocketsAreBarney)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hassassinCrossbowDebug)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(crossbowBoltDirectionAffectedByWater)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(kingpinDebug)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(minimumRespawnDelay)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(r_glowshell_debug)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(cl_viewpunch)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY(cl_explosion)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY(soundSentenceSave)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY(pissedNPCs)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(hud_logo)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(hud_brokentrans)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(cl_fvox)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(cl_ladder)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY(precacheAll)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(cl_interp_entity)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(hud_swapFirstTwoBuckets)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY(ignoreMultiplayerSkillOverride)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(m_rawinput)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(cl_earlyaccess)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(cl_viewroll)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(drawDebugCrowbar)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY(blastExtraArmorDamageMode)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(hud_batteryhiddendead)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(hyperBarney)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY(monsterKilledToss)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(interpolation_movetypestep_mindelta)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY(sv_bloodparticlemode)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(cl_interp_view_extra)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(cl_interp_view_standard)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(cl_interp_viewmodel)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY(sv_explosionknockback)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(cl_gaussfollowattachment)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(cl_mp5_viewpunch_mod)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(cl_breakholster)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(pausecorrection1)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY(pausecorrection2)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(hud_rpg_clipless)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY(sv_rpg_clipless)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(cl_viewmodel_fidget)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY(sv_player_midair_fix)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY(sv_player_midair_accel)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(pathfindIgnoreNearestNodeCache)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(pathfindIgnoreStaticRoutes)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(pathfindForcePointHull)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(pathfindNearestNodeExtra)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY(sv_turret_postdeath)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(cl_mp5_evil_skip)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(egonRadiusDamageMode)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(egonFireRateMode)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY(sv_explosion_shake)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(sv_explosion_shake_range)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(sv_explosion_shake_amp)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(sv_explosion_shake_freq)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(sv_explosion_shake_duration)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(sv_gargantua_throwattack)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(cl_viewpunch_mod)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(cl_gauss_viewpunch_mod)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(pathfindMonsterclipFreshLogic)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY(sv_rpg_projectile_model)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY(hmilitaryDeadInvestigate)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY(playerDeadTruce)\
	EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY(playerDeadTalkerBehavior)\
	DUMMY



#define EASY_CVAR_CREATE_SERVER_MASS\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(gruntsCanHaveMP5Grenade)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(hud_version)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(hud_batterydraw)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(strobeDurationMin)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(strobeDurationMax)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(strobeRadiusMin)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(strobeRadiusMax)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(strobeSpawnDistHori)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(strobeSpawnDistVertMin)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(strobeSpawnDistVertMax)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(strobeMultiColor)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(raveLaserEnabled)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(raveLaserSpawnFreq)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(raveLaserLength)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMin)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMax)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMin)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMax)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(raveLaserBrightnessMin)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(raveLaserBrightnessMax)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(raveLaserDurationMin)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(raveLaserDurationMax)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(raveLaserThicknessMin)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(raveLaserThicknessMax)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(raveLaserNoiseMin)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(raveLaserNoiseMax)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(raveLaserFrameRateMin)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(raveLaserFrameRateMax)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(raveLaserMultiColor)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(cameraPosFixedX)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(cameraPosFixedY)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(cameraPosFixedZ)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(cameraPosOffX)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(cameraPosOffY)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(cameraPosOffZ)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(cameraRotFixedX)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(cameraRotFixedY)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(cameraRotFixedZ)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(cameraRotOffX)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(cameraRotOffY)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(cameraRotOffZ)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(imAllFuckedUp)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(thatWasntGrass)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(thatWasntPunch)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(fogNear)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(fogFar)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(NPCsTalkMore)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(myCameraSucks)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(shutupstuka)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(chromeEffect)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hassaultSpinMovement)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hassaultIdleSpinSound)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hassaultFireSound)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(muteBulletHitSounds)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(mutePlayerPainSounds)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hassaultIdleSpinSoundChannel)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hassaultSpinUpDownSoundChannel)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hassaultFireSoundChannel)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(geigerChannel)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hassaultWaitTime)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hassaultSpinupRemainTime)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hassaultResidualAttackTime)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hassaultSpinupStartTime)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hassaultVoicePitchMin)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hassaultVoicePitchMax)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hassaultFireSpread)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hassaultAllowGrenades)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(muteTempEntityGroundHitSound)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(houndeyeAttackMode)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(muteRicochetSound)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(mutePlayerWeaponFire)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(muteCrowbarSounds)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(scientistHealNPC)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(scientistHealNPCDebug)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(bulletholeAlertRange)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(fleshhitmakessound)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(nothingHurts)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(seeMonsterHealth)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(scientistHealNPCFract)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(bullsquidRangeDisabled)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(applyLKPPathFixToAll)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(timedDamageAffectsMonsters)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(scientistHealCooldown)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(crazyMonsterPrintouts)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(movementIsCompletePrintout)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(bulletHoleAlertPrintout)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(bulletholeAlertStukaOnly)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(barneyPrintouts)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(monsterSpawnPrintout)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(zombieBulletResistance)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(zombieBulletPushback)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(houndeyePrintout)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(quakeExplosionSound)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(explosionDebrisSoundVolume)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(noFlinchOnHard)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hassaultDrawLKP)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(meleeDrawBloodModeA)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(meleeDrawBloodModeB)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(drawDebugBloodTrace)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(meleeDrawBloodModeBFix)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(meleeDrawBloodModeAOffset)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(meleeDrawBloodModeBOffset)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY(wpn_glocksilencer)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(panthereyeHasCloakingAbility)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hgruntSpeedMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hgruntForceStrafeFireAnim)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hgruntLockRunAndGunTime)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hgruntHeadshotGore)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hgruntAllowStrafeFire)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(thoroughHitBoxUpdates)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hgruntTinyClip)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hgruntStrafeAlwaysHasAmmo)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hgruntBrassEjectForwardOffset)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(cl_muzzleflash)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(agrunt_muzzleflash)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(event5011Allowed)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(event5021Allowed)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(event5031Allowed)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(event5002Allowed)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(event5004Allowed)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(eventsAreFabulous)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(glockOldReloadLogic)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(glockOldReloadLogicBarney)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(barneyDroppedGlockAmmoCap)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(drawCollisionBoundsAtDeath)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(drawHitBoundsAtDeath)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(islaveReviveFriendMode)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(islaveReviveFriendChance)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(islaveReviveFriendRange)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(islaveReviveSelfMinDelay)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(islaveReviveSelfMaxDelay)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(islaveReviveSelfChance)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hgruntRunAndGunDistance)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hgruntPrintout)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(testVar)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(painArrowColorMode)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(painFlashColorMode)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(painFlashSuitless)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(painFlashDrownMode)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(firstPersonIdleDelayMin)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(firstPersonIdleDelayMax)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(forceDrawBatteryNumber)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(offsetgivedistance)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(offsetgivelookvertical)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(canShowWeaponSelectAtDeath)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(endlessFlashlightBattery)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(preE3UsesFailColors)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(E3UsesFailColors)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(preE3ShowsDamageIcons)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(E3ShowsDamageIcons)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(playerCrossbowMode)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hassassinCrossbowMode)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(tripmineAnimWaitsForFinish)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(revolverLaserScope)\
	DUMMY\
	DUMMY\
	DUMMY\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(auto_adjust_fov)\
	DUMMY\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(cheat_infiniteclip)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(cheat_infiniteammo)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(cheat_minimumfiredelay)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(cheat_minimumfiredelaycustom)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(cheat_nogaussrecoil)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(autoSneaky)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY(sv_longjump_chargemode)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(cheat_touchNeverExplodes)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(gaussRecoilSendsUpInSP)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(drawDebugPathfinding)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(STUcheckDistH)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(STUcheckDistV)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(STUcheckDistD)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(STUextraTriangH)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(STUextraTriangV)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(STUrepelMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(STUSpeedMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(STUExplodeTest)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(STUYawSpeedMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(STUDetection)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(stukaAdvancedCombat)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(drawDebugPathfinding2)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hassaultFriendlyFire)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(myStrobe)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(peopleStrobe)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(forceWorldLightOff)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(wildHeads)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(raveEffectSpawnInterval)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(drawBarnacleDebug)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(fogTest)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(sparksAllMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(sparksEnvMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(sparksButtonMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(sparksPlayerCrossbowMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(sparksComputerHitMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(sparksTurretDeathMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(sparksOspreyHitMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(sparksExplosionMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(sparksBeamMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(sparksAIFailMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(normalSpeedMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(noclipSpeedMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(jumpForceMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(apacheForceCinBounds)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(apacheBottomBoundAdj)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(apacheInfluence)\
	DUMMY\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(allowAlphaCrosshairWithoutGuns)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(alphaCrosshairBlockedOnFrozen)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hgruntRunAndGunDotMin)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(panthereyeJumpDotTol)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(panthereyePrintout)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(gargantuaPrintout)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(squadmonsterPrintout)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hassaultPrintout)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(barnaclePrintout)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(friendlyPrintout)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(stukaPrintout)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(timedDamageEndlessOnHard)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(mirrorsReflectOnlyNPCs)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(mirrorsDoNotReflectPlayer)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(r_shadows)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(altSquadRulesRuntime)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hgruntLockStrafeTime)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(pathfindIgnoreIsolatedNodes)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(drawNodeAll)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(drawNodeSpecial)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(drawNodeConnections)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(drawNodeAlternateTime)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(nodeSearchStartVerticalOffset)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(blockChangeLevelTrigger)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(blockMusicTrigger)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(blockMultiTrigger)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(blockTeleportTrigger)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(blockHurtTrigger)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(blockAutosaveTrigger)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hideNodeGraphRebuildNotice)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(barnacleTongueRetractDelay)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY(sv_germancensorship)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(allowGermanModels)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(germanRobotGibs)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(germanRobotBleedsOil)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(germanRobotDamageDecal)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(germanRobotGibsDecal)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(egonEffectsMode)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(egonHitCloud)\
	DUMMY\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(handGrenadePickupYieldsOne)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(handGrenadesUseOldBounceSound)\
	DUMMY\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(hud_moveselect_mousewheelsound)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(timedDamageDeathRemoveMode)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(hud_moveselect_sound)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(barnacleCanGib)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(sentryCanGib)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(miniturretCanGib)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(turretCanGib)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(turretBleedsOil)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(turretDamageDecal)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(turretGibDecal)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(canDropInSinglePlayer)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(timedDamageIgnoresArmor)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(itemBatteryPrerequisite)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(playerExtraPainSoundsMode)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(timedDamageDisableViewPunch)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(batteryDrainsAtDeath)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(batteryDrainsAtAdrenalineMode)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(canTakeLongJump)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(printOutCommonTimables)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(playerBrightLight)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(disablePainPunchAutomatic)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(gargantuaCorpseDeath)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(gargantuaFallSound)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(gargantuaBleeds)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(shrapMode)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(shrapRand)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(shrapRandHeightExtra)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(explosionShrapnelMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(useAlphaSparks)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(emergencyFix)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(timedDamageReviveRemoveMode)\
	DUMMY\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(ospreyIgnoresGruntCount)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(mp5GrenadeInheritsPlayerVelocity)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(crossbowInheritsPlayerVelocity)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(fastHornetsInheritsPlayerVelocity)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(snarkInheritsPlayerVelocity)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(chumtoadInheritsPlayerVelocity)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(weaponPickupPlaysAnyReloadSounds)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(glockUseLastBulletAnim)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(playerBarnacleVictimViewOffset)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hgruntMovementDeltaCheck)\
	EASY_CVAR_CREATE_SERVER_SERVERONLY_DEBUGONLY(hiddenMemPrintout)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hassaultExtraMuzzleFlashRadius)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hassaultExtraMuzzleFlashBrightness)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hassaultExtraMuzzleFlashForward)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(leaderlessSquadAllowed)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(nodeConnectionBreakableCheck)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(playerReviveInvincibilityTime)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(playerReviveBuddhaMode)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(playerReviveTimeBlocksTimedDamage)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hassaultBulletDamageMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hassaultBulletsPerShot)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hassaultFireAnimSpeedMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hassaultMeleeAnimSpeedMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hassassinCrossbowReloadSoundDelay)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hgruntStrafeAnimSpeedMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hgruntRunAndGunAnimSpeedMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(cheat_iwantguts)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(nodeDetailPrintout)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(soundAttenuationStuka)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(soundVolumeStuka)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(cineChangelevelFix)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(drawDebugCine)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(cineAllowSequenceOverwrite)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(stukaInflictsBleeding)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(animationKilledBoundsRemoval)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(gargantuaKilledBoundsAssist)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(bullsquidSpitTrajTimeMin)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(bullsquidSpitTrajTimeMax)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(bullsquidSpitTrajDistMin)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(bullsquidSpitTrajDistMax)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(bullsquidSpitGravityMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY(cl_bullsquidspit)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY(cl_bullsquidspitarc)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(bullsquidSpitUseAlphaModel)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(bullsquidSpitUseAlphaEffect)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(bullsquidSpitEffectSpread)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(bullsquidSpitEffectMin)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(bullsquidSpitEffectMax)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(bullsquidSpitEffectHitMin)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(bullsquidSpitEffectHitMax)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(bullsquidSpitEffectSpawn)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(bullsquidSpitEffectHitSpawn)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(bullsquidSpitSpriteScale)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(bullsquidSpitAlphaScale)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(scientistBravery)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(barneyUnholsterTime)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(barneyUnholsterAnimChoice)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(trailTypeTest)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hornetTrail)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hornetTrailSolidColor)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hornetDeathModEasy)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hornetDeathModMedium)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hornetDeathModHard)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hornetZoomPuff)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hornetSpiral)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hornetSpeedMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hornetSpeedDartMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY(cl_rockettrail)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(rocketTrailAlphaInterval)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(rocketTrailAlphaScale)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(rocketSkipIgnite)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(agruntHornetRandomness)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hornetSpiralPeriod)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hornetSpiralAmplitude)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY(cl_hornetspiral)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY(cl_hornettrail)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(hud_drawammobar)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(hud_weaponselecthideslower)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(hud_drawsidebarmode)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY(gauss_mode)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(painFlashDmgMin)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(painFlashDmgExMult)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(painFlashCumulativeMinDrowning)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(painFlashCumulativeMax)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(painFlashDrawOpacityMax)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(painArrowDrawOpacityMin)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(painArrowDrawOpacityMax)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(painFlashFadeMult)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(painArrowFadeMult)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(painFlashArmorBlock)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(painFlashDirTolerance)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(painArrowCumulativeAppearMin)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(painArrowCumulativeDmgJump)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(painFlashPrintouts)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(itemFlashCumulativeJump)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(itemFlashDrawOpacityMin)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(itemFlashDrawOpacityMax)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(itemFlashFadeMult)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(chumtoadPrintout)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(crossbowReloadSoundDelay)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(crossbowFirePlaysReloadSound)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(pathfindFidgetFailTime)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(pathfindPrintout)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(pathfindTopRampFixDistance)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(pathfindTopRampFixDraw)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(iHaveAscended)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(pathfindLooseMapNodes)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(pathfindRampFix)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(chumtoadPlayDeadFoolChance)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(animationFramerateMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(pathfindNodeToleranceMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(friendlyPianoFollowVolume)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(friendlyPianoOtherVolume)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(showtriggers)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(tentacleAlertSound)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(tentacleSwingSound1)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(tentacleSwingSound2)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(playerFollowerMax)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(announcerIsAJerk)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(playerUseDrawDebug)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(playerChumtoadThrowDrawDebug)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(peaceOut)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(drawViewModel)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(drawHUD)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(disablePauseSinglePlayer)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(playerBulletHitEffectForceServer)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(forceAllowServersideTextureSounds)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(playerWeaponSpreadMode)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(monsterAIForceFindDistance)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(baseEntityDamagePushNormalMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(baseEntityDamagePushVerticalBoost)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(baseEntityDamagePushVerticalMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(baseEntityDamagePushVerticalMinimum)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(viewModelPrintouts)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(viewModelSyncFixPrintouts)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(textureHitSoundPrintouts)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hgruntAllowGrenades)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(scheduleInterruptPrintouts)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(animationPrintouts)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hassaultMeleeAttackEnabled)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(pathfindStumpedWaitTime)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(pathfindStumpedMode)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(pathfindStumpedForgetEnemy)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(pathfindEdgeCheck)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(RadiusDamageDrawDebug)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(AlienRadiationImmunity)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(customLogoSprayMode)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(monsterFadeOutRate)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(playerFadeOutRate)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(drawDebugEnemyLKP)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(trackchangePrintouts)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(trackTrainPrintouts)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(cl_holster)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(playerWeaponTracerMode)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(monsterWeaponTracerMode)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(decalTracerExclusivity)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(monsterToPlayerHitgroupSpecial)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(multiplayerCrowbarHitSoundMode)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(healthcolor_fullRedMin)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(healthcolor_brightness)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(healthcolor_yellowMark)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(cl_drawExtraZeros)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(pathfindLargeBoundFix)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(flyerKilledFallingLoop)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(floaterDummy)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(barneyDummy)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(ladderCycleMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(ladderSpeedMulti)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(barnacleGrabNoInterpolation)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hideDamage)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(timedDamage_brightnessMax)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(timedDamage_brightnessMin)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(timedDamage_brightnessCap)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(timedDamage_brightnessFloor)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(timedDamage_flashSpeed)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(timedDamage_debug)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(wallHealthDoor_closeDelay)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(houndeye_attack_canGib)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(myRocketsAreBarney)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hassassinCrossbowDebug)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(crossbowBoltDirectionAffectedByWater)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(kingpinDebug)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(minimumRespawnDelay)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(r_glowshell_debug)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(cl_viewpunch)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY(cl_explosion)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY(soundSentenceSave)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY(pissedNPCs)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(hud_logo)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(hud_brokentrans)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(cl_fvox)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(cl_ladder)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY(precacheAll)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(cl_interp_entity)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(hud_swapFirstTwoBuckets)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY(ignoreMultiplayerSkillOverride)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(m_rawinput)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(cl_earlyaccess)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(cl_viewroll)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(drawDebugCrowbar)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY(blastExtraArmorDamageMode)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(hud_batteryhiddendead)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(hyperBarney)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY(monsterKilledToss)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(interpolation_movetypestep_mindelta)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY(sv_bloodparticlemode)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(cl_interp_view_extra)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(cl_interp_view_standard)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(cl_interp_viewmodel)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY(sv_explosionknockback)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(cl_gaussfollowattachment)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(cl_mp5_viewpunch_mod)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(cl_breakholster)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(pausecorrection1)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY(pausecorrection2)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(hud_rpg_clipless)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY(sv_rpg_clipless)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(cl_viewmodel_fidget)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY(sv_player_midair_fix)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY(sv_player_midair_accel)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(pathfindIgnoreNearestNodeCache)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(pathfindIgnoreStaticRoutes)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(pathfindForcePointHull)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(pathfindNearestNodeExtra)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY(sv_turret_postdeath)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(cl_mp5_evil_skip)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(egonRadiusDamageMode)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(egonFireRateMode)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY(sv_explosion_shake)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(sv_explosion_shake_range)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(sv_explosion_shake_amp)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(sv_explosion_shake_freq)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(sv_explosion_shake_duration)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(sv_gargantua_throwattack)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(cl_viewpunch_mod)\
	EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(cl_gauss_viewpunch_mod)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(pathfindMonsterclipFreshLogic)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY(sv_rpg_projectile_model)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY(hmilitaryDeadInvestigate)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY(playerDeadTruce)\
	EASY_CVAR_CREATE_SERVER_A_SERVERONLY(playerDeadTalkerBehavior)\
	DUMMY



#define EASY_CVAR_CREATE_CLIENT_MASS\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(gruntsCanHaveMP5Grenade)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(hud_version)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(hud_batterydraw)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(strobeDurationMin)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(strobeDurationMax)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(strobeRadiusMin)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(strobeRadiusMax)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(strobeSpawnDistHori)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(strobeSpawnDistVertMin)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(strobeSpawnDistVertMax)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(strobeMultiColor)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(raveLaserEnabled)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(raveLaserSpawnFreq)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(raveLaserLength)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMin)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMax)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMin)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMax)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(raveLaserBrightnessMin)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(raveLaserBrightnessMax)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(raveLaserDurationMin)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(raveLaserDurationMax)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(raveLaserThicknessMin)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(raveLaserThicknessMax)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(raveLaserNoiseMin)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(raveLaserNoiseMax)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(raveLaserFrameRateMin)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(raveLaserFrameRateMax)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(raveLaserMultiColor)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(cameraPosFixedX)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(cameraPosFixedY)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(cameraPosFixedZ)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(cameraPosOffX)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(cameraPosOffY)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(cameraPosOffZ)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(cameraRotFixedX)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(cameraRotFixedY)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(cameraRotFixedZ)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(cameraRotOffX)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(cameraRotOffY)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(cameraRotOffZ)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(imAllFuckedUp)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(thatWasntGrass)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(thatWasntPunch)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(fogNear)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(fogFar)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(NPCsTalkMore)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(myCameraSucks)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(shutupstuka)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(chromeEffect)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hassaultSpinMovement)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hassaultIdleSpinSound)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hassaultFireSound)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(muteBulletHitSounds)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(mutePlayerPainSounds)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hassaultIdleSpinSoundChannel)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hassaultSpinUpDownSoundChannel)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hassaultFireSoundChannel)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(geigerChannel)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hassaultWaitTime)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hassaultSpinupRemainTime)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hassaultResidualAttackTime)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hassaultSpinupStartTime)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hassaultVoicePitchMin)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hassaultVoicePitchMax)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hassaultFireSpread)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hassaultAllowGrenades)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(muteTempEntityGroundHitSound)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(houndeyeAttackMode)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(muteRicochetSound)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(mutePlayerWeaponFire)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(muteCrowbarSounds)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(scientistHealNPC)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(scientistHealNPCDebug)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(bulletholeAlertRange)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(fleshhitmakessound)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(nothingHurts)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(seeMonsterHealth)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(scientistHealNPCFract)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(bullsquidRangeDisabled)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(applyLKPPathFixToAll)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(timedDamageAffectsMonsters)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(scientistHealCooldown)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(crazyMonsterPrintouts)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(movementIsCompletePrintout)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(bulletHoleAlertPrintout)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(bulletholeAlertStukaOnly)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(barneyPrintouts)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(monsterSpawnPrintout)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(zombieBulletResistance)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(zombieBulletPushback)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(houndeyePrintout)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(quakeExplosionSound)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(explosionDebrisSoundVolume)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(noFlinchOnHard)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hassaultDrawLKP)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(meleeDrawBloodModeA)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(meleeDrawBloodModeB)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(drawDebugBloodTrace)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(meleeDrawBloodModeBFix)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(meleeDrawBloodModeAOffset)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(meleeDrawBloodModeBOffset)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY(wpn_glocksilencer)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(panthereyeHasCloakingAbility)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hgruntSpeedMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hgruntForceStrafeFireAnim)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hgruntLockRunAndGunTime)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hgruntHeadshotGore)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hgruntAllowStrafeFire)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(thoroughHitBoxUpdates)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hgruntTinyClip)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hgruntStrafeAlwaysHasAmmo)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hgruntBrassEjectForwardOffset)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(cl_muzzleflash)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(agrunt_muzzleflash)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(event5011Allowed)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(event5021Allowed)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(event5031Allowed)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(event5002Allowed)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(event5004Allowed)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(eventsAreFabulous)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(glockOldReloadLogic)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(glockOldReloadLogicBarney)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(barneyDroppedGlockAmmoCap)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(drawCollisionBoundsAtDeath)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(drawHitBoundsAtDeath)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(islaveReviveFriendMode)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(islaveReviveFriendChance)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(islaveReviveFriendRange)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(islaveReviveSelfMinDelay)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(islaveReviveSelfMaxDelay)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(islaveReviveSelfChance)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hgruntRunAndGunDistance)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hgruntPrintout)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(testVar)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(painArrowColorMode)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(painFlashColorMode)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(painFlashSuitless)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(painFlashDrownMode)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(firstPersonIdleDelayMin)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(firstPersonIdleDelayMax)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(forceDrawBatteryNumber)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(offsetgivedistance)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(offsetgivelookvertical)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(canShowWeaponSelectAtDeath)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(endlessFlashlightBattery)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(preE3UsesFailColors)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(E3UsesFailColors)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(preE3ShowsDamageIcons)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(E3ShowsDamageIcons)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(playerCrossbowMode)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hassassinCrossbowMode)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(tripmineAnimWaitsForFinish)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(revolverLaserScope)\
	DUMMY\
	DUMMY\
	DUMMY\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(auto_adjust_fov)\
	DUMMY\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(cheat_infiniteclip)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(cheat_infiniteammo)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(cheat_minimumfiredelay)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(cheat_minimumfiredelaycustom)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(cheat_nogaussrecoil)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(autoSneaky)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY(sv_longjump_chargemode)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(cheat_touchNeverExplodes)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(gaussRecoilSendsUpInSP)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(drawDebugPathfinding)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(STUcheckDistH)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(STUcheckDistV)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(STUcheckDistD)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(STUextraTriangH)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(STUextraTriangV)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(STUrepelMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(STUSpeedMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(STUExplodeTest)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(STUYawSpeedMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(STUDetection)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(stukaAdvancedCombat)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(drawDebugPathfinding2)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hassaultFriendlyFire)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(myStrobe)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(peopleStrobe)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(forceWorldLightOff)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(wildHeads)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(raveEffectSpawnInterval)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(drawBarnacleDebug)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(fogTest)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(sparksAllMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(sparksEnvMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(sparksButtonMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(sparksPlayerCrossbowMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(sparksComputerHitMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(sparksTurretDeathMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(sparksOspreyHitMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(sparksExplosionMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(sparksBeamMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(sparksAIFailMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(normalSpeedMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(noclipSpeedMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(jumpForceMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(apacheForceCinBounds)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(apacheBottomBoundAdj)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(apacheInfluence)\
	DUMMY\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(allowAlphaCrosshairWithoutGuns)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(alphaCrosshairBlockedOnFrozen)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hgruntRunAndGunDotMin)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(panthereyeJumpDotTol)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(panthereyePrintout)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(gargantuaPrintout)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(squadmonsterPrintout)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hassaultPrintout)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(barnaclePrintout)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(friendlyPrintout)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(stukaPrintout)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(timedDamageEndlessOnHard)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(mirrorsReflectOnlyNPCs)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(mirrorsDoNotReflectPlayer)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(r_shadows)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(altSquadRulesRuntime)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hgruntLockStrafeTime)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(pathfindIgnoreIsolatedNodes)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(drawNodeAll)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(drawNodeSpecial)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(drawNodeConnections)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(drawNodeAlternateTime)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(nodeSearchStartVerticalOffset)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(blockChangeLevelTrigger)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(blockMusicTrigger)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(blockMultiTrigger)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(blockTeleportTrigger)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(blockHurtTrigger)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(blockAutosaveTrigger)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hideNodeGraphRebuildNotice)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(barnacleTongueRetractDelay)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY(sv_germancensorship)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(allowGermanModels)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(germanRobotGibs)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(germanRobotBleedsOil)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(germanRobotDamageDecal)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(germanRobotGibsDecal)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(egonEffectsMode)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(egonHitCloud)\
	DUMMY\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(handGrenadePickupYieldsOne)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(handGrenadesUseOldBounceSound)\
	DUMMY\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(hud_moveselect_mousewheelsound)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(timedDamageDeathRemoveMode)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(hud_moveselect_sound)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(barnacleCanGib)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(sentryCanGib)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(miniturretCanGib)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(turretCanGib)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(turretBleedsOil)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(turretDamageDecal)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(turretGibDecal)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(canDropInSinglePlayer)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(timedDamageIgnoresArmor)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(itemBatteryPrerequisite)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(playerExtraPainSoundsMode)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(timedDamageDisableViewPunch)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(batteryDrainsAtDeath)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(batteryDrainsAtAdrenalineMode)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(canTakeLongJump)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(printOutCommonTimables)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(playerBrightLight)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(disablePainPunchAutomatic)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(gargantuaCorpseDeath)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(gargantuaFallSound)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(gargantuaBleeds)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(shrapMode)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(shrapRand)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(shrapRandHeightExtra)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(explosionShrapnelMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(useAlphaSparks)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(emergencyFix)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(timedDamageReviveRemoveMode)\
	DUMMY\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(ospreyIgnoresGruntCount)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(mp5GrenadeInheritsPlayerVelocity)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(crossbowInheritsPlayerVelocity)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(fastHornetsInheritsPlayerVelocity)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(snarkInheritsPlayerVelocity)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(chumtoadInheritsPlayerVelocity)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(weaponPickupPlaysAnyReloadSounds)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(glockUseLastBulletAnim)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(playerBarnacleVictimViewOffset)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hgruntMovementDeltaCheck)\
	EASY_CVAR_CREATE_CLIENT_SERVERONLY_DEBUGONLY(hiddenMemPrintout)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hassaultExtraMuzzleFlashRadius)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hassaultExtraMuzzleFlashBrightness)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hassaultExtraMuzzleFlashForward)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(leaderlessSquadAllowed)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(nodeConnectionBreakableCheck)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(playerReviveInvincibilityTime)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(playerReviveBuddhaMode)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(playerReviveTimeBlocksTimedDamage)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hassaultBulletDamageMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hassaultBulletsPerShot)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hassaultFireAnimSpeedMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hassaultMeleeAnimSpeedMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hassassinCrossbowReloadSoundDelay)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hgruntStrafeAnimSpeedMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hgruntRunAndGunAnimSpeedMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(cheat_iwantguts)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(nodeDetailPrintout)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(soundAttenuationStuka)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(soundVolumeStuka)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(cineChangelevelFix)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(drawDebugCine)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(cineAllowSequenceOverwrite)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(stukaInflictsBleeding)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(animationKilledBoundsRemoval)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(gargantuaKilledBoundsAssist)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(bullsquidSpitTrajTimeMin)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(bullsquidSpitTrajTimeMax)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(bullsquidSpitTrajDistMin)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(bullsquidSpitTrajDistMax)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(bullsquidSpitGravityMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY(cl_bullsquidspit)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY(cl_bullsquidspitarc)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(bullsquidSpitUseAlphaModel)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(bullsquidSpitUseAlphaEffect)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(bullsquidSpitEffectSpread)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(bullsquidSpitEffectMin)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(bullsquidSpitEffectMax)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(bullsquidSpitEffectHitMin)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(bullsquidSpitEffectHitMax)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(bullsquidSpitEffectSpawn)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(bullsquidSpitEffectHitSpawn)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(bullsquidSpitSpriteScale)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(bullsquidSpitAlphaScale)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(scientistBravery)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(barneyUnholsterTime)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(barneyUnholsterAnimChoice)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(trailTypeTest)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hornetTrail)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hornetTrailSolidColor)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hornetDeathModEasy)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hornetDeathModMedium)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hornetDeathModHard)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hornetZoomPuff)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hornetSpiral)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hornetSpeedMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hornetSpeedDartMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY(cl_rockettrail)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(rocketTrailAlphaInterval)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(rocketTrailAlphaScale)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(rocketSkipIgnite)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(agruntHornetRandomness)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hornetSpiralPeriod)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hornetSpiralAmplitude)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY(cl_hornetspiral)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY(cl_hornettrail)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(hud_drawammobar)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(hud_weaponselecthideslower)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(hud_drawsidebarmode)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY(gauss_mode)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(painFlashDmgMin)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(painFlashDmgExMult)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(painFlashCumulativeMinDrowning)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(painFlashCumulativeMax)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(painFlashDrawOpacityMax)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(painArrowDrawOpacityMin)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(painArrowDrawOpacityMax)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(painFlashFadeMult)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(painArrowFadeMult)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(painFlashArmorBlock)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(painFlashDirTolerance)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(painArrowCumulativeAppearMin)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(painArrowCumulativeDmgJump)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(painFlashPrintouts)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(itemFlashCumulativeJump)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(itemFlashDrawOpacityMin)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(itemFlashDrawOpacityMax)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(itemFlashFadeMult)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(chumtoadPrintout)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(crossbowReloadSoundDelay)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(crossbowFirePlaysReloadSound)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(pathfindFidgetFailTime)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(pathfindPrintout)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(pathfindTopRampFixDistance)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(pathfindTopRampFixDraw)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(iHaveAscended)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(pathfindLooseMapNodes)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(pathfindRampFix)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(chumtoadPlayDeadFoolChance)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(animationFramerateMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(pathfindNodeToleranceMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(friendlyPianoFollowVolume)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(friendlyPianoOtherVolume)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(showtriggers)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(tentacleAlertSound)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(tentacleSwingSound1)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(tentacleSwingSound2)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(playerFollowerMax)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(announcerIsAJerk)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(playerUseDrawDebug)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(playerChumtoadThrowDrawDebug)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(peaceOut)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(drawViewModel)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(drawHUD)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(disablePauseSinglePlayer)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(playerBulletHitEffectForceServer)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(forceAllowServersideTextureSounds)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(playerWeaponSpreadMode)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(monsterAIForceFindDistance)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(baseEntityDamagePushNormalMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(baseEntityDamagePushVerticalBoost)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(baseEntityDamagePushVerticalMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(baseEntityDamagePushVerticalMinimum)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(viewModelPrintouts)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(viewModelSyncFixPrintouts)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(textureHitSoundPrintouts)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hgruntAllowGrenades)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(scheduleInterruptPrintouts)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(animationPrintouts)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hassaultMeleeAttackEnabled)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(pathfindStumpedWaitTime)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(pathfindStumpedMode)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(pathfindStumpedForgetEnemy)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(pathfindEdgeCheck)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(RadiusDamageDrawDebug)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(AlienRadiationImmunity)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(customLogoSprayMode)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(monsterFadeOutRate)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(playerFadeOutRate)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(drawDebugEnemyLKP)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(trackchangePrintouts)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(trackTrainPrintouts)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(cl_holster)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(playerWeaponTracerMode)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(monsterWeaponTracerMode)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(decalTracerExclusivity)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(monsterToPlayerHitgroupSpecial)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(multiplayerCrowbarHitSoundMode)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(healthcolor_fullRedMin)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(healthcolor_brightness)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(healthcolor_yellowMark)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(cl_drawExtraZeros)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(pathfindLargeBoundFix)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(flyerKilledFallingLoop)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(floaterDummy)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(barneyDummy)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(ladderCycleMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(ladderSpeedMulti)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(barnacleGrabNoInterpolation)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hideDamage)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(timedDamage_brightnessMax)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(timedDamage_brightnessMin)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(timedDamage_brightnessCap)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(timedDamage_brightnessFloor)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(timedDamage_flashSpeed)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(timedDamage_debug)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(wallHealthDoor_closeDelay)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(houndeye_attack_canGib)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(myRocketsAreBarney)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hassassinCrossbowDebug)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(crossbowBoltDirectionAffectedByWater)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(kingpinDebug)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(minimumRespawnDelay)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(r_glowshell_debug)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(cl_viewpunch)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY(cl_explosion)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY(soundSentenceSave)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY(pissedNPCs)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(hud_logo)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(hud_brokentrans)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(cl_fvox)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(cl_ladder)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY(precacheAll)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(cl_interp_entity)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(hud_swapFirstTwoBuckets)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY(ignoreMultiplayerSkillOverride)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(m_rawinput)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(cl_earlyaccess)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(cl_viewroll)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(drawDebugCrowbar)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY(blastExtraArmorDamageMode)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(hud_batteryhiddendead)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(hyperBarney)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY(monsterKilledToss)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(interpolation_movetypestep_mindelta)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY(sv_bloodparticlemode)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(cl_interp_view_extra)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(cl_interp_view_standard)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(cl_interp_viewmodel)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY(sv_explosionknockback)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(cl_gaussfollowattachment)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(cl_mp5_viewpunch_mod)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(cl_breakholster)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(pausecorrection1)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY(pausecorrection2)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(hud_rpg_clipless)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY(sv_rpg_clipless)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(cl_viewmodel_fidget)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY(sv_player_midair_fix)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY(sv_player_midair_accel)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(pathfindIgnoreNearestNodeCache)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(pathfindIgnoreStaticRoutes)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(pathfindForcePointHull)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(pathfindNearestNodeExtra)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY(sv_turret_postdeath)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(cl_mp5_evil_skip)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(egonRadiusDamageMode)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(egonFireRateMode)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY(sv_explosion_shake)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(sv_explosion_shake_range)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(sv_explosion_shake_amp)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(sv_explosion_shake_freq)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(sv_explosion_shake_duration)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(sv_gargantua_throwattack)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(cl_viewpunch_mod)\
	EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(cl_gauss_viewpunch_mod)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(pathfindMonsterclipFreshLogic)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY(sv_rpg_projectile_model)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY(hmilitaryDeadInvestigate)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY(playerDeadTruce)\
	EASY_CVAR_CREATE_CLIENT_A_SERVERONLY(playerDeadTalkerBehavior)\
	DUMMY



#define EASY_CVAR_RESET_MASS\
	EASY_CVAR_RESET_DEBUGONLY(gruntsCanHaveMP5Grenade)\
	EASY_CVAR_RESET(hud_version)\
	EASY_CVAR_RESET(hud_batterydraw)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(strobeDurationMin)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(strobeDurationMax)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(strobeRadiusMin)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(strobeRadiusMax)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(strobeSpawnDistHori)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(strobeSpawnDistVertMin)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(strobeSpawnDistVertMax)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(strobeMultiColor)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(raveLaserEnabled)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(raveLaserSpawnFreq)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(raveLaserLength)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMin)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMax)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMin)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMax)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(raveLaserBrightnessMin)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(raveLaserBrightnessMax)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(raveLaserDurationMin)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(raveLaserDurationMax)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(raveLaserThicknessMin)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(raveLaserThicknessMax)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(raveLaserNoiseMin)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(raveLaserNoiseMax)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(raveLaserFrameRateMin)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(raveLaserFrameRateMax)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(raveLaserMultiColor)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(cameraPosFixedX)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(cameraPosFixedY)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(cameraPosFixedZ)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(cameraPosOffX)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(cameraPosOffY)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(cameraPosOffZ)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(cameraRotFixedX)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(cameraRotFixedY)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(cameraRotFixedZ)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(cameraRotOffX)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(cameraRotOffY)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(cameraRotOffZ)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(imAllFuckedUp)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(thatWasntGrass)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(fogNear)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(fogFar)\
	EASY_CVAR_RESET_DEBUGONLY(NPCsTalkMore)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(myCameraSucks)\
	EASY_CVAR_RESET_DEBUGONLY(shutupstuka)\
	EASY_CVAR_RESET(chromeEffect)\
	EASY_CVAR_RESET_DEBUGONLY(hassaultSpinMovement)\
	EASY_CVAR_RESET_DEBUGONLY(hassaultIdleSpinSound)\
	EASY_CVAR_RESET_DEBUGONLY(hassaultFireSound)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteBulletHitSounds)\
	EASY_CVAR_RESET_DEBUGONLY(mutePlayerPainSounds)\
	EASY_CVAR_RESET_DEBUGONLY(hassaultIdleSpinSoundChannel)\
	EASY_CVAR_RESET_DEBUGONLY(hassaultSpinUpDownSoundChannel)\
	EASY_CVAR_RESET_DEBUGONLY(hassaultFireSoundChannel)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(geigerChannel)\
	EASY_CVAR_RESET_DEBUGONLY(hassaultWaitTime)\
	EASY_CVAR_RESET_DEBUGONLY(hassaultSpinupRemainTime)\
	EASY_CVAR_RESET_DEBUGONLY(hassaultResidualAttackTime)\
	EASY_CVAR_RESET_DEBUGONLY(hassaultSpinupStartTime)\
	EASY_CVAR_RESET_DEBUGONLY(hassaultVoicePitchMin)\
	EASY_CVAR_RESET_DEBUGONLY(hassaultVoicePitchMax)\
	EASY_CVAR_RESET_DEBUGONLY(hassaultFireSpread)\
	EASY_CVAR_RESET_DEBUGONLY(hassaultAllowGrenades)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(muteTempEntityGroundHitSound)\
	EASY_CVAR_RESET_DEBUGONLY(houndeyeAttackMode)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteRicochetSound)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(mutePlayerWeaponFire)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteCrowbarSounds)\
	EASY_CVAR_RESET_DEBUGONLY(scientistHealNPC)\
	EASY_CVAR_RESET_DEBUGONLY(scientistHealNPCDebug)\
	EASY_CVAR_RESET_DEBUGONLY(bulletholeAlertRange)\
	EASY_CVAR_RESET_DEBUGONLY(fleshhitmakessound)\
	EASY_CVAR_RESET_DEBUGONLY(nothingHurts)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(seeMonsterHealth)\
	EASY_CVAR_RESET_DEBUGONLY(scientistHealNPCFract)\
	EASY_CVAR_RESET_DEBUGONLY(bullsquidRangeDisabled)\
	EASY_CVAR_RESET_DEBUGONLY(applyLKPPathFixToAll)\
	EASY_CVAR_RESET_DEBUGONLY(timedDamageAffectsMonsters)\
	EASY_CVAR_RESET_DEBUGONLY(scientistHealCooldown)\
	EASY_CVAR_RESET_DEBUGONLY(crazyMonsterPrintouts)\
	EASY_CVAR_RESET_DEBUGONLY(movementIsCompletePrintout)\
	EASY_CVAR_RESET_DEBUGONLY(bulletHoleAlertPrintout)\
	EASY_CVAR_RESET_DEBUGONLY(bulletholeAlertStukaOnly)\
	EASY_CVAR_RESET_DEBUGONLY(barneyPrintouts)\
	EASY_CVAR_RESET_DEBUGONLY(monsterSpawnPrintout)\
	EASY_CVAR_RESET_DEBUGONLY(zombieBulletResistance)\
	EASY_CVAR_RESET_DEBUGONLY(zombieBulletPushback)\
	EASY_CVAR_RESET_DEBUGONLY(houndeyePrintout)\
	EASY_CVAR_RESET_DEBUGONLY(quakeExplosionSound)\
	EASY_CVAR_RESET_DEBUGONLY(explosionDebrisSoundVolume)\
	EASY_CVAR_RESET_DEBUGONLY(noFlinchOnHard)\
	EASY_CVAR_RESET_DEBUGONLY(hassaultDrawLKP)\
	EASY_CVAR_RESET_DEBUGONLY(meleeDrawBloodModeA)\
	EASY_CVAR_RESET_DEBUGONLY(meleeDrawBloodModeB)\
	EASY_CVAR_RESET_DEBUGONLY(drawDebugBloodTrace)\
	EASY_CVAR_RESET_DEBUGONLY(meleeDrawBloodModeBFix)\
	EASY_CVAR_RESET_DEBUGONLY(meleeDrawBloodModeAOffset)\
	EASY_CVAR_RESET_DEBUGONLY(meleeDrawBloodModeBOffset)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST(wpn_glocksilencer)\
	EASY_CVAR_RESET_DEBUGONLY(panthereyeHasCloakingAbility)\
	EASY_CVAR_RESET_DEBUGONLY(hgruntSpeedMulti)\
	EASY_CVAR_RESET_DEBUGONLY(hgruntForceStrafeFireAnim)\
	EASY_CVAR_RESET_DEBUGONLY(hgruntLockRunAndGunTime)\
	EASY_CVAR_RESET_DEBUGONLY(hgruntHeadshotGore)\
	EASY_CVAR_RESET_DEBUGONLY(hgruntAllowStrafeFire)\
	EASY_CVAR_RESET_DEBUGONLY(thoroughHitBoxUpdates)\
	EASY_CVAR_RESET_DEBUGONLY(hgruntTinyClip)\
	EASY_CVAR_RESET_DEBUGONLY(hgruntStrafeAlwaysHasAmmo)\
	EASY_CVAR_RESET_DEBUGONLY(hgruntBrassEjectForwardOffset)\
	EASY_CVAR_RESET(cl_muzzleflash)\
	EASY_CVAR_RESET_DEBUGONLY(agrunt_muzzleflash)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(event5011Allowed)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(event5021Allowed)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(event5031Allowed)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(event5002Allowed)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(event5004Allowed)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(eventsAreFabulous)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(glockOldReloadLogic)\
	EASY_CVAR_RESET_DEBUGONLY(glockOldReloadLogicBarney)\
	EASY_CVAR_RESET_DEBUGONLY(barneyDroppedGlockAmmoCap)\
	EASY_CVAR_RESET_DEBUGONLY(drawCollisionBoundsAtDeath)\
	EASY_CVAR_RESET_DEBUGONLY(drawHitBoundsAtDeath)\
	EASY_CVAR_RESET_DEBUGONLY(islaveReviveFriendMode)\
	EASY_CVAR_RESET_DEBUGONLY(islaveReviveFriendChance)\
	EASY_CVAR_RESET_DEBUGONLY(islaveReviveFriendRange)\
	EASY_CVAR_RESET_DEBUGONLY(islaveReviveSelfMinDelay)\
	EASY_CVAR_RESET_DEBUGONLY(islaveReviveSelfMaxDelay)\
	EASY_CVAR_RESET_DEBUGONLY(islaveReviveSelfChance)\
	EASY_CVAR_RESET_DEBUGONLY(hgruntRunAndGunDistance)\
	EASY_CVAR_RESET_DEBUGONLY(hgruntPrintout)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(painArrowColorMode)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(painFlashColorMode)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(painFlashSuitless)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(painFlashDrownMode)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(firstPersonIdleDelayMin)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(firstPersonIdleDelayMax)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(forceDrawBatteryNumber)\
	EASY_CVAR_RESET_DEBUGONLY(offsetgivedistance)\
	EASY_CVAR_RESET_DEBUGONLY(offsetgivelookvertical)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(canShowWeaponSelectAtDeath)\
	EASY_CVAR_RESET_DEBUGONLY(endlessFlashlightBattery)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(preE3UsesFailColors)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(E3UsesFailColors)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(preE3ShowsDamageIcons)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(E3ShowsDamageIcons)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerCrossbowMode)\
	EASY_CVAR_RESET_DEBUGONLY(hassassinCrossbowMode)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(tripmineAnimWaitsForFinish)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(revolverLaserScope)\
	DUMMY\
	DUMMY\
	DUMMY\
	EASY_CVAR_RESET_CLIENTONLY(auto_adjust_fov)\
	DUMMY\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_nogaussrecoil)\
	EASY_CVAR_RESET_DEBUGONLY(autoSneaky)\
	EASY_CVAR_RESET(sv_longjump_chargemode)\
	EASY_CVAR_RESET_DEBUGONLY(cheat_touchNeverExplodes)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(gaussRecoilSendsUpInSP)\
	EASY_CVAR_RESET_DEBUGONLY(drawDebugPathfinding)\
	EASY_CVAR_RESET_DEBUGONLY(STUcheckDistH)\
	EASY_CVAR_RESET_DEBUGONLY(STUcheckDistV)\
	EASY_CVAR_RESET_DEBUGONLY(STUcheckDistD)\
	EASY_CVAR_RESET_DEBUGONLY(STUextraTriangH)\
	EASY_CVAR_RESET_DEBUGONLY(STUextraTriangV)\
	EASY_CVAR_RESET_DEBUGONLY(STUrepelMulti)\
	EASY_CVAR_RESET_DEBUGONLY(STUSpeedMulti)\
	EASY_CVAR_RESET_DEBUGONLY(STUExplodeTest)\
	EASY_CVAR_RESET_DEBUGONLY(STUYawSpeedMulti)\
	EASY_CVAR_RESET_DEBUGONLY(STUDetection)\
	EASY_CVAR_RESET_DEBUGONLY(stukaAdvancedCombat)\
	EASY_CVAR_RESET_DEBUGONLY(drawDebugPathfinding2)\
	EASY_CVAR_RESET_DEBUGONLY(hassaultFriendlyFire)\
	EASY_CVAR_RESET_DEBUGONLY(myStrobe)\
	EASY_CVAR_RESET_DEBUGONLY(peopleStrobe)\
	EASY_CVAR_RESET_DEBUGONLY(forceWorldLightOff)\
	EASY_CVAR_RESET_DEBUGONLY(wildHeads)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(raveEffectSpawnInterval)\
	EASY_CVAR_RESET_DEBUGONLY(drawBarnacleDebug)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(fogTest)\
	EASY_CVAR_RESET_DEBUGONLY(sparksAllMulti)\
	EASY_CVAR_RESET_DEBUGONLY(sparksEnvMulti)\
	EASY_CVAR_RESET_DEBUGONLY(sparksButtonMulti)\
	EASY_CVAR_RESET_DEBUGONLY(sparksPlayerCrossbowMulti)\
	EASY_CVAR_RESET_DEBUGONLY(sparksComputerHitMulti)\
	EASY_CVAR_RESET_DEBUGONLY(sparksTurretDeathMulti)\
	EASY_CVAR_RESET_DEBUGONLY(sparksOspreyHitMulti)\
	EASY_CVAR_RESET_DEBUGONLY(sparksExplosionMulti)\
	EASY_CVAR_RESET_DEBUGONLY(sparksBeamMulti)\
	EASY_CVAR_RESET_DEBUGONLY(sparksAIFailMulti)\
	EASY_CVAR_RESET_DEBUGONLY(normalSpeedMulti)\
	EASY_CVAR_RESET_DEBUGONLY(noclipSpeedMulti)\
	EASY_CVAR_RESET_DEBUGONLY(jumpForceMulti)\
	EASY_CVAR_RESET_DEBUGONLY(apacheForceCinBounds)\
	EASY_CVAR_RESET_DEBUGONLY(apacheBottomBoundAdj)\
	EASY_CVAR_RESET_DEBUGONLY(apacheInfluence)\
	DUMMY\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(allowAlphaCrosshairWithoutGuns)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(alphaCrosshairBlockedOnFrozen)\
	EASY_CVAR_RESET_DEBUGONLY(hgruntRunAndGunDotMin)\
	EASY_CVAR_RESET_DEBUGONLY(panthereyeJumpDotTol)\
	EASY_CVAR_RESET_DEBUGONLY(panthereyePrintout)\
	EASY_CVAR_RESET_DEBUGONLY(gargantuaPrintout)\
	EASY_CVAR_RESET_DEBUGONLY(squadmonsterPrintout)\
	EASY_CVAR_RESET_DEBUGONLY(hassaultPrintout)\
	EASY_CVAR_RESET_DEBUGONLY(barnaclePrintout)\
	EASY_CVAR_RESET_DEBUGONLY(friendlyPrintout)\
	EASY_CVAR_RESET_DEBUGONLY(stukaPrintout)\
	EASY_CVAR_RESET_DEBUGONLY(timedDamageEndlessOnHard)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(mirrorsReflectOnlyNPCs)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(mirrorsDoNotReflectPlayer)\
	EASY_CVAR_RESET(r_shadows)\
	EASY_CVAR_RESET_DEBUGONLY(altSquadRulesRuntime)\
	EASY_CVAR_RESET_DEBUGONLY(hgruntLockStrafeTime)\
	EASY_CVAR_RESET_DEBUGONLY(pathfindIgnoreIsolatedNodes)\
	EASY_CVAR_RESET_DEBUGONLY(drawNodeAll)\
	EASY_CVAR_RESET_DEBUGONLY(drawNodeSpecial)\
	EASY_CVAR_RESET_DEBUGONLY(drawNodeConnections)\
	EASY_CVAR_RESET_DEBUGONLY(drawNodeAlternateTime)\
	EASY_CVAR_RESET_DEBUGONLY(nodeSearchStartVerticalOffset)\
	EASY_CVAR_RESET_DEBUGONLY(blockChangeLevelTrigger)\
	EASY_CVAR_RESET_DEBUGONLY(blockMusicTrigger)\
	EASY_CVAR_RESET_DEBUGONLY(blockMultiTrigger)\
	EASY_CVAR_RESET_DEBUGONLY(blockTeleportTrigger)\
	EASY_CVAR_RESET_DEBUGONLY(blockHurtTrigger)\
	EASY_CVAR_RESET_DEBUGONLY(blockAutosaveTrigger)\
	EASY_CVAR_RESET_DEBUGONLY(hideNodeGraphRebuildNotice)\
	EASY_CVAR_RESET_DEBUGONLY(barnacleTongueRetractDelay)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST(sv_germancensorship)\
	EASY_CVAR_RESET_DEBUGONLY(allowGermanModels)\
	EASY_CVAR_RESET_DEBUGONLY(germanRobotGibs)\
	EASY_CVAR_RESET_DEBUGONLY(germanRobotBleedsOil)\
	EASY_CVAR_RESET_DEBUGONLY(germanRobotDamageDecal)\
	EASY_CVAR_RESET_DEBUGONLY(germanRobotGibsDecal)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonEffectsMode)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonHitCloud)\
	DUMMY\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(handGrenadePickupYieldsOne)\
	EASY_CVAR_RESET_DEBUGONLY(handGrenadesUseOldBounceSound)\
	DUMMY\
	EASY_CVAR_RESET(hud_moveselect_mousewheelsound)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(timedDamageDeathRemoveMode)\
	EASY_CVAR_RESET(hud_moveselect_sound)\
	EASY_CVAR_RESET_DEBUGONLY(barnacleCanGib)\
	EASY_CVAR_RESET_DEBUGONLY(sentryCanGib)\
	EASY_CVAR_RESET_DEBUGONLY(miniturretCanGib)\
	EASY_CVAR_RESET_DEBUGONLY(turretCanGib)\
	EASY_CVAR_RESET_DEBUGONLY(turretBleedsOil)\
	EASY_CVAR_RESET_DEBUGONLY(turretDamageDecal)\
	EASY_CVAR_RESET_DEBUGONLY(turretGibDecal)\
	EASY_CVAR_RESET_DEBUGONLY(canDropInSinglePlayer)\
	EASY_CVAR_RESET_DEBUGONLY(timedDamageIgnoresArmor)\
	EASY_CVAR_RESET_DEBUGONLY(itemBatteryPrerequisite)\
	EASY_CVAR_RESET_DEBUGONLY(playerExtraPainSoundsMode)\
	EASY_CVAR_RESET_DEBUGONLY(timedDamageDisableViewPunch)\
	EASY_CVAR_RESET_DEBUGONLY(batteryDrainsAtDeath)\
	EASY_CVAR_RESET_DEBUGONLY(batteryDrainsAtAdrenalineMode)\
	EASY_CVAR_RESET_DEBUGONLY(canTakeLongJump)\
	EASY_CVAR_RESET_DEBUGONLY(printOutCommonTimables)\
	EASY_CVAR_RESET_DEBUGONLY(playerBrightLight)\
	EASY_CVAR_RESET_DEBUGONLY(disablePainPunchAutomatic)\
	EASY_CVAR_RESET_DEBUGONLY(gargantuaCorpseDeath)\
	EASY_CVAR_RESET_DEBUGONLY(gargantuaFallSound)\
	EASY_CVAR_RESET_DEBUGONLY(gargantuaBleeds)\
	EASY_CVAR_RESET_DEBUGONLY(shrapMode)\
	EASY_CVAR_RESET_DEBUGONLY(shrapRand)\
	EASY_CVAR_RESET_DEBUGONLY(shrapRandHeightExtra)\
	EASY_CVAR_RESET_DEBUGONLY(explosionShrapnelMulti)\
	EASY_CVAR_RESET_DEBUGONLY(useAlphaSparks)\
	EASY_CVAR_RESET_DEBUGONLY(emergencyFix)\
	EASY_CVAR_RESET_DEBUGONLY(timedDamageReviveRemoveMode)\
	DUMMY\
	EASY_CVAR_RESET_DEBUGONLY(ospreyIgnoresGruntCount)\
	EASY_CVAR_RESET_DEBUGONLY(mp5GrenadeInheritsPlayerVelocity)\
	EASY_CVAR_RESET_DEBUGONLY(crossbowInheritsPlayerVelocity)\
	EASY_CVAR_RESET_DEBUGONLY(fastHornetsInheritsPlayerVelocity)\
	EASY_CVAR_RESET_DEBUGONLY(snarkInheritsPlayerVelocity)\
	EASY_CVAR_RESET_DEBUGONLY(chumtoadInheritsPlayerVelocity)\
	EASY_CVAR_RESET_DEBUGONLY(weaponPickupPlaysAnyReloadSounds)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(glockUseLastBulletAnim)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerBarnacleVictimViewOffset)\
	EASY_CVAR_RESET_DEBUGONLY(hgruntMovementDeltaCheck)\
	EASY_CVAR_RESET_DEBUGONLY(hassaultExtraMuzzleFlashRadius)\
	EASY_CVAR_RESET_DEBUGONLY(hassaultExtraMuzzleFlashBrightness)\
	EASY_CVAR_RESET_DEBUGONLY(hassaultExtraMuzzleFlashForward)\
	EASY_CVAR_RESET_DEBUGONLY(leaderlessSquadAllowed)\
	EASY_CVAR_RESET_DEBUGONLY(nodeConnectionBreakableCheck)\
	EASY_CVAR_RESET_DEBUGONLY(playerReviveInvincibilityTime)\
	EASY_CVAR_RESET_DEBUGONLY(playerReviveBuddhaMode)\
	EASY_CVAR_RESET_DEBUGONLY(playerReviveTimeBlocksTimedDamage)\
	EASY_CVAR_RESET_DEBUGONLY(hassaultBulletDamageMulti)\
	EASY_CVAR_RESET_DEBUGONLY(hassaultBulletsPerShot)\
	EASY_CVAR_RESET_DEBUGONLY(hassaultFireAnimSpeedMulti)\
	EASY_CVAR_RESET_DEBUGONLY(hassaultMeleeAnimSpeedMulti)\
	EASY_CVAR_RESET_DEBUGONLY(hassassinCrossbowReloadSoundDelay)\
	EASY_CVAR_RESET_DEBUGONLY(hgruntStrafeAnimSpeedMulti)\
	EASY_CVAR_RESET_DEBUGONLY(hgruntRunAndGunAnimSpeedMulti)\
	EASY_CVAR_RESET_DEBUGONLY(cheat_iwantguts)\
	EASY_CVAR_RESET_DEBUGONLY(nodeDetailPrintout)\
	EASY_CVAR_RESET_DEBUGONLY(soundAttenuationStuka)\
	EASY_CVAR_RESET_DEBUGONLY(soundVolumeStuka)\
	EASY_CVAR_RESET_DEBUGONLY(cineChangelevelFix)\
	EASY_CVAR_RESET_DEBUGONLY(drawDebugCine)\
	EASY_CVAR_RESET_DEBUGONLY(cineAllowSequenceOverwrite)\
	EASY_CVAR_RESET_DEBUGONLY(stukaInflictsBleeding)\
	EASY_CVAR_RESET_DEBUGONLY(animationKilledBoundsRemoval)\
	EASY_CVAR_RESET_DEBUGONLY(gargantuaKilledBoundsAssist)\
	EASY_CVAR_RESET_DEBUGONLY(bullsquidSpitTrajTimeMin)\
	EASY_CVAR_RESET_DEBUGONLY(bullsquidSpitTrajTimeMax)\
	EASY_CVAR_RESET_DEBUGONLY(bullsquidSpitTrajDistMin)\
	EASY_CVAR_RESET_DEBUGONLY(bullsquidSpitTrajDistMax)\
	EASY_CVAR_RESET_DEBUGONLY(bullsquidSpitGravityMulti)\
	EASY_CVAR_RESET(cl_bullsquidspit)\
	EASY_CVAR_RESET(cl_bullsquidspitarc)\
	EASY_CVAR_RESET_DEBUGONLY(bullsquidSpitUseAlphaModel)\
	EASY_CVAR_RESET_DEBUGONLY(bullsquidSpitUseAlphaEffect)\
	EASY_CVAR_RESET_DEBUGONLY(bullsquidSpitEffectSpread)\
	EASY_CVAR_RESET_DEBUGONLY(bullsquidSpitEffectMin)\
	EASY_CVAR_RESET_DEBUGONLY(bullsquidSpitEffectMax)\
	EASY_CVAR_RESET_DEBUGONLY(bullsquidSpitEffectHitMin)\
	EASY_CVAR_RESET_DEBUGONLY(bullsquidSpitEffectHitMax)\
	EASY_CVAR_RESET_DEBUGONLY(bullsquidSpitEffectSpawn)\
	EASY_CVAR_RESET_DEBUGONLY(bullsquidSpitEffectHitSpawn)\
	EASY_CVAR_RESET_DEBUGONLY(bullsquidSpitSpriteScale)\
	EASY_CVAR_RESET_DEBUGONLY(bullsquidSpitAlphaScale)\
	EASY_CVAR_RESET_DEBUGONLY(scientistBravery)\
	EASY_CVAR_RESET_DEBUGONLY(barneyUnholsterTime)\
	EASY_CVAR_RESET_DEBUGONLY(barneyUnholsterAnimChoice)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(trailTypeTest)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetTrail)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetTrailSolidColor)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetDeathModEasy)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetDeathModMedium)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetDeathModHard)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetZoomPuff)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetSpiral)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetSpeedMulti)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetSpeedDartMulti)\
	EASY_CVAR_RESET(cl_rockettrail)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketTrailAlphaInterval)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketTrailAlphaScale)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketSkipIgnite)\
	EASY_CVAR_RESET_DEBUGONLY(agruntHornetRandomness)\
	EASY_CVAR_RESET_DEBUGONLY(hornetSpiralPeriod)\
	EASY_CVAR_RESET_DEBUGONLY(hornetSpiralAmplitude)\
	EASY_CVAR_RESET(cl_hornetspiral)\
	EASY_CVAR_RESET(cl_hornettrail)\
	EASY_CVAR_RESET(hud_drawammobar)\
	EASY_CVAR_RESET(hud_weaponselecthideslower)\
	EASY_CVAR_RESET(hud_drawsidebarmode)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST(gauss_mode)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(painFlashDmgMin)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(painFlashDmgExMult)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(painFlashCumulativeMinDrowning)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(painFlashCumulativeMax)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(painFlashDrawOpacityMax)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(painArrowDrawOpacityMin)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(painArrowDrawOpacityMax)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(painFlashFadeMult)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(painArrowFadeMult)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(painFlashArmorBlock)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(painFlashDirTolerance)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(painArrowCumulativeAppearMin)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(painArrowCumulativeDmgJump)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(painFlashPrintouts)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(itemFlashCumulativeJump)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(itemFlashDrawOpacityMin)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(itemFlashDrawOpacityMax)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(itemFlashFadeMult)\
	EASY_CVAR_RESET_DEBUGONLY(chumtoadPrintout)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(crossbowReloadSoundDelay)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(crossbowFirePlaysReloadSound)\
	EASY_CVAR_RESET_DEBUGONLY(pathfindFidgetFailTime)\
	EASY_CVAR_RESET_DEBUGONLY(pathfindPrintout)\
	EASY_CVAR_RESET_DEBUGONLY(pathfindTopRampFixDistance)\
	EASY_CVAR_RESET_DEBUGONLY(pathfindTopRampFixDraw)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(iHaveAscended)\
	EASY_CVAR_RESET_DEBUGONLY(pathfindLooseMapNodes)\
	EASY_CVAR_RESET_DEBUGONLY(pathfindRampFix)\
	EASY_CVAR_RESET_DEBUGONLY(chumtoadPlayDeadFoolChance)\
	EASY_CVAR_RESET_DEBUGONLY(animationFramerateMulti)\
	EASY_CVAR_RESET_DEBUGONLY(pathfindNodeToleranceMulti)\
	EASY_CVAR_RESET_DEBUGONLY(friendlyPianoFollowVolume)\
	EASY_CVAR_RESET_DEBUGONLY(friendlyPianoOtherVolume)\
	EASY_CVAR_RESET_DEBUGONLY(showtriggers)\
	EASY_CVAR_RESET_DEBUGONLY(tentacleAlertSound)\
	EASY_CVAR_RESET_DEBUGONLY(tentacleSwingSound1)\
	EASY_CVAR_RESET_DEBUGONLY(tentacleSwingSound2)\
	EASY_CVAR_RESET_DEBUGONLY(playerFollowerMax)\
	EASY_CVAR_RESET_DEBUGONLY(announcerIsAJerk)\
	EASY_CVAR_RESET_DEBUGONLY(playerUseDrawDebug)\
	EASY_CVAR_RESET_DEBUGONLY(playerChumtoadThrowDrawDebug)\
	EASY_CVAR_RESET_DEBUGONLY(peaceOut)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(drawViewModel)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(drawHUD)\
	EASY_CVAR_RESET_DEBUGONLY(disablePauseSinglePlayer)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerBulletHitEffectForceServer)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(forceAllowServersideTextureSounds)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerWeaponSpreadMode)\
	EASY_CVAR_RESET_DEBUGONLY(monsterAIForceFindDistance)\
	EASY_CVAR_RESET_DEBUGONLY(baseEntityDamagePushNormalMulti)\
	EASY_CVAR_RESET_DEBUGONLY(baseEntityDamagePushVerticalBoost)\
	EASY_CVAR_RESET_DEBUGONLY(baseEntityDamagePushVerticalMulti)\
	EASY_CVAR_RESET_DEBUGONLY(baseEntityDamagePushVerticalMinimum)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelSyncFixPrintouts)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(textureHitSoundPrintouts)\
	EASY_CVAR_RESET_DEBUGONLY(hgruntAllowGrenades)\
	EASY_CVAR_RESET_DEBUGONLY(scheduleInterruptPrintouts)\
	EASY_CVAR_RESET_DEBUGONLY(animationPrintouts)\
	EASY_CVAR_RESET_DEBUGONLY(hassaultMeleeAttackEnabled)\
	EASY_CVAR_RESET_DEBUGONLY(pathfindStumpedWaitTime)\
	EASY_CVAR_RESET_DEBUGONLY(pathfindStumpedMode)\
	EASY_CVAR_RESET_DEBUGONLY(pathfindStumpedForgetEnemy)\
	EASY_CVAR_RESET_DEBUGONLY(pathfindEdgeCheck)\
	EASY_CVAR_RESET_DEBUGONLY(RadiusDamageDrawDebug)\
	EASY_CVAR_RESET_DEBUGONLY(AlienRadiationImmunity)\
	EASY_CVAR_RESET_DEBUGONLY(customLogoSprayMode)\
	EASY_CVAR_RESET_DEBUGONLY(monsterFadeOutRate)\
	EASY_CVAR_RESET_DEBUGONLY(playerFadeOutRate)\
	EASY_CVAR_RESET_DEBUGONLY(drawDebugEnemyLKP)\
	EASY_CVAR_RESET_DEBUGONLY(trackchangePrintouts)\
	EASY_CVAR_RESET_DEBUGONLY(trackTrainPrintouts)\
	EASY_CVAR_RESET_CLIENTONLY(cl_holster)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerWeaponTracerMode)\
	EASY_CVAR_RESET_DEBUGONLY(monsterWeaponTracerMode)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(decalTracerExclusivity)\
	EASY_CVAR_RESET_DEBUGONLY(monsterToPlayerHitgroupSpecial)\
	EASY_CVAR_RESET_DEBUGONLY(multiplayerCrowbarHitSoundMode)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(healthcolor_fullRedMin)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(healthcolor_brightness)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(healthcolor_yellowMark)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(cl_drawExtraZeros)\
	EASY_CVAR_RESET_DEBUGONLY(pathfindLargeBoundFix)\
	EASY_CVAR_RESET_DEBUGONLY(flyerKilledFallingLoop)\
	EASY_CVAR_RESET_DEBUGONLY(floaterDummy)\
	EASY_CVAR_RESET_DEBUGONLY(barneyDummy)\
	EASY_CVAR_RESET_DEBUGONLY(ladderCycleMulti)\
	EASY_CVAR_RESET_DEBUGONLY(ladderSpeedMulti)\
	EASY_CVAR_RESET_DEBUGONLY(barnacleGrabNoInterpolation)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hideDamage)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(timedDamage_brightnessMax)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(timedDamage_brightnessMin)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(timedDamage_brightnessCap)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(timedDamage_brightnessFloor)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(timedDamage_flashSpeed)\
	EASY_CVAR_RESET_CLIENTONLY_DEBUGONLY(timedDamage_debug)\
	EASY_CVAR_RESET_DEBUGONLY(wallHealthDoor_closeDelay)\
	EASY_CVAR_RESET_DEBUGONLY(houndeye_attack_canGib)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(myRocketsAreBarney)\
	EASY_CVAR_RESET_DEBUGONLY(hassassinCrossbowDebug)\
	EASY_CVAR_RESET_DEBUGONLY(crossbowBoltDirectionAffectedByWater)\
	EASY_CVAR_RESET_DEBUGONLY(kingpinDebug)\
	EASY_CVAR_RESET_DEBUGONLY(minimumRespawnDelay)\
	EASY_CVAR_RESET(r_glowshell_debug)\
	EASY_CVAR_RESET(cl_viewpunch)\
	EASY_CVAR_RESET(cl_explosion)\
	EASY_CVAR_RESET(soundSentenceSave)\
	EASY_CVAR_RESET(pissedNPCs)\
	EASY_CVAR_RESET(hud_logo)\
	EASY_CVAR_RESET_CLIENTONLY(hud_brokentrans)\
	EASY_CVAR_RESET_CLIENTONLY(cl_fvox)\
	EASY_CVAR_RESET_CLIENTONLY(cl_ladder)\
	EASY_CVAR_RESET(precacheAll)\
	EASY_CVAR_RESET(cl_interp_entity)\
	EASY_CVAR_RESET(hud_swapFirstTwoBuckets)\
	EASY_CVAR_RESET(ignoreMultiplayerSkillOverride)\
	EASY_CVAR_RESET_CLIENTONLY(m_rawinput)\
	EASY_CVAR_RESET_CLIENTONLY(cl_earlyaccess)\
	EASY_CVAR_RESET_CLIENTONLY(cl_viewroll)\
	EASY_CVAR_RESET_DEBUGONLY(drawDebugCrowbar)\
	EASY_CVAR_RESET(blastExtraArmorDamageMode)\
	EASY_CVAR_RESET(hud_batteryhiddendead)\
	EASY_CVAR_RESET_DEBUGONLY(hyperBarney)\
	EASY_CVAR_RESET(monsterKilledToss)\
	EASY_CVAR_RESET(interpolation_movetypestep_mindelta)\
	EASY_CVAR_RESET(sv_bloodparticlemode)\
	EASY_CVAR_RESET(cl_interp_view_extra)\
	EASY_CVAR_RESET(cl_interp_view_standard)\
	EASY_CVAR_RESET(cl_interp_viewmodel)\
	EASY_CVAR_RESET(sv_explosionknockback)\
	EASY_CVAR_RESET(cl_gaussfollowattachment)\
	EASY_CVAR_RESET(cl_mp5_viewpunch_mod)\
	EASY_CVAR_RESET(cl_breakholster)\
	EASY_CVAR_RESET(pausecorrection1)\
	EASY_CVAR_RESET(pausecorrection2)\
	EASY_CVAR_RESET(hud_rpg_clipless)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST(sv_rpg_clipless)\
	EASY_CVAR_RESET(cl_viewmodel_fidget)\
	EASY_CVAR_RESET(sv_player_midair_fix)\
	EASY_CVAR_RESET(sv_player_midair_accel)\
	EASY_CVAR_RESET_DEBUGONLY(pathfindIgnoreNearestNodeCache)\
	EASY_CVAR_RESET_DEBUGONLY(pathfindIgnoreStaticRoutes)\
	EASY_CVAR_RESET_DEBUGONLY(pathfindForcePointHull)\
	EASY_CVAR_RESET_DEBUGONLY(pathfindNearestNodeExtra)\
	EASY_CVAR_RESET(sv_turret_postdeath)\
	EASY_CVAR_RESET(cl_mp5_evil_skip)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonRadiusDamageMode)\
	EASY_CVAR_RESET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonFireRateMode)\
	EASY_CVAR_RESET(sv_explosion_shake)\
	EASY_CVAR_RESET_DEBUGONLY(sv_explosion_shake_range)\
	EASY_CVAR_RESET_DEBUGONLY(sv_explosion_shake_amp)\
	EASY_CVAR_RESET_DEBUGONLY(sv_explosion_shake_freq)\
	EASY_CVAR_RESET_DEBUGONLY(sv_explosion_shake_duration)\
	EASY_CVAR_RESET_DEBUGONLY(sv_gargantua_throwattack)\
	EASY_CVAR_RESET_CLIENTONLY(cl_viewpunch_mod)\
	EASY_CVAR_RESET_CLIENTONLY(cl_gauss_viewpunch_mod)\
	EASY_CVAR_RESET_DEBUGONLY(pathfindMonsterclipFreshLogic)\
	EASY_CVAR_RESET(sv_rpg_projectile_model)\
	EASY_CVAR_RESET(hmilitaryDeadInvestigate)\
	EASY_CVAR_RESET(playerDeadTruce)\
	EASY_CVAR_RESET(playerDeadTalkerBehavior)\
	DUMMY



#define EASY_CVAR_EXTERN_MASS\
	EASY_CVAR_EXTERN_DEBUGONLY(gruntsCanHaveMP5Grenade)\
	EASY_CVAR_EXTERN(hud_version)\
	EASY_CVAR_EXTERN(hud_batterydraw)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(strobeDurationMin)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(strobeDurationMax)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(strobeRadiusMin)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(strobeRadiusMax)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(strobeSpawnDistHori)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(strobeSpawnDistVertMin)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(strobeSpawnDistVertMax)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(strobeMultiColor)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserEnabled)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserSpawnFreq)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserLength)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMin)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMax)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMin)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMax)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserBrightnessMin)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserBrightnessMax)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserDurationMin)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserDurationMax)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserThicknessMin)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserThicknessMax)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserNoiseMin)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserNoiseMax)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserFrameRateMin)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserFrameRateMax)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserMultiColor)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(cameraPosFixedX)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(cameraPosFixedY)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(cameraPosFixedZ)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(cameraPosOffX)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(cameraPosOffY)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(cameraPosOffZ)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(cameraRotFixedX)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(cameraRotFixedY)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(cameraRotFixedZ)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(cameraRotOffX)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(cameraRotOffY)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(cameraRotOffZ)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(imAllFuckedUp)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(thatWasntGrass)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(fogNear)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(fogFar)\
	EASY_CVAR_EXTERN_DEBUGONLY(NPCsTalkMore)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(myCameraSucks)\
	EASY_CVAR_EXTERN_DEBUGONLY(shutupstuka)\
	EASY_CVAR_EXTERN(chromeEffect)\
	EASY_CVAR_EXTERN_DEBUGONLY(hassaultSpinMovement)\
	EASY_CVAR_EXTERN_DEBUGONLY(hassaultIdleSpinSound)\
	EASY_CVAR_EXTERN_DEBUGONLY(hassaultFireSound)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteBulletHitSounds)\
	EASY_CVAR_EXTERN_DEBUGONLY(mutePlayerPainSounds)\
	EASY_CVAR_EXTERN_DEBUGONLY(hassaultIdleSpinSoundChannel)\
	EASY_CVAR_EXTERN_DEBUGONLY(hassaultSpinUpDownSoundChannel)\
	EASY_CVAR_EXTERN_DEBUGONLY(hassaultFireSoundChannel)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(geigerChannel)\
	EASY_CVAR_EXTERN_DEBUGONLY(hassaultWaitTime)\
	EASY_CVAR_EXTERN_DEBUGONLY(hassaultSpinupRemainTime)\
	EASY_CVAR_EXTERN_DEBUGONLY(hassaultResidualAttackTime)\
	EASY_CVAR_EXTERN_DEBUGONLY(hassaultSpinupStartTime)\
	EASY_CVAR_EXTERN_DEBUGONLY(hassaultVoicePitchMin)\
	EASY_CVAR_EXTERN_DEBUGONLY(hassaultVoicePitchMax)\
	EASY_CVAR_EXTERN_DEBUGONLY(hassaultFireSpread)\
	EASY_CVAR_EXTERN_DEBUGONLY(hassaultAllowGrenades)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(muteTempEntityGroundHitSound)\
	EASY_CVAR_EXTERN_DEBUGONLY(houndeyeAttackMode)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteRicochetSound)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(mutePlayerWeaponFire)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteCrowbarSounds)\
	EASY_CVAR_EXTERN_DEBUGONLY(scientistHealNPC)\
	EASY_CVAR_EXTERN_DEBUGONLY(scientistHealNPCDebug)\
	EASY_CVAR_EXTERN_DEBUGONLY(bulletholeAlertRange)\
	EASY_CVAR_EXTERN_DEBUGONLY(fleshhitmakessound)\
	EASY_CVAR_EXTERN_DEBUGONLY(nothingHurts)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(seeMonsterHealth)\
	EASY_CVAR_EXTERN_DEBUGONLY(scientistHealNPCFract)\
	EASY_CVAR_EXTERN_DEBUGONLY(bullsquidRangeDisabled)\
	EASY_CVAR_EXTERN_DEBUGONLY(applyLKPPathFixToAll)\
	EASY_CVAR_EXTERN_DEBUGONLY(timedDamageAffectsMonsters)\
	EASY_CVAR_EXTERN_DEBUGONLY(scientistHealCooldown)\
	EASY_CVAR_EXTERN_DEBUGONLY(crazyMonsterPrintouts)\
	EASY_CVAR_EXTERN_DEBUGONLY(movementIsCompletePrintout)\
	EASY_CVAR_EXTERN_DEBUGONLY(bulletHoleAlertPrintout)\
	EASY_CVAR_EXTERN_DEBUGONLY(bulletholeAlertStukaOnly)\
	EASY_CVAR_EXTERN_DEBUGONLY(barneyPrintouts)\
	EASY_CVAR_EXTERN_DEBUGONLY(monsterSpawnPrintout)\
	EASY_CVAR_EXTERN_DEBUGONLY(zombieBulletResistance)\
	EASY_CVAR_EXTERN_DEBUGONLY(zombieBulletPushback)\
	EASY_CVAR_EXTERN_DEBUGONLY(houndeyePrintout)\
	EASY_CVAR_EXTERN_DEBUGONLY(quakeExplosionSound)\
	EASY_CVAR_EXTERN_DEBUGONLY(explosionDebrisSoundVolume)\
	EASY_CVAR_EXTERN_DEBUGONLY(noFlinchOnHard)\
	EASY_CVAR_EXTERN_DEBUGONLY(hassaultDrawLKP)\
	EASY_CVAR_EXTERN_DEBUGONLY(meleeDrawBloodModeA)\
	EASY_CVAR_EXTERN_DEBUGONLY(meleeDrawBloodModeB)\
	EASY_CVAR_EXTERN_DEBUGONLY(drawDebugBloodTrace)\
	EASY_CVAR_EXTERN_DEBUGONLY(meleeDrawBloodModeBFix)\
	EASY_CVAR_EXTERN_DEBUGONLY(meleeDrawBloodModeAOffset)\
	EASY_CVAR_EXTERN_DEBUGONLY(meleeDrawBloodModeBOffset)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(wpn_glocksilencer)\
	EASY_CVAR_EXTERN_DEBUGONLY(panthereyeHasCloakingAbility)\
	EASY_CVAR_EXTERN_DEBUGONLY(hgruntSpeedMulti)\
	EASY_CVAR_EXTERN_DEBUGONLY(hgruntForceStrafeFireAnim)\
	EASY_CVAR_EXTERN_DEBUGONLY(hgruntLockRunAndGunTime)\
	EASY_CVAR_EXTERN_DEBUGONLY(hgruntHeadshotGore)\
	EASY_CVAR_EXTERN_DEBUGONLY(hgruntAllowStrafeFire)\
	EASY_CVAR_EXTERN_DEBUGONLY(thoroughHitBoxUpdates)\
	EASY_CVAR_EXTERN_DEBUGONLY(hgruntTinyClip)\
	EASY_CVAR_EXTERN_DEBUGONLY(hgruntStrafeAlwaysHasAmmo)\
	EASY_CVAR_EXTERN_DEBUGONLY(hgruntBrassEjectForwardOffset)\
	EASY_CVAR_EXTERN(cl_muzzleflash)\
	EASY_CVAR_EXTERN_DEBUGONLY(agrunt_muzzleflash)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(event5011Allowed)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(event5021Allowed)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(event5031Allowed)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(event5002Allowed)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(event5004Allowed)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(eventsAreFabulous)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(glockOldReloadLogic)\
	EASY_CVAR_EXTERN_DEBUGONLY(glockOldReloadLogicBarney)\
	EASY_CVAR_EXTERN_DEBUGONLY(barneyDroppedGlockAmmoCap)\
	EASY_CVAR_EXTERN_DEBUGONLY(drawCollisionBoundsAtDeath)\
	EASY_CVAR_EXTERN_DEBUGONLY(drawHitBoundsAtDeath)\
	EASY_CVAR_EXTERN_DEBUGONLY(islaveReviveFriendMode)\
	EASY_CVAR_EXTERN_DEBUGONLY(islaveReviveFriendChance)\
	EASY_CVAR_EXTERN_DEBUGONLY(islaveReviveFriendRange)\
	EASY_CVAR_EXTERN_DEBUGONLY(islaveReviveSelfMinDelay)\
	EASY_CVAR_EXTERN_DEBUGONLY(islaveReviveSelfMaxDelay)\
	EASY_CVAR_EXTERN_DEBUGONLY(islaveReviveSelfChance)\
	EASY_CVAR_EXTERN_DEBUGONLY(hgruntRunAndGunDistance)\
	EASY_CVAR_EXTERN_DEBUGONLY(hgruntPrintout)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(painArrowColorMode)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(painFlashColorMode)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(painFlashSuitless)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(painFlashDrownMode)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(firstPersonIdleDelayMin)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(firstPersonIdleDelayMax)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(forceDrawBatteryNumber)\
	EASY_CVAR_EXTERN_DEBUGONLY(offsetgivedistance)\
	EASY_CVAR_EXTERN_DEBUGONLY(offsetgivelookvertical)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(canShowWeaponSelectAtDeath)\
	EASY_CVAR_EXTERN_DEBUGONLY(endlessFlashlightBattery)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(preE3UsesFailColors)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(E3UsesFailColors)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(preE3ShowsDamageIcons)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(E3ShowsDamageIcons)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerCrossbowMode)\
	EASY_CVAR_EXTERN_DEBUGONLY(hassassinCrossbowMode)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(tripmineAnimWaitsForFinish)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(revolverLaserScope)\
	DUMMY\
	DUMMY\
	DUMMY\
	EASY_CVAR_EXTERN_CLIENTONLY(auto_adjust_fov)\
	DUMMY\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_nogaussrecoil)\
	EASY_CVAR_EXTERN_DEBUGONLY(autoSneaky)\
	EASY_CVAR_EXTERN(sv_longjump_chargemode)\
	EASY_CVAR_EXTERN_DEBUGONLY(cheat_touchNeverExplodes)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(gaussRecoilSendsUpInSP)\
	EASY_CVAR_EXTERN_DEBUGONLY(drawDebugPathfinding)\
	EASY_CVAR_EXTERN_DEBUGONLY(STUcheckDistH)\
	EASY_CVAR_EXTERN_DEBUGONLY(STUcheckDistV)\
	EASY_CVAR_EXTERN_DEBUGONLY(STUcheckDistD)\
	EASY_CVAR_EXTERN_DEBUGONLY(STUextraTriangH)\
	EASY_CVAR_EXTERN_DEBUGONLY(STUextraTriangV)\
	EASY_CVAR_EXTERN_DEBUGONLY(STUrepelMulti)\
	EASY_CVAR_EXTERN_DEBUGONLY(STUSpeedMulti)\
	EASY_CVAR_EXTERN_DEBUGONLY(STUExplodeTest)\
	EASY_CVAR_EXTERN_DEBUGONLY(STUYawSpeedMulti)\
	EASY_CVAR_EXTERN_DEBUGONLY(STUDetection)\
	EASY_CVAR_EXTERN_DEBUGONLY(stukaAdvancedCombat)\
	EASY_CVAR_EXTERN_DEBUGONLY(drawDebugPathfinding2)\
	EASY_CVAR_EXTERN_DEBUGONLY(hassaultFriendlyFire)\
	EASY_CVAR_EXTERN_DEBUGONLY(myStrobe)\
	EASY_CVAR_EXTERN_DEBUGONLY(peopleStrobe)\
	EASY_CVAR_EXTERN_DEBUGONLY(forceWorldLightOff)\
	EASY_CVAR_EXTERN_DEBUGONLY(wildHeads)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(raveEffectSpawnInterval)\
	EASY_CVAR_EXTERN_DEBUGONLY(drawBarnacleDebug)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(fogTest)\
	EASY_CVAR_EXTERN_DEBUGONLY(sparksAllMulti)\
	EASY_CVAR_EXTERN_DEBUGONLY(sparksEnvMulti)\
	EASY_CVAR_EXTERN_DEBUGONLY(sparksButtonMulti)\
	EASY_CVAR_EXTERN_DEBUGONLY(sparksPlayerCrossbowMulti)\
	EASY_CVAR_EXTERN_DEBUGONLY(sparksComputerHitMulti)\
	EASY_CVAR_EXTERN_DEBUGONLY(sparksTurretDeathMulti)\
	EASY_CVAR_EXTERN_DEBUGONLY(sparksOspreyHitMulti)\
	EASY_CVAR_EXTERN_DEBUGONLY(sparksExplosionMulti)\
	EASY_CVAR_EXTERN_DEBUGONLY(sparksBeamMulti)\
	EASY_CVAR_EXTERN_DEBUGONLY(sparksAIFailMulti)\
	EASY_CVAR_EXTERN_DEBUGONLY(normalSpeedMulti)\
	EASY_CVAR_EXTERN_DEBUGONLY(noclipSpeedMulti)\
	EASY_CVAR_EXTERN_DEBUGONLY(jumpForceMulti)\
	EASY_CVAR_EXTERN_DEBUGONLY(apacheForceCinBounds)\
	EASY_CVAR_EXTERN_DEBUGONLY(apacheBottomBoundAdj)\
	EASY_CVAR_EXTERN_DEBUGONLY(apacheInfluence)\
	DUMMY\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(allowAlphaCrosshairWithoutGuns)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(alphaCrosshairBlockedOnFrozen)\
	EASY_CVAR_EXTERN_DEBUGONLY(hgruntRunAndGunDotMin)\
	EASY_CVAR_EXTERN_DEBUGONLY(panthereyeJumpDotTol)\
	EASY_CVAR_EXTERN_DEBUGONLY(panthereyePrintout)\
	EASY_CVAR_EXTERN_DEBUGONLY(gargantuaPrintout)\
	EASY_CVAR_EXTERN_DEBUGONLY(squadmonsterPrintout)\
	EASY_CVAR_EXTERN_DEBUGONLY(hassaultPrintout)\
	EASY_CVAR_EXTERN_DEBUGONLY(barnaclePrintout)\
	EASY_CVAR_EXTERN_DEBUGONLY(friendlyPrintout)\
	EASY_CVAR_EXTERN_DEBUGONLY(stukaPrintout)\
	EASY_CVAR_EXTERN_DEBUGONLY(timedDamageEndlessOnHard)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(mirrorsReflectOnlyNPCs)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(mirrorsDoNotReflectPlayer)\
	EASY_CVAR_EXTERN(r_shadows)\
	EASY_CVAR_EXTERN_DEBUGONLY(altSquadRulesRuntime)\
	EASY_CVAR_EXTERN_DEBUGONLY(hgruntLockStrafeTime)\
	EASY_CVAR_EXTERN_DEBUGONLY(pathfindIgnoreIsolatedNodes)\
	EASY_CVAR_EXTERN_DEBUGONLY(drawNodeAll)\
	EASY_CVAR_EXTERN_DEBUGONLY(drawNodeSpecial)\
	EASY_CVAR_EXTERN_DEBUGONLY(drawNodeConnections)\
	EASY_CVAR_EXTERN_DEBUGONLY(drawNodeAlternateTime)\
	EASY_CVAR_EXTERN_DEBUGONLY(nodeSearchStartVerticalOffset)\
	EASY_CVAR_EXTERN_DEBUGONLY(blockChangeLevelTrigger)\
	EASY_CVAR_EXTERN_DEBUGONLY(blockMusicTrigger)\
	EASY_CVAR_EXTERN_DEBUGONLY(blockMultiTrigger)\
	EASY_CVAR_EXTERN_DEBUGONLY(blockTeleportTrigger)\
	EASY_CVAR_EXTERN_DEBUGONLY(blockHurtTrigger)\
	EASY_CVAR_EXTERN_DEBUGONLY(blockAutosaveTrigger)\
	EASY_CVAR_EXTERN_DEBUGONLY(hideNodeGraphRebuildNotice)\
	EASY_CVAR_EXTERN_DEBUGONLY(barnacleTongueRetractDelay)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(sv_germancensorship)\
	EASY_CVAR_EXTERN_DEBUGONLY(allowGermanModels)\
	EASY_CVAR_EXTERN_DEBUGONLY(germanRobotGibs)\
	EASY_CVAR_EXTERN_DEBUGONLY(germanRobotBleedsOil)\
	EASY_CVAR_EXTERN_DEBUGONLY(germanRobotDamageDecal)\
	EASY_CVAR_EXTERN_DEBUGONLY(germanRobotGibsDecal)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonEffectsMode)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonHitCloud)\
	DUMMY\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(handGrenadePickupYieldsOne)\
	EASY_CVAR_EXTERN_DEBUGONLY(handGrenadesUseOldBounceSound)\
	DUMMY\
	EASY_CVAR_EXTERN(hud_moveselect_mousewheelsound)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(timedDamageDeathRemoveMode)\
	EASY_CVAR_EXTERN(hud_moveselect_sound)\
	EASY_CVAR_EXTERN_DEBUGONLY(barnacleCanGib)\
	EASY_CVAR_EXTERN_DEBUGONLY(sentryCanGib)\
	EASY_CVAR_EXTERN_DEBUGONLY(miniturretCanGib)\
	EASY_CVAR_EXTERN_DEBUGONLY(turretCanGib)\
	EASY_CVAR_EXTERN_DEBUGONLY(turretBleedsOil)\
	EASY_CVAR_EXTERN_DEBUGONLY(turretDamageDecal)\
	EASY_CVAR_EXTERN_DEBUGONLY(turretGibDecal)\
	EASY_CVAR_EXTERN_DEBUGONLY(canDropInSinglePlayer)\
	EASY_CVAR_EXTERN_DEBUGONLY(timedDamageIgnoresArmor)\
	EASY_CVAR_EXTERN_DEBUGONLY(itemBatteryPrerequisite)\
	EASY_CVAR_EXTERN_DEBUGONLY(playerExtraPainSoundsMode)\
	EASY_CVAR_EXTERN_DEBUGONLY(timedDamageDisableViewPunch)\
	EASY_CVAR_EXTERN_DEBUGONLY(batteryDrainsAtDeath)\
	EASY_CVAR_EXTERN_DEBUGONLY(batteryDrainsAtAdrenalineMode)\
	EASY_CVAR_EXTERN_DEBUGONLY(canTakeLongJump)\
	EASY_CVAR_EXTERN_DEBUGONLY(printOutCommonTimables)\
	EASY_CVAR_EXTERN_DEBUGONLY(playerBrightLight)\
	EASY_CVAR_EXTERN_DEBUGONLY(disablePainPunchAutomatic)\
	EASY_CVAR_EXTERN_DEBUGONLY(gargantuaCorpseDeath)\
	EASY_CVAR_EXTERN_DEBUGONLY(gargantuaFallSound)\
	EASY_CVAR_EXTERN_DEBUGONLY(gargantuaBleeds)\
	EASY_CVAR_EXTERN_DEBUGONLY(shrapMode)\
	EASY_CVAR_EXTERN_DEBUGONLY(shrapRand)\
	EASY_CVAR_EXTERN_DEBUGONLY(shrapRandHeightExtra)\
	EASY_CVAR_EXTERN_DEBUGONLY(explosionShrapnelMulti)\
	EASY_CVAR_EXTERN_DEBUGONLY(useAlphaSparks)\
	EASY_CVAR_EXTERN_DEBUGONLY(emergencyFix)\
	EASY_CVAR_EXTERN_DEBUGONLY(timedDamageReviveRemoveMode)\
	DUMMY\
	EASY_CVAR_EXTERN_DEBUGONLY(ospreyIgnoresGruntCount)\
	EASY_CVAR_EXTERN_DEBUGONLY(mp5GrenadeInheritsPlayerVelocity)\
	EASY_CVAR_EXTERN_DEBUGONLY(crossbowInheritsPlayerVelocity)\
	EASY_CVAR_EXTERN_DEBUGONLY(fastHornetsInheritsPlayerVelocity)\
	EASY_CVAR_EXTERN_DEBUGONLY(snarkInheritsPlayerVelocity)\
	EASY_CVAR_EXTERN_DEBUGONLY(chumtoadInheritsPlayerVelocity)\
	EASY_CVAR_EXTERN_DEBUGONLY(weaponPickupPlaysAnyReloadSounds)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(glockUseLastBulletAnim)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerBarnacleVictimViewOffset)\
	EASY_CVAR_EXTERN_DEBUGONLY(hgruntMovementDeltaCheck)\
	EASY_CVAR_EXTERN_DEBUGONLY(hiddenMemPrintout)\
	EASY_CVAR_EXTERN_DEBUGONLY(hassaultExtraMuzzleFlashRadius)\
	EASY_CVAR_EXTERN_DEBUGONLY(hassaultExtraMuzzleFlashBrightness)\
	EASY_CVAR_EXTERN_DEBUGONLY(hassaultExtraMuzzleFlashForward)\
	EASY_CVAR_EXTERN_DEBUGONLY(leaderlessSquadAllowed)\
	EASY_CVAR_EXTERN_DEBUGONLY(nodeConnectionBreakableCheck)\
	EASY_CVAR_EXTERN_DEBUGONLY(playerReviveInvincibilityTime)\
	EASY_CVAR_EXTERN_DEBUGONLY(playerReviveBuddhaMode)\
	EASY_CVAR_EXTERN_DEBUGONLY(playerReviveTimeBlocksTimedDamage)\
	EASY_CVAR_EXTERN_DEBUGONLY(hassaultBulletDamageMulti)\
	EASY_CVAR_EXTERN_DEBUGONLY(hassaultBulletsPerShot)\
	EASY_CVAR_EXTERN_DEBUGONLY(hassaultFireAnimSpeedMulti)\
	EASY_CVAR_EXTERN_DEBUGONLY(hassaultMeleeAnimSpeedMulti)\
	EASY_CVAR_EXTERN_DEBUGONLY(hassassinCrossbowReloadSoundDelay)\
	EASY_CVAR_EXTERN_DEBUGONLY(hgruntStrafeAnimSpeedMulti)\
	EASY_CVAR_EXTERN_DEBUGONLY(hgruntRunAndGunAnimSpeedMulti)\
	EASY_CVAR_EXTERN_DEBUGONLY(cheat_iwantguts)\
	EASY_CVAR_EXTERN_DEBUGONLY(nodeDetailPrintout)\
	EASY_CVAR_EXTERN_DEBUGONLY(soundAttenuationStuka)\
	EASY_CVAR_EXTERN_DEBUGONLY(soundVolumeStuka)\
	EASY_CVAR_EXTERN_DEBUGONLY(cineChangelevelFix)\
	EASY_CVAR_EXTERN_DEBUGONLY(drawDebugCine)\
	EASY_CVAR_EXTERN_DEBUGONLY(cineAllowSequenceOverwrite)\
	EASY_CVAR_EXTERN_DEBUGONLY(stukaInflictsBleeding)\
	EASY_CVAR_EXTERN_DEBUGONLY(animationKilledBoundsRemoval)\
	EASY_CVAR_EXTERN_DEBUGONLY(gargantuaKilledBoundsAssist)\
	EASY_CVAR_EXTERN_DEBUGONLY(bullsquidSpitTrajTimeMin)\
	EASY_CVAR_EXTERN_DEBUGONLY(bullsquidSpitTrajTimeMax)\
	EASY_CVAR_EXTERN_DEBUGONLY(bullsquidSpitTrajDistMin)\
	EASY_CVAR_EXTERN_DEBUGONLY(bullsquidSpitTrajDistMax)\
	EASY_CVAR_EXTERN_DEBUGONLY(bullsquidSpitGravityMulti)\
	EASY_CVAR_EXTERN(cl_bullsquidspit)\
	EASY_CVAR_EXTERN(cl_bullsquidspitarc)\
	EASY_CVAR_EXTERN_DEBUGONLY(bullsquidSpitUseAlphaModel)\
	EASY_CVAR_EXTERN_DEBUGONLY(bullsquidSpitUseAlphaEffect)\
	EASY_CVAR_EXTERN_DEBUGONLY(bullsquidSpitEffectSpread)\
	EASY_CVAR_EXTERN_DEBUGONLY(bullsquidSpitEffectMin)\
	EASY_CVAR_EXTERN_DEBUGONLY(bullsquidSpitEffectMax)\
	EASY_CVAR_EXTERN_DEBUGONLY(bullsquidSpitEffectHitMin)\
	EASY_CVAR_EXTERN_DEBUGONLY(bullsquidSpitEffectHitMax)\
	EASY_CVAR_EXTERN_DEBUGONLY(bullsquidSpitEffectSpawn)\
	EASY_CVAR_EXTERN_DEBUGONLY(bullsquidSpitEffectHitSpawn)\
	EASY_CVAR_EXTERN_DEBUGONLY(bullsquidSpitSpriteScale)\
	EASY_CVAR_EXTERN_DEBUGONLY(bullsquidSpitAlphaScale)\
	EASY_CVAR_EXTERN_DEBUGONLY(scientistBravery)\
	EASY_CVAR_EXTERN_DEBUGONLY(barneyUnholsterTime)\
	EASY_CVAR_EXTERN_DEBUGONLY(barneyUnholsterAnimChoice)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(trailTypeTest)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetTrail)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetTrailSolidColor)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetDeathModEasy)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetDeathModMedium)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetDeathModHard)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetZoomPuff)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetSpiral)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetSpeedMulti)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetSpeedDartMulti)\
	EASY_CVAR_EXTERN(cl_rockettrail)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketTrailAlphaInterval)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketTrailAlphaScale)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketSkipIgnite)\
	EASY_CVAR_EXTERN_DEBUGONLY(agruntHornetRandomness)\
	EASY_CVAR_EXTERN_DEBUGONLY(hornetSpiralPeriod)\
	EASY_CVAR_EXTERN_DEBUGONLY(hornetSpiralAmplitude)\
	EASY_CVAR_EXTERN(cl_hornetspiral)\
	EASY_CVAR_EXTERN(cl_hornettrail)\
	EASY_CVAR_EXTERN(hud_drawammobar)\
	EASY_CVAR_EXTERN(hud_weaponselecthideslower)\
	EASY_CVAR_EXTERN(hud_drawsidebarmode)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(gauss_mode)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(painFlashDmgMin)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(painFlashDmgExMult)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(painFlashCumulativeMinDrowning)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(painFlashCumulativeMax)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(painFlashDrawOpacityMax)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(painArrowDrawOpacityMin)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(painArrowDrawOpacityMax)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(painFlashFadeMult)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(painArrowFadeMult)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(painFlashArmorBlock)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(painFlashDirTolerance)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(painArrowCumulativeAppearMin)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(painArrowCumulativeDmgJump)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(painFlashPrintouts)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(itemFlashCumulativeJump)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(itemFlashDrawOpacityMin)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(itemFlashDrawOpacityMax)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(itemFlashFadeMult)\
	EASY_CVAR_EXTERN_DEBUGONLY(chumtoadPrintout)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(crossbowReloadSoundDelay)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(crossbowFirePlaysReloadSound)\
	EASY_CVAR_EXTERN_DEBUGONLY(pathfindFidgetFailTime)\
	EASY_CVAR_EXTERN_DEBUGONLY(pathfindPrintout)\
	EASY_CVAR_EXTERN_DEBUGONLY(pathfindTopRampFixDistance)\
	EASY_CVAR_EXTERN_DEBUGONLY(pathfindTopRampFixDraw)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(iHaveAscended)\
	EASY_CVAR_EXTERN_DEBUGONLY(pathfindLooseMapNodes)\
	EASY_CVAR_EXTERN_DEBUGONLY(pathfindRampFix)\
	EASY_CVAR_EXTERN_DEBUGONLY(chumtoadPlayDeadFoolChance)\
	EASY_CVAR_EXTERN_DEBUGONLY(animationFramerateMulti)\
	EASY_CVAR_EXTERN_DEBUGONLY(pathfindNodeToleranceMulti)\
	EASY_CVAR_EXTERN_DEBUGONLY(friendlyPianoFollowVolume)\
	EASY_CVAR_EXTERN_DEBUGONLY(friendlyPianoOtherVolume)\
	EASY_CVAR_EXTERN_DEBUGONLY(showtriggers)\
	EASY_CVAR_EXTERN_DEBUGONLY(tentacleAlertSound)\
	EASY_CVAR_EXTERN_DEBUGONLY(tentacleSwingSound1)\
	EASY_CVAR_EXTERN_DEBUGONLY(tentacleSwingSound2)\
	EASY_CVAR_EXTERN_DEBUGONLY(playerFollowerMax)\
	EASY_CVAR_EXTERN_DEBUGONLY(announcerIsAJerk)\
	EASY_CVAR_EXTERN_DEBUGONLY(playerUseDrawDebug)\
	EASY_CVAR_EXTERN_DEBUGONLY(playerChumtoadThrowDrawDebug)\
	EASY_CVAR_EXTERN_DEBUGONLY(peaceOut)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(drawViewModel)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(drawHUD)\
	EASY_CVAR_EXTERN_DEBUGONLY(disablePauseSinglePlayer)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerBulletHitEffectForceServer)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(forceAllowServersideTextureSounds)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerWeaponSpreadMode)\
	EASY_CVAR_EXTERN_DEBUGONLY(monsterAIForceFindDistance)\
	EASY_CVAR_EXTERN_DEBUGONLY(baseEntityDamagePushNormalMulti)\
	EASY_CVAR_EXTERN_DEBUGONLY(baseEntityDamagePushVerticalBoost)\
	EASY_CVAR_EXTERN_DEBUGONLY(baseEntityDamagePushVerticalMulti)\
	EASY_CVAR_EXTERN_DEBUGONLY(baseEntityDamagePushVerticalMinimum)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelSyncFixPrintouts)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(textureHitSoundPrintouts)\
	EASY_CVAR_EXTERN_DEBUGONLY(hgruntAllowGrenades)\
	EASY_CVAR_EXTERN_DEBUGONLY(scheduleInterruptPrintouts)\
	EASY_CVAR_EXTERN_DEBUGONLY(animationPrintouts)\
	EASY_CVAR_EXTERN_DEBUGONLY(hassaultMeleeAttackEnabled)\
	EASY_CVAR_EXTERN_DEBUGONLY(pathfindStumpedWaitTime)\
	EASY_CVAR_EXTERN_DEBUGONLY(pathfindStumpedMode)\
	EASY_CVAR_EXTERN_DEBUGONLY(pathfindStumpedForgetEnemy)\
	EASY_CVAR_EXTERN_DEBUGONLY(pathfindEdgeCheck)\
	EASY_CVAR_EXTERN_DEBUGONLY(RadiusDamageDrawDebug)\
	EASY_CVAR_EXTERN_DEBUGONLY(AlienRadiationImmunity)\
	EASY_CVAR_EXTERN_DEBUGONLY(customLogoSprayMode)\
	EASY_CVAR_EXTERN_DEBUGONLY(monsterFadeOutRate)\
	EASY_CVAR_EXTERN_DEBUGONLY(playerFadeOutRate)\
	EASY_CVAR_EXTERN_DEBUGONLY(drawDebugEnemyLKP)\
	EASY_CVAR_EXTERN_DEBUGONLY(trackchangePrintouts)\
	EASY_CVAR_EXTERN_DEBUGONLY(trackTrainPrintouts)\
	EASY_CVAR_EXTERN(cl_holster)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerWeaponTracerMode)\
	EASY_CVAR_EXTERN_DEBUGONLY(monsterWeaponTracerMode)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(decalTracerExclusivity)\
	EASY_CVAR_EXTERN_DEBUGONLY(monsterToPlayerHitgroupSpecial)\
	EASY_CVAR_EXTERN_DEBUGONLY(multiplayerCrowbarHitSoundMode)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(healthcolor_fullRedMin)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(healthcolor_brightness)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(healthcolor_yellowMark)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(cl_drawExtraZeros)\
	EASY_CVAR_EXTERN_DEBUGONLY(pathfindLargeBoundFix)\
	EASY_CVAR_EXTERN_DEBUGONLY(flyerKilledFallingLoop)\
	EASY_CVAR_EXTERN_DEBUGONLY(floaterDummy)\
	EASY_CVAR_EXTERN_DEBUGONLY(barneyDummy)\
	EASY_CVAR_EXTERN_DEBUGONLY(ladderCycleMulti)\
	EASY_CVAR_EXTERN_DEBUGONLY(ladderSpeedMulti)\
	EASY_CVAR_EXTERN_DEBUGONLY(barnacleGrabNoInterpolation)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hideDamage)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(timedDamage_brightnessMax)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(timedDamage_brightnessMin)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(timedDamage_brightnessCap)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(timedDamage_brightnessFloor)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(timedDamage_flashSpeed)\
	EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(timedDamage_debug)\
	EASY_CVAR_EXTERN_DEBUGONLY(wallHealthDoor_closeDelay)\
	EASY_CVAR_EXTERN_DEBUGONLY(houndeye_attack_canGib)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(myRocketsAreBarney)\
	EASY_CVAR_EXTERN_DEBUGONLY(hassassinCrossbowDebug)\
	EASY_CVAR_EXTERN_DEBUGONLY(crossbowBoltDirectionAffectedByWater)\
	EASY_CVAR_EXTERN_DEBUGONLY(kingpinDebug)\
	EASY_CVAR_EXTERN_DEBUGONLY(minimumRespawnDelay)\
	EASY_CVAR_EXTERN(r_glowshell_debug)\
	EASY_CVAR_EXTERN(cl_viewpunch)\
	EASY_CVAR_EXTERN(cl_explosion)\
	EASY_CVAR_EXTERN(soundSentenceSave)\
	EASY_CVAR_EXTERN(pissedNPCs)\
	EASY_CVAR_EXTERN(hud_logo)\
	EASY_CVAR_EXTERN(hud_brokentrans)\
	EASY_CVAR_EXTERN(cl_fvox)\
	EASY_CVAR_EXTERN(cl_ladder)\
	EASY_CVAR_EXTERN(precacheAll)\
	EASY_CVAR_EXTERN(cl_interp_entity)\
	EASY_CVAR_EXTERN(hud_swapFirstTwoBuckets)\
	EASY_CVAR_EXTERN(ignoreMultiplayerSkillOverride)\
	EASY_CVAR_EXTERN(m_rawinput)\
	EASY_CVAR_EXTERN(cl_earlyaccess)\
	EASY_CVAR_EXTERN(cl_viewroll)\
	EASY_CVAR_EXTERN_DEBUGONLY(drawDebugCrowbar)\
	EASY_CVAR_EXTERN(blastExtraArmorDamageMode)\
	EASY_CVAR_EXTERN(hud_batteryhiddendead)\
	EASY_CVAR_EXTERN_DEBUGONLY(hyperBarney)\
	EASY_CVAR_EXTERN(monsterKilledToss)\
	EASY_CVAR_EXTERN(interpolation_movetypestep_mindelta)\
	EASY_CVAR_EXTERN(sv_bloodparticlemode)\
	EASY_CVAR_EXTERN(cl_interp_view_extra)\
	EASY_CVAR_EXTERN(cl_interp_view_standard)\
	EASY_CVAR_EXTERN(cl_interp_viewmodel)\
	EASY_CVAR_EXTERN(sv_explosionknockback)\
	EASY_CVAR_EXTERN(cl_gaussfollowattachment)\
	EASY_CVAR_EXTERN(cl_mp5_viewpunch_mod)\
	EASY_CVAR_EXTERN(cl_breakholster)\
	EASY_CVAR_EXTERN(pausecorrection1)\
	EASY_CVAR_EXTERN(pausecorrection2)\
	EASY_CVAR_EXTERN(hud_rpg_clipless)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(sv_rpg_clipless)\
	EASY_CVAR_EXTERN(cl_viewmodel_fidget)\
	EASY_CVAR_EXTERN(sv_player_midair_fix)\
	EASY_CVAR_EXTERN(sv_player_midair_accel)\
	EASY_CVAR_EXTERN_DEBUGONLY(pathfindIgnoreNearestNodeCache)\
	EASY_CVAR_EXTERN_DEBUGONLY(pathfindIgnoreStaticRoutes)\
	EASY_CVAR_EXTERN_DEBUGONLY(pathfindForcePointHull)\
	EASY_CVAR_EXTERN_DEBUGONLY(pathfindNearestNodeExtra)\
	EASY_CVAR_EXTERN(sv_turret_postdeath)\
	EASY_CVAR_EXTERN(cl_mp5_evil_skip)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonRadiusDamageMode)\
	EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonFireRateMode)\
	EASY_CVAR_EXTERN(sv_explosion_shake)\
	EASY_CVAR_EXTERN_DEBUGONLY(sv_explosion_shake_range)\
	EASY_CVAR_EXTERN_DEBUGONLY(sv_explosion_shake_amp)\
	EASY_CVAR_EXTERN_DEBUGONLY(sv_explosion_shake_freq)\
	EASY_CVAR_EXTERN_DEBUGONLY(sv_explosion_shake_duration)\
	EASY_CVAR_EXTERN_DEBUGONLY(sv_gargantua_throwattack)\
	EASY_CVAR_EXTERN_CLIENTONLY(cl_viewpunch_mod)\
	EASY_CVAR_EXTERN_CLIENTONLY(cl_gauss_viewpunch_mod)\
	EASY_CVAR_EXTERN_DEBUGONLY(pathfindMonsterclipFreshLogic)\
	EASY_CVAR_EXTERN(sv_rpg_projectile_model)\
	EASY_CVAR_EXTERN(hmilitaryDeadInvestigate)\
	EASY_CVAR_EXTERN(playerDeadTruce)\
	EASY_CVAR_EXTERN(playerDeadTalkerBehavior)\
	DUMMY





#define EASY_CVAR_SYNCH_SERVER_MASS\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(gruntsCanHaveMP5Grenade)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(hud_version)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(hud_batterydraw)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(strobeDurationMin)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(strobeDurationMax)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(strobeRadiusMin)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(strobeRadiusMax)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(strobeSpawnDistHori)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(strobeSpawnDistVertMin)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(strobeSpawnDistVertMax)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(strobeMultiColor)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserEnabled)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserSpawnFreq)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserLength)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMin)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMax)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMin)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMax)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserBrightnessMin)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserBrightnessMax)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserDurationMin)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserDurationMax)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserThicknessMin)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserThicknessMax)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserNoiseMin)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserNoiseMax)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserFrameRateMin)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserFrameRateMax)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(raveLaserMultiColor)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(cameraPosFixedX)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(cameraPosFixedY)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(cameraPosFixedZ)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(cameraPosOffX)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(cameraPosOffY)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(cameraPosOffZ)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(cameraRotFixedX)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(cameraRotFixedY)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(cameraRotFixedZ)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(cameraRotOffX)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(cameraRotOffY)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(cameraRotOffZ)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(imAllFuckedUp)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(thatWasntGrass)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(fogNear)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(fogFar)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(NPCsTalkMore)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(myCameraSucks)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(shutupstuka)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(chromeEffect)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hassaultSpinMovement)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hassaultIdleSpinSound)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hassaultFireSound)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteBulletHitSounds)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(mutePlayerPainSounds)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hassaultIdleSpinSoundChannel)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hassaultSpinUpDownSoundChannel)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hassaultFireSoundChannel)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(geigerChannel)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hassaultWaitTime)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hassaultSpinupRemainTime)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hassaultResidualAttackTime)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hassaultSpinupStartTime)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hassaultVoicePitchMin)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hassaultVoicePitchMax)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hassaultFireSpread)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hassaultAllowGrenades)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(muteTempEntityGroundHitSound)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(houndeyeAttackMode)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteRicochetSound)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(mutePlayerWeaponFire)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteCrowbarSounds)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(scientistHealNPC)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(scientistHealNPCDebug)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(bulletholeAlertRange)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(fleshhitmakessound)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(nothingHurts)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(seeMonsterHealth)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(scientistHealNPCFract)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(bullsquidRangeDisabled)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(applyLKPPathFixToAll)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(timedDamageAffectsMonsters)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(scientistHealCooldown)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(crazyMonsterPrintouts)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(movementIsCompletePrintout)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(bulletHoleAlertPrintout)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(bulletholeAlertStukaOnly)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(barneyPrintouts)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(monsterSpawnPrintout)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(zombieBulletResistance)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(zombieBulletPushback)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(houndeyePrintout)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(quakeExplosionSound)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(explosionDebrisSoundVolume)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(noFlinchOnHard)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hassaultDrawLKP)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(meleeDrawBloodModeA)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(meleeDrawBloodModeB)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(drawDebugBloodTrace)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(meleeDrawBloodModeBFix)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(meleeDrawBloodModeAOffset)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(meleeDrawBloodModeBOffset)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST(wpn_glocksilencer)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(panthereyeHasCloakingAbility)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hgruntSpeedMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hgruntForceStrafeFireAnim)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hgruntLockRunAndGunTime)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hgruntHeadshotGore)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hgruntAllowStrafeFire)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(thoroughHitBoxUpdates)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hgruntTinyClip)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hgruntStrafeAlwaysHasAmmo)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hgruntBrassEjectForwardOffset)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(cl_muzzleflash)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(agrunt_muzzleflash)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(event5011Allowed)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(event5021Allowed)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(event5031Allowed)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(event5002Allowed)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(event5004Allowed)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(eventsAreFabulous)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(glockOldReloadLogic)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(glockOldReloadLogicBarney)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(barneyDroppedGlockAmmoCap)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(drawCollisionBoundsAtDeath)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(drawHitBoundsAtDeath)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(islaveReviveFriendMode)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(islaveReviveFriendChance)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(islaveReviveFriendRange)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(islaveReviveSelfMinDelay)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(islaveReviveSelfMaxDelay)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(islaveReviveSelfChance)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hgruntRunAndGunDistance)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hgruntPrintout)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(painArrowColorMode)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(painFlashColorMode)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(painFlashSuitless)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(painFlashDrownMode)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(firstPersonIdleDelayMin)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(firstPersonIdleDelayMax)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(forceDrawBatteryNumber)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(offsetgivedistance)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(offsetgivelookvertical)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(canShowWeaponSelectAtDeath)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(endlessFlashlightBattery)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(preE3UsesFailColors)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(E3UsesFailColors)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(preE3ShowsDamageIcons)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(E3ShowsDamageIcons)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerCrossbowMode)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hassassinCrossbowMode)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(tripmineAnimWaitsForFinish)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(revolverLaserScope)\
	DUMMY\
	DUMMY\
	DUMMY\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY(auto_adjust_fov)\
	DUMMY\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_nogaussrecoil)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(autoSneaky)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(sv_longjump_chargemode)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(cheat_touchNeverExplodes)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(gaussRecoilSendsUpInSP)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(drawDebugPathfinding)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(STUcheckDistH)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(STUcheckDistV)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(STUcheckDistD)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(STUextraTriangH)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(STUextraTriangV)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(STUrepelMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(STUSpeedMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(STUExplodeTest)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(STUYawSpeedMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(STUDetection)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(stukaAdvancedCombat)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(drawDebugPathfinding2)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hassaultFriendlyFire)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(myStrobe)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(peopleStrobe)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(forceWorldLightOff)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(wildHeads)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(raveEffectSpawnInterval)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(drawBarnacleDebug)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(fogTest)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(sparksAllMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(sparksEnvMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(sparksButtonMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(sparksPlayerCrossbowMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(sparksComputerHitMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(sparksTurretDeathMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(sparksOspreyHitMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(sparksExplosionMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(sparksBeamMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(sparksAIFailMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(normalSpeedMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(noclipSpeedMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(jumpForceMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(apacheForceCinBounds)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(apacheBottomBoundAdj)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(apacheInfluence)\
	DUMMY\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(allowAlphaCrosshairWithoutGuns)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(alphaCrosshairBlockedOnFrozen)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hgruntRunAndGunDotMin)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(panthereyeJumpDotTol)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(panthereyePrintout)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(gargantuaPrintout)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(squadmonsterPrintout)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hassaultPrintout)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(barnaclePrintout)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(friendlyPrintout)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(stukaPrintout)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(timedDamageEndlessOnHard)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(mirrorsReflectOnlyNPCs)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(mirrorsDoNotReflectPlayer)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(r_shadows)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(altSquadRulesRuntime)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hgruntLockStrafeTime)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(pathfindIgnoreIsolatedNodes)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(drawNodeAll)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(drawNodeSpecial)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(drawNodeConnections)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(drawNodeAlternateTime)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(nodeSearchStartVerticalOffset)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(blockChangeLevelTrigger)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(blockMusicTrigger)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(blockMultiTrigger)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(blockTeleportTrigger)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(blockHurtTrigger)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(blockAutosaveTrigger)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hideNodeGraphRebuildNotice)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(barnacleTongueRetractDelay)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST(sv_germancensorship)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(allowGermanModels)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(germanRobotGibs)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(germanRobotBleedsOil)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(germanRobotDamageDecal)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(germanRobotGibsDecal)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonEffectsMode)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonHitCloud)\
	DUMMY\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(handGrenadePickupYieldsOne)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(handGrenadesUseOldBounceSound)\
	DUMMY\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(hud_moveselect_mousewheelsound)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(timedDamageDeathRemoveMode)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(hud_moveselect_sound)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(barnacleCanGib)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(sentryCanGib)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(miniturretCanGib)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(turretCanGib)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(turretBleedsOil)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(turretDamageDecal)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(turretGibDecal)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(canDropInSinglePlayer)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(timedDamageIgnoresArmor)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(itemBatteryPrerequisite)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(playerExtraPainSoundsMode)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(timedDamageDisableViewPunch)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(batteryDrainsAtDeath)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(batteryDrainsAtAdrenalineMode)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(canTakeLongJump)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(printOutCommonTimables)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(playerBrightLight)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(disablePainPunchAutomatic)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(gargantuaCorpseDeath)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(gargantuaFallSound)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(gargantuaBleeds)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(shrapMode)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(shrapRand)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(shrapRandHeightExtra)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(explosionShrapnelMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(useAlphaSparks)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(emergencyFix)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(timedDamageReviveRemoveMode)\
	DUMMY\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(ospreyIgnoresGruntCount)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(mp5GrenadeInheritsPlayerVelocity)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(crossbowInheritsPlayerVelocity)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(fastHornetsInheritsPlayerVelocity)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(snarkInheritsPlayerVelocity)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(chumtoadInheritsPlayerVelocity)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(weaponPickupPlaysAnyReloadSounds)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(glockUseLastBulletAnim)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerBarnacleVictimViewOffset)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hgruntMovementDeltaCheck)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hiddenMemPrintout)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hassaultExtraMuzzleFlashRadius)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hassaultExtraMuzzleFlashBrightness)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hassaultExtraMuzzleFlashForward)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(leaderlessSquadAllowed)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(nodeConnectionBreakableCheck)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(playerReviveInvincibilityTime)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(playerReviveBuddhaMode)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(playerReviveTimeBlocksTimedDamage)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hassaultBulletDamageMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hassaultBulletsPerShot)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hassaultFireAnimSpeedMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hassaultMeleeAnimSpeedMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hassassinCrossbowReloadSoundDelay)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hgruntStrafeAnimSpeedMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hgruntRunAndGunAnimSpeedMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(cheat_iwantguts)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(nodeDetailPrintout)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(soundAttenuationStuka)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(soundVolumeStuka)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(cineChangelevelFix)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(drawDebugCine)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(cineAllowSequenceOverwrite)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(stukaInflictsBleeding)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(animationKilledBoundsRemoval)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(gargantuaKilledBoundsAssist)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(bullsquidSpitTrajTimeMin)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(bullsquidSpitTrajTimeMax)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(bullsquidSpitTrajDistMin)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(bullsquidSpitTrajDistMax)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(bullsquidSpitGravityMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(cl_bullsquidspit)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(cl_bullsquidspitarc)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(bullsquidSpitUseAlphaModel)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(bullsquidSpitUseAlphaEffect)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(bullsquidSpitEffectSpread)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(bullsquidSpitEffectMin)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(bullsquidSpitEffectMax)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(bullsquidSpitEffectHitMin)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(bullsquidSpitEffectHitMax)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(bullsquidSpitEffectSpawn)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(bullsquidSpitEffectHitSpawn)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(bullsquidSpitSpriteScale)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(bullsquidSpitAlphaScale)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(scientistBravery)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(barneyUnholsterTime)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(barneyUnholsterAnimChoice)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(trailTypeTest)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetTrail)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetTrailSolidColor)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetDeathModEasy)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetDeathModMedium)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetDeathModHard)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetZoomPuff)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetSpiral)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetSpeedMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hornetSpeedDartMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(cl_rockettrail)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketTrailAlphaInterval)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketTrailAlphaScale)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketSkipIgnite)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(agruntHornetRandomness)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hornetSpiralPeriod)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hornetSpiralAmplitude)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(cl_hornetspiral)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(cl_hornettrail)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(hud_drawammobar)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(hud_weaponselecthideslower)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(hud_drawsidebarmode)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST(gauss_mode)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(painFlashDmgMin)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(painFlashDmgExMult)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(painFlashCumulativeMinDrowning)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(painFlashCumulativeMax)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(painFlashDrawOpacityMax)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(painArrowDrawOpacityMin)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(painArrowDrawOpacityMax)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(painFlashFadeMult)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(painArrowFadeMult)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(painFlashArmorBlock)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(painFlashDirTolerance)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(painArrowCumulativeAppearMin)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(painArrowCumulativeDmgJump)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(painFlashPrintouts)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(itemFlashCumulativeJump)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(itemFlashDrawOpacityMin)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(itemFlashDrawOpacityMax)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(itemFlashFadeMult)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(chumtoadPrintout)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(crossbowReloadSoundDelay)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(crossbowFirePlaysReloadSound)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(pathfindFidgetFailTime)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(pathfindPrintout)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(pathfindTopRampFixDistance)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(pathfindTopRampFixDraw)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(iHaveAscended)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(pathfindLooseMapNodes)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(pathfindRampFix)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(chumtoadPlayDeadFoolChance)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(animationFramerateMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(pathfindNodeToleranceMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(friendlyPianoFollowVolume)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(friendlyPianoOtherVolume)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(showtriggers)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(tentacleAlertSound)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(tentacleSwingSound1)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(tentacleSwingSound2)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(playerFollowerMax)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(announcerIsAJerk)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(playerUseDrawDebug)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(playerChumtoadThrowDrawDebug)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(peaceOut)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(drawViewModel)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(drawHUD)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(disablePauseSinglePlayer)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerBulletHitEffectForceServer)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(forceAllowServersideTextureSounds)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerWeaponSpreadMode)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(monsterAIForceFindDistance)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(baseEntityDamagePushNormalMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(baseEntityDamagePushVerticalBoost)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(baseEntityDamagePushVerticalMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(baseEntityDamagePushVerticalMinimum)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelSyncFixPrintouts)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(textureHitSoundPrintouts)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hgruntAllowGrenades)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(scheduleInterruptPrintouts)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(animationPrintouts)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hassaultMeleeAttackEnabled)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(pathfindStumpedWaitTime)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(pathfindStumpedMode)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(pathfindStumpedForgetEnemy)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(pathfindEdgeCheck)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(RadiusDamageDrawDebug)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(AlienRadiationImmunity)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(customLogoSprayMode)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(monsterFadeOutRate)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(playerFadeOutRate)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(drawDebugEnemyLKP)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(trackchangePrintouts)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(trackTrainPrintouts)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(cl_holster)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerWeaponTracerMode)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(monsterWeaponTracerMode)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(decalTracerExclusivity)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(monsterToPlayerHitgroupSpecial)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(multiplayerCrowbarHitSoundMode)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(healthcolor_fullRedMin)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(healthcolor_brightness)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(healthcolor_yellowMark)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(cl_drawExtraZeros)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(pathfindLargeBoundFix)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(flyerKilledFallingLoop)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(floaterDummy)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(barneyDummy)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(ladderCycleMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(ladderSpeedMulti)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(barnacleGrabNoInterpolation)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hideDamage)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(timedDamage_brightnessMax)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(timedDamage_brightnessMin)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(timedDamage_brightnessCap)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(timedDamage_brightnessFloor)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(timedDamage_flashSpeed)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY_DEBUGONLY(timedDamage_debug)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(wallHealthDoor_closeDelay)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(houndeye_attack_canGib)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(myRocketsAreBarney)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hassassinCrossbowDebug)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(crossbowBoltDirectionAffectedByWater)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(kingpinDebug)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(minimumRespawnDelay)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(r_glowshell_debug)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(cl_viewpunch)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(cl_explosion)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(soundSentenceSave)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(pissedNPCs)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(hud_logo)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(hud_brokentrans)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(cl_fvox)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(cl_ladder)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(precacheAll)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(cl_interp_entity)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(hud_swapFirstTwoBuckets)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(ignoreMultiplayerSkillOverride)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(m_rawinput)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(cl_earlyaccess)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(cl_viewroll)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(drawDebugCrowbar)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(blastExtraArmorDamageMode)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(hud_batteryhiddendead)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(hyperBarney)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(monsterKilledToss)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(interpolation_movetypestep_mindelta)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(sv_bloodparticlemode)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(cl_interp_view_extra)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(cl_interp_view_standard)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(cl_interp_viewmodel)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(sv_explosionknockback)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(cl_gaussfollowattachment)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(cl_mp5_viewpunch_mod)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(cl_breakholster)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(pausecorrection1)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(pausecorrection2)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(hud_rpg_clipless)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST(sv_rpg_clipless)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(cl_viewmodel_fidget)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(sv_player_midair_fix)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(sv_player_midair_accel)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(pathfindIgnoreNearestNodeCache)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(pathfindIgnoreStaticRoutes)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(pathfindForcePointHull)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(pathfindNearestNodeExtra)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(sv_turret_postdeath)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(cl_mp5_evil_skip)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonRadiusDamageMode)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonFireRateMode)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(sv_explosion_shake)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(sv_explosion_shake_range)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(sv_explosion_shake_amp)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(sv_explosion_shake_freq)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(sv_explosion_shake_duration)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(sv_gargantua_throwattack)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY(cl_viewpunch_mod)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_CLIENTONLY(cl_gauss_viewpunch_mod)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER_DEBUGONLY(pathfindMonsterclipFreshLogic)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(sv_rpg_projectile_model)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(hmilitaryDeadInvestigate)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(playerDeadTruce)\
	EASY_CVAR_SYNCH_NOSAVE_SERVER(playerDeadTalkerBehavior)\
	DUMMY


