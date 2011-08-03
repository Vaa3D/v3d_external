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

#define EMPTY 1

//
Q_EXPORT_PLUGIN2(imageBlend, ImageBlendPlugin);

// func
int image_blending(V3DPluginCallback2 &callback, QWidget *parent);

// func mutual information for pair images with the same size
template <class Tdata>
double mi_computing(Tdata *pImg1, Tdata *pImg2, V3DLONG szImg, int datatype)
{
    size_t start_t = clock();
    
    // joint histogram
    double **jointHistogram = NULL;
    double *img1Hist=NULL;
    double *img2Hist=NULL;
    
    V3DLONG szHist;
    
    if(datatype==1) // 8-bit UINT8
    {
        szHist = 256;
    }
    else if(datatype==2) // 12-bit UINT16
    {
        szHist = 4096;
    }
    else
    {
        std::cout<<"Datatype is not supported!"<<endl;
        return -1;
    }
    
    V3DLONG denomHist = szHist*szHist;
    
    try
    {
        jointHistogram = new double * [szHist];
        for(int i=0; i<szHist; i++)
        {
            jointHistogram[i] = new double [szHist];
            memset(jointHistogram[i], 0, sizeof(double)*szHist);
        }
        
        img1Hist = new double [szHist];
        img2Hist = new double [szHist];
        
        memset(img1Hist, 0, sizeof(double)*szHist);
        memset(img2Hist, 0, sizeof(double)*szHist);
    }
    catch(...)
    {
        qDebug()<<"Fail to allocate memory for joint histogram!";
        return -1;
    }
    
    //
    for(V3DLONG i=0; i<szImg; i++)
    {
        jointHistogram[ (V3DLONG)pImg1[i] ][ (V3DLONG)pImg2[i] ] ++;  
    }
    
    double jointEntropy=0, img1Entropy=0, img2Entropy=0;
    
    // normalized joint histogram
    for(V3DLONG i=0; i<szHist; i++)
    {
        for(V3DLONG j=0; j<szHist; j++)
        {
            jointHistogram[i][j] /= (double)(denomHist);
            
            double val = jointHistogram[i][j]?jointHistogram[i][j]:1;
            
            jointEntropy += val * log2(val);
        }
    }

    // marginal histogram
    for(V3DLONG i=0; i<szHist; i++)
    {
        for(V3DLONG j=0; j<szHist; j++)
        {
            img1Hist[i] += jointHistogram[i][j];
            img2Hist[i] += jointHistogram[j][i];
        } 
    }
    
    for(V3DLONG i=0; i<szHist; i++)
    {
        double val1 = img1Hist[i]?img1Hist[i]:1;
        double val2 = img2Hist[i]?img2Hist[i]:1;
        
        img1Entropy += val1 * log2(val1);
        img2Entropy += val2 * log2(val2);
    }
    
    // de-alloc
    if(jointHistogram) {
        for(int i=0; i<szHist; i++)
        {
            delete[] jointHistogram[i];
        }     
        delete []jointHistogram; jointHistogram=NULL;
    }
    if(img1Hist) {delete []img1Hist; img1Hist=NULL;}
    if(img2Hist) {delete []img2Hist; img2Hist=NULL;}
    
    qDebug() << "time elapse for computing mutual information ... " <<clock()-start_t;
    
    // MI
    return (jointEntropy - img1Entropy - img2Entropy);    
    
}

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
        image_blending(callback, parent);
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
	return QStringList() << "imageBlend";
}

bool ImageBlendPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & v3d, QWidget * parent)
{
    // to do
    
    return true;
}

