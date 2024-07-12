#HLSDK server Android port
#Copyright (c) nicknekit

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := server_aomdc

include $(XASH3D_CONFIG)

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a-hard)
LOCAL_MODULE_FILENAME = libserver_hardfp
endif

LOCAL_CFLAGS += -D_LINUX -DCLIENT_WEAPONS -Dstricmp=strcasecmp -Dstrnicmp=strncasecmp -D_snprintf=snprintf \
	-fno-exceptions -DNO_VOICEGAMEMGR -w

LOCAL_CPPFLAGS := $(LOCAL_CFLAGS)

LOCAL_C_INCLUDES := $(SDL_PATH)/include \
		    $(LOCAL_PATH)/. \
		    $(LOCAL_PATH)/wpn_shared \
		    $(LOCAL_PATH)/../common \
		    $(LOCAL_PATH)/../engine/common \
		    $(LOCAL_PATH)/../engine \
		    $(LOCAL_PATH)/../public \
		    $(LOCAL_PATH)/../pm_shared \
		    $(LOCAL_PATH)/../game_shared \
			$(LOCAL_PATH)/aomdc

LOCAL_SRC_FILES := aflock.cpp agrunt.cpp schedule.cpp airtank.cpp \
		alias.cpp animating.cpp animation.cpp apache.cpp barnacle.cpp barney.cpp \
		bigmomma.cpp bloater.cpp bmodels.cpp bullsquid.cpp buttons.cpp cbase.cpp \
		client.cpp combat.cpp controller.cpp defaultai.cpp \
		doors.cpp effects.cpp explode.cpp flyingmonster.cpp func_break.cpp \
		func_tank.cpp game.cpp gamerules.cpp gargantua.cpp genericmonster.cpp \
		ggrenade.cpp globals.cpp gman.cpp h_ai.cpp hassassin.cpp \
		h_battery.cpp h_cine.cpp h_cycler.cpp headcrab.cpp healthkit.cpp h_export.cpp \
		hgrunt.cpp hornet.cpp houndeye.cpp ichthyosaur.cpp islave_deamon.cpp \
		items.cpp leech.cpp lights.cpp locus.cpp maprules.cpp monstermaker.cpp monsters.cpp \
		monsterstate.cpp mortar.cpp movewith.cpp multiplay_gamerules.cpp nihilanth.cpp \
		nodes.cpp observer.cpp osprey.cpp pathcorner.cpp plane.cpp plats.cpp player.cpp playermonster.cpp \
		rat.cpp roach.cpp rpg.cpp scientist.cpp scripted.cpp shotgun.cpp \
		singleplay_gamerules.cpp skill.cpp sound.cpp soundent.cpp spectator.cpp squadmonster.cpp \
		squeakgrenade.cpp stats.cpp subs.cpp talkmonster.cpp teamplay_gamerules.cpp tempmonster.cpp \
		tentacle.cpp triggers.cpp turret.cpp util.cpp weapons.cpp world.cpp xen.cpp \
		zombie.cpp aomdc/knife.cpp aomdc/axe.cpp \
                aomdc/hammer.cpp aomdc/spear.cpp aomdc/glock.cpp aomdc/beretta.cpp \
                aomdc/p228.cpp aomdc/deagle.cpp aomdc/revolver.cpp aomdc/mp5k.cpp \
                aomdc/uzi.cpp aomdc/gmgeneral.cpp aomdc/zombie2.cpp aomdc/zombie3.cpp aomdc/zombie4.cpp \
                aomdc/ghost.cpp aomdc/wheelchair.cpp aomdc/baddavid.cpp \
	   ../pm_shared/pm_debug.c ../pm_shared/pm_math.c ../pm_shared/pm_shared.c

LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)
