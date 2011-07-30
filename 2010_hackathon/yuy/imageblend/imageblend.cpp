/* imageblend.cpp
 * 2011-07-30: the program is created by Yang Yu
 */


#include <QtGui>

#include <cmath>
#include <stdlib.h>
#include <ctime>

#include <sstream>
#include <iostream>

#include "basic_surf_objs.h"
#include "stackutil.h"
#include "volimg_proc.h"
#include "img_definition.h"
#include "basic_landmark.h"

#include "mg_utilities.h"
#include "mg_image_lib.h"

#include "basic_landmark.h"
#include "basic_4dimage.h"

#include "imageblend.h"

//
Q_EXPORT_PLUGIN2(imageBlend, ImageBlendPlugin);

// func
int image_blending(V3DPluginCallback2 &callback, QWidget *parent);

//plugin funcs
const QString title = "Image Blending";
QStringList ImageBlendPlugin::menulist() const
{
    return QStringList() << tr("Image Blend")
						 << tr("About");
}

void ImageBlendPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("Image Blend"))
    {
        if(!image_blending(callback, parent))
        {
            QMessageBox::information(parent, "Version info", QString("Fail to call function!"));
            return;
        }
    }
	else if (menu_name == tr("About"))
	{
		QMessageBox::information(parent, "Version info", QString("ImageBlend Plugin %1 (July 30, 2011) developed by Yang Yu. (Janelia Research Farm Campus, HHMI)").arg(getPluginVersion()));
		return;
	}
}

// function call
QStringList ImageBlendPlugin::funclist() const
{
	return QStringList() << "ImageBlend";
}

bool ImageBlendPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & v3d, QWidget * parent)
{
    // to do
    
    return true;
}

