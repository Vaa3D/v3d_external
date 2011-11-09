#ifndef EXPORTFILE_H
#define EXPORTFILE_H

#include <QString>
#include <QThread>
#include <QMutex>
#include <QFileInfo>
#include <QList>

#include <iostream>

#include "../v3d/v3d_core.h"
#include "../basic_c_fun/color_xyz.h"

template <class Tinput, class Tmask, class Tref, class Toutput>
Toutput* getCurrentStack(Tinput *input1d, Tmask *mask1d, Tref *ref1d, V3DLONG *szStack, QList<bool> maskStatusList, QList<bool> overlayStatusList, int datatype);

// export XYZC(T) tif file
class ExportFile : public QThread
{
    Q_OBJECT
    
public:
    ExportFile();
    ~ExportFile();
	
public:
    bool init(My4DImage *pOriginalInput, My4DImage *pMaskInput, My4DImage *pRefInput, QList<bool> maskStatusListInput, QList<bool> overlayStatusListInput, QString filenameInput);

protected:
    void run();
	
public:
    volatile bool stopped; //

    My4DImage *pOriginal;
    My4DImage *pMask;
    My4DImage *pRef;
    QList<bool> maskStatusList;
    QList<bool> overlayStatusList;
    QString filename;
	
private:
    QMutex mutex;
    
};



#endif // EXPORTFILE_H
