#!C:/Programming/TclTk-8.4.13/bin/wish84

console eval {
	wm title . "convert_models.tcl Console"
	set ::tk::console::maxLines 2048
	.console configure -font {Courier 9}
}
console show
update

proc InitTextWindow {} {
	text .text -yscrollcommand {.y set} -font {Courier 9} -height 24 \
		-wrap none
	scrollbar .y -orient vertical -command {.text yview }
	pack .text -side left -expand yes -fill both
	pack .y -side right -expand no -fill y
	return
}
InitTextWindow

proc N {path} {
    file nativename $path
}

set IVIEW "C:/Program Files/IrfanView/i_view32.exe"
set SDKBIN [N "C:/Program Files (x86)/Steam/SteamApps/common/Source SDK Base 2013 Singleplayer/bin"]
set STUDIOMDL "$SDKBIN/studiomdl.exe"
set VTEX "$SDKBIN/vtex.exe"
set GAME [N "C:/Programming/heart of evil 2013/sp/game/hoe"]
set SRC [N "C:/Programming/heart of evil 2013/sp/game/hoe"]
set ROOT $SRC

proc MSG {s} {
	.text insert end $s
	.text see insert
}

proc wrtChanToText {chan} {
    if {[eof $chan]} {
	set ::EXEC_DONE 1
	fconfigure $chan -blocking true
	if { [catch {
	    close $chan
	} msg] } {
	    MSG $msg\n
	}
	return
    }
    set text [read $chan]
    if {[string length $text] > 0} {
	MSG $text
    }
    return
}

proc EXEC {args} {
    MSG "EXEC: $args\n"
	if {$::TEST} return
#    catch \{
#	eval exec $args
#    \} result
#    puts $result
    set cmd $args
    set chan [open |$cmd r]
    fconfigure $chan -blocking false
    fileevent $chan readable [list wrtChanToText $chan]
    set ::EXEC_DONE 0
    vwait ::EXEC_DONE
}

if 0 {
foreach dir [glob -directory $ROOT/modelsrc/original/ -types d -tails *] {
    if {![file exists $ROOT/modelsrc/updated/$dir]} {
	file mkdir $ROOT/modelsrc/updated/$dir
    }
    break
}
}

proc NearestPowerOfTwo {n} {
    for {set i 1} {1} {incr i} {
	if {$n <= pow(2,$i)} {
	    return [expr int(pow(2,$i))]
	}
    }
}

#package require Img

set D1 $ROOT/modelsrc/original/Barney
set D2 $ROOT/materialsrc/Models/Barney
set F BX_Face1
if 0 {
#
# Convert .bmp to .tga using IrfanView
#
if {![file exists $D2]} {
    file mkdir $D2
}
set I [image create photo -file $f1]
set width [NearestPowerOfTwo [image width $I]]
set height [NearestPowerOfTwo [image height $I]]
image delete $I
MSG "$f1 -> $f2 /resize=($width,$height)\n"
exec $IVIEW $f1 /convert=$f2
exec $IVIEW $f2 /resize=($width,$height) /resample
}

#
# Convert .bmp to .tga using Img extension
# Scale to power of two, filling extra space with blue
#
proc Convert_BMP_to_TGA {D1 D2 F} {

    set f1 [N $D1/$F.bmp]
    set f2 [N $D2/$F.tga]

    if {![file exists $D2]} {
	MSG "mkdir $D2\n"
	file mkdir $D2
    }
    set I [image create photo -file $f1]
    set width [image width $I]
    set height [image height $I]
    set scaleWidth [NearestPowerOfTwo $width]
    set scaleHeight [NearestPowerOfTwo $height]
    if {$width != $scaleWidth} {
	MSG "$F: width not power of 2, $width -> $scaleWidth\n"
	$I put blue -to $width 0 $scaleWidth $height
	incr width
    }
    if {$height != $scaleHeight} {
	MSG "$F height not power of 2  $height -> $scaleHeight\n"
	$I put blue -to 0 $height $scaleWidth $scaleHeight
    }
    $I write $f2 -format tga
    MSG "wrote $f2\n"
    image delete $I
}

proc ConvertDir_BMP_to_TGA {D} {
    set D1 $::ROOT/modelsrc/original/$D
    set D2 $::ROOT/materialsrc/Models/$D
    foreach F [glob -directory $D1 -tails *.bmp] {
	Convert_BMP_to_TGA $D1 $D2 [file rootname $F]
    }
}