// stitching 2 images and display in V3D
int image_blending(V3DPluginCallback2 &callback, QWidget *parent)
{
    
    ImageBlendingDialog dialog(callback, parent, NULL);
	if (dialog.exec()!=QDialog::Accepted)
		return -1;

    QString m_InputFileName1 = dialog.fn_img1;
    QString m_InputFileName2 = dialog.fn_img2;
    
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
    
    //
    V3DLONG pagesz = sz_img1[0]*sz_img1[1]*sz_img1[2];
    
    // find reference : suppose reference color channels similar enough
    V3DLONG ref1=0, ref2=0, nullcolor1 = -1, nullcolor2 = -1;
    bool b_img1existNULL=false, b_img2existNULL=false;
    
    // step 1: find null color channel
    for(V3DLONG c=0; c<sz_img1[3]; c++) // image 1
    {
        V3DLONG offset_c = c*pagesz;
        V3DLONG sumint1 = 0;
        for (V3DLONG k=0; k<sz_img1[2]; k++) 
        {
            V3DLONG offset_k = offset_c + k*sz_img1[0]*sz_img1[1];
            for(V3DLONG j=0; j<sz_img1[1]; j++)
            {
                V3DLONG offset_j = offset_k + j*sz_img1[0];
                for(V3DLONG i=0; i<sz_img1[0]; i++)
                {
                    V3DLONG idx = offset_j + i;
                    
                    if(datatype_img1 == V3D_UINT8)
                    {
                        sumint1 += p1dImg1[idx];
                    }
                    else if(datatype_img1 == V3D_UINT16)
                    {
                        sumint1 += ((unsigned short *)p1dImg1)[idx];
                    }
                    else if(datatype_img1 == V3D_FLOAT32)
                    {
                        sumint1 += ((float *)p1dImg1)[idx];
                    }
                    else
                    {
                    
                    }
                }
            }
        }
        
        qDebug()<<"sum ..."<<sumint1<<c;
        
        if(sumint1<EMPTY)
        {
            b_img1existNULL = true;
            nullcolor1 = c;
        }        
    }
    
    for(V3DLONG c=0; c<sz_img2[3]; c++) // image 2
    {
        V3DLONG offset_c = c*pagesz;
        V3DLONG sumint2 = 0;
        for (V3DLONG k=0; k<sz_img1[2]; k++) 
        {
            V3DLONG offset_k = offset_c + k*sz_img1[0]*sz_img1[1];
            for(V3DLONG j=0; j<sz_img1[1]; j++)
            {
                V3DLONG offset_j = offset_k + j*sz_img1[0];
                for(V3DLONG i=0; i<sz_img1[0]; i++)
                {
                    V3DLONG idx = offset_j + i;
                    
                    if(datatype_img1 == V3D_UINT8)
                    {
                        sumint2 += p1dImg2[idx];
                    }
                    else if(datatype_img1 == V3D_UINT16)
                    {
                        sumint2 += ((unsigned short *)p1dImg2)[idx];
                    }
                    else if(datatype_img1 == V3D_FLOAT32)
                    {
                        sumint2 += ((float *)p1dImg2)[idx];
                    }
                    else
                    {
                        
                    }
                }
            }
        }
        
        qDebug()<<"sum ..."<<sumint2<<c;
        
        if(sumint2<EMPTY)
        {
            b_img2existNULL = true;
            nullcolor2 = c;
        }
    }
    
    // step 2: find ref color channel by compute MI
    double scoreMI = -1e10; // -INF
    for(V3DLONG c1=0; c1<sz_img1[3]; c1++)
    {
        if(b_img1existNULL)
        {
            if(c1==nullcolor1) continue;
        }
        
        for(V3DLONG c2=0; c2<sz_img2[3]; c2++)
        {
            if(b_img2existNULL)
            {
                if(c2==nullcolor2) continue;
            }
            
            if(datatype_img1 == V3D_UINT8)
            {
                unsigned char* pImg1Proxy = p1dImg1 + c1*pagesz;
                unsigned char* pImg2Proxy = p1dImg2 + c2*pagesz;
                
                double valMI = mi_computing<unsigned char>(pImg1Proxy, pImg2Proxy, pagesz, 1);
                
                if(valMI>scoreMI)
                {
                    scoreMI = valMI;
                    
                    ref1 = c1;
                    ref2 = c2;
                }
            }
            else if(datatype_img1 == V3D_UINT16)
            {
                unsigned short* pImg1Proxy = ((unsigned short *)p1dImg1) + c1*pagesz;
                unsigned short* pImg2Proxy = ((unsigned short *)p1dImg2) + c2*pagesz;
                
                double valMI = mi_computing<unsigned short>(pImg1Proxy, pImg2Proxy, pagesz, 2);
                
                qDebug()<<"mi ..."<<valMI<<c1<<c2;
                
                if(valMI>scoreMI)
                {
                    scoreMI = valMI;
                    
                    ref1 = c1;
                    ref2 = c2;
                }
            }
            else if(datatype_img1 == V3D_FLOAT32)
            {
                printf("Currently this program dose not support FLOAT32.\n"); // temporary
                return -1;
            }
            else
            {
                printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                return -1;
            }
            
        }
    }
    
    qDebug()<<"ref ..."<<ref1<<ref2<<"null color ..."<<b_img1existNULL<<nullcolor1<<b_img2existNULL<<nullcolor2;
    
    // image blending	
    // suppose image1 and image2 have a common reference
    // the blended image color dim = image1 color dim + image2 color dim - 1
    V3DLONG colordim = sz_img1[3]+sz_img2[3]-1;
    
    if(b_img1existNULL) colordim--;
    if(b_img2existNULL) colordim--;
    
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
        V3DLONG c1=0, c2=0;
        
		for(V3DLONG c=0; c<colordim-1; c++)
        {
            V3DLONG offset_c = c*pagesz;
            
            V3DLONG offset_c1, offset_c2;
            bool b_img1;
            
            if(c1<sz_img1[3])
            {
                b_img1 = true;
                
                if(b_img1existNULL)
                {
                    if(c1!=nullcolor1)
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
                
                if(c1!=ref1)
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
                
                qDebug()<<"color 1 ..."<<c1<<c;
                
                c1++;
            }
            else
            {
                b_img1 = false;
            }
            
            if(!b_img1)
            {                
                if(b_img2existNULL)
                {
                    if(c2!=nullcolor2)
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
                
                qDebug()<<"color 2 ..."<<c2<<c;
                
                c2++;
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
            data1d[offset + i] = 0.5*p1dImg1[offset1+i] + 0.5*p1dImg2[offset2+i];
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
        V3DLONG c1=0, c2=0;
        
		for(V3DLONG c=0; c<colordim-1; c++)
        {
            V3DLONG offset_c = c*pagesz;
            
            V3DLONG offset_c1, offset_c2;
            bool b_img1;
            
            if(c1<sz_img1[3])
            {
                b_img1 = true;
                
                if(b_img1existNULL)
                {
                    if(c1!=nullcolor1)
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
                
                if(c1!=ref1)
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
                
                qDebug()<<"color 1 ..."<<c1<<c;
                
                c1++;
            }
            else
            {
                b_img1 = false;
            }
            
            if(!b_img1)
            {                
                if(b_img2existNULL)
                {
                    if(c2!=nullcolor2)
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
                
                qDebug()<<"color 2 ..."<<c2<<c;
                
                c2++;
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
                            data1d[idx] = ((unsigned short *)p1dImg1)[offset_j1 + i];
                        }
                        else
                        {
                            data1d[idx] = ((unsigned short *)p1dImg2)[offset_j2 + i];
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
            data1d[offset + i] = 0.5*((unsigned short *)p1dImg1)[offset1+i] + 0.5*((unsigned short *)p1dImg2)[offset2+i];
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
        V3DLONG c1=0, c2=0;
        
		for(V3DLONG c=0; c<colordim-1; c++)
        {
            V3DLONG offset_c = c*pagesz;
            
            V3DLONG offset_c1, offset_c2;
            bool b_img1;
            
            if(c1<sz_img1[3])
            {
                b_img1 = true;
                
                if(b_img1existNULL)
                {
                    if(c1!=nullcolor1)
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
                
                if(c1!=ref1)
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
                
                qDebug()<<"color 1 ..."<<c1<<c;
                
                c1++;
            }
            else
            {
                b_img1 = false;
            }
            
            if(!b_img1)
            {                
                if(b_img2existNULL)
                {
                    if(c2!=nullcolor2)
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
                
                qDebug()<<"color 2 ..."<<c2<<c;
                
                c2++;
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
                            data1d[idx] = ((float *)p1dImg1)[offset_j1 + i];
                        }
                        else
                        {
                            data1d[idx] = ((float *)p1dImg2)[offset_j2 + i];
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
            data1d[offset + i] = 0.5*((float *)p1dImg1)[offset1+i] + 0.5*((float *)p1dImg2)[offset2+i];
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
    
    // de-alloc
    if(p1dImg1) {delete []p1dImg1; p1dImg1=NULL;}
    if(p1dImg2) {delete []p1dImg2; p1dImg2=NULL;}
	
    //
	return 0;
	
}

