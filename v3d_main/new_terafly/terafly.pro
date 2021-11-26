# 20170623: by RZC. Change name of new_terafly/teramanager.pro to new_terafly.pro
# 20201112: by RZC. Add 4 new cpp files
SOURCES += ../new_terafly/src/terarepo/src/imagemanager/MultiCycleVolume.cpp
SOURCES += ../new_terafly/src/terarepo/src/imagemanager/MultiSliceVolume.cpp
SOURCES += ../new_terafly/src/terarepo/src/imagemanager/ComposedVolume.cpp
SOURCES += ../new_terafly/src/terarepo/src/stitcher/TPAlgoLQP.cpp


QT += core5compat widgets opengl openglwidgets
#CONFIG   += c++11
# enable Vaa3D-new_terafly mode
DEFINES += _VAA3D_TERAFLY_PLUGIN_MODE
#QMAKE_CXXFLAGS += -std=c++11
#generic set up
CONFIG += use_experimental_features
#DEFINES += QT_NO_DEBUG_STREAM
#DEFINES += QT_NO_DEBUG_OUTPUT
#DEFINES += QT_NO_WARNING_OUTPUT
#DEFINES += _USE_QT_DIALOGS
#DEFINES += new_terafly_enable_debug_annotations

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

#HDF5 headers and precompiled library and dependencies (libz and libszip)
INCLUDEPATH += $$V3DMAINPATH/common_lib/include/hdf5

win32 {
     message("WARNING: hdf5: no support for 32 bit windows")
   ## 64bit
     INCLUDEPATH += $$V3DMAINPATH/common_lib/include/hdf5-win64
     LIBS += -L$$V3DMAINPATH/common_lib/mingw64 -lhdf5 -lszip -lzlib
}

macx {
INCLUDEPATH += $$V3DMAINPATH/common_lib/include/hdf5
LIBS += -L$$V3DMAINPATH/common_lib/lib_mac64 -lhdf5 -lszip -lz
}

