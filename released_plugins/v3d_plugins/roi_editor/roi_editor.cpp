/*
 *  add_roi.cpp
 *  add_roi
 *
 *  Created by Yang, Jinzhu on 11/24/10.
 *
 */

#include "roi_editor.h"
#include "v3d_message.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(threshold, ThPlugin);

void thimg(V3DPluginCallback &callback, QWidget *parent, int method_code);

//plugin funcs
const QString title = "ROI edit";
QStringList ThPlugin::menulist() const
{
    return QStringList() 
	<< tr("copy from")
        << tr("paste to")
	<< tr("delete ROIs in all tri-view planes")
	<< tr("delete xy-plane ROI")
	<< tr("delete yz-plane ROI")
	<< tr("delete zx-plane ROI")
	<< tr("Help");
}

void ThPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
	if (menu_name == tr("paste"))
	{
    	thimg(callback, parent,1 );
    }
	else if (menu_name == tr("delete all"))
	{
		thimg(callback, parent,2);
	}
	else if (menu_name == tr("delete xy"))
	{
		thimg(callback, parent,3);
	}
	else if (menu_name == tr("delete yz"))
	{
		thimg(callback, parent,4);
	}
	else if(menu_name == tr("delete zx"))
	{
		thimg(callback, parent,5);
		
	}
	else if (menu_name == tr("Help"))
	{
		QMessageBox::information(0, title, QObject::tr("Copy ROI from the current image to the specified image and Delete the specified image ROI, Version info Change Pixel Value 1.0 (2010-11-24): this plugin is developed by Jinzhu Yang"));	                                    		
	   
		//v3d_msg("Fail to allocate memory in Distance Transform./n ,fda ");		
		return;
	}
	
}

void thimg(V3DPluginCallback &callback, QWidget *parent, int method_code)
{
	V3DLONG h;
	V3DLONG d;
	ROIList pRoiList;
	
	v3dhandleList win_list = callback.getImageWindowList();
	if(win_list.size()<1) 
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return;
	}
	
	//printf("&d\n",win_list.size());
	
	v3dhandle curwin = callback.currentImageWindow();
	
	if (method_code == 1)
	{
		
		AdaTDialog dialog(callback, parent);
		if (dialog.exec()!=QDialog::Accepted)
			return;	
		
		long ind=dialog.combo_channel->currentIndex();
		printf("ind=%d\n",ind);
		
		pRoiList=callback.getROI(curwin);
		
		printf("%d %d \n",pRoiList[0].size(),win_list.size());
		
		
		if(callback.setROI(win_list[ind],pRoiList))
		{
			callback.updateImageWindow(win_list[ind]);
			
			//QMessageBox::information(0, title, QObject::tr("paste1."));				
			
			//printf("&d\n",pRoiList[0].size());			
		}		
	}
	else if(method_code == 2)
	{
			AdaTDialog dialog(callback, parent);
			if (dialog.exec()!=QDialog::Accepted)
				return;	
			
		    long ind=dialog.combo_channel->currentIndex();
			
			 printf("ind=%d\n",ind);
			
			pRoiList=callback.getROI(win_list[ind]);
			
			printf("%d %d \n",pRoiList[0].size(),win_list.size());
		   
			//pRoiList=callback.getROI(curwin);
		  
			//pRoiList.clear();
			for(int i=0;i<3;i++)
			{
				pRoiList[i].clear();
			}
			
			//QMessageBox::information(0, title, QObject::tr("delete1."));		
			
			if(callback.setROI(win_list[ind],pRoiList))
			{
				callback.updateImageWindow(win_list[ind]);
				
			//	QMessageBox::information(0, title, QObject::tr("delete2."));				
				
				//printf("&d\n",pRoiList[0].size());			
			}
			
	}
	else if (method_code == 3)	
	{
		AdaTDialog dialog(callback, parent);
		if (dialog.exec()!=QDialog::Accepted)
			return;	
		
		long ind=dialog.combo_channel->currentIndex();
		
		printf("ind=%d\n",ind);
		
		pRoiList=callback.getROI(win_list[ind]);
		
		printf("%d %d \n",pRoiList[0].size(),win_list.size());
		
		pRoiList[0].clear();
		
		
		//QMessageBox::information(0, title, QObject::tr("delete1."));		
		
		if(callback.setROI(win_list[ind],pRoiList))
		{
			callback.updateImageWindow(win_list[ind]);
			
			//	QMessageBox::information(0, title, QObject::tr("delete2."));				
			
			//printf("&d\n",pRoiList[0].size());			
		}
					
	}
	else if (method_code == 4)
	{
		AdaTDialog dialog(callback, parent);
		if (dialog.exec()!=QDialog::Accepted)
			return;	
		
		long ind=dialog.combo_channel->currentIndex();
		
		printf("ind=%d\n",ind);
		
		pRoiList=callback.getROI(win_list[ind]);
		
		printf("%d %d \n",pRoiList[1].size(),win_list.size());
		
		pRoiList[1].clear();
		
		
		//QMessageBox::information(0, title, QObject::tr("delete1."));		
		
		if(callback.setROI(win_list[ind],pRoiList))
		{
			callback.updateImageWindow(win_list[ind]);
			
			//	QMessageBox::information(0, title, QObject::tr("delete2."));				
			
			//printf("&d\n",pRoiList[0].size());			
		}		
			
	}
	else if (method_code == 5)
	{
		AdaTDialog dialog(callback, parent);
		if (dialog.exec()!=QDialog::Accepted)
			return;	
		
		long ind=dialog.combo_channel->currentIndex();
		
		printf("ind=%d\n",ind);
		
		pRoiList=callback.getROI(win_list[ind]);
		
		printf("%d %d \n",pRoiList[2].size(),win_list.size());
		
		pRoiList[2].clear();
		
		//QMessageBox::information(0, title, QObject::tr("delete1."));		
		
		if(callback.setROI(win_list[ind],pRoiList))
		{
			callback.updateImageWindow(win_list[ind]);
			
			//	QMessageBox::information(0, title, QObject::tr("delete2."));				
			
			//printf("&d\n",pRoiList[0].size());			
		}				
	}
	
	int start_t = clock(); // record time point
	int end_t = clock();
	printf("time eclapse %d s for dist computing!\n", (end_t-start_t)/1000000);
}


void AdaTDialog::update()
{
	//get current data
	
	//Dn = Dnumber->text().toLong()-1;
	//Dh = Ddistance->text().toLong()-1;
	
	//printf("channel %ld val %d x %ld y %ld z %ld ind %ld \n", c, data1d[ind], nx, ny, nz, ind);
	
}
