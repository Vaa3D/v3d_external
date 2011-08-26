#include "JacsUtil.h"
#include <QRegExp>

QString convertPathToMac(QString path) {
    // TODO: extract this into a property
    return path.replace(QRegExp("/groups/scicomp/jacsData/"), "/Volumes/jacsData/");
}
