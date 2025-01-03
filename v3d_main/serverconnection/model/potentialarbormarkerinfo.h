#ifndef POTENTIALARBORMARKERINFO_H
#define POTENTIALARBORMARKERINFO_H

#include <QString>
#include "basic_c_fun/color_xyz.h"


class PotentialArborMarkerInfo
{
public:
    PotentialArborMarkerInfo(int id, QString name, QString somaId, QString image, XYZ loc);
    ~PotentialArborMarkerInfo();

    QString getBrainId();
    QString getSomaId();
    int getArborId();
    XYZ getLocation();
    bool isBoring();
    void setBoring(bool boring);
    bool isAlreadyUpdate();
    void setAlreadyUpdate(bool update);


private:
    int id;
    QString somaId;
    QString image;
    QString name;
    XYZ loc;
    bool isboring;
    bool alreadyUpload;
};

#endif // POTENTIALARBORMARKERINFO_H
