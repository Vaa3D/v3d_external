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
		LandmarkList empty_marklist;
		callback->setLandmark(curwin, empty_marklist);
		qRegisterMetaType<LocationSimple>("LocationSimple");
	}
	~V3dMonitor()
	{
		if(isRunning()) this->terminate();
	}
	void run()
	{
		while(1)
		{
			LandmarkList new_landmarks = callback->getLandmark(curwin);
			if(new_landmarks.size() != 0)
			{
				emit mark_changed(new_landmarks.last());
			}
			LandmarkList empty_marklist;
			callback->setLandmark(curwin, empty_marklist);
			usleep(5000); // sleep 1ms
		}
		exec();

	}
signals:
	void mark_changed(LocationSimple loc);
private:
	V3DPluginCallback2 * callback;
	v3dhandle curwin;
};

#endif
