/*
 *  data_combination.cpp
 *  data_combination
 *
 *  Created by Yang, Jinzhu and Hanchuan Peng, on 11/22/10.
 *
 */

#include "montage_image_sections.h"
#include "v3d_message.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(threshold, ThPlugin);

template <class T> 
void DataCombination(T *apsInput, T * aspOutput, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer, V3DLONG h, V3DLONG d)
{
	V3DLONG i, j,k,n,count;
	double t, temp;
	
	V3DLONG mCount = iImageHeight * iImageWidth;
	V3DLONG mCount1 = iImageWidth*iImageLayer;
	///////////////////////////data combination
	//iImageLayer = 50;
	if (iImageLayer<= iImageWidth)
	{
		for (i=0; i<iImageLayer; i++)
		{
			for (j=0; j<iImageHeight; j++)
			{
				for (k=0; k<iImageWidth; k++)
				{
					aspOutput[j* mCount1 + k  + (i * iImageWidth) ]= apsInput[i * mCount + j*iImageWidth + k];
										
				}
			}
		}			
		
	}
		//////////////						
}

void thimg(V3DPluginCallback &callback, QWidget *parent, int method_code);

//plugin funcs
const QString title = "data combination threshold transform";
QStringList ThPlugin::menulist() const
{
    return QStringList() 
	<< tr("data_combination");
	//<< tr("3D (set parameters)")
	//<< tr("Help");
}

void ThPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
	if (menu_name == tr("data_combination"))
	{
    	thimg(callback, parent,1 );
    }
	else if (menu_name == tr("Help"))
	{
		     
		//QMessageBox::information(parent, "Version info, data combinationa1.0 (2010-11-24): this plugin is developed by Jinzhu Yang");		
		//QMessageBox::information(parent," The adaptive segmentation function, each pixel threshold is statistical, method is to calculated average each piont of three-dimensional  6 neighborhood ");
		//v3d_msg("Fail to allocate memory in Distance Transform./n ,fda ");		
		return;
	}

}

void thimg(V3DPluginCallback &callback, QWidget *parent, int method_code)
{
	v3dhandle curwin = callback.currentImageWindow();
	V3DLONG h;
	V3DLONG d;
	if (!curwin)
	{
		v3d_msg("You don't have any image open in the main window.");
		return;
	}
	int start_t = clock(); // record time point
	
	Image4DSimple* subject = callback.getImage(curwin);
	QString m_InputFileName = callback.getImageName(curwin);
	
	if (!subject)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return;
	}	
	Image4DProxy<Image4DSimple> pSub(subject);
	
	V3DLONG sz0 = subject->getXDim();
    V3DLONG sz1 = subject->getYDim();
    V3DLONG sz2 = subject->getZDim();
	V3DLONG sz3 = subject->getCDim();
	V3DLONG pagesz_sub = sz0*sz1*sz2;
	
	//----------------------------------------------------------------------------------------------------------------------------------
	V3DLONG channelsz = sz0*sz1*sz2;
	void *pData=NULL;
	
	V3DLONG sz_data[4]; sz_data[0]=sz0; sz_data[1]=sz1; sz_data[2]=sz2; sz_data[3]=1;
		switch (subject->getDatatype()) 
		{
			case V3D_UINT8:
				
				try
				{
					pData = (void *)(new unsigned char [sz3*channelsz]); 
				}
					catch (...)
				{
					v3d_msg("Fail to allocate memory in data combination.");
					if (pData) {delete []pData; pData=0;}
					return;
				}
				
				{
					unsigned char * pSubtmp_uint8 = pSub.begin();
				
					for (V3DLONG ich=0; ich<sz3; ich++)
						DataCombination(pSubtmp_uint8+ich*channelsz, (unsigned char *)pData+ich*channelsz, sz0, sz1, sz2, h, d  );
				}
				
				break;
				
			case V3D_UINT16:
				
				try
				{
					pData = (void *)(new short int [sz3*channelsz]); 
				}
					catch (...)
				{
					v3d_msg("Fail to allocate memory in data combination.");
					if (pData) {delete []pData; pData=0;}
					return;
				}
				
				{
					short int * pSubtmp_uint16 = (short int *)pSub.begin();
				
					for (V3DLONG ich=0; ich<sz3; ich++)
						DataCombination(pSubtmp_uint16+ich*channelsz, (short int *)pData+ich*channelsz, sz0, sz1, sz2, h, d );
				}
				
				break;
			case V3D_FLOAT32:
				
				try
				{
					pData = (void *)(new float [sz3*channelsz]); 
				}
					catch (...)
				{
					v3d_msg("Fail to allocate memory in data combination.");
					if (pData) {delete []pData; pData=0;}
					return;
				}
				
				{
					float * pSubtmp_float32 = (float *)pSub.begin();
					
					for (V3DLONG ich=0; ich<sz3; ich++)
						DataCombination(pSubtmp_float32+ich*channelsz, (float *)pData+ich*channelsz, sz0, sz1, sz2, h, d );
				}
				
				break;
			default:
				break;
		}
	
	//----------------------------------------------------------------------------------------------------------------------------------
	
	int end_t = clock();
	printf("time eclapse %d s for dist computing!\n", (end_t-start_t)/1000000);
	
	Image4DSimple p4DImage;
	p4DImage.setData((unsigned char*)pData, sz0*sz2, sz1, 1, sz3, subject->getDatatype());
	
	v3dhandle newwin;
	if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
		newwin = callback.currentImageWindow();
	else
		newwin = callback.newImageWindow();
	
	callback.setImage(newwin, &p4DImage);
	callback.setImageName(newwin, QString("data combination"));
	callback.updateImageWindow(newwin);
}


void AdaTDialog::update()
{
	//get current data
	
	Dn = Dnumber->text().toLong()-1;
	Dh = Ddistance->text().toLong()-1;

		//printf("channel %ld val %d x %ld y %ld z %ld ind %ld \n", c, data1d[ind], nx, ny, nz, ind);
	
}