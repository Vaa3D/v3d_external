#include "JacsUtil.h"
#include <QRegExp>
#include "../entity_model/Entity.h"

QString convertPathToMac(QString path)
{
    // TODO: extract this into a property
    return path.replace(QRegExp("/groups/scicomp/jacsData/"), "/Volumes/jacsData/");
}

int getNeuronNumber(const Entity *entity)
{
    if (entity == 0) return -1;
    QString neuronNumStr = entity->getValueByAttributeName("Number");
    if (!neuronNumStr.isEmpty())
    {
        bool ok;
        int neuronNum = neuronNumStr.toInt(&ok);
        if (ok)
        {
            return neuronNum;
        }
   }

   return -1;
}


