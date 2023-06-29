# recompile any maps that are older than the VMF

console eval {
	wm title . "compile_newer Console"
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

proc S {path} { return [file nativename $path] ; file attributes $path -shortname }

set SDKBIN [S "C:/Program Files (x86)/Steam/SteamApps/common/Source SDK Base 2013 Singleplayer/bin"]
set GAME [S "C:/Programming/heart of evil 2013/sp/game/hoe"]
set SRC [S "C:/Programming/heart of evil 2013/sp/game/hoe"]

set BSP [S "$SDKBIN/vbsp.exe"]
set VIS [S "$SDKBIN/vvis.exe"]
set RAD [S "$SDKBIN/vrad.exe"]

#set ::env(VPROJECT) $GAME

proc MSG {s} {
	.text insert end $s
	.text see insert
}

proc wrtChanToText {chan} {
	if {[eof $chan]} {
		set ::EXEC_DONE 1
		fconfigure $chan -blocking true
		if {[catch {
		    close $chan
		} msg]} {
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
	MSG $args\n
	if {$::TEST} return
	set cmd $args
	set chan [open |$cmd r]
	fconfigure $chan -blocking false
	fileevent $chan readable [list wrtChanToText $chan]
	set ::EXEC_DONE 0
	vwait ::EXEC_DONE
	return
}

proc compilemap {vmf} {
	set vmfPath [file join $::SRC mapsrc $vmf]
	if {$::RUN_BSP} {
		MSG "running vbsp @ $vmf...\n" ; update
		catch {
			cd $::SDKBIN
			if {$::ONLY_ENTS} {
				set path1 [file join $::SRC mapsrc $vmf].bsp
				set path2 [file join $::GAME maps $vmf].bsp
				if {![file exists $path1]} {
					if {![file exists $path2]} {
						MSG "*** ONLY_ENTS: can't be done, no map\n"
						return
					}
					MSG "ONLY_ENTS: copying from GAME\n"
					if {!$::TEST} {
						file copy $path2 $path1
					}
				}
				EXEC $::BSP -game $::GAME -onlyents $vmfPath
			} else {
				EXEC $::BSP -game $::GAME $vmfPath
			}
		} result
		MSG $result\n
		MSG [string repeat = 80]\n
		if {$::STOP} return
	}

	if {!$::ONLY_ENTS} {

		set prt [file join $::SRC mapsrc $vmf].prt
		if {![file exists $prt]} return

		if {$::RUN_VIS} {
			MSG "running vvis @ $vmf...\n" ; update
			catch {
#				exec $::VIS -game $::GAME -fast [file join $::SRC mapsrc $vmf]
#				EXEC $::VIS -low -threads 1 -fast -game $::GAME [file join $::SRC mapsrc $vmf]
				EXEC $::VIS -low -game $::GAME $vmfPath
			} result
			MSG $result\n
			MSG [string repeat = 80]\n
			if {$::STOP} return
		}

		if {$::RUN_RAD} {
			MSG "running vrad @ $vmf...\n" ; update
			catch {
				if {$::FINAL} {
					if {$::HDR && $::LDR} {
						EXEC $::RAD -low -final -both -game $::GAME $vmfPath
					} elseif {$::HDR} {
						EXEC $::RAD -low -final -hdr -game $::GAME $vmfPath
					} else {
						EXEC $::RAD -low -final -game $::GAME $vmfPath
					}
				} else {
					if {$::HDR && $::LDR} {
						EXEC $::RAD -low -both -game $::GAME $vmfPath
					} elseif {$::HDR} {
						EXEC $::RAD -low -hdr -game $::GAME $vmfPath
					} else {
						EXEC $::RAD -low -game $::GAME $vmfPath
					}
				}
			} result
			MSG $result\n
			MSG [string repeat = 80]\n
			update
		}

	# !ONLY_ENTS
	}

	if {!$::TEST} {
#		file rename -force [file join $::SRC mapsrc $vmf].bsp [file join $::GAME maps $vmf].bsp
		file copy -force [file join $::SRC mapsrc $vmf].bsp [file join $::GAME maps $vmf].bsp
	}
}

set vmfs {
	nama0
	nama1
	nama2

	namb0
	namb1
	namb2
	namb3
	namb4
	namb5

	namc0
	namc1
	namc2
	namc3
	namc4
	namc5

	namd0
	namd1
	namd2
	namd3
	namd4
	namd5

	name0
	name1
	name2
	name3
	name4
	name5

	namf0
	namf1
	namf2
	namf3
	namf4
	namf5

	namg0
	namg1
	namg2
	namg3
	namg4
	namg5

	alamo0
	alamo1

	village

	background01
	background05
}
set vmfs {
	arena-barnacle
	arena-bullsquid
	arena-houndeye
	arena-small
	arena
}
#set vmfs [glob -nocomplain -directory [file join $SRC mapsrc] nam*.vmf alamo*.vmf village.vmf]
#set vmfs [glob -nocomplain -directory [file join $SRC mapsrc] namd*.vmf]
#set vmfs [list [file join $SRC mapsrc arena-small.vmf]]
set skip {
}
set ONLY_ENTS 0
set RUN_BSP 1
set RUN_VIS 1
set HDR 0
set LDR 1
set RUN_RAD 1
set FINAL 0
set FORCE 0
set STOP 0
set TEST 0
foreach vmf $vmfs {
#	set bsp [file rootname [file tail $vmf]]
	if {$vmf in $skip} continue
	set bspPath [file join $GAME maps $vmf].bsp
	set vmfPath [file join $::SRC mapsrc $vmf].vmf
	if {$FORCE || ![file exists $bspPath] || [file mtime $bspPath] < [file mtime $vmfPath]} {
#		MSG "$vmf newer than $bsp\n"
#		compilemap [file rootname [file tail $vmf]]
		compilemap $vmf
	}
	if {$::STOP} break
}
foreach vmf $vmfs {
	set bspPath [file join $GAME maps $vmf].bsp
	set ainPath [file join $GAME maps graphs $vmf].ain
	if {![file exists $ainPath] || ![file exists $bspPath] ||
		[file mtime $ainPath] < [file mtime $bspPath]} {
		MSG "graphs/$vmf.ain needs rebuilding\n"
	}
}
MSG DONE!
