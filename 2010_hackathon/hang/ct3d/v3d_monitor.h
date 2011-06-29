#ifndef _V3D_MONITOR_H__
#define _V3D_MONITOR_H__
#include <QThread>
#include "v3d_interface.h"

class V3dMonitor : public QThread
{
	Q_OBJECT
public:
	V3dMonitor(V3DPluginCallback2 * callback, v3dhandle curwin)
	{
		this->callback = callback;
		this->curwin = curwin;
		landmarks = callback->getLandmark(curwin);
		qRegisterMetaType<LocationSimple>("LocationSimple");
	}
	void run()
	{
		while(1)
		{
			LandmarkList new_landmarks = callback->getLandmark(curwin);
			if(new_landmarks.size() != landmarks.size())
			{
				qDebug("mark changed !");
				if(new_landmarks.size() > landmarks.size())
				{
					emit mark_changed(new_landmarks.last());
				}
				else emit mark_changed(landmarks.last());
			}
			landmarks = new_landmarks;
			usleep(1000); // sleep 1ms
			//delay(1); //delay 4ms
		}
		exec();

	}
signals:
	void mark_changed(LocationSimple loc);
private:
	V3DPluginCallback2 * callback;
	v3dhandle curwin;
	LandmarkList landmarks;
};

#endif
