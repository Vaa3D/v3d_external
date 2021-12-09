/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it.

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




/****************************************************************************
**
V3D main program

By Hanchuan Peng

July 21, 2006
060924: v3d v1.2

Last update: 2008-04-25: try to add command line based utilities
Last update: 2010-04-12: add a global try-catch to catch all exceptions
Last update: 2010-11-19: change some of the help info
Last update: 2011-04-19: fix some potential problem of null mainWin pointer
Last update: 2011-08-25: remove some uncalled old code, and adjust the inconsistent return values of the main function

****************************************************************************/

#define COMPILE_TO_COMMANDLINE 1

#include "../3drenderer/v3dr_common.h"

#include "v3d_compile_constraints.h"

#include <QApplication>
#include <QFile>

#include <iostream>
#include <vector>

#include "mainwindow.h"
#include "v3d_application.h"
#include "customdebug.h"

#include <string>
using namespace std;

#include "v3d_core.h"

#include "v3d_commandlineparser.h"
#include "v3d_version_info.h"
#include "../plugin_loader/v3d_plugin_loader.h"
#include "terafly/src/control/CPlugin.h"
V3dApplication* V3dApplication::theApp = 0;

int main(int argc, char **argv)
{
//    char *infile=argv[1];
//    char *outfile=argv[2];
//    int xc,yc,zc,len;
//    xc=atoi(argv[3]);
//    yc=atoi(argv[4]);
//    zc=atoi(argv[5]);
//    len=atoi(argv[6]);

    char *infile="/Users/huanglei/Desktop/dataserver/image/18454/RES\(26298x35000x11041\)";
    char *outfile="/Users/huanglei/Desktop/2.v3dpbd";
    int xc,yc,zc,len;
    xc=14530;
    yc=10693;
    zc=3124;
    len=128;

    std::cout<<infile<<"\n"
            <<outfile<<"\n"
           <<xc<<"\n"<<yc<<"\n"<<zc<<"\n"<<len<<"\n";
    auto sz = new V3DLONG [5];
    if (!sz)
    {
        return -1;
    }
    sz[0] = terafly::PluginInterface::getXDim(infile);
    sz[1] = terafly::PluginInterface::getYDim(infile);
    sz[2] = terafly::PluginInterface::getZDim(infile);
    sz[3] = terafly::PluginInterface::getCDim(infile);
    sz[4] = terafly::PluginInterface::getTDim(infile);
    V3DLONG in_sz[4]={len,len,len,sz[3]};

    V3DLONG x0 = xc-len/2;
    V3DLONG x1 = xc+len/2;
    V3DLONG y0 = yc-len/2;
    V3DLONG y1 = yc+len/2;
    V3DLONG z0 = zc-len/2;
    V3DLONG z1 = zc+len/2;

    if(x0<0 || x1>=sz[0] || y0<0 || y1>=sz[1] || z0<0 || z1>=sz[2]){
        qDebug()<<"the cordinate is out of size";
        return -2;
    }
    unsigned char * cropped_image=terafly::PluginInterface::getSubVolume(infile,x0,x1,y0,y1,z0,z1);

    if(cropped_image==NULL){
        return -3;
    }

    Image4DSimple * outimg = new Image4DSimple;
    if (!outimg)
        return -4;;
    outimg->setData(cropped_image, in_sz[0], in_sz[1], in_sz[2], in_sz[3], V3D_UINT8);
    outimg->saveImage(outfile);
    return 0;
}
