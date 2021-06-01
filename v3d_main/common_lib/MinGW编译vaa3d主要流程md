## MinGW编译vaa3d主要步骤

参考原始教程设置环境变量以及boost，tiff
不同的在于QMAKESPEC=win32-g++
cmake tiff的时候需要选择MinGW Makefile

#### 1 安装QT4

用mingw进行编译，确保生成新的**QtCore4.dll**, **QtGui4.dll**, **QtNetwork4.dll**, **QtOpenGL4.dll**,**QtXml4.dll**. 之后就可以停止编译。过程可能会遇到问题。参考以下博客即可解决

```
https://blog.csdn.net/trouble_makers/article/details/77944438
```

安装成功之后，设置环境变量。**并且将这五个dll移到../v3d_external/v3d_main/common_lib/mingw_dll文件夹中，方便脚本处理**。

qt编译完成后，需要将\mkspecs\win32-g++\qmake.conf修改的**QMAKE_CXXFLAGS**换成原来的。让其支持c++11。

#### 2 下载mingw8.1

```
并且设置环境变量。下载boost，并将里面包含的的boost文件夹移动到../v3d_external/v3d_main/common_lib/include目录下。
```

#### 3 下载vaa3d源码

修改terafly.pro文件里面的lib链接，将winlib64改成mingw即可

修改v3d_essential.pro里面的lib，将winlib64所在447行的LIBS替换成如下形式

```
win32:LIBS += -L../common_lib/mingw -lglew -lhdf5 -lszip -lzlib -lSDL2 -lteem  -lbz2 -lz -lopenvr_api -lwsock32
```

#### 4 修改vaa3d的一些代码

```
4.1 将v3d_external\v3d_main\common_lib\include\hdf5\H5public.h的155行注释掉

4.2 将v3d_external\v3d_main\basic_c_fun\stackutil.h的72行注释掉

4.3 将v3d_external\v3d_main\neuron_annotator\utility\ImageLoaderBasic.cpp的2091，2205，2206，2229，2230，2234的(V3DLONG)改成(long long)

4.4 将v3d_external-master\v3d_main\3drenderer\GLee2glew.c内容注释掉。
```

#### 5 使用脚本编译vaa3d

```
因为可能会出现其他错误，所以用cmd可以看得到错误，根据报错提示进行修改即可。
在\v3d_external\v3d_main下cmd,输入mingw.bat。
```

#### 6 可能出现的问题

```
如果有错误，可能的原因是一些东西没有定义。比如newmat，那么可以在v3d_external\v3d_main\jba\c++下打开cmd，输入mingw32-make，将生成后的libv3dnewmat.a文件放进common_lib\mingw\下 替换原有的.a文件即可。
```

#### 7 编译插件

```
运行v3d_external\released_plugins_more\build_plugindemo.bat，编译插件。
```

