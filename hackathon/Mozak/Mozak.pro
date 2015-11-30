
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64

CONFIG += MOZAK_STANDALONE

MOZAK_STANDALONE {
	# For the auto-launching standalone version in CGS git repo: ./hackathon/mozak
	include(../../v3d_main/v3d/vaa3d_msvc.pro)
	DEFINES += CGS_AUTOLAUNCH
	DEFINES += V3D_SKIP_AUTO_VERSION_CHECK
	DEFINES += RENDERER_RIGHT_CLICK_MENU_DISABLED
	DEFINES += HIDE_ANO_TOOLBAR
	DEFINES += UNREVERSE_MOUSE_WHEEL_ZOOM # not working yet
	DEFINES += FORCE_BBOX_MODE
	VAA3DPATH = ../..
} else {
	# Mozak as a Vaa3D plugin in Vaa3D tools SVN: vaa3d_tools/hackathon/mozak
	VAA3DPATH = ../../../v3d_external
	
	SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
	INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

	TARGET	= $$qtLibraryTarget(Mozak)
	DESTDIR	= $$VAA3DPATH/bin/plugins/Mozak/
}

HEADERS	+= 	MozakPlugin.h \
			MozakUI.h \
			Mozak3DView.h
SOURCES	+= 	MozakPlugin.cpp \
			MozakUI.cpp \
			Mozak3DView.cpp

