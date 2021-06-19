#by Hanchuan Peng
# 20170809, - a quick solution to fix the dynamic library files locations for Mac. Need finetune this script later
#find ./ -name *dylib | wc


for i in $(find ./ -name *dylib);

do echo $i;
otool -L $i;

install_name_tool -change /usr/local/Trolltech/Qt-4.7.1/lib/QtCore.framework/Versions/4/QtCore /Applications/vaa3d_3400/vaa3d64.app/Contents/Frameworks/QtCore.framework/Versions/4/QtCore $i;
install_name_tool -change /usr/local/Trolltech/Qt-4.7.1/lib/QtGui.framework/Versions/4/QtGui /Applications/vaa3d_3400/vaa3d64.app/Contents/Frameworks/QtGui.framework/Versions/4/QtGui $i;

otool -L $i;

done


