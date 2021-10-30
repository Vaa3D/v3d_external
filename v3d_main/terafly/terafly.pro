# 170623: by RZC. change name of terafly/teramanager.pro to terafly.pro

# enable Vaa3D-TeraFly mode
DEFINES += _VAA3D_TERAFLY_PLUGIN_MODE

#generic set up
CONFIG += use_experimental_features
#DEFINES += QT_NO_DEBUG_STREAM
#DEFINES += QT_NO_DEBUG_OUTPUT
#DEFINES += QT_NO_WARNING_OUTPUT
#DEFINES += _USE_QT_DIALOGS
#DEFINES += terafly_enable_debug_annotations

# set optimization for g++/clang compilers
#win32{
#}
#else{
#    # remove possible other optimization flags
#    QMAKE_CXXFLAGS -= -O
#    QMAKE_CXXFLAGS -= -O1
#    QMAKE_CXXFLAGS -= -O2

#    # add the desired -O3 if not present
#    QMAKE_CXXFLAGS += -O3
#}

#QMAKE_CXXFLAGS += -Wall
#QMAKE_CXXFLAGS += -pedantic
#QMAKE_CXXFLAGS += -Werror


#set up Vaa3D and Qt source path
V3DMAINPATH =  ..
QT_PATH = $$dirname(QMAKE_QMAKE)/..
QMAKE_CXXFLAGS +=-std=c++0x
#HDF5 headers and precompiled library and dependencies (libz and libszip)
INCLUDEPATH += $$V3DMAINPATH/common_lib/include/hdf5

win32 {
     message("WARNING: hdf5: no support for 32 bit windows")
   ## 64bit
     INCLUDEPATH += $$V3DMAINPATH/common_lib/include/hdf5-win64
     LIBS += -L$$V3DMAINPATH/common_lib/winlib64 -lhdf5 -lszip -lzlib
}

macx {
INCLUDEPATH += $$V3DMAINPATH/common_lib/include/hdf5
LIBS += -L$$V3DMAINPATH/common_lib/lib_mac64 -lhdf5 -lszip -lz
}

unix {
INCLUDEPATH += $$V3DMAINPATH/common_lib/include/hdf5
LIBS += -L$$V3DMAINPATH/common_lib/lib_unix64 -lhdf5 -lszip -lz -ldl
}
#Vaa3D headers and sources needed by the plugin
INCLUDEPATH+= $$QT_PATH/demos/shared
INCLUDEPATH += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/3drenderer
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
INCLUDEPATH += $$V3DMAINPATH/basic_c_fun/customary_structs
INCLUDEPATH += $$V3DMAINPATH
INCLUDEPATH += $$V3DMAINPATH/v3d

#enable experimental features
DEFINES += USE_EXPERIMENTAL_FEATURES
#DEFINES += terafly_enable_debug_max_level

#enable gui progress bar
DEFINES += WITH_QT

#enable HDF5
DEFINES += ENABLE_BDV_HDF5
DEFINES += ENABLE_IMS_HDF5

#just define dummy symbols needed by TeraStitcher code
DEFINES += TERASTITCHER_MAJOR
DEFINES += TERASTITCHER_MINOR
DEFINES += TERASTITCHER_PATCH
DEFINES += TERASTITCHER2_MAJOR
DEFINES += TERASTITCHER2_MINOR
DEFINES += TERASTITCHER2_PATCH
DEFINES += TERACONVERTER_MAJOR
DEFINES += TERACONVERTER_MINOR
DEFINES += TERACONVERTER_PATCH

#setup iomanager
INCLUDEPATH += ../terafly/src/terarepo/src/iomanager
HEADERS += ../terafly/src/terarepo/src/iomanager/iomanager.config.h \

HEADERS += ../terafly/src/terarepo/src/iomanager/ioplugins.h
HEADERS += ../terafly/src/terarepo/src/iomanager/IOPluginAPI.h
HEADERS += ../terafly/src/terarepo/src/iomanager/plugins/exampleplugin2D/exampleplugin2D.h
HEADERS += ../terafly/src/terarepo/src/iomanager/plugins/IMS_HDF5/IMS_HDF5.h
HEADERS += ../terafly/src/terarepo/src/iomanager/plugins/dcimg/dcimg.h
#HEADERS += ../terafly/src/terarepo/src/iomanager/plugins/opencv2D/opencv2D.h
HEADERS += ../terafly/src/terarepo/src/iomanager/plugins/tiff2D/tiff2D.h
HEADERS += ../terafly/src/terarepo/src/iomanager/plugins/tiff3D/tiff3D.h
SOURCES += ../terafly/src/terarepo/src/iomanager/iomanager.config.cpp \

