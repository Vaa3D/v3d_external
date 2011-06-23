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
 ***
	by Yang Yu
	June 6, 2011

 **
 ****************************************************************************/


//#include "pluginfunchandler.h"
#include "mainwindow.h"

void PLUGINFH::doPluginFunc(V3D_CL_INTERFACE i_v3d, V3d_PluginLoader& mypluginloader, QString v3dpluginFind, void *mainwin)
{
    // prepare 
    V3DPluginArgItem arg;
    V3DPluginArgList pluginfunc_input;
    V3DPluginArgList pluginfunc_output;
    
    QList<Image4DSimple *> imgList;
    
    if(i_v3d.openV3D)
    {
        MainWindow *pMainWin = (MainWindow *)(mainwin);
        
        for(int i=0; i<i_v3d.fileList.size(); i++)
        {
            char *filenameinput = i_v3d.fileList.at(i);
            
            QString fileName(filenameinput);
            
            // load image
            if (!fileName.isEmpty()) 
            {
                // find triview window
                XFormWidget *existing_imgwin = pMainWin->findMdiChild(fileName);	
                
                // handling
                if(!existing_imgwin)
                {
                    pMainWin->loadV3DFile(filenameinput, true, false); // 3d viewer is not opened by default
                    
                    existing_imgwin = pMainWin->findMdiChild(fileName);
                    if(!existing_imgwin) 
                    {
                        // try open image file in currrent dir
                        QString tryFileName = QDir::currentPath() + "/" + fileName;
                        v3d_msg(QString("Try to open the file [%1].").arg(tryFileName), 1);
                        
                        pMainWin->loadV3DFile(tryFileName, true, false);
                        
                        if(!existing_imgwin) 
                        {
                            v3d_msg(QString("The file [%1] does not exist! Try run plugin directly.").arg(fileName), 1);
                            return;	
                        }
                    }
                }
                
                // 
                imgList.push_back( (Image4DSimple*)(pMainWin->getImage(existing_imgwin)) );
                
            }
            else
            {
                v3d_msg(QString("The file [%1] does not exist! Do nothing.").arg(fileName), 1);
                return;	
            }
            
        }
    }
    
    // imageProcess plugin
    if(string(i_v3d.pluginfunc)=="imAdd" || string(i_v3d.pluginfunc)=="imSub" || string(i_v3d.pluginfunc)=="imMul" || string(i_v3d.pluginfunc)=="imDiv")
    {
        if(imgList.size()<2)
        {
            v3d_msg(QString("At least two images needed."), 0);
            return;
        }
        
        arg.type = ""; arg.p = imgList.at(0); pluginfunc_input << arg;
        arg.type = ""; arg.p = imgList.at(1); pluginfunc_input << arg;
        
        
        V3DLONG totalplxs = imgList.at(0)->getTotalBytes();
        unsigned char *p = NULL;
        
        try{
            p = new unsigned char [totalplxs];
        }
        catch(...){
            v3d_msg(QString("Fail to allocate memory."), 0);
            return;
        }
        
        Image4DSimple *pOutImg4D = new Image4DSimple;        
        pOutImg4D->setData(p, imgList.at(0));
        
        arg.type = ""; arg.p = pOutImg4D; pluginfunc_output << arg;
 
    }
    else
    {
        // other plugins
        arg.type = ""; arg.p = (void *)(&imgList); pluginfunc_input << arg;
        arg.type = "cmd"; arg.p = (void *)(&(i_v3d.cmdArgList)); pluginfunc_input << arg;
        
        arg.type =""; arg.p = (void *)(&(i_v3d.outputList)); pluginfunc_output << arg;
        
        //for(int i=0; i<i_v3d.cmdArgList.size(); i++)            qDebug()<<i_v3d.cmdArgList.at(i);
    }    
    
    // run
    bool success = mypluginloader.callPluginFunc(v3dpluginFind, i_v3d.pluginfunc, pluginfunc_input, pluginfunc_output);
    
    //if(i_v3d.hideV3D) ((MainWindow *)(mainwin))->exit();
    
    if(!success)
    {
        v3d_msg(QString("Fail to call plugin function."), 0);
        return;
    }
    
    // show result
    if(i_v3d.openV3D && !i_v3d.hideV3D)
    {
        MainWindow *pMainWin = (MainWindow *)(mainwin);

        if(string(i_v3d.pluginfunc)=="imAdd")
        {
            char *saveFile = "addImage.raw";
            ((Image4DSimple *)(pluginfunc_output.at(0).p))->saveImage(saveFile);
            
            pMainWin->loadV3DFile(saveFile, true, false);
            
        }
        else if(string(i_v3d.pluginfunc)=="imSub")
        {
            char *saveFile = "subImage.raw";
            ((Image4DSimple *)(pluginfunc_output.at(0).p))->saveImage(saveFile);
            
            pMainWin->loadV3DFile(saveFile, true, false);
        
        }
        else if(string(i_v3d.pluginfunc)=="imMul")
        {
            char *saveFile = "mulImage.raw";
            ((Image4DSimple *)(pluginfunc_output.at(0).p))->saveImage(saveFile);
            
            pMainWin->loadV3DFile(saveFile, true, false);
        
        }
        else if(string(i_v3d.pluginfunc)=="imDiv")
        {
            char *saveFile = "divImage.raw";
            ((Image4DSimple *)(pluginfunc_output.at(0).p))->saveImage(saveFile);
            
            pMainWin->loadV3DFile(saveFile, true, false);
        
        }
    }
    
}
