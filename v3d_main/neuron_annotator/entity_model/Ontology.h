#ifndef ONTOLOGY_H
#define ONTOLOGY_H

#include <QtCore>

class Entity;

class Ontology
{

private:
    Entity *_root;
    QMap<QKeySequence, qint64> *_keyBindMap;
    QHash<qint64, Entity*> *_termMap;

private:
    void populateTermMap(Entity *entity);

public:
    Ontology(Entity *root, QMap<QKeySequence, qint64> *keyBindMap);
    ~Ontology();
    inline QMap<QKeySequence, qint64> * keyBindMap() const { return _keyBindMap; }
    inline Entity* root() const { return _root; }
    inline QHash<qint64, Entity*> *termMap() const { return _termMap; }
    inline Entity* getTermById(const qint64 & id) const { return _termMap->value(id); }
};

#endif // ONTOLOGY_H