#
# Convert .tga to .vtf using vtex.exe
#
proc ConvertDir_TGA_to_VTF {D} {
    set D2 $::ROOT/materialsrc/Models/$D
    EXEC $::VTEX -game $::GAME -nopause -mkdir -shader vertexlitgeneric $D2/*.tga
}

proc Check { msg } {
    set answer [tk_messageBox -message $msg -type yesno]
    return [expr {$answer eq "yes"}]
}

proc Convert_QC {D F} {
    if { ![Check "Convert $F?"] } exit

    set D1 $::ROOT/modelsrc/original/$D
    set D2 $::ROOT/modelsrc/updated/$D
#    if {[file exists $D2/$F]} return

    set chan [open $D1/$F]
    set input [read $chan]
    close $chan
    set output ""
    foreach line [split $input \n] {
	if {[regexp {\$modelname "(.*)"} $line v0 v1]} {
#	    set path [join [lrange [file split $v1] end-[llength [file split $D]] end] "\\"]
#	    set line "\$modelname \"[join [concat [file split $D] [file tail $v1]] \\]\""
	    set line "\$modelname \"[file tail $v1]\""
	}
	if {[regexp {\$cd "(.*)"} $line v0 v1]} {
	    continue
	}
	if {[regexp {\$cdtexture "(.*)"} $line v0 v1]} {
	    set line "\$cdmaterials \"models\\[join [file split $D] \\]\""
	}
	if {$line eq "\$externaltextures"} {
	    # ignore obsolete command
	    continue
	}
	if {[string match "\$sequencegroup*" $line]} {
	    # ignore obsolete command
	    continue
	}
	if {[regexp {studio "(.*)"} $line v0 v1]} {
	    set line "studio \"$v1.smd\""
	}
	append output $line \n
    }

    if {![file exists $D2]} {
	file mkdir $D2
    }
    set chan [open $D2/$F w]
    puts -nonewline $chan $output
    close $chan
}

proc Convert_SMD {D1 D2 F} {

    set chan [open $D1/$F]
    set input [read $chan]
    close $chan
    set output ""

    foreach line [split $input \n] {
	if {[regexp {(.*)\.bmp} $line v0 v1]} {
	    set line $v1.tga
	}
	append output $line \n
    }

    set chan [open $D2/$F w]
    puts -nonewline $chan $output
    close $chan
}

proc ConvertDir_SMD {D} {

    if { ![Check "Convert $D .smd files?"] } exit

    set D1 $::ROOT/modelsrc/original/$D
    set D2 $::ROOT/modelsrc/updated/$D
    if {![file exists $D2]} {
	file mkdir $D2
    }
    foreach F [glob -directory $D1 -tails *.smd] {
	Convert_SMD $D1 $D2 $F
    }
}

proc Compile_MDL {D F} {
    set ROOT $::ROOT/modelsrc
    if 1 {
	cd $::SDKBIN
	EXEC studiomdl.exe -verbose -nop4 -game $::GAME $ROOT/$D/$F
    } else {
	# cd so studiomdl can find the .smd files
	cd $ROOT/$D
	EXEC $::STUDIOMDL -game $::GAME -verbose -printgraph $ROOT/$D/$F
    }
#    CopyModelToOB $ROOT/$D/$F
}

proc CopyModelToOB {qc} {
    set chan [open $qc]
    set buf [read $chan]
    close $chan

    foreach line [split $buf \n] {
	if {[string match "\$modelname*" $line]} {
	    set line [string map {"\\" "/"} $line]
	    set modelpath [string trim [lindex $line 1] "\""]
	    set modeldir [file dirname $modelpath]
	    set modelfile [file tail $modelpath]
	    set modelroot [file rootname $modelfile]
	    foreach file [glob -nocomplain -directory $::GAME/models/$modeldir "$modelroot.*"] {
		set dest $::GAME_OB/models/$modeldir
		if {![file exists $::GAME_OB/models/$modeldir]} {
		    MSG "creating dir $::GAME_OB/models/$modeldir\n"
		    file mkdir $::GAME_OB/models/$modeldir
		}
		MSG "copy [file tail $file] to OB\n"
		file copy -force -- $file $dest
	    }
	}
    }
}

proc LatestModificationTime D {
    set latest 0
    foreach file [glob -directory $D *] {
	if {$file eq "." || $file eq ".."} continue
	if {[file isdirectory $file]} {
	    set latest [expr {max($latest,[LatestModificationTime $file])}]
	} else {
	    set latest [expr {max($latest,[file mtime $file])}]
	}
    }
    return $latest
}

proc Compile_MDL_IfNeeded {D F} {
    global FORCE
    if {$FORCE} {
	Compile_MDL $D $F
	return
    }
    
    global GAME
    global ROOT
    set latestSrcModificationTime [LatestModificationTime $ROOT/modelsrc/$D]

    set chan [open $ROOT/modelsrc/$D/$F]
    set buf [read $chan]
    close $chan

    set compile 0

    foreach line [split $buf \n] {
	if {[string match "\$modelname*" $line]} {
	    set line [string map {"\\" "/"} $line]
	    set modelpath [string trim [lindex $line 1] "\""]
	    set modeldir [file dirname $modelpath]
	    set modelfile [file tail $modelpath]
	    set modelroot [file rootname $modelfile]
	    set allmodelfiles [glob -nocomplain -directory $GAME/models/$modeldir $modelroot.mdl $modelroot.*.vtx $modelroot.vvd $modelroot.phy]
	    if {[llength $allmodelfiles] < 4} {
		MSG "not all $modelpath files exist, rebuilding\n"
		set compile 1
		break
	    }
	    foreach file $allmodelfiles {
		if {[file mtime $file] < $latestSrcModificationTime} {
		    MSG "$file is older than a modelsrc file, rebuilding\n"
		    if {!$FORCE} {
			eval file delete $allmodelfiles
		    }
		    set compile 1
		    break
		}
	    }
	    break
	}
    }

    if {$compile} {
	Compile_MDL $D $F
    }
}

#lappend D Make_sdk ; lappend QC Male_sdk.qc
#lappend D humans_sdk/Male_sdk ; lappend QC Male_06_sdk.qc

lappend D minibus ; lappend QC minibus.qc
lappend D minibus ; lappend QC tire.qc
lappend D truck ; lappend QC truck.qc
lappend D GhostAnim ; lappend QC GhostAnim.qc
lappend D Vehicles/choreo_vehicle_nama0_wake ; lappend QC vehicle.qc
lappend D Vehicles/choreo_vehicle_nama2b0_huey ; lappend QC huey.qc
lappend D Vehicles/choreo_vehicle_namb2_ladder ; lappend QC ladder.qc
lappend D Vehicles/choreo_vehicle_namb0_huey_exit ; lappend QC vehicle.qc
lappend D Vehicles/choreo_vehicle_namd1_monkey ; lappend QC vehicle.qc
lappend D Vehicles/choreo_vehicle_namd4_minibus ; lappend QC vehicle.qc
lappend D Vehicles/choreo_vehicle_name0e1_crash ; lappend QC vehicle.qc

lappend D armymedic ; lappend QC armymedic.qc
lappend D Barney ; lappend QC barney.qc
lappend D barneyzombie ; lappend QC barneyzombie.qc
lappend D charlie ; lappend QC charlie.qc
lappend D charlie/propagandacard ; lappend QC propagandacard.qc
lappend D civilsuspect ; lappend QC civilsuspect.qc
lappend D eaten ; lappend QC eaten.qc
lappend D gruntmedic ; lappend QC gruntmedic.qc
lappend D kurtz ; lappend QC kurtz.qc
lappend D mikeforce ; lappend QC mikeforce.qc
lappend D namGrunt ; lappend QC namGrunt.qc
lappend D officer1 ; lappend QC officer1.qc
lappend D officer2 ; lappend QC officer2.qc
lappend D player ; lappend QC mikeforce.qc
lappend D sarge ; lappend QC sarge.qc
lappend D SOG ; lappend QC SOG.qc

#lappend D heavy ; lappend QC heavy-tnb.qc
#lappend D player_dod/blue ; lappend QC blue_player.qc
#lappend D player_dod/animation ; lappend QC sdk_player_shared.qc

lappend D armymedic/Head ; lappend QC armymedichead.qc
lappend D Barney/Head ; lappend QC barneyhead.qc
lappend D barneyzombie/Head ; lappend QC barneyzombiehead.qc
lappend D charlie/Head ; lappend QC charliehead.qc
lappend D kurtz/Head ; lappend QC kurtzhead.qc
lappend D mikeforce/Head ; lappend QC mikeforcehead.qc
lappend D namGrunt/Head ; lappend QC namGruntHead.qc
lappend D peasant/Head ; lappend QC peasanthead_male.qc
lappend D peasant/Head ; lappend QC peasanthead_female.qc
lappend D SOG/Head ; lappend QC soghead.qc

lappend D armymedic/Head ; lappend QC armymedichead_Off.qc
lappend D charlie/Head ; lappend QC charliehead_Off.qc
lappend D mikeforce/Head ; lappend QC mikeforcehead_Off.qc
lappend D namGrunt/Head ; lappend QC namGruntHead_Off.qc
lappend D peasant/Head ; lappend QC peasanthead_male_Off.qc
lappend D peasant/Head ; lappend QC peasanthead_female_Off.qc
lappend D SOG/Head ; lappend QC soghead_Off.qc

lappend D armymedic/Helmet ; lappend QC helmet.qc
lappend D charlie/Helmet ; lappend QC helmet_Brown.qc
lappend D charlie/Helmet ; lappend QC helmet_Green.qc
lappend D charlie/Helmet ; lappend QC helmet_VC1.qc
lappend D charlie/Helmet ; lappend QC helmet_VC2.qc
lappend D mikeforce/Helmet ; lappend QC beret.qc
lappend D mikeforce/Helmet ; lappend QC beret_tiger.qc
lappend D mikeforce/Helmet ; lappend QC boonie.qc
lappend D namGrunt/Helmet ; lappend QC boonie.qc
lappend D namGrunt/Helmet ; lappend QC cap.qc
lappend D namGrunt/Helmet ; lappend QC helmet.qc
lappend D peasant/Helmet ; lappend QC male.qc
lappend D peasant/Helmet ; lappend QC female.qc
lappend D SOG/Helmet ; lappend QC beret.qc

lappend D mikeforce ; lappend QC mikeforce_byarms.qc
lappend D mikeforce ; lappend QC mikeforce_byfeet.qc
lappend D mikeforce ; lappend QC mikeforce_headless.qc
lappend D charlie ; lappend QC charlie_byarms.qc
lappend D charlie ; lappend QC charlie_byfeet.qc
lappend D namGrunt ; lappend QC namGrunt_byarms.qc
lappend D namGrunt ; lappend QC namGrunt_byfeet.qc
lappend D namGrunt ; lappend QC namGrunt_headless.qc

lappend D peasant ; lappend QC peasant_Male1.qc
lappend D peasant ; lappend QC peasant_Male2.qc
lappend D peasant ; lappend QC peasant_Male3.qc
lappend D peasant ; lappend QC peasant_Female1.qc
lappend D peasant ; lappend QC peasant_Female2.qc
lappend D peasant ; lappend QC peasant_Female3.qc

lappend D peasant/shackle ; lappend QC shackle.qc

lappend D ape/gorilla ; lappend QC gorilla.qc
lappend D barnacle ; lappend QC barnacle.qc
lappend D bigrat ; lappend QC bigrat.qc
#lappend D bullsquid ; lappend QC bullsquid.qc
lappend D kophyaeger ; lappend QC kophyaeger.qc
lappend D kophyaeger/adult ; lappend QC kophadult.qc
lappend D w_squeak ; lappend QC mdldecompiler.qc

lappend D chumtoad ; lappend QC chumtoad.qc

lappend D roach ; lappend QC roach_small.qc
lappend D roach ; lappend QC roach_medium.qc
lappend D roach ; lappend QC roach_default.qc

lappend D superzombie ; lappend QC superzombie.qc
lappend D superzombie/Head ; lappend QC superzombiehead.qc

lappend D them ; lappend QC them.qc
lappend D them ; lappend QC them_byfeet.qc
lappend D them ; lappend QC them_head.qc

lappend D v_letter ; lappend QC v_letter.qc

lappend D grenade/v_grenade ; lappend QC v_grenade.qc
lappend D grenade/w_grenade ; lappend QC w_grenade.qc

lappend D tripmine/v_tripmine ; lappend QC v_tripmine.qc
lappend D tripmine ; lappend QC w_tripmine.qc

lappend D mikeforce ; lappend QC w_whisky.qc

lappend D Chainsaw/v_chainsaw ; lappend QC v_chainsaw.qc
lappend D Chainsaw/w_chainsaw ; lappend QC w_chainsaw.qc
lappend D Chainsaw/w_gas ; lappend QC w_gas.qc

lappend D huey ; lappend QC huey.qc
lappend D huey/ladder ; lappend QC ladder.qc
lappend D huey ; lappend QC speakers.qc
lappend D huey/gibs ; lappend QC huey_part_cabin.qc
lappend D huey/gibs ; lappend QC huey_part_engine.qc
lappend D huey/gibs ; lappend QC huey_part_left_gun.qc
lappend D huey/gibs ; lappend QC huey_part_left_strut.qc
lappend D huey/gibs ; lappend QC huey_part_roof.qc
lappend D huey/gibs ; lappend QC huey_part_tail.qc

lappend D hueyteam/hcopilot ; lappend QC hcopilot.qc
lappend D hueyteam/hdoorgnr ; lappend QC hdoorgnr.qc
lappend D hueyteam/hpilot ; lappend QC hpilot.qc

lappend D lamp ; lappend QC lantern.qc
lappend D lamp ; lappend QC lantern2.qc
lappend D lamp ; lappend QC lantern_broken.qc
lappend D lamp ; lappend QC lantern2_broken.qc

lappend D machete/w_machete ; lappend QC w_machete.qc
lappend D machete/v_machete ; lappend QC v_machete.qc

lappend D w_argrenade ; lappend QC w_argrenade.qc
lappend D w_argrenades ; lappend QC w_argrenades.qc
lappend D w_battery ; lappend QC w_battery.qc
lappend D can ; lappend QC can.qc
lappend D w_flakjacket ; lappend QC w_flakjacket.qc
lappend D medkit ; lappend QC w_medkit_US.qc
lappend D medkit ; lappend QC w_medkit_NVA.qc
lappend D medcrate ; lappend QC w_medcrate.qc
lappend D w_security ; lappend QC w_security.qc
lappend D w_suit ; lappend QC w_suit.qc

lappend D goddess ; lappend QC goddess.qc

lappend D leafgibs ; lappend QC leafgibs.qc

lappend D gibs/hgibs ; lappend QC hgibs_B_Bone1.qc
lappend D gibs/hgibs ; lappend QC hgibs_B_Gib1.qc
lappend D gibs/hgibs ; lappend QC hgibs_Flesh1.qc
lappend D gibs/hgibs ; lappend QC hgibs_Flesh2.qc
lappend D gibs/hgibs ; lappend QC hgibs_Flesh3.qc
lappend D gibs/hgibs ; lappend QC hgibs_Flesh4.qc
lappend D gibs/hgibs ; lappend QC hgibs_Guts1.qc
lappend D gibs/hgibs ; lappend QC hgibs_HMeat1.qc
lappend D gibs/hgibs ; lappend QC hgibs_Legbone1.qc
lappend D gibs/hgibs ; lappend QC hgibs_Lung1.qc
lappend D gibs/hgibs ; lappend QC hgibs_Skull1.qc
lappend D gibs/hgibs ; lappend QC stickygib.qc

#lappend D bush ; lappend QC bush.qc

lappend D palm ; lappend QC palm.qc
lappend D palm/palm1 ; lappend QC palm1.qc
lappend D palm/palm2 ; lappend QC palm2.qc
lappend D palm/palm3 ; lappend QC palm3.qc
lappend D palm/palmgibs ; lappend QC palmgibs.qc

lappend D bowl1 ; lappend QC bowl1.qc
lappend D plate1 ; lappend QC plate1.qc
lappend D plate2 ; lappend QC plate2.qc
lappend D platemoose ; lappend QC platemoose.qc

lappend D snark/v_squeak ; lappend QC v_squeak.qc

lappend D spx_crossbreed ; lappend QC spx_Male1.qc
lappend D spx_crossbreed ; lappend QC spx_Male2.qc
lappend D spx_crossbreed ; lappend QC spx_Male3.qc
lappend D spx_crossbreed ; lappend QC spx_Female1.qc
lappend D spx_crossbreed ; lappend QC spx_Female2.qc
lappend D spx_crossbreed ; lappend QC spx_Female3.qc
lappend D spx_crossbreed ; lappend QC spx_charlie.qc
lappend D spx_crossbreed ; lappend QC spx_mike.qc
lappend D spx_crossbreed ; lappend QC spx_namGrunt.qc

lappend D spx_crossbreed/gibs ; lappend QC Male1_gib1.qc
lappend D spx_crossbreed/gibs ; lappend QC Male1_gib2.qc
lappend D spx_crossbreed/gibs ; lappend QC Male2_gib1.qc
lappend D spx_crossbreed/gibs ; lappend QC Male2_gib2.qc
lappend D spx_crossbreed/gibs ; lappend QC Male3_gib1.qc
lappend D spx_crossbreed/gibs ; lappend QC Male3_gib2.qc
lappend D spx_crossbreed/gibs ; lappend QC namGrunt_gib1.qc
lappend D spx_crossbreed/gibs ; lappend QC namGrunt_gib2.qc
lappend D spx_crossbreed/gibs ; lappend QC namGrunt_gib3.qc

lappend D spx_baby ; lappend QC spx_baby.qc

lappend D spx_mature ; lappend QC spx_mature.qc
lappend D spx_mature/acidspit ; lappend QC acidspit.qc
lappend D spx_mature/gibs ; lappend QC wingLF.qc
lappend D spx_mature/gibs ; lappend QC wingRF.qc
lappend D spx_mature/gibs ; lappend QC wingLR.qc
lappend D spx_mature/gibs ; lappend QC wingRR.qc

#lappend D zombie ; lappend QC zombie_Male1.qc
#lappend D zombie ; lappend QC zombie_Male2.qc
#lappend D zombie ; lappend QC zombie_Male3.qc
#lappend D zombie ; lappend QC zombie_Female1.qc
#lappend D zombie ; lappend QC zombie_Female2.qc
#lappend D zombie ; lappend QC zombie_Female3.qc

lappend D 870/elephantshell ; lappend QC elephantshell.qc
lappend D 870/elephantshell2 ; lappend QC elephantshell.qc
lappend D 870/shotgunshell ; lappend QC shotgunshell.qc
lappend D 870/shotgunshell2 ; lappend QC shotgunshell.qc
lappend D 870/v_870 ; lappend QC v_870.qc
lappend D 870/w_870 ; lappend QC w_870.qc
lappend D 870/w_elephantbox ; lappend QC w_elephantbox.qc
lappend D 870/w_shot_10shells ; lappend QC w_elephant_10shells.qc
lappend D 870/w_shot_22shells ; lappend QC w_elephant_22shells.qc
lappend D 870/w_shot_10shells ; lappend QC w_shot_10shells.qc
lappend D 870/w_shot_22shells ; lappend QC w_shot_22shells.qc
lappend D 870/w_shotbox ; lappend QC w_shotbox.qc

lappend D AK47/v_ak47 ; lappend QC v_ak47.qc
lappend D AK47/w_ak47 ; lappend QC w_ak47.qc
lappend D AK47/w_ak47clip ; lappend QC w_ak47clip.qc

lappend D colt1911a1/v_colt1911a1 ; lappend QC v_colt1911a1.qc
lappend D colt1911a1/w_colt1911a1 ; lappend QC w_colt1911a1.qc
lappend D colt1911a1/w_1143mm_40box ; lappend QC w_1143mm_40box.qc
lappend D colt1911a1/w_1143mm_50box ; lappend QC w_1143mm_50box.qc
lappend D colt1911a1/w_1143mmclip ; lappend QC w_1143mmclip.qc

lappend D M16/v_m16 ; lappend QC v_m16.qc
lappend D M16/w_m16 ; lappend QC w_m16.qc
lappend D M16/w_m16box ; lappend QC w_m16box.qc
lappend D M16/w_m16clip ; lappend QC w_m16clip.qc

lappend D M21/v_m21 ; lappend QC v_m21.qc
lappend D M21/w_m21 ; lappend QC w_m21.qc

lappend D M60/v_m60 ; lappend QC v_m60.qc
lappend D M60/w_m60 ; lappend QC w_m60.qc
lappend D M60/w_m60box ; lappend QC w_m60box.qc
lappend D M60/m60ammobox ; lappend QC w_m60box.qc

lappend D M79/v_m79 ; lappend QC v_m79.qc
lappend D M79/w_m79 ; lappend QC w_m79.qc
lappend D M79/w_m79vest ; lappend QC w_m79vest.qc

lappend D RPG7/v_rpg7 ; lappend QC v_rpg7.qc
lappend D RPG7/w_rpg7 ; lappend QC w_rpg7.qc
lappend D RPG7/rpg7rocket ; lappend QC rpgrocket.qc
lappend D RPG7/rpg7rocket_nam ; lappend QC rpg7rocket.qc

set FORCE 0
set TEST 0

foreach d $D qc $QC {
#    ConvertDir_BMP_to_TGA $d
#    ConvertDir_TGA_to_VTF $d
#    Convert_QC $d $qc
#    ConvertDir_SMD $d
    Compile_MDL_IfNeeded $d $qc
    MSG [string repeat = 80]\n
    update
}

MSG Done.

update
#after 1000 exit