SOURCES += ../terafly/src/terarepo/src/iomanager/plugins/exampleplugin2D/exampleplugin2D.cpp
SOURCES += ../terafly/src/terarepo/src/iomanager/plugins/IMS_HDF5/IMS_HDF5.cpp
SOURCES += ../terafly/src/terarepo/src/iomanager/plugins/dcimg/dcimg.cpp
#SOURCES += ../terafly/src/terarepo/src/iomanager/plugins/opencv2D/opencv2D.cpp
SOURCES += ../terafly/src/terarepo/src/iomanager/plugins/tiff2D/tiff2D.cpp
SOURCES += ../terafly/src/terarepo/src/iomanager/plugins/tiff3D/tiff3D.cpp

#setup imagemanager
INCLUDEPATH += ../terafly/src/terarepo/src/imagemanager
HEADERS += ../terafly/src/terarepo/src/imagemanager/BDVVolume.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/MultiCycleVolume.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/MultiSliceVolume.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/HDF5Mngr.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/imBlock.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/dirent_win.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/IM_config.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/RawFmtMngr.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/RawVolume.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/SimpleVolume.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/SimpleVolumeRaw.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/Stack.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/StackRaw.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/StackedVolume.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/Tiff3DMngr.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/TiledMCVolume.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/TiledVolume.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/TimeSeries.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/VirtualFmtMngr.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/VirtualVolume.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/UnstitchedVolume.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/IMS_HDF5Mngr.h
HEADERS += ../terafly/src/terarepo/src/imagemanager/ComposedVolume.h
SOURCES += ../terafly/src/terarepo/src/imagemanager/IMS_HDF5Mngr.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/BDVVolume.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/HDF5Mngr.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/imBlock.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/IM_config.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/RawFmtMngr.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/RawVolume.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/SimpleVolume.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/SimpleVolumeRaw.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/Stack.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/StackRaw.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/StackedVolume.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/Tiff3DMngr.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/TiledMCVolume.cpp
#SOURCES += ../terafly/src/terarepo/src/imagemanager/TiledVolume.cpp
SOURCES += ../terafly/src/TiledVolume_partialdata.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/TimeSeries.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/VirtualFmtMngr.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/VirtualVolume.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/UnstitchedVolume.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/MultiCycleVolume.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/MultiSliceVolume.cpp
SOURCES += ../terafly/src/terarepo/src/imagemanager/ComposedVolume.cpp
# set up stitcher
INCLUDEPATH += ../terafly/src/terarepo/src/stitcher
HEADERS += ../terafly/src/terarepo/src/stitcher/S_config.h
HEADERS += ../terafly/src/terarepo/src/stitcher/Displacement.h
HEADERS += ../terafly/src/terarepo/src/stitcher/DisplacementMIPNCC.h
HEADERS += ../terafly/src/terarepo/src/stitcher/PDAlgo.h
HEADERS += ../terafly/src/terarepo/src/stitcher/PDAlgoMIPNCC.h
HEADERS += ../terafly/src/terarepo/src/stitcher/StackRestorer.h
HEADERS += ../terafly/src/terarepo/src/stitcher/StackStitcher.h
HEADERS += ../terafly/src/terarepo/src/stitcher/TPAlgo.h
HEADERS += ../terafly/src/terarepo/src/stitcher/TPAlgoMST.h
HEADERS += ../terafly/src/terarepo/src/stitcher/resumer.h
HEADERS += ../terafly/src/terarepo/src/stitcher/TPAlgoLQP.h

