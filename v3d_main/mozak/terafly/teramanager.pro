# 170624: by RZC. only inclued mozak/terafly/src/control&presentation, replace linking mozak/terafly/src/core by submodule in terafly/src/terarepo

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
V3DMAINPATH =  ../..
QT_PATH = $$dirname(QMAKE_QMAKE)/..

#HDF5 headers and precompiled library and dependencies (libz and libszip)
INCLUDEPATH += $$V3DMAINPATH/common_lib/include/hdf5

macx {
  LIBS += -L$$V3DMAINPATH/common_lib/lib_mac64 -lhdf5 -lszip -lz
  }

unix {
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

#setup iomanager
INCLUDEPATH += ../terafly/src/core/iomanager
HEADERS += ../terafly/src/core/iomanager/iomanager.config.h
HEADERS += ../terafly/src/core/iomanager/ioplugins.h
HEADERS += ../terafly/src/core/iomanager/IOPluginAPI.h
HEADERS += ../terafly/src/core/iomanager/ProgressBar.h
HEADERS += ../terafly/src/core/iomanager/plugins/exampleplugin2D/exampleplugin2D.h
#HEADERS += ../terafly/src/core/iomanager/plugins/opencv2D/opencv2D.h
HEADERS += ../terafly/src/core/iomanager/plugins/tiff2D/tiff2D.h
HEADERS += ../terafly/src/core/iomanager/plugins/tiff3D/tiff3D.h
SOURCES += ../terafly/src/core/iomanager/iomanager.config.cpp
SOURCES += ../terafly/src/core/iomanager/ProgressBar.cpp
SOURCES += ../terafly/src/core/iomanager/plugins/exampleplugin2D/exampleplugin2D.cpp
#SOURCES += ../terafly/src/core/iomanager/plugins/opencv2D/opencv2D.cpp
SOURCES += ../terafly/src/core/iomanager/plugins/tiff2D/tiff2D.cpp
SOURCES += ../terafly/src/core/iomanager/plugins/tiff3D/tiff3D.cpp

#setup imagemanager
INCLUDEPATH += ../terafly/src/core/imagemanager
HEADERS += ../terafly/src/core/imagemanager/BDVVolume.h
HEADERS += ../terafly/src/core/imagemanager/HDF5Mngr.h
HEADERS += ../terafly/src/core/imagemanager/imBlock.h
HEADERS += ../terafly/src/core/imagemanager/dirent_win.h
HEADERS += ../terafly/src/core/imagemanager/IM_config.h
HEADERS += ../terafly/src/core/imagemanager/ProgressBar.h
HEADERS += ../terafly/src/core/imagemanager/RawFmtMngr.h
HEADERS += ../terafly/src/core/imagemanager/RawVolume.h
HEADERS += ../terafly/src/core/imagemanager/SimpleVolume.h
HEADERS += ../terafly/src/core/imagemanager/SimpleVolumeRaw.h
HEADERS += ../terafly/src/core/imagemanager/Stack.h
HEADERS += ../terafly/src/core/imagemanager/StackRaw.h
HEADERS += ../terafly/src/core/imagemanager/StackedVolume.h
HEADERS += ../terafly/src/core/imagemanager/Tiff3DMngr.h
HEADERS += ../terafly/src/core/imagemanager/TiledMCVolume.h
HEADERS += ../terafly/src/core/imagemanager/TiledVolume.h
HEADERS += ../terafly/src/core/imagemanager/TimeSeries.h
HEADERS += ../terafly/src/core/imagemanager/VirtualFmtMngr.h
HEADERS += ../terafly/src/core/imagemanager/VirtualVolume.h
HEADERS += ../terafly/src/core/imagemanager/UnstitchedVolume.h
SOURCES += ../terafly/src/core/imagemanager/BDVVolume.cpp
SOURCES += ../terafly/src/core/imagemanager/HDF5Mngr.cpp
SOURCES += ../terafly/src/core/imagemanager/imBlock.cpp
SOURCES += ../terafly/src/core/imagemanager/IM_config.cpp
SOURCES += ../terafly/src/core/imagemanager/imProgressBar.cpp
SOURCES += ../terafly/src/core/imagemanager/RawFmtMngr.cpp
SOURCES += ../terafly/src/core/imagemanager/RawVolume.cpp
SOURCES += ../terafly/src/core/imagemanager/SimpleVolume.cpp
SOURCES += ../terafly/src/core/imagemanager/SimpleVolumeRaw.cpp
SOURCES += ../terafly/src/core/imagemanager/Stack.cpp
SOURCES += ../terafly/src/core/imagemanager/StackRaw.cpp
SOURCES += ../terafly/src/core/imagemanager/StackedVolume.cpp
SOURCES += ../terafly/src/core/imagemanager/Tiff3DMngr.cpp
SOURCES += ../terafly/src/core/imagemanager/TiledMCVolume.cpp
SOURCES += ../terafly/src/core/imagemanager/TiledVolume.cpp
SOURCES += ../terafly/src/core/imagemanager/TimeSeries.cpp
SOURCES += ../terafly/src/core/imagemanager/VirtualFmtMngr.cpp
SOURCES += ../terafly/src/core/imagemanager/VirtualVolume.cpp
SOURCES += ../terafly/src/core/imagemanager/UnstitchedVolume.cpp


# set up stitcher
INCLUDEPATH += ../terafly/src/core/stitcher
HEADERS += ../terafly/src/core/stitcher/S_config.h
HEADERS += ../terafly/src/core/stitcher/Displacement.h
HEADERS += ../terafly/src/core/stitcher/DisplacementMIPNCC.h
HEADERS += ../terafly/src/core/stitcher/PDAlgo.h
HEADERS += ../terafly/src/core/stitcher/PDAlgoMIPNCC.h
HEADERS += ../terafly/src/core/stitcher/StackRestorer.h
HEADERS += ../terafly/src/core/stitcher/StackStitcher.h
HEADERS += ../terafly/src/core/stitcher/TPAlgo.h
HEADERS += ../terafly/src/core/stitcher/TPAlgoMST.h
HEADERS += ../terafly/src/core/stitcher/resumer.h
SOURCES += ../terafly/src/core/stitcher/Displacement.cpp
SOURCES += ../terafly/src/core/stitcher/DisplacementMIPNCC.cpp
SOURCES += ../terafly/src/core/stitcher/MergeTiles.cpp
SOURCES += ../terafly/src/core/stitcher/PDAlgo.cpp
SOURCES += ../terafly/src/core/stitcher/PDAlgoMIPNCC.cpp
SOURCES += ../terafly/src/core/stitcher/StackRestorer.cpp
SOURCES += ../terafly/src/core/stitcher/StackStitcher.cpp
SOURCES += ../terafly/src/core/stitcher/TPAlgo.cpp
SOURCES += ../terafly/src/core/stitcher/TPAlgoMST.cpp
SOURCES += ../terafly/src/core/stitcher/resumer.cpp

# set up volumeconverter
INCLUDEPATH += ../terafly/src/core/volumeconverter
HEADERS += ../terafly/src/core/volumeconverter/S_config.h
HEADERS += ../terafly/src/core/volumeconverter/VolumeConverter.h
SOURCES += ../terafly/src/core/volumeconverter/VolumeConverter.cpp

# set up volumemanager
INCLUDEPATH += ../terafly/src/core/volumemanager
HEADERS += ../terafly/src/core/volumemanager/vmBlock.h
HEADERS += ../terafly/src/core/volumemanager/vmBlockVolume.h
HEADERS += ../terafly/src/core/volumemanager/vmStack.h
HEADERS += ../terafly/src/core/volumemanager/vmStackedVolume.h
HEADERS += ../terafly/src/core/volumemanager/vmVirtualStack.h
HEADERS += ../terafly/src/core/volumemanager/vmVirtualVolume.h
HEADERS += ../terafly/src/core/volumemanager/volumemanager.config.h
SOURCES += ../terafly/src/core/volumemanager/vmBlock.cpp
SOURCES += ../terafly/src/core/volumemanager/vmBlockVolume.cpp
SOURCES += ../terafly/src/core/volumemanager/vmStack.cpp
SOURCES += ../terafly/src/core/volumemanager/vmStackedVolume.cpp
SOURCES += ../terafly/src/core/volumemanager/vmVirtualStack.cpp
SOURCES += ../terafly/src/core/volumemanager/vmVirtualVolume.cpp
SOURCES += ../terafly/src/core/volumemanager/volumemanager.config.cpp

# set up crossmips
INCLUDEPATH += ../terafly/src/core/crossmips
HEADERS += ../terafly/src/core/crossmips/my_defs.h
HEADERS += ../terafly/src/core/crossmips/CrossMIPs.h
HEADERS += ../terafly/src/core/crossmips/compute_funcs.h
SOURCES += ../terafly/src/core/crossmips/compute_funcs.cpp
SOURCES += ../terafly/src/core/crossmips/libcrossmips.cpp

# set up tinyxml
INCLUDEPATH += ../terafly/src/core/tinyxml
HEADERS += ../terafly/src/core/tinyxml/tinyxml.h
HEADERS += ../terafly/src/core/tinyxml/tinystr.h
SOURCES += ../terafly/src/core/tinyxml/tinystr.cpp
SOURCES += ../terafly/src/core/tinyxml/tinyxmlparser.cpp
SOURCES += ../terafly/src/core/tinyxml/tinyxmlerror.cpp
SOURCES += ../terafly/src/core/tinyxml/tinyxml.cpp


#set up TeraFly plugin (control and presentation classes)
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
HEADERS += ../terafly/src/control/COperation.h
INCLUDEPATH += ../terafly/src/presentation
HEADERS += ../terafly/src/presentation/PConverter.h
HEADERS += ../terafly/src/presentation/PDialogImport.h
HEADERS += ../terafly/src/presentation/PDialogProofreading.h
HEADERS += ../terafly/src/presentation/PMain.h
HEADERS += ../terafly/src/presentation/QArrowButton.h
HEADERS += ../terafly/src/presentation/QGradientBar.h
HEADERS += ../terafly/src/presentation/QHelpBox.h
HEADERS += ../terafly/src/presentation/QGLRefSys.h
HEADERS += ../terafly/src/presentation/QPixmapToolTip.h
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
SOURCES += ../terafly/src/presentation/PConverter.cpp
SOURCES += ../terafly/src/presentation/PDialogImport.cpp
SOURCES += ../terafly/src/presentation/PDialogProofreading.cpp
SOURCES += ../terafly/src/presentation/PMain.cpp
SOURCES += ../terafly/src/presentation/PAbout.cpp
SOURCES += ../terafly/src/presentation/PLog.cpp
SOURCES += ../terafly/src/presentation/PAnoToolBar.cpp
SOURCES += ../terafly/src/presentation/QPixmapToolTip.cpp
SOURCES += ../terafly/src/presentation/QArrowButton.cpp
SOURCES += ../terafly/src/presentation/QGradientBar.cpp
SOURCES += ../terafly/src/presentation/QHelpBox.cpp
SOURCES += ../terafly/src/presentation/QGLRefSys.cpp
SOURCES += ../terafly/src/control/QUndoMarkerCreate.cpp
SOURCES += ../terafly/src/control/QUndoMarkerDelete.cpp
SOURCES += ../terafly/src/control/QUndoMarkerDeleteROI.cpp
