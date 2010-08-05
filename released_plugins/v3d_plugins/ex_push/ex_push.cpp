/* ex_push.cpp
 * an example program to test the push function in the plugin interface
 * 2010-08-3: by Hanchuan Peng
 */


#include "ex_push.h"
#include "v3d_message.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(ex_push, ExPushPlugin);

void dopush(V3DPluginCallback &callback, QWidget *parent, int method_code);

//plugin funcs
const QString title = "Example for pushing image and objects";
QStringList ExPushPlugin::menulist() const
{
    return QStringList()
		<< tr("Close and Open 3D viewer and Push Image")
		<< tr("Push Object and Screenshot of global 3d viewer")
		<< tr("Set time points")
		<< tr("About");
}

void ExPushPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
    if (menu_name == tr("Close and Open 3D viewer and Push Image"))
    {
    	dopush(callback, parent, 1);
    }
	else if (menu_name == tr("Push Object and Screenshot of global 3d viewer"))
	{
    	dopush(callback, parent, 2);
	}
	else if (menu_name == tr("Set time points"))
	{
    	dopush(callback, parent, 3);
	}
	else
	{
    	dopush(callback, parent, 0);
	}
}

void dopush(V3DPluginCallback &callback, QWidget *parent, int method_code)
{
	v3dhandle curwin = callback.currentImageWindow();
	if (!curwin)
	{
		v3d_msg("You don't have any image open in the main window.");
		return;
	}
	Image4DSimple *oldimg = callback.getImage(curwin);
	V3DLONG oldsz0=oldimg->getXDim(), oldsz1=oldimg->getYDim(), oldsz2=oldimg->getZDim();

	if (method_code==1) //push image
	{
		//close the current window
		callback.close3DWindow(curwin);

//		//for (int j=1; j<1000; j++) //try to empty all existing events
//		{
//			QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
//		}

		//now should be able to open the new window
		callback.open3DWindow(curwin);

		//now push the data to the 3d viewer's display
		for (int curloop=0; curloop<1; curloop++)
		{
//			Image4DSimple * oldimg = callback.getImage(curwin);
//
			Image4DSimple p4DImage;
			unsigned char * pData = 0;
			V3DLONG sz0=oldsz0*0.9, sz1=oldsz1*.9, sz2=oldsz2*.9, sz3=oldimg->getCDim();
			V3DLONG totallen = sz0*sz1*sz2*sz3;
			pData = new unsigned char [totallen];
			memcpy(pData, oldimg->getRawData(), totallen);

			p4DImage.setData((unsigned char*)pData, sz0, sz1, sz2, sz3, V3D_UINT8);

			callback.setImage(curwin, &p4DImage);
			callback.setImageName(curwin, QString("push now %1").arg(curloop));
			callback.updateImageWindow(curwin);

			callback.pushImageIn3DWindow(curwin);
		}
	}
	else if (method_code==2) //push marker and swc
	{
		//ensure the 3d viewer window is open; if not, then open it
		callback.open3DWindow(curwin);

		//now push the data to the 3d viewer's display
		for (int curloop=0; curloop<100; curloop++)
		{
			LandmarkList curlist;
			for (int i=0;i<20; i++)
			{
				LocationSimple s;
				s.x = rand()%oldsz0;
				s.y = rand()%oldsz1;
				s.z = rand()%oldsz2;
				if (i<10)
					s.radius = 10*i;
				else {
					s.radius=s.radius*0.5;
				}

				curlist << s;
			}

			callback.setLandmark(curwin, curlist);
			callback.setImageName(curwin, QString("push now %1").arg(curloop));

			callback.pushObjectIn3DWindow(curwin);
			callback.updateImageWindow(curwin);
			
			QString BMPfilename = QString("aaa_%1").arg(curloop);
			callback.screenShot3DWindow(curwin, BMPfilename);
		}
	}
}
