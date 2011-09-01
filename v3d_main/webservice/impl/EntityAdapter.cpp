#include "EntityAdapter.h"
#include <QMap>
#include <QtGui>

EntityAdapter::EntityAdapter() {

    keyNameMap_Java2Qt.insert("ctrl","Meta"); // Qt names these two keys
    keyNameMap_Java2Qt.insert("meta","Ctrl"); // backwards for extra fun.
    keyNameMap_Java2Qt.insert("shift","Shift");
    keyNameMap_Java2Qt.insert("alt","Alt");

    keyNameMap_Java2Qt.insert("CAPS_LOCK","CapsLock");
    keyNameMap_Java2Qt.insert("ESCAPE","Esc");
    keyNameMap_Java2Qt.insert("BACK_QUOTE","`");
    keyNameMap_Java2Qt.insert("MINUS","-");
    keyNameMap_Java2Qt.insert("EQUALS","=");
    keyNameMap_Java2Qt.insert("OPEN_BRACKET","[");
    keyNameMap_Java2Qt.insert("CLOSE_BRACKET","]");
    keyNameMap_Java2Qt.insert("BACK_SLASH","\\");
    keyNameMap_Java2Qt.insert("SEMICOLON",";");
    keyNameMap_Java2Qt.insert("QUOTE","'");
    keyNameMap_Java2Qt.insert("ENTER","Return");
    keyNameMap_Java2Qt.insert("COMMA",",");
    keyNameMap_Java2Qt.insert("PERIOD",".");
    keyNameMap_Java2Qt.insert("SLASH","/");
    keyNameMap_Java2Qt.insert("HOME","Home");
    keyNameMap_Java2Qt.insert("PAGE_UP","PgUp");
    keyNameMap_Java2Qt.insert("DELETE","Del");
    keyNameMap_Java2Qt.insert("END","End");
    keyNameMap_Java2Qt.insert("PAGE_DOWN","PgDown");
    keyNameMap_Java2Qt.insert("UP","Up");
    keyNameMap_Java2Qt.insert("DOWN","Down");
    keyNameMap_Java2Qt.insert("LEFT","Left");
    keyNameMap_Java2Qt.insert("RIGHT","Right");
    keyNameMap_Java2Qt.insert("CLEAR","Clear");

    shiftKeyNameMap_Java2Qt.insert("`","~");
    shiftKeyNameMap_Java2Qt.insert("1","!");
    shiftKeyNameMap_Java2Qt.insert("2","@");
    shiftKeyNameMap_Java2Qt.insert("3","#");
    shiftKeyNameMap_Java2Qt.insert("4","$");
    shiftKeyNameMap_Java2Qt.insert("5","%");
    shiftKeyNameMap_Java2Qt.insert("6","^");
    shiftKeyNameMap_Java2Qt.insert("7","&");
    shiftKeyNameMap_Java2Qt.insert("8","*");
    shiftKeyNameMap_Java2Qt.insert("9","(");
    shiftKeyNameMap_Java2Qt.insert("0",")");
    shiftKeyNameMap_Java2Qt.insert("-","_");
    shiftKeyNameMap_Java2Qt.insert("=","+");
    shiftKeyNameMap_Java2Qt.insert("[","{");
    shiftKeyNameMap_Java2Qt.insert("]","}");
    shiftKeyNameMap_Java2Qt.insert("\\","|");
    shiftKeyNameMap_Java2Qt.insert(";",":");
    shiftKeyNameMap_Java2Qt.insert("'","\"");
    shiftKeyNameMap_Java2Qt.insert(",","<");
    shiftKeyNameMap_Java2Qt.insert(".",">");
    shiftKeyNameMap_Java2Qt.insert("/","/");
}

QString EntityAdapter::convertKeyBind(QString & javaKeyBind)
{
    QStringList list = javaKeyBind.split(QRegExp("\\s+"),QString::SkipEmptyParts);

    bool shiftPressed = false;

    QMutableListIterator<QString> i(list);
    while (i.hasNext())
    {
        QString val = i.next();
        if (val == "pressed")
        {
            i.remove();
        }
        else
        {
            val = keyNameMap_Java2Qt.value(val, val);
            if (shiftPressed)
            {
                val = shiftKeyNameMap_Java2Qt.value(val, val);
            }
            if (val == "Shift")
            {
                shiftPressed = true;
            }
            i.setValue(val);
        }
    }

    QString qtKeyBind(list.join("+"));

    //qDebug() << "Converted key bind: "<< javaKeyBind << " => " << qtKeyBind;
    return qtKeyBind;
}

//---------------------------------------------------------------------------------------
// Static functions
//---------------------------------------------------------------------------------------

EntityAdapter &EntityAdapter::get()
{
  static EntityAdapter obj;
  return obj;
}

Entity* EntityAdapter::convert(cds::fw__entity *fwEntity)
{
    Entity *entity = new Entity();

    if (fwEntity->id != NULL) entity->id = new qint64(*fwEntity->id);
    if (fwEntity->name != NULL) entity->name = new QString(fwEntity->name->c_str());
    if (fwEntity->entityType != NULL) entity->entityType = new QString(fwEntity->entityType->c_str());
    if (fwEntity->entityStatus != NULL) entity->entityStatus = new QString(fwEntity->entityStatus->c_str());

    if (fwEntity->entityDataSet != NULL)
    {
        entity->entityDataSet.clear();
        std::vector<class cds::fw__entityData *> children = fwEntity->entityDataSet->entityData;

        for (int c = 0; c < children.size(); ++c)
        {
            cds::fw__entityData *childED = children[c];
            EntityData *entityData = new EntityData();

            if (childED->id != NULL) entityData->id = new qint64(*childED->id);
            if (childED->orderIndex != NULL) entityData->orderIndex = new qint64(*childED->orderIndex);
            if (childED->entityAttribute != NULL) entityData->attributeName = new QString(childED->entityAttribute->c_str());
            if (childED->value != NULL) entityData->value = new QString(childED->value->c_str());
            if (childED->user != NULL) entityData->user = new QString(childED->user->c_str());

            entityData->parentEntity = entity;

            if (childED->childEntity != NULL)
            {
                // Recursively convert children
                entityData->childEntity = convert(childED->childEntity);
            }

            entity->entityDataSet.insert(entityData);
        }

    }
    return entity;
}


QMap<QKeySequence, qint64>* EntityAdapter::convert(cds::fw__ontologyKeyBindings *keyBindings)
{
    QMap<QKeySequence, qint64> *map = new QMap<QKeySequence, qint64>();
    if (keyBindings->keyBindingSet == NULL) return map;

    std::vector<class cds::fw__ontologyKeyBind *> bindVector = keyBindings->keyBindingSet->keyBinding;
    for (int c = 0; c < bindVector.size(); ++c)
    {
        cds::fw__ontologyKeyBind *keyBind = bindVector[c];
        if (keyBind->key != NULL && keyBind->ontologyTermId != NULL)
        {
            QString javaKeyBind(QString::fromStdString(*keyBind->key));
            QString qtKeyBind(get().convertKeyBind(javaKeyBind));
            qint64 termId(*keyBind->ontologyTermId);
            QKeySequence keySeq(qtKeyBind, QKeySequence::PortableText);
            map->insert(keySeq, termId);
        }
    }
    return map;
}

