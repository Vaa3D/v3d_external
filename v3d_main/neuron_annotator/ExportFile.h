#ifndef EXPORTFILE_H
#define EXPORTFILE_H

#include <QString>
#include <QThread>
#include <QMutex>
#include <QFileInfo>

#include <iostream>

#include "../v3d/v3d_core.h"
#include "../basic_c_fun/color_xyz.h"

// export XYZC(T) tif file
class ExportFile : public QThread
{
    Q_OBJECT
    
public:
    ExportFile();
    ~ExportFile();
	
public:
    bool init(My4DImage *p4DimgInput, RGBA8 *p3DtexInput, QString filenameInput); // preferred

protected:
    void run();
	
public:
    volatile bool stopped; //

    My4DImage* p4Dimg;
    RGBA8* p3Dtex;
    QString filename;
	
private:
    QMutex mutex;
    
};



#endif // EXPORTFILE_H