SOURCES += ../terafly/src/terarepo/src/stitcher/Displacement.cpp
SOURCES += ../terafly/src/terarepo/src/stitcher/DisplacementMIPNCC.cpp
SOURCES += ../terafly/src/terarepo/src/stitcher/MergeTiles.cpp
SOURCES += ../terafly/src/terarepo/src/stitcher/PDAlgo.cpp
SOURCES += ../terafly/src/terarepo/src/stitcher/PDAlgoMIPNCC.cpp
SOURCES += ../terafly/src/terarepo/src/stitcher/StackRestorer.cpp
SOURCES += ../terafly/src/terarepo/src/stitcher/StackStitcher.cpp
SOURCES += ../terafly/src/terarepo/src/stitcher/TPAlgo.cpp
SOURCES += ../terafly/src/terarepo/src/stitcher/TPAlgoMST.cpp
SOURCES += ../terafly/src/terarepo/src/stitcher/resumer.cpp
SOURCES += ../terafly/src/terarepo/src/stitcher/TPAlgoLQP.cpp
# set up volumeconverter
INCLUDEPATH += ../terafly/src/terarepo/src/utils/volumeconverter
#HEADERS += ../terafly/src/terarepo/src/utils/volumeconverter/S_config.h
HEADERS += ../terafly/src/terarepo/src/utils/volumeconverter/VolumeConverter.h
SOURCES += ../terafly/src/terarepo/src/utils/volumeconverter/VolumeConverter.cpp
HEADERS += ../terafly/src/terarepo/src/utils/volumeconverter/vcresumer.h
SOURCES += ../terafly/src/terarepo/src/utils/volumeconverter/vcresumer.cpp

# set up volumemanager
INCLUDEPATH += ../terafly/src/terarepo/src/volumemanager
HEADERS += ../terafly/src/terarepo/src/volumemanager/vmBlock.h
HEADERS += ../terafly/src/terarepo/src/volumemanager/vmBlockVolume.h
HEADERS += ../terafly/src/terarepo/src/volumemanager/vmStack.h
HEADERS += ../terafly/src/terarepo/src/volumemanager/vmStackedVolume.h
HEADERS += ../terafly/src/terarepo/src/volumemanager/vmVirtualStack.h
HEADERS += ../terafly/src/terarepo/src/volumemanager/vmVirtualVolume.h
HEADERS += ../terafly/src/terarepo/src/volumemanager/vmCacheManager.h
HEADERS += ../terafly/src/terarepo/src/volumemanager/volumemanager.config.h
HEADERS += ../terafly/src/terarepo/src/volumemanager/vmMCVolume.h
SOURCES += ../terafly/src/terarepo/src/volumemanager/vmBlock.cpp
SOURCES += ../terafly/src/terarepo/src/volumemanager/vmBlockVolume.cpp
SOURCES += ../terafly/src/terarepo/src/volumemanager/vmStack.cpp
SOURCES += ../terafly/src/terarepo/src/volumemanager/vmStackedVolume.cpp
SOURCES += ../terafly/src/terarepo/src/volumemanager/vmVirtualStack.cpp
SOURCES += ../terafly/src/terarepo/src/volumemanager/vmVirtualVolume.cpp
SOURCES += ../terafly/src/terarepo/src/volumemanager/vmCacheManager.cpp
SOURCES += ../terafly/src/terarepo/src/volumemanager/volumemanager.config.cpp
SOURCES += ../terafly/src/terarepo/src/volumemanager/vmMCVolume.cpp

# set up crossmips
INCLUDEPATH += ../terafly/src/terarepo/src/crossmips
HEADERS += ../terafly/src/terarepo/src/crossmips/my_defs.h
HEADERS += ../terafly/src/terarepo/src/crossmips/CrossMIPs.h
HEADERS += ../terafly/src/terarepo/src/crossmips/compute_funcs.h
SOURCES += ../terafly/src/terarepo/src/crossmips/compute_funcs.cpp
SOURCES += ../terafly/src/terarepo/src/crossmips/libcrossmips.cpp

# set up tinyxml
INCLUDEPATH += ../terafly/src/terarepo/src/3rdparty/tinyxml
HEADERS += ../terafly/src/terarepo/src/3rdparty/tinyxml/tinyxml.h
HEADERS += ../terafly/src/terarepo/src/3rdparty/tinyxml/tinystr.h
SOURCES += ../terafly/src/terarepo/src/3rdparty/tinyxml/tinystr.cpp
SOURCES += ../terafly/src/terarepo/src/3rdparty/tinyxml/tinyxmlparser.cpp
SOURCES += ../terafly/src/terarepo/src/3rdparty/tinyxml/tinyxmlerror.cpp
SOURCES += ../terafly/src/terarepo/src/3rdparty/tinyxml/tinyxml.cpp

# set up common
INCLUDEPATH += ../terafly/src/terarepo/src/common
HEADERS += ../terafly/src/terarepo/src/common/config.h
HEADERS += ../terafly/src/terarepo/src/common/ProgressBar.h
HEADERS += ../terafly/src/terarepo/src/common/QProgressSender.h
SOURCES += ../terafly/src/terarepo/src/common/config.cpp
SOURCES += ../terafly/src/terarepo/src/common/ProgressBar.cpp
SOURCES += ../terafly/src/terarepo/src/common/QProgressSender.cpp


