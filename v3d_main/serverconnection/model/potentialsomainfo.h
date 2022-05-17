#ifndef POTENTIALSOMAINFO_H
#define POTENTIALSOMAINFO_H

#include <QString>
#include "basic_c_fun/color_xyz.h"


class PotentialSomaInfo
{
public:
    PotentialSomaInfo(int id, QString brainId, XYZ* location);

    int getId();
    QString getBrainId();
    XYZ* getLoaction();
    bool isBoring();
    void setBoring(bool boring);
    bool isAlreadyUpload();
    void setAleadyUpload(bool alreadyUpload);
    void setCreatedTime();
//    bool isStillFreash();

private:
    int id;
    QString brainId;
    XYZ* location;
    bool isboring = false;
    long createdTime;
    bool isFresh = true;
    bool alreadyUpload = false;
    long shelfLife = 7 * 60 * 1000;
};

#endif // POTENTIALSOMAINFO_H
