#!/bin/bash

#this is a simple MacX Vaa3D installer-program maker, with the assumpition that the binary of Vaa3D and the dependency files exist at specific locations
# by Hanchuan Peng
# 2011-2012

VEXEPATH="../bin/vaa3d64.app/Contents"
QLIBPATH="/usr/local/Trolltech/Qt-4.7.1/lib"

macdeployqt ../bin/vaa3d64.app  # this sentence will do the following tricks basically, but is better to ensure all Qt files are copied


# the following can be revised to add other dependency library files if needed

#if [ -d $VEXEPATH/Frameworks ];
#then
#  echo "The folder exist - do not create new $VEXEPATH/Frameworks folder."
#else
#  mkdir $VEXEPATH/Frameworks
#fi
#
#cp    $QLIBPATH/QtOpenGL.framework/Versions/4/QtOpenGL $VEXEPATH/Frameworks/.
#cp    $QLIBPATH/QtGui.framework/Versions/4/QtGui $VEXEPATH/Frameworks/.
#cp -r $QLIBPATH/QtGui.framework//Versions/4/Resources/qt_menu.nib $VEXEPATH/Resources/.
#cp    $QLIBPATH/QtCore.framework/Versions/4/QtCore $VEXEPATH/Frameworks/.
#cp    $QLIBPATH/QtSvg.framework/Versions/4/QtSvg $VEXEPATH/Frameworks/.
#cp    $QLIBPATH/QtXml.framework/Versions/4/QtXml $VEXEPATH/Frameworks/.
#cp    $QLIBPATH/QtNetwork.framework/Versions/4/QtNetwork $VEXEPATH/Frameworks/.
#
#install_name_tool -change $QLIBPATH/QtCore.framework/Versions/4/QtCore $VEXEPATH/Frameworks/QtCore $VEXEPATH/MacOS/vaa3d64 
#install_name_tool -change $QLIBPATH/QtGui.framework/Versions/4/QtGui $VEXEPATH/Frameworks/QtGui $VEXEPATH/MacOS/vaa3d64 
#install_name_tool -change $QLIBPATH/QtOpenGL.framework/Versions/4/QtOpenGL $VEXEPATH/Frameworks/QtOpenGL $VEXEPATH/MacOS/vaa3d64 
#install_name_tool -change $QLIBPATH/QtSvg.framework/Versions/4/QtSvg $VEXEPATH/Frameworks/QtSvg $VEXEPATH/MacOS/vaa3d64                             
#install_name_tool -change $QLIBPATH/QtXml.framework/Versions/4/QtXml $VEXEPATH/Frameworks/QtXml $VEXEPATH/MacOS/vaa3d64                             
#install_name_tool -change $QLIBPATH/QtNetwork.framework/Versions/4/QtNetwork $VEXEPATH/Frameworks/QtNetwork $VEXEPATH/MacOS/vaa3d64                             

echo "Done! You can run vaa3d binary now with all dependency libraries in its own local folder."

# the following seems not working, by PHC 20120826
#cd ../bin/plugins
#for i in $(find ./ name *dylib);
#  do echo $i;
#  macdeployqt $i;
#done;
#  
#cd ../../installer