// stitching 2 images and display in V3D
int image_blending(V3DPluginCallback2 &callback, QWidget *parent)
{
    qDebug()<<"open dialog ...";
    
    ImageBlendingDialog dialog(callback, parent, NULL);
	if (dialog.exec()!=QDialog::Accepted)
		return -1;
    
    qDebug()<<"dialog opened ...";

    QString m_InputFileName1 = dialog.fn_img1;
    QString m_InputFileName2 = dialog.fn_img2;
    
    qDebug()<<"got file names ...";
    
    // load images
    V3DLONG *sz_img1 = 0; 
    int datatype_img1 = 0;
    unsigned char* p1dImg1 = 0;
    
    if (loadImage(const_cast<char *>(m_InputFileName1.toStdString().c_str()), p1dImg1, sz_img1, datatype_img1)!=true)
    {
        fprintf (stderr, "Error happens in reading the image1 file [%s]. Exit. \n",m_InputFileName1.toStdString().c_str());
        return -1;
    }
    
    V3DLONG *sz_img2 = 0; 
    int datatype_img2 = 0;
    unsigned char* p1dImg2 = 0;
    
    if (loadImage(const_cast<char *>(m_InputFileName2.toStdString().c_str()), p1dImg2, sz_img2, datatype_img2)!=true)
    {
        fprintf (stderr, "Error happens in reading the image1 file [%s]. Exit. \n",m_InputFileName2.toStdString().c_str());
        return -2;
    }
    
    // check dims datatype
    if(datatype_img1 != datatype_img2)
    {
        std::cout<<"Images are different data types! Do nothing!"<<endl;
        return -3;
    }
    
    if(sz_img1[0] != sz_img2[0] || sz_img1[1] != sz_img2[1] || sz_img1[2] != sz_img2[2] ) // x, y, z
    {
        std::cout<<"Images are different dimensions! Do nothing!"<<endl;
        return -4;
    }
    
    // find reference : suppose reference color channel with most information
    V3DLONG sum1 = 0, sum2 = 0;
    V3DLONG ref1, ref2;
    
    for(V3DLONG c=0; c<sz_img1[3]; c++)
    {
        V3DLONG offset_c = c*sz_img1[0]*sz_img1[1]*sz_img1[2];
        V3DLONG sumint1 = 0, sumint2 = 0;
        for (V3DLONG k=0; k<sz_img1[2]; k++) 
        {
            V3DLONG offset_k = offset_c + k*sz_img1[0]*sz_img1[1];
            for(V3DLONG j=0; j<sz_img1[1]; j++)
            {
                V3DLONG offset_j = offset_k + j*sz_img1[0];
                for(V3DLONG i=0; i<sz_img1[0]; i++)
                {
                    V3DLONG idx = offset_j + i;
                    
                    sumint1 += p1dImg1[idx];
                    sumint2 += p1dImg2[idx];
                }
            }
        }
        
        if(sumint1>sum1)
        {
            sum1 = sumint1;
            ref1 = c;
        }
        
        if(sumint2>sum2)
        {
            sum2 = sumint2;
            ref2 = c;
        }
    }
    
    // image blending	
    // suppose image1 and image2 have a common reference
    // the blended image color dim = image1 color dim + image2 color dim - 1
    V3DLONG pagesz = sz_img1[0]*sz_img1[1]*sz_img1[2];
    V3DLONG colordim = sz_img1[3]+sz_img2[3]-1;
    V3DLONG totalplxs = colordim * pagesz;
    
	if(datatype_img1 == V3D_UINT8)
	{
		//
		unsigned char* data1d = NULL;
		try
		{
			data1d = new unsigned char [totalplxs];
			
			memset(data1d, 0, sizeof(unsigned char)*totalplxs);
		}
		catch(...)
		{
			printf("Fail to allocate memory.\n");
			return -1;
		}
		
		//
		for(V3DLONG c=0; c<colordim-1; c++)
        {
            V3DLONG offset_c = c*pagesz;
            
            V3DLONG offset_c1, offset_c2;
            bool b_img1;
            if(c<sz_img1[3])
            {
                b_img1 = true;
                
                V3DLONG c1 = c;
                
                if(c!=ref1)
                {
                    offset_c1 = c1*pagesz;
                }
                else
                {
                    c1++;
                    
                    if(c1<sz_img1[3])
                        offset_c1 = c1*pagesz;
                    else
                        b_img1 = false;
                }
                 
            }
            else
            {
                b_img1 = false;
                
                V3DLONG c2 = c - sz_img1[3] - 1;
                
                if(c2!=ref2)
                {
                    offset_c2 = c2*pagesz;
                }
                else
                {
                    c2++;
                    
                    if(c2<sz_img2[3])
                        offset_c2 = c2*pagesz;
                    else
                        continue;
                }
            }
            
            for (V3DLONG k=0; k<sz_img1[2]; k++) 
            {
                V3DLONG offset_k = offset_c + k*sz_img1[0]*sz_img1[1];
                
                V3DLONG offset_k1 = offset_k - offset_c + offset_c1;
                V3DLONG offset_k2 = offset_k - offset_c + offset_c2;
                
                for(V3DLONG j=0; j<sz_img1[1]; j++)
                {
                    V3DLONG offset_j = offset_k + j*sz_img1[0];
                    
                    V3DLONG offset_j1 = offset_k1 + j*sz_img1[0];
                    V3DLONG offset_j2 = offset_k2 + j*sz_img1[0];
                    
                    for(V3DLONG i=0; i<sz_img1[0]; i++)
                    {
                        V3DLONG idx = offset_j + i;
                        
                        if (b_img1) 
                        {
                            data1d[idx] = p1dImg1[offset_j1 + i];
                        }
                        else
                        {
                            data1d[idx] = p1dImg2[offset_j2 + i];
                        }
                    }
                }
            }
        }
        
        V3DLONG offset = (colordim-1)*pagesz;
        V3DLONG offset1 = ref1*pagesz;
        V3DLONG offset2 = ref2*pagesz;
        
        for(V3DLONG i=0; i<pagesz; i++)
        {
            data1d[offset + i] = p1dImg1[offset1+i] + p1dImg2[offset2+i];
        }
		
		//display
		Image4DSimple p4DImage;
		p4DImage.setData((unsigned char*)data1d, sz_img1[0], sz_img1[1], sz_img1[2], colordim, V3D_UINT8); //
		
		v3dhandle newwin = callback.newImageWindow();
		callback.setImage(newwin, &p4DImage);
		callback.setImageName(newwin, "Blended Image");
		callback.updateImageWindow(newwin);
		
	}
	else if(datatype_img1 == V3D_UINT16)
	{
		//
		unsigned short* data1d = NULL;
		try
		{
			data1d = new unsigned short [totalplxs];
			
			memset(data1d, 0, sizeof(unsigned short)*totalplxs);
		}
		catch(...)
		{
			printf("Fail to allocate memory.\n");
			return -1;
		}
		
		//
		for(V3DLONG c=0; c<colordim-1; c++)
        {
            V3DLONG offset_c = c*pagesz;
            
            V3DLONG offset_c1, offset_c2;
            bool b_img1;
            if(c<sz_img1[3])
            {
                b_img1 = true;
                
                V3DLONG c1 = c;
                
                if(c!=ref1)
                {
                    offset_c1 = c1*pagesz;
                }
                else
                {
                    c1++;
                    
                    if(c1<sz_img1[3])
                        offset_c1 = c1*pagesz;
                    else
                        b_img1 = false;
                }
                
            }
            else
            {
                b_img1 = false;
                
                V3DLONG c2 = c - sz_img1[3] - 1;
                
                if(c2!=ref2)
                {
                    offset_c2 = c2*pagesz;
                }
                else
                {
                    c2++;
                    
                    if(c2<sz_img2[3])
                        offset_c2 = c2*pagesz;
                    else
                        continue;
                }
            }
            
            for (V3DLONG k=0; k<sz_img1[2]; k++) 
            {
                V3DLONG offset_k = offset_c + k*sz_img1[0]*sz_img1[1];
                
                V3DLONG offset_k1 = offset_k - offset_c + offset_c1;
                V3DLONG offset_k2 = offset_k - offset_c + offset_c2;
                
                for(V3DLONG j=0; j<sz_img1[1]; j++)
                {
                    V3DLONG offset_j = offset_k + j*sz_img1[0];
                    
                    V3DLONG offset_j1 = offset_k1 + j*sz_img1[0];
                    V3DLONG offset_j2 = offset_k2 + j*sz_img1[0];
                    
                    for(V3DLONG i=0; i<sz_img1[0]; i++)
                    {
                        V3DLONG idx = offset_j + i;
                        
                        if (b_img1) 
                        {
                            data1d[idx] = p1dImg1[offset_j1 + i];
                        }
                        else
                        {
                            data1d[idx] = p1dImg2[offset_j2 + i];
                        }
                    }
                }
            }
        }
        
        V3DLONG offset = (colordim-1)*pagesz;
        V3DLONG offset1 = ref1*pagesz;
        V3DLONG offset2 = ref2*pagesz;
        
        for(V3DLONG i=0; i<pagesz; i++)
        {
            data1d[offset + i] = p1dImg1[offset1+i] + p1dImg2[offset2+i];
        }
		
		//display
		Image4DSimple p4DImage;
		p4DImage.setData((unsigned char*)data1d, sz_img1[0], sz_img1[1], sz_img1[2], colordim, V3D_UINT16); //
		
		v3dhandle newwin = callback.newImageWindow();
		callback.setImage(newwin, &p4DImage);
		callback.setImageName(newwin, "Blended Image");
		callback.updateImageWindow(newwin);
	}
	else if(datatype_img1 == V3D_FLOAT32)
	{
		//
		float* data1d = NULL;
		try
		{
			data1d = new float [totalplxs];
			
			memset(data1d, 0, sizeof(float)*totalplxs);
		}
		catch(...)
		{
			printf("Fail to allocate memory.\n");
			return -1;
		}
		
		//
		for(V3DLONG c=0; c<colordim-1; c++)
        {
            V3DLONG offset_c = c*pagesz;
            
            V3DLONG offset_c1, offset_c2;
            bool b_img1;
            if(c<sz_img1[3])
            {
                b_img1 = true;
                
                V3DLONG c1 = c;
                
                if(c!=ref1)
                {
                    offset_c1 = c1*pagesz;
                }
                else
                {
                    c1++;
                    
                    if(c1<sz_img1[3])
                        offset_c1 = c1*pagesz;
                    else
                        b_img1 = false;
                }
                
            }
            else
            {
                b_img1 = false;
                
                V3DLONG c2 = c - sz_img1[3] - 1;
                
                if(c2!=ref2)
                {
                    offset_c2 = c2*pagesz;
                }
                else
                {
                    c2++;
                    
                    if(c2<sz_img2[3])
                        offset_c2 = c2*pagesz;
                    else
                        continue;
                }
            }
            
            for (V3DLONG k=0; k<sz_img1[2]; k++) 
            {
                V3DLONG offset_k = offset_c + k*sz_img1[0]*sz_img1[1];
                
                V3DLONG offset_k1 = offset_k - offset_c + offset_c1;
                V3DLONG offset_k2 = offset_k - offset_c + offset_c2;
                
                for(V3DLONG j=0; j<sz_img1[1]; j++)
                {
                    V3DLONG offset_j = offset_k + j*sz_img1[0];
                    
                    V3DLONG offset_j1 = offset_k1 + j*sz_img1[0];
                    V3DLONG offset_j2 = offset_k2 + j*sz_img1[0];
                    
                    for(V3DLONG i=0; i<sz_img1[0]; i++)
                    {
                        V3DLONG idx = offset_j + i;
                        
                        if (b_img1) 
                        {
                            data1d[idx] = p1dImg1[offset_j1 + i];
                        }
                        else
                        {
                            data1d[idx] = p1dImg2[offset_j2 + i];
                        }
                    }
                }
            }
        }
        
        V3DLONG offset = (colordim-1)*pagesz;
        V3DLONG offset1 = ref1*pagesz;
        V3DLONG offset2 = ref2*pagesz;
        
        for(V3DLONG i=0; i<pagesz; i++)
        {
            data1d[offset + i] = p1dImg1[offset1+i] + p1dImg2[offset2+i];
        }
		
		//display
		Image4DSimple p4DImage;
		p4DImage.setData((unsigned char*)data1d, sz_img1[0], sz_img1[1], sz_img1[2], colordim, V3D_FLOAT32); //
		
		v3dhandle newwin = callback.newImageWindow();
		callback.setImage(newwin, &p4DImage);
		callback.setImageName(newwin, "Blended Image");
		callback.updateImageWindow(newwin);
	}
	else 
	{
		printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
		return -1;
	}
	
	return 0;
	
}

