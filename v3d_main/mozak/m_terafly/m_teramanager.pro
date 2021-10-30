# 170624: by RZC. only inclued mozak/m_terafly/src/control&presentation, replace linking mozak/m_terafly/src/core by submodule in terafly/src/terarepo


#set up TeraFly plugin (control and presentation classes)

#RESOURCES += ../mozak/m_terafly/icons.qrc

INCLUDEPATH += ../mozak/m_terafly/src/control
INCLUDEPATH += ../mozak/m_terafly/src/presentation

HEADERS += ../mozak/m_terafly/src/control/m_V3Dsubclasses.h
HEADERS += ../mozak/m_terafly/src/control/m_CAnnotations.h
HEADERS += ../mozak/m_terafly/src/control/m_CConverter.h
HEADERS += ../mozak/m_terafly/src/control/m_CViewer.h
HEADERS += ../mozak/m_terafly/src/control/m_CImport.h
HEADERS += ../mozak/m_terafly/src/control/m_CPlugin.h
HEADERS += ../mozak/m_terafly/src/control/m_CSettings.h
HEADERS += ../mozak/m_terafly/src/control/m_CVolume.h
HEADERS += ../mozak/m_terafly/src/control/m_CImageUtils.h
HEADERS += ../mozak/m_terafly/src/control/m_V3Dsubclasses.h
HEADERS += ../mozak/m_terafly/src/control/m_COperation.h
HEADERS += ../mozak/m_terafly/src/control/m_QUndoMarkerCreate.h
HEADERS += ../mozak/m_terafly/src/control/m_QUndoMarkerDelete.h
HEADERS += ../mozak/m_terafly/src/control/m_QUndoMarkerDeleteROI.h

HEADERS += ../mozak/m_terafly/src/presentation/m_PConverter.h
HEADERS += ../mozak/m_terafly/src/presentation/m_PDialogImport.h
HEADERS += ../mozak/m_terafly/src/presentation/m_PDialogProofreading.h
HEADERS += ../mozak/m_terafly/src/presentation/m_PMain.h
HEADERS += ../mozak/m_terafly/src/presentation/m_QArrowButton.h
HEADERS += ../mozak/m_terafly/src/presentation/m_QGradientBar.h
HEADERS += ../mozak/m_terafly/src/presentation/m_QHelpBox.h
HEADERS += ../mozak/m_terafly/src/presentation/m_QGLRefSys.h
HEADERS += ../mozak/m_terafly/src/presentation/m_QPixmapToolTip.h
HEADERS += ../mozak/m_terafly/src/presentation/m_PAbout.h
HEADERS += ../mozak/m_terafly/src/presentation/m_PLog.h
HEADERS += ../mozak/m_terafly/src/presentation/m_PAnoToolBar.h

SOURCES += ../mozak/m_terafly/src/control/m_CAnnotations.cpp
SOURCES += ../mozak/m_terafly/src/control/m_CConverter.cpp
SOURCES += ../mozak/m_terafly/src/control/m_CViewer.cpp
SOURCES += ../mozak/m_terafly/src/control/m_CImport.cpp
SOURCES += ../mozak/m_terafly/src/control/m_CPlugin.cpp
SOURCES += ../mozak/m_terafly/src/control/m_CSettings.cpp
SOURCES += ../mozak/m_terafly/src/control/m_CVolume.cpp
SOURCES += ../mozak/m_terafly/src/control/m_CImageUtils.cpp
SOURCES += ../mozak/m_terafly/src/control/m_COperation.cpp
SOURCES += ../mozak/m_terafly/src/control/m_V3Dsubclasses.cpp
SOURCES += ../mozak/m_terafly/src/control/m_QUndoMarkerCreate.cpp
SOURCES += ../mozak/m_terafly/src/control/m_QUndoMarkerDelete.cpp
SOURCES += ../mozak/m_terafly/src/control/m_QUndoMarkerDeleteROI.cpp

SOURCES += ../mozak/m_terafly/src/presentation/m_PConverter.cpp
SOURCES += ../mozak/m_terafly/src/presentation/m_PDialogImport.cpp
SOURCES += ../mozak/m_terafly/src/presentation/m_PDialogProofreading.cpp
SOURCES += ../mozak/m_terafly/src/presentation/m_PMain.cpp
SOURCES += ../mozak/m_terafly/src/presentation/m_PAbout.cpp
SOURCES += ../mozak/m_terafly/src/presentation/m_PLog.cpp
SOURCES += ../mozak/m_terafly/src/presentation/m_PAnoToolBar.cpp
SOURCES += ../mozak/m_terafly/src/presentation/m_QPixmapToolTip.cpp
SOURCES += ../mozak/m_terafly/src/presentation/m_QArrowButton.cpp
SOURCES += ../mozak/m_terafly/src/presentation/m_QGradientBar.cpp
SOURCES += ../mozak/m_terafly/src/presentation/m_QHelpBox.cpp
SOURCES += ../mozak/m_terafly/src/presentation/m_QGLRefSys.cpp
