#include "potentialsomainfo.h"



PotentialSomaInfo::PotentialSomaInfo(int id, QString brainId, XYZ* location)
{
    this->id = id;
    this->brainId = brainId;
    this->location = location;
}

int PotentialSomaInfo::getId()
{
    return id;
}

QString PotentialSomaInfo::getBrainId()
{
    return brainId;
}

XYZ* PotentialSomaInfo::getLoaction()
{
    return location;
}

bool PotentialSomaInfo::isBoring()
{
    return isboring;
}

void PotentialSomaInfo::setBoring(bool boring)
{
    isboring = boring;
}

bool PotentialSomaInfo::isAlreadyUpload()
{
    return alreadyUpload;
}

void PotentialSomaInfo::setAleadyUpload(bool alreadyUpload)
{
    this->alreadyUpload = alreadyUpload;
}

void PotentialSomaInfo::setCreatedTime()
{
    this->createdTime = createdTime;
}


