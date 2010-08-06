/* movieZCswitch.cpp
 * 2009-09-22: create this program by Yang Yu
 * 2009-09-28. Last edit by Hanchuan Peng. only change the texts in the options
 * 2010-08-06. by hanchuan Peng, to adapt to the patch of the image4dsimple data structure
 */

#include <QtGui>

#include <string>
#include <exception>
#include <iostream>
#include <algorithm>

#include "movieZCswitch.h"


//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(movieZCswitch, MovieZCswitchPlugin)

int changeMS(V3DPluginCallback &callback, QWidget *parent);


const QString title = "5D Stack Converter";
QStringList MovieZCswitchPlugin::menulist() const
{
    return QStringList() << tr("5D Stack Converter")
						 << tr("about this plugin");
}

void MovieZCswitchPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
	if (menu_name == tr("5D Stack Converter"))
    {
    	changeMS(callback, parent);
    }
	else if (menu_name == tr("about this plugin"))
	{
		QMessageBox::information(parent, "Version info", "5D Stack Converter Plugin Demo 1.0 (2009-Sep-22) developed by Yang Yu. (Hanchuan Peng Lab, Janelia Research Farm Campus, HHMI)");
	}
	
}

int changeMS(V3DPluginCallback &callback, QWidget *parent)
{

	v3dhandle oldwin = callback.currentImageWindow();
	Image4DSimple* image = callback.getImage(oldwin);
	if (! image)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return -1;
	}
	if (image->getDatatype()!=V3D_UINT8)
	{
		QMessageBox::information(0, title, QObject::tr("This program only supports 8-bit data. Your current image data type is not supported."));
		return -2;
	}		
	
	V3DLONG N = image->getTotalBytes();
	unsigned char* data1d = new unsigned char[N];
	
	Image4DSimple p4DImage;
	
	unsigned char* image1d = image->getRawData();
	V3DLONG sx=image->getXDim(), sy=image->getYDim(), sz=image->getZDim(), sc=image->getCDim();
	
	V3DLONG ts_max = (sz>sc)?sz:sc;
	
	//get Z->C or C->Z command
	bool ok1, ok2;
	int timepoints=0;

	QStringList items;
	items << QObject::tr("4D {XYZ,Color} Stack --> 5D {XYZ,Color,Time} Stack") << QObject::tr("5D {XYZ,Color,Time} Stack --> 4D {XYZ,Color} Stack");

	QString item = QInputDialog::getItem(parent, QObject::tr("Change Movie Stack"),
										QObject::tr("Which direction do you want to change:"), items, 0, false, &ok1);
	
	
	if(ok1)
	{
		timepoints = QInputDialog::getInteger(parent, QObject::tr("Set time points"),
									  QObject::tr("Enter the number of time points:"),
									  1, 1, ts_max, 1, &ok2);
	}
	else
		return -1;
	

	
	//Get the old image info

	if(!(QString::compare(item, "4D {XYZ,Color} Stack --> 5D {XYZ,Color,Time} Stack")))
	{
		p4DImage.setTimePackType(TIME_PACK_C);

//		V3DLONG imagecount = image->getTDim();
		
		V3DLONG pagesz=sx*sy;
		V3DLONG channelsz=sx*sy*sz;
		
		p4DImage.setTDim(timepoints);
		V3DLONG imagecount = timepoints;
		
		sz /= imagecount;
		
		for(V3DLONG no=0; no<imagecount; no++)
		{
			for(V3DLONG c=0; c<sc; c++)
			{
				for(V3DLONG k =0; k<sz; k++)
				{
					V3DLONG offsetc = k*pagesz + c*pagesz*sz + no*pagesz*sz*sc;
					V3DLONG offsets = k*pagesz + c*channelsz + no*pagesz*sz;
					for(V3DLONG i=0; i<pagesz; i++)
					{
						data1d[i+offsetc] = image1d[i + offsets];
					}
				}
			}
		}
		
		sc = sc*imagecount;

	}
	else if(!(QString::compare(item, "5D {XYZ,Color,Time} Stack --> 4D {XYZ,Color} Stack")))
	{
		p4DImage.setTimePackType(TIME_PACK_Z);
		
		V3DLONG pagesz=sx*sy;
//		V3DLONG channelsz=sx*sy*sz;
//		V3DLONG imagecount = image->getTDim();
		
		V3DLONG imagecount = timepoints;

		sc /= imagecount;

		for(V3DLONG no=0; no<imagecount; no++)
		{
			for(V3DLONG c=0; c<sc; c++)
			{
				for(V3DLONG k =0; k<sz; k++)
				{
					V3DLONG offsetc = k*pagesz + c*pagesz*sz + no*pagesz*sz*sc;
					V3DLONG offsets = k*pagesz + c*pagesz*sz*imagecount + no*pagesz*sz;
					for(V3DLONG i=0; i<pagesz; i++)
					{
						data1d[i+offsets] = image1d[i + offsetc];
					}
				}
			}
		}
		
		sz = sz*imagecount;

	}
	else
	{
		QMessageBox::information(0, title, QObject::tr("This program only supports time series data. Your current image data type is not supported."));
		return -3;
	}

	// show in v3d
	p4DImage.setData(data1d, sx,sy,sz,sc, image->getDatatype());
	
	v3dhandle newwin = callback.newImageWindow();
	callback.setImage(newwin, &p4DImage);
	callback.setImageName(newwin,  callback.getImageName(oldwin)+"_changed");
	callback.updateImageWindow(newwin);

	return 0;
}



