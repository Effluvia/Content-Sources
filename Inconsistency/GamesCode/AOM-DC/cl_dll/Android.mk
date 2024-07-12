#hlsdk-2.3 client port for android
#Copyright (c) mittorn

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := client_aomdc

APP_PLATFORM := android-19

include $(XASH3D_CONFIG)

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a-hard)
LOCAL_MODULE_FILENAME = libclient_hardfp
endif

LOCAL_CFLAGS += -DVR -DCLIENT_DLL=1

SRCS=
SRCS_C=
SRCS+=./ev_hldm.cpp
SRCS+=./hl/hl_baseentity.cpp
SRCS+=./hl/hl_events.cpp
SRCS+=./hl/hl_objects.cpp
SRCS+=./hl/hl_weapons.cpp
SRCS+=../dlls/aomdc/knife.cpp
SRCS+=../dlls/aomdc/axe.cpp
SRCS+=../dlls/aomdc/hammer.cpp
SRCS+=../dlls/aomdc/spear.cpp
SRCS+=../dlls/aomdc/glock.cpp
SRCS+=../dlls/aomdc/beretta.cpp
SRCS+=../dlls/aomdc/p228.cpp
SRCS+=../dlls/aomdc/deagle.cpp
SRCS+=../dlls/aomdc/revolver.cpp
SRCS+=../dlls/aomdc/mp5k.cpp
SRCS+=../dlls/aomdc/uzi.cpp
SRCS+=../dlls/aomdc/gmgeneral.cpp
SRCS+=../dlls/shotgun.cpp
SRCS+=./aomdc/blackbar.cpp
#SRCS+=./aomdc/neffect.cpp
#SRCS+=../game_shared/voice_banmgr.cpp
#SRCS+=../game_shared/voice_status.cpp
SRCS+=./ammo.cpp
SRCS+=./ammo_secondary.cpp
SRCS+=./ammohistory.cpp
SRCS+=./battery.cpp
SRCS+=./cdll_int.cpp
SRCS+=./com_weapons.cpp
SRCS+=./death.cpp
SRCS+=./demo.cpp
SRCS+=./entity.cpp
SRCS+=./ev_common.cpp
SRCS+=./events.cpp
SRCS+=./flashlight.cpp
SRCS+=./GameStudioModelRenderer.cpp
SRCS+=./geiger.cpp
SRCS+=./health.cpp
SRCS+=./hud.cpp
SRCS+=./hud_msg.cpp
SRCS+=./hud_redraw.cpp
#SRCS+=./hud_servers.cpp
SRCS+=./hud_spectator.cpp
SRCS+=./hud_update.cpp
SRCS+=./in_camera.cpp
SRCS+=./input.cpp
SRCS+=./input_goldsource.cpp
SRCS+=./input_mouse.cpp
#SRCS+=./inputw32.cpp
SRCS+=./Matrices.cpp
SRCS+=./menu.cpp
SRCS+=./message.cpp
SRCS+=./overview.cpp
SRCS+=./parsemsg.cpp
SRCS+=./particlemgr.cpp
SRCS+=./particlemsg.cpp
SRCS+=./particlesys.cpp
SRCS_C+=../pm_shared/pm_debug.c
SRCS_C+=../pm_shared/pm_math.c
SRCS_C+=../pm_shared/pm_shared.c
SRCS+=./saytext.cpp
SRCS+=./status_icons.cpp
SRCS+=./statusbar.cpp
#SRCS+=./stealth.cpp
SRCS+=./studio_util.cpp
SRCS+=./StudioModelRenderer.cpp
SRCS+=./text_message.cpp
SRCS+=./train.cpp
SRCS+=./tri.cpp
SRCS+=./util.cpp
SRCS+=./view.cpp
SRCS+=./input_xash3d.cpp
SRCS+=./scoreboard.cpp
SRCS+=./MOTD.cpp
SRCS+=./vr_renderer.cpp
SRCS+=./vr_helper.cpp

INCLUDES =  -I../common -I. -I../game_shared -I../pm_shared -I../engine -I../dlls -I../utils/false_vgui/include
DEFINES = -Wno-write-strings -DLINUX -D_LINUX -Dstricmp=strcasecmp -Dstrnicmp=strncasecmp -DCLIENT_WEAPONS -DCLIENT_DLL -w -D_snprintf=snprintf

LOCAL_C_INCLUDES := $(LOCAL_PATH)/. \
		 $(LOCAL_PATH)/../common \
		 $(LOCAL_PATH)/../engine \
		 $(LOCAL_PATH)/../game_shared \
		 $(LOCAL_PATH)/../dlls \
		 $(LOCAL_PATH)/../pm_shared \
		 $(LOCAL_PATH)/../utils/false_vgui/include
LOCAL_CFLAGS += $(DEFINES) $(INCLUDES)

ifeq ($(GOLDSOURCE_SUPPORT),1)
	DEFINES += -DGOLDSOURCE_SUPPORT
	ifeq ($(shell uname -s),Linux)
		LOCAL_LDLIBS += -ldl
	endif
endif

LOCAL_SRC_FILES := $(SRCS) $(SRCS_C)

include $(BUILD_SHARED_LIBRARY)
