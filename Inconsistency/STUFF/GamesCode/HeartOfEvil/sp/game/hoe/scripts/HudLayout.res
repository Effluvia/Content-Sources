"Resource/HudLayout.res"
{
	// HOE
	HudAmmo870
	{
		"fieldName"				"HudAmmo870"

		"PaintBackgroundType"	"2"

		"xpos"	"r132"
		"ypos"	"r36"
		"wide"	"132"
		"tall"  "36"
		"visible" "1"
		"enabled" "1"

		"text_xpos" "8"
		"text_ypos" "22"

		"digit_xpos" "98"
		"digit_ypos" "18"
		"digit2_xpos" "118"
		"digit2_ypos" "18"

		"ammo_xpos" "10"
		"ammo_ypos" "0"
		"ammo_spacing" "11"
	}

	// HOE
	HudUseable
	{
		"fieldName"				"HudUseable"
		"bottom"				"424"
		"PaintBackgroundType"	"2"
		"TextColor"				"255 255 255 220"
		"TextFont"				"HudUseableText"
	}

	// HOE
	HudTask
	{
		"fieldName"				"HudTask"
		"bottom"				"200"
		"PaintBackgroundType"	"2"
		"TextColor"				"255 255 255 220"
		"TextFont"				"HudUseableText"
	}

	// HOE
	HudScope
	{
		"fieldName"	"HudScope"
		"xpos"	        "0"
		"ypos"	        "0"
		"visible"       "1"
		"enabled"       "1"
		"PaintBackgroundType"	"0"

	//      These screw up the scope for users using a non standard screen ratio (i.e. 16:9) (lodle)
	//	"wide"	        "640"
	//	"tall"          "480"
	}

	// HOE
	HudActiveWeapon
	{
		"fieldName" "HudActiveWeapon"
//		"xpos"	"r150"
//		"ypos"	"386"
//xpos "r296"
xpos "c-68"
ypos "r38"
		"wide"	"136"
		"tall"  "36"
		"visible" "1"
		"enabled" "1"

		"paintbackground"		"1"
		"PaintBackgroundType"	"2"
	}

	HudHealth
	{
		"fieldName"		"HudHealth"
		"xpos"	"4"
		"ypos"	"r30"
		"wide"	"80"
		"tall"  "28"
		"visible" "1"
		"enabled" "1"

		"paintbackground"		"1"
		"PaintBackgroundType"	"2"
		
		"text_xpos" "8"
		"text_ypos" "6" // HOE was "20"
		"digit_xpos" "35"
		"digit_ypos" "1"

		"TextFont"			"HudHealth" // HOE added
	}
	
	HudSuit
	{
		"fieldName"		"HudSuit"
		"xpos"	"100"
		"ypos"	"r30"
		"wide"	"80"
		"tall"  "28"
		"visible" "1"
		"enabled" "1"

		"paintbackground"		"1"
		"PaintBackgroundType"	"2"

		"text_xpos" "8"
		"text_ypos" "6" // HOE was "20"
		"digit_xpos" "35"
		"digit_ypos" "1"

		"TextFont"			"HudSuit" // HOE added
	}

	HudAmmo
	{
		"fieldName" "HudAmmo"
		"xpos"	"r116"
		"ypos"	"r30"
		"wide"	"112"
		"tall"  "28"
		"visible" "1"
		"enabled" "1"

		"PaintBackgroundType"	"2"

		"text_xpos" "8"
		"text_ypos" "11" // HOE was "20"
		"digit_xpos" "44"
		"digit_ypos" "2"
		"digit2_xpos" "78"
		"digit2_ypos" "10"
	}

	HudAmmoSecondary
	{
		"fieldName" "HudAmmoSecondary"
		"xpos"	"r76"
		"ypos"	"432"
		"wide"	"60"
		"tall"  "36"
		"visible" "1"
		"enabled" "1"

		"PaintBackgroundType"	"2"

		"text_xpos" "8"
		"text_ypos" "22"
		"digit_xpos" "36"
		"digit_ypos" "2"
	}

	HudPosture	[$WIN32]
	{
		"fieldName" 		"HudPosture"
		"visible" 		"1"
		"PaintBackgroundType"	"2"
		"xpos"	"16"
		"ypos"	"316"
		"tall"  "36"
		"wide"	"36"
		"font"	"WeaponIconsSmall"
		"icon_xpos"	"10"
		"icon_ypos" 	"0"
	}

	HudSuitPower
	{
		"fieldName" "HudSuitPower"
		"visible" "1"
		"enabled" "1"
		"xpos"	"4"
		"ypos"	"396"
		"wide"	"102"
		"tall"	"26"
		
		"AuxPowerLowColor" "255 0 0 220"
		"AuxPowerHighColor" "255 220 0 220"
		"AuxPowerDisabledAlpha" "70"

		"BarInsetX" "8"
		"BarInsetY" "15"
		"BarWidth" "92"
		"BarHeight" "4"
		"BarChunkWidth" "6"
		"BarChunkGap" "3"

		"text_xpos" "8"
		"text_ypos" "4"
		"text2_xpos" "8"
		"text2_ypos" "22"
		"text2_gap" "10"

		"PaintBackgroundType"	"2"
	}
	
	HudFlashlight
	{
		"fieldName" "HudFlashlight"
		"visible" "1"
		"PaintBackgroundType"	"2"
		"xpos"	"196"		[$WIN32]
		"ypos"	"r28"		[$WIN32]
		"xpos_hidef"	"306"		[$X360]		// aligned to left
		"xpos_lodef"	"c-18"		[$X360]		// centered in screen
		"ypos"	"428"		[$X360]				
		"tall"  "24"
		"wide"	"36"
		"font"	"WeaponIconsSmall"
		
		"icon_xpos"	"4"
		"icon_ypos" "-8"
		
		"BarInsetX" "4"
		"BarInsetY" "18"
		"BarWidth" "28"
		"BarHeight" "2"
		"BarChunkWidth" "2"
		"BarChunkGap" "1"
	}

	HudLocator
	{
		"fieldName" "HudLocator"
		"visible" "1"
		"PaintBackgroundType"	"2"
		"xpos"	"c8"	[$WIN32]
		"ypos"	"r36"	[$WIN32]
		"xpos"	"c-32"	[$X360]
		"ypos_hidef"	"r52"	[$X360]
		"ypos_lodef"	"r95"	[$X360]		// 52 is aligned to bottom of HudSuit
		"wide"	"64"
		"tall"  "24"
	}

	HudDamageIndicator
	{
		"fieldName" "HudDamageIndicator"
		"visible" "1"
		"enabled" "1"
		"DmgColorLeft" "255 0 0 0"
		"DmgColorRight" "255 0 0 0"
		
		"dmg_xpos" "30"
		"dmg_ypos" "100"
		"dmg_wide" "36"
		"dmg_tall1" "240"
		"dmg_tall2" "200"
	}

	HudZoom
	{
		"fieldName" "HudZoom"
		"visible" "1"
		"enabled" "1"
		"Circle1Radius" "66"
		"Circle2Radius"	"74"
		"DashGap"	"16"
		"DashHeight" "4"
		"BorderThickness" "88"
	}

	HudWeaponSelection
	{
		"fieldName" "HudWeaponSelection"
		"ypos" 	"16"
		"visible" "1"
		"enabled" "1"
		"SmallBoxSize" "32"
		"LargeBoxWide" "160" // HOE was "112"
		"LargeBoxTall" "60" // HOE was "80"
		"BoxGap" "8"
		"SelectionNumberXPos" "4"
		"SelectionNumberYPos" "4"
		"SelectionGrowTime"	"0.4"
		"TextYPos" "48" // HOE was "64"
	}

	HudCrosshair
	{
		"fieldName" "HudCrosshair"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudDeathNotice
	{
		"fieldName" "HudDeathNotice"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudVehicle
	{
		"fieldName" "HudVehicle"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	ScorePanel
	{
		"fieldName" "ScorePanel"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudTrain
	{
		"fieldName" "HudTrain"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudMOTD
	{
		"fieldName" "HudMOTD"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudMessage
	{
		"fieldName" "HudMessage"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudMenu
	{
		"fieldName" "HudMenu"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudCloseCaption
	{
		"fieldName" "HudCloseCaption"
		"visible"	"1"
		"enabled"	"1"
		"xpos"		"c-250"
		"ypos"		"276"
		"wide"		"500"
		"tall"		"136"

		"BgAlpha"	"128"

		"GrowTime"		"0.25"
		"ItemHiddenTime"	"0.2"  // Nearly same as grow time so that the item doesn't start to show until growth is finished
		"ItemFadeInTime"	"0.15"	// Once ItemHiddenTime is finished, takes this much longer to fade in
		"ItemFadeOutTime"	"0.3"
		"topoffset"		"0"		[$WIN32]

		"IconSize" "20" // HOE: Added
	}

	HudChat
	{
		"fieldName" "HudChat"
		"visible" "0"
		"enabled" "1"
		"xpos"	"0"
		"ypos"	"0"
		"wide"	 "4"
		"tall"	 "4"
	}

	HudHistoryResource
	{
		"fieldName" "HudHistoryResource"
		"visible" "1"
		"enabled" "1"
		"xpos"	"r252"
		"ypos"	"40"
		"wide"	 "248"
		"tall"	 "320"

		"history_gap"	"56"
		"icon_inset"	"28"
		"text_inset"	"26"
		"NumberFont"	"HudNumbersSmall"
	}

	HudGeiger
	{
		"fieldName" "HudGeiger"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HUDQuickInfo
	{
		"fieldName" "HUDQuickInfo"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudWeapon
	{
		"fieldName" "HudWeapon"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}
	HudAnimationInfo
	{
		"fieldName" "HudAnimationInfo"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudPredictionDump
	{
		"fieldName" "HudPredictionDump"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudHintDisplay
	{
		"fieldName"	"HudHintDisplay"
		"visible"	"0"
		"enabled" "1"
		"xpos"	"r120"
		"ypos"	"r340"
		"wide"	"100"
		"tall"	"200"
		"text_xpos"	"8"
		"text_ypos"	"8"
		"text_xgap"	"8"
		"text_ygap"	"8"
		"TextColor"	"255 170 0 220"

		"PaintBackgroundType"	"2"
	}

	HudHintKeyDisplay
	{
		"fieldName"	"HudHintKeyDisplay"
		"visible"	"0"
		"enabled" 	"1"
		"xpos"		"r120"	[$WIN32]
		"ypos"		"r340"	[$WIN32]
		"xpos"		"r148"	[$X360]
		"ypos"		"r338"	[$X360]
		"wide"		"100"
		"tall"		"200"
		"text_xpos"	"8"
		"text_ypos"	"8"
		"text_xgap"	"8"
		"text_ygap"	"8"
		"TextColor"	"255 170 0 220"

		"PaintBackgroundType"	"2"
	}

	HudSquadStatus
	{
		"fieldName"	"HudSquadStatus"
		"visible"	"1"
		"enabled" "1"
		"xpos"	"r108"
		"ypos"	"380"
		"wide"	"104"
		"tall"	"46"
		"text_xpos"	"8"
		"text_ypos" "12" // HOE: was 34 from top, now from bottom
		"SquadIconColor"	"255 255 255 150" // HOE - same as "Normal"
		"SquadTextColor"	"255 255 255 150" // HOE - same as "Normal"
		"IconInsetX"	"8"
		"IconInsetY"	"0"
		"IconGap"		"24"
		"IconGapY"		"34" // HOE

		"paintbackground"		"1"
		"PaintBackgroundType"	"2"
	}

	HudPoisonDamageIndicator
	{
		"fieldName"	"HudPoisonDamageIndicator"
		"visible"	"0"
		"enabled" "1"
		"xpos"	"16"
		"ypos"	"346"
		"wide"	"136"
		"tall"	"38"
		"text_xpos"	"8"
		"text_ypos"	"8"
		"text_ygap" "14"
		"TextColor"	"255 170 0 220"
		"PaintBackgroundType"	"2"
	}
	HudCredits
	{
		"fieldName"	"HudCredits"
		"TextFont"	"Default"
		"visible"	"1"
		"xpos"	"0"
		"ypos"	"0"
		"wide"	"640"
		"tall"	"480"
		"TextColor"	"255 255 255 192"

	}
	
	HUDAutoAim
	{
		"fieldName" "HUDAutoAim"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudCommentary
	{
		"fieldName" "HudCommentary"
		"xpos"	"c-190"
		"ypos"	"350"
		"wide"	"380"
		"tall"  "40"
		"visible" "1"
		"enabled" "1"
		
		"PaintBackgroundType"	"2"
		
		"bar_xpos"		"50"
		"bar_ypos"		"20"
		"bar_height"	"8"
		"bar_width"		"320"
		"speaker_xpos"	"50"
		"speaker_ypos"	"8"
		"count_xpos_from_right"	"10"	// Counts from the right side
		"count_ypos"	"8"
		
		"icon_texture"	"vgui/hud/icon_commentary"
		"icon_xpos"		"0"
		"icon_ypos"		"0"		
		"icon_width"	"40"
		"icon_height"	"40"
	}
	
	HudHDRDemo
	{
		"fieldName" "HudHDRDemo"
		"xpos"	"0"
		"ypos"	"0"
		"wide"	"640"
		"tall"  "480"
		"visible" "1"
		"enabled" "1"
		
		"Alpha"	"255"
		"PaintBackgroundType"	"2"
		
		"BorderColor"	"0 0 0 255"
		"BorderLeft"	"16"
		"BorderRight"	"16"
		"BorderTop"		"16"
		"BorderBottom"	"64"
		"BorderCenter"	"0"
		
		"TextColor"		"255 255 255 255"
		"LeftTitleY"	"422"
		"RightTitleY"	"422"
	}

	AchievementNotificationPanel	
	{
		"fieldName"				"AchievementNotificationPanel"
		"visible"				"1"
		"enabled"				"1"
		"xpos"					"0"
		"ypos"					"180"
		"wide"					"f10"	[$WIN32]
		"wide"					"f60"	[$X360]
		"tall"					"100"
	}
}