unix {
    INCLUDEPATH += $$V3DMAINPATH/common_lib/include/hdf5

    LIBS += -L$$V3DMAINPATH/common_lib/lib_ubuntu -lhdf5 -lszip -lz -ldl
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
#DEFINES += new_terafly_enable_debug_max_level

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
INCLUDEPATH += ../new_terafly/src/terarepo/src/iomanager

HEADERS += ../new_terafly/src/terarepo/src/iomanager/iomanager.config.h
HEADERS += ../new_terafly/src/terarepo/src/iomanager/ioplugins.h
HEADERS += ../new_terafly/src/terarepo/src/iomanager/IOPluginAPI.h
HEADERS += ../new_terafly/src/terarepo/src/iomanager/plugins/exampleplugin2D/exampleplugin2D.h
HEADERS += ../new_terafly/src/terarepo/src/iomanager/plugins/IMS_HDF5/IMS_HDF5.h
HEADERS += ../new_terafly/src/terarepo/src/iomanager/plugins/dcimg/dcimg.h
#HEADERS += ../new_terafly/src/terarepo/src/iomanager/plugins/opencv2D/opencv2D.h
HEADERS += ../new_terafly/src/terarepo/src/iomanager/plugins/tiff2D/tiff2D.h
HEADERS += ../new_terafly/src/terarepo/src/iomanager/plugins/tiff3D/tiff3D.h
SOURCES += ../new_terafly/src/terarepo/src/iomanager/iomanager.config.cpp
SOURCES += ../new_terafly/src/terarepo/src/iomanager/plugins/exampleplugin2D/exampleplugin2D.cpp
SOURCES += ../new_terafly/src/terarepo/src/iomanager/plugins/IMS_HDF5/IMS_HDF5.cpp
SOURCES += ../new_terafly/src/terarepo/src/iomanager/plugins/dcimg/dcimg.cpp
#SOURCES += ../new_terafly/src/terarepo/src/iomanager/plugins/opencv2D/opencv2D.cpp
SOURCES += ../new_terafly/src/terarepo/src/iomanager/plugins/tiff2D/tiff2D.cpp
SOURCES += ../new_terafly/src/terarepo/src/iomanager/plugins/tiff3D/tiff3D.cpp

#setup imagemanager
INCLUDEPATH += ../new_terafly/src/terarepo/src/imagemanager
HEADERS += ../new_terafly/src/terarepo/src/imagemanager/BDVVolume.h
HEADERS += ../new_terafly/src/terarepo/src/imagemanager/HDF5Mngr.h
HEADERS += ../new_terafly/src/terarepo/src/imagemanager/imBlock.h
HEADERS += ../new_terafly/src/terarepo/src/imagemanager/dirent_win.h
HEADERS += ../new_terafly/src/terarepo/src/imagemanager/IM_config.h
HEADERS += ../new_terafly/src/terarepo/src/imagemanager/RawFmtMngr.h
HEADERS += ../new_terafly/src/terarepo/src/imagemanager/RawVolume.h
HEADERS += ../new_terafly/src/terarepo/src/imagemanager/SimpleVolume.h
HEADERS += ../new_terafly/src/terarepo/src/imagemanager/SimpleVolumeRaw.h
HEADERS += ../new_terafly/src/terarepo/src/imagemanager/Stack.h
HEADERS += ../new_terafly/src/terarepo/src/imagemanager/StackRaw.h
HEADERS += ../new_terafly/src/terarepo/src/imagemanager/StackedVolume.h
HEADERS += ../new_terafly/src/terarepo/src/imagemanager/Tiff3DMngr.h
HEADERS += ../new_terafly/src/terarepo/src/imagemanager/TiledMCVolume.h
HEADERS += ../new_terafly/src/terarepo/src/imagemanager/TiledVolume.h
HEADERS += ../new_terafly/src/terarepo/src/imagemanager/TimeSeries.h
HEADERS += ../new_terafly/src/terarepo/src/imagemanager/VirtualFmtMngr.h
HEADERS += ../new_terafly/src/terarepo/src/imagemanager/VirtualVolume.h
HEADERS += ../new_terafly/src/terarepo/src/imagemanager/UnstitchedVolume.h
HEADERS += ../new_terafly/src/terarepo/src/imagemanager/IMS_HDF5Mngr.h
SOURCES += ../new_terafly/src/terarepo/src/imagemanager/IMS_HDF5Mngr.cpp
SOURCES += ../new_terafly/src/terarepo/src/imagemanager/BDVVolume.cpp
SOURCES += ../new_terafly/src/terarepo/src/imagemanager/HDF5Mngr.cpp
SOURCES += ../new_terafly/src/terarepo/src/imagemanager/imBlock.cpp
SOURCES += ../new_terafly/src/terarepo/src/imagemanager/IM_config.cpp
SOURCES += ../new_terafly/src/terarepo/src/imagemanager/RawFmtMngr.cpp
SOURCES += ../new_terafly/src/terarepo/src/imagemanager/RawVolume.cpp
SOURCES += ../new_terafly/src/terarepo/src/imagemanager/SimpleVolume.cpp
SOURCES += ../new_terafly/src/terarepo/src/imagemanager/SimpleVolumeRaw.cpp
SOURCES += ../new_terafly/src/terarepo/src/imagemanager/Stack.cpp
SOURCES += ../new_terafly/src/terarepo/src/imagemanager/StackRaw.cpp
SOURCES += ../new_terafly/src/terarepo/src/imagemanager/StackedVolume.cpp
SOURCES += ../new_terafly/src/terarepo/src/imagemanager/Tiff3DMngr.cpp
SOURCES += ../new_terafly/src/terarepo/src/imagemanager/TiledMCVolume.cpp
#SOURCES += ../new_terafly/src/terarepo/src/imagemanager/TiledVolume.cpp
SOURCES += ../new_terafly/src/TiledVolume_partialdata.cpp
SOURCES += ../new_terafly/src/terarepo/src/imagemanager/TimeSeries.cpp
SOURCES += ../new_terafly/src/terarepo/src/imagemanager/VirtualFmtMngr.cpp
SOURCES += ../new_terafly/src/terarepo/src/imagemanager/VirtualVolume.cpp
SOURCES += ../new_terafly/src/terarepo/src/imagemanager/UnstitchedVolume.cpp

# set up stitcher
INCLUDEPATH += ../new_terafly/src/terarepo/src/stitcher
HEADERS += ../new_terafly/src/terarepo/src/stitcher/S_config.h
HEADERS += ../new_terafly/src/terarepo/src/stitcher/Displacement.h
HEADERS += ../new_terafly/src/terarepo/src/stitcher/DisplacementMIPNCC.h
HEADERS += ../new_terafly/src/terarepo/src/stitcher/PDAlgo.h
HEADERS += ../new_terafly/src/terarepo/src/stitcher/PDAlgoMIPNCC.h
HEADERS += ../new_terafly/src/terarepo/src/stitcher/StackRestorer.h
HEADERS += ../new_terafly/src/terarepo/src/stitcher/StackStitcher.h
HEADERS += ../new_terafly/src/terarepo/src/stitcher/TPAlgo.h
HEADERS += ../new_terafly/src/terarepo/src/stitcher/TPAlgoMST.h
HEADERS += ../new_terafly/src/terarepo/src/stitcher/resumer.h
SOURCES += ../new_terafly/src/terarepo/src/stitcher/Displacement.cpp
SOURCES += ../new_terafly/src/terarepo/src/stitcher/DisplacementMIPNCC.cpp
SOURCES += ../new_terafly/src/terarepo/src/stitcher/MergeTiles.cpp
SOURCES += ../new_terafly/src/terarepo/src/stitcher/PDAlgo.cpp
SOURCES += ../new_terafly/src/terarepo/src/stitcher/PDAlgoMIPNCC.cpp
SOURCES += ../new_terafly/src/terarepo/src/stitcher/StackRestorer.cpp
SOURCES += ../new_terafly/src/terarepo/src/stitcher/StackStitcher.cpp
SOURCES += ../new_terafly/src/terarepo/src/stitcher/TPAlgo.cpp
SOURCES += ../new_terafly/src/terarepo/src/stitcher/TPAlgoMST.cpp
SOURCES += ../new_terafly/src/terarepo/src/stitcher/resumer.cpp

# set up volumeconverter
INCLUDEPATH += ../new_terafly/src/terarepo/src/utils/volumeconverter
#HEADERS += ../new_terafly/src/terarepo/src/utils/volumeconverter/S_config.h
HEADERS += ../new_terafly/src/terarepo/src/utils/volumeconverter/VolumeConverter.h
SOURCES += ../new_terafly/src/terarepo/src/utils/volumeconverter/VolumeConverter.cpp
HEADERS += ../new_terafly/src/terarepo/src/utils/volumeconverter/vcresumer.h
SOURCES += ../new_terafly/src/terarepo/src/utils/volumeconverter/vcresumer.cpp

# set up volumemanager
INCLUDEPATH += ../new_terafly/src/terarepo/src/volumemanager
HEADERS += ../new_terafly/src/terarepo/src/volumemanager/vmBlock.h
HEADERS += ../new_terafly/src/terarepo/src/volumemanager/vmBlockVolume.h
HEADERS += ../new_terafly/src/terarepo/src/volumemanager/vmStack.h
HEADERS += ../new_terafly/src/terarepo/src/volumemanager/vmStackedVolume.h
HEADERS += ../new_terafly/src/terarepo/src/volumemanager/vmVirtualStack.h
HEADERS += ../new_terafly/src/terarepo/src/volumemanager/vmVirtualVolume.h
HEADERS += ../new_terafly/src/terarepo/src/volumemanager/vmCacheManager.h
HEADERS += ../new_terafly/src/terarepo/src/volumemanager/volumemanager.config.h
HEADERS += ../new_terafly/src/terarepo/src/volumemanager/vmMCVolume.h
SOURCES += ../new_terafly/src/terarepo/src/volumemanager/vmBlock.cpp
SOURCES += ../new_terafly/src/terarepo/src/volumemanager/vmBlockVolume.cpp
SOURCES += ../new_terafly/src/terarepo/src/volumemanager/vmStack.cpp
SOURCES += ../new_terafly/src/terarepo/src/volumemanager/vmStackedVolume.cpp
SOURCES += ../new_terafly/src/terarepo/src/volumemanager/vmVirtualStack.cpp
SOURCES += ../new_terafly/src/terarepo/src/volumemanager/vmVirtualVolume.cpp
SOURCES += ../new_terafly/src/terarepo/src/volumemanager/vmCacheManager.cpp
SOURCES += ../new_terafly/src/terarepo/src/volumemanager/volumemanager.config.cpp
SOURCES += ../new_terafly/src/terarepo/src/volumemanager/vmMCVolume.cpp

# set up crossmips
INCLUDEPATH += ../new_terafly/src/terarepo/src/crossmips
HEADERS += ../new_terafly/src/terarepo/src/crossmips/my_defs.h
HEADERS += ../new_terafly/src/terarepo/src/crossmips/CrossMIPs.h
HEADERS += ../new_terafly/src/terarepo/src/crossmips/compute_funcs.h
SOURCES += ../new_terafly/src/terarepo/src/crossmips/compute_funcs.cpp
SOURCES += ../new_terafly/src/terarepo/src/crossmips/libcrossmips.cpp

# set up tinyxml
INCLUDEPATH += ../new_terafly/src/terarepo/src/3rdparty/tinyxml
HEADERS += ../new_terafly/src/terarepo/src/3rdparty/tinyxml/tinyxml.h
HEADERS += ../new_terafly/src/terarepo/src/3rdparty/tinyxml/tinystr.h
SOURCES += ../new_terafly/src/terarepo/src/3rdparty/tinyxml/tinystr.cpp
SOURCES += ../new_terafly/src/terarepo/src/3rdparty/tinyxml/tinyxmlparser.cpp
SOURCES += ../new_terafly/src/terarepo/src/3rdparty/tinyxml/tinyxmlerror.cpp
SOURCES += ../new_terafly/src/terarepo/src/3rdparty/tinyxml/tinyxml.cpp

# set up common
INCLUDEPATH += ../new_terafly/src/terarepo/src/common
HEADERS += ../new_terafly/src/terarepo/src/common/config.h
HEADERS += ../new_terafly/src/terarepo/src/common/ProgressBar.h
HEADERS += ../new_terafly/src/terarepo/src/common/QProgressSender.h
SOURCES += ../new_terafly/src/terarepo/src/common/config.cpp
SOURCES += ../new_terafly/src/terarepo/src/common/ProgressBar.cpp
SOURCES += ../new_terafly/src/terarepo/src/common/QProgressSender.cpp


#set up new_terafly (control and presentation classes)
RESOURCES += ../new_terafly/icons.qrc
INCLUDEPATH += ../new_terafly/src/control
HEADERS += ../new_terafly/src/control/V3Dsubclasses.h
HEADERS += ../new_terafly/src/control/CAnnotations.h
HEADERS += ../new_terafly/src/control/CConverter.h
HEADERS += ../new_terafly/src/control/CViewer.h
HEADERS += ../new_terafly/src/control/CImport.h
HEADERS += ../new_terafly/src/control/CPlugin.h
HEADERS += ../new_terafly/src/control/CSettings.h
HEADERS += ../new_terafly/src/control/CVolume.h
HEADERS += ../new_terafly/src/control/CImageUtils.h
HEADERS += ../new_terafly/src/control/V3Dsubclasses.h
HEADERS += ../new_terafly/src/control/VirtualPyramid.h
HEADERS += ../new_terafly/src/control/COperation.h
INCLUDEPATH += ../new_terafly/src/presentation
HEADERS += ../new_terafly/src/presentation/PConverter.h
HEADERS += ../new_terafly/src/presentation/PDialogImport.h
HEADERS += ../new_terafly/src/presentation/PDialogProofreading.h
HEADERS += ../new_terafly/src/presentation/PDialogVirtualPyramid.h
HEADERS += ../new_terafly/src/presentation/PMain.h
HEADERS += ../new_terafly/src/presentation/PTabVolumeInfo.h
HEADERS += ../new_terafly/src/presentation/QArrowButton.h
HEADERS += ../new_terafly/src/presentation/QGradientBar.h
HEADERS += ../new_terafly/src/presentation/QHelpBox.h
HEADERS += ../new_terafly/src/presentation/QGLRefSys.h
HEADERS += ../new_terafly/src/presentation/QPixmapToolTip.h
HEADERS += ../new_terafly/src/presentation/QPrefixSuffixLineEdit.h
HEADERS += ../new_terafly/src/presentation/PAbout.h
HEADERS += ../new_terafly/src/presentation/PLog.h
HEADERS += ../new_terafly/src/presentation/PAnoToolBar.h
HEADERS += ../new_terafly/src/control/QUndoMarkerCreate.h
HEADERS += ../new_terafly/src/control/QUndoMarkerDelete.h
HEADERS += ../new_terafly/src/control/QUndoMarkerDeleteROI.h

win32{
HEADERS += ../new_terafly/src/presentation/fileserver.h
SOURCES += ../new_terafly/src/presentation/fileserver.cpp
}

SOURCES += ../new_terafly/src/control/CAnnotations.cpp
SOURCES += ../new_terafly/src/control/CConverter.cpp
SOURCES += ../new_terafly/src/control/CViewer.cpp
SOURCES += ../new_terafly/src/control/CImport.cpp
SOURCES += ../new_terafly/src/control/CPlugin.cpp
SOURCES += ../new_terafly/src/control/CSettings.cpp
SOURCES += ../new_terafly/src/control/CVolume.cpp
SOURCES += ../new_terafly/src/control/CImageUtils.cpp
SOURCES += ../new_terafly/src/control/COperation.cpp
SOURCES += ../new_terafly/src/control/V3Dsubclasses.cpp
SOURCES += ../new_terafly/src/control/VirtualPyramid.cpp
SOURCES += ../new_terafly/src/presentation/PConverter.cpp
SOURCES += ../new_terafly/src/presentation/PDialogImport.cpp
SOURCES += ../new_terafly/src/presentation/PDialogProofreading.cpp
SOURCES += ../new_terafly/src/presentation/PDialogVirtualPyramid.cpp
SOURCES += ../new_terafly/src/presentation/PMain.cpp
SOURCES += ../new_terafly/src/presentation/PTabVolumeInfo.cpp
SOURCES += ../new_terafly/src/presentation/PAbout.cpp
SOURCES += ../new_terafly/src/presentation/PLog.cpp
SOURCES += ../new_terafly/src/presentation/PAnoToolBar.cpp
SOURCES += ../new_terafly/src/presentation/QPixmapToolTip.cpp
SOURCES += ../new_terafly/src/presentation/QArrowButton.cpp
SOURCES += ../new_terafly/src/presentation/QGradientBar.cpp
SOURCES += ../new_terafly/src/presentation/QHelpBox.cpp
SOURCES += ../new_terafly/src/presentation/QGLRefSys.cpp
SOURCES += ../new_terafly/src/presentation/QPrefixSuffixLineEdit.cpp
SOURCES += ../new_terafly/src/control/QUndoMarkerCreate.cpp
SOURCES += ../new_terafly/src/control/QUndoMarkerDelete.cpp
SOURCES += ../new_terafly/src/control/QUndoMarkerDeleteROI.cpp
