#ifndef ICONS_H
#define ICONS_H

#include <QHash>
#include <QIcon>

class Entity;

// This class is a direct port of the Icons.java found in the Console.
class Icons
{

private:
    QHash<QString,QVariant> cache;

private:
    Icons(); // This class is a singleton because it has a local cache

public:
    QVariant getCachedIcon(const QString & filename);
    static QVariant getIcon(const QString & filename);
    static Icons &get(); // Get the singleton
    static QVariant getOntologyIcon(Entity *entity);
    static QVariant getIcon(Entity *entity);

};

#endif // ICONS_H
