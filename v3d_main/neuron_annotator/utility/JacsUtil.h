#ifndef JACSUTIL_H
#define JACSUTIL_H

#include <QString>
#include <QVariant>

class Entity;

QString convertPathToMac(QString path);

QVariant getIcon(Entity *entity);

#endif // JACSUTIL_H