#set up TeraFly (control and presentation classes)
RESOURCES += ../terafly/icons.qrc
INCLUDEPATH += ../terafly/src/control
HEADERS += ../terafly/src/control/V3Dsubclasses.h
HEADERS += ../terafly/src/control/CAnnotations.h
HEADERS += ../terafly/src/control/CConverter.h
HEADERS += ../terafly/src/control/CViewer.h
HEADERS += ../terafly/src/control/CImport.h
HEADERS += ../terafly/src/control/CPlugin.h
HEADERS += ../terafly/src/control/CSettings.h
HEADERS += ../terafly/src/control/CVolume.h
HEADERS += ../terafly/src/control/CImageUtils.h
HEADERS += ../terafly/src/control/V3Dsubclasses.h
HEADERS += ../terafly/src/control/VirtualPyramid.h
HEADERS += ../terafly/src/control/COperation.h
INCLUDEPATH += ../terafly/src/presentation
HEADERS += ../terafly/src/presentation/PConverter.h
HEADERS += ../terafly/src/presentation/PDialogImport.h
HEADERS += ../terafly/src/presentation/PDialogProofreading.h
HEADERS += ../terafly/src/presentation/PDialogVirtualPyramid.h
HEADERS += ../terafly/src/presentation/PMain.h
HEADERS += ../terafly/src/presentation/PTabVolumeInfo.h
HEADERS += ../terafly/src/presentation/QArrowButton.h
HEADERS += ../terafly/src/presentation/QGradientBar.h
HEADERS += ../terafly/src/presentation/QHelpBox.h
HEADERS += ../terafly/src/presentation/QGLRefSys.h
HEADERS += ../terafly/src/presentation/QPixmapToolTip.h
HEADERS += ../terafly/src/presentation/QPrefixSuffixLineEdit.h
HEADERS += ../terafly/src/presentation/PAbout.h
HEADERS += ../terafly/src/presentation/PLog.h
HEADERS += ../terafly/src/presentation/PAnoToolBar.h
HEADERS += ../terafly/src/control/QUndoMarkerCreate.h
HEADERS += ../terafly/src/control/QUndoMarkerDelete.h
HEADERS += ../terafly/src/control/QUndoMarkerDeleteROI.h
SOURCES += ../terafly/src/control/CAnnotations.cpp
SOURCES += ../terafly/src/control/CConverter.cpp
SOURCES += ../terafly/src/control/CViewer.cpp
SOURCES += ../terafly/src/control/CImport.cpp
SOURCES += ../terafly/src/control/CPlugin.cpp
SOURCES += ../terafly/src/control/CSettings.cpp
SOURCES += ../terafly/src/control/CVolume.cpp
SOURCES += ../terafly/src/control/CImageUtils.cpp
SOURCES += ../terafly/src/control/COperation.cpp
SOURCES += ../terafly/src/control/V3Dsubclasses.cpp
SOURCES += ../terafly/src/control/VirtualPyramid.cpp
SOURCES += ../terafly/src/presentation/PConverter.cpp
SOURCES += ../terafly/src/presentation/PDialogImport.cpp
SOURCES += ../terafly/src/presentation/PDialogProofreading.cpp
SOURCES += ../terafly/src/presentation/PDialogVirtualPyramid.cpp
SOURCES += ../terafly/src/presentation/PMain.cpp
SOURCES += ../terafly/src/presentation/PTabVolumeInfo.cpp
SOURCES += ../terafly/src/presentation/PAbout.cpp
SOURCES += ../terafly/src/presentation/PLog.cpp
SOURCES += ../terafly/src/presentation/PAnoToolBar.cpp
SOURCES += ../terafly/src/presentation/QPixmapToolTip.cpp
SOURCES += ../terafly/src/presentation/QArrowButton.cpp
SOURCES += ../terafly/src/presentation/QGradientBar.cpp
SOURCES += ../terafly/src/presentation/QHelpBox.cpp
SOURCES += ../terafly/src/presentation/QGLRefSys.cpp
SOURCES += ../terafly/src/presentation/QPrefixSuffixLineEdit.cpp
SOURCES += ../terafly/src/control/QUndoMarkerCreate.cpp
SOURCES += ../terafly/src/control/QUndoMarkerDelete.cpp
SOURCES += ../terafly/src/control/QUndoMarkerDeleteROI.cpp
