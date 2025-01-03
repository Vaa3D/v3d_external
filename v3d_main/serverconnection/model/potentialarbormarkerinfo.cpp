#include "potentialarbormarkerinfo.h"

PotentialArborMarkerInfo::PotentialArborMarkerInfo(int id, QString name, QString somaId, QString image, XYZ loc)
{
    this->id = id;
    this->name = name;
    this->somaId = somaId;
    this->image = image;
    this->loc = loc;
    this->isboring = false;
    this->alreadyUpload = false;
}

PotentialArborMarkerInfo::~PotentialArborMarkerInfo()
{

}

QString PotentialArborMarkerInfo::getBrainId()
{
    return image;
}

QString PotentialArborMarkerInfo::getSomaId()
{
    return somaId;
}

int PotentialArborMarkerInfo::getArborId()
{
    return id;
}

XYZ PotentialArborMarkerInfo::getLocation()
{
    return loc;
}

bool PotentialArborMarkerInfo::isBoring()
{
    return isboring;
}

void PotentialArborMarkerInfo::setBoring(bool boring)
{
    this->isboring = boring;
}

bool PotentialArborMarkerInfo::isAlreadyUpdate()
{
    return alreadyUpload;
}

void PotentialArborMarkerInfo::setAlreadyUpdate(bool update)
{
    this->alreadyUpload = update;
}
