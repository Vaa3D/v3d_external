/**
*  100811 RZC: change to handle any type image using  Image4DProxy's value_at/put_at
**/
#include <iostream>
#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "partial_align_plugin.h"
#include "v3d_message.h"

using namespace std;
Q_EXPORT_PLUGIN2(partial_align, PartialAlignPlugin);

const QString title = "Partial Alignment Plugin";
QStringList PartialAlignPlugin::menulist() const
{
    return QStringList()
    << tr("Calc features for target")
	<< tr("Search features in subject")
    << tr("about");
}

void PartialAlignPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &v3d, QWidget *parent)
{
    if (menu_name == tr("Calc features for target"))
    {
		//v3d_msg(QObject::tr("Under developping."));
		v3dhandle handle = v3d.currentImageWindow();
		if(!handle)
		{
			v3d_msg("Need at least one image!");
			return;
		}
		LandmarkList landmark_list = v3d.getLandmark(handle);
		LandmarkList::iterator it = landmark_list.begin();
		while(it != landmark_list.end())
		{
			cout<<(*it).x<<" "<<(*it).y<<" "<<(*it).z<<endl;
			it++;
		}

    }
	else if(menu_name == tr("Search features in subject"))
	{
		v3d_msg(QObject::tr("Under developping."));
	}
    else
    {
    	v3d_msg("Partial Alignment Plugin Demo version 1.0"
    			"\ndeveloped by Hang Xiao 2011. (Janelia Farm Research Campus, HHMI)");
    }
}

