#include "JacsUtil.h"
#include "../entity_model/Entity.h"
#include <QRegExp>
#include <QIcon>

QString convertPathToMac(QString path) {
    // TODO: extract this into a property
    return path.replace(QRegExp("/groups/scicomp/jacsData/"), "/Volumes/jacsData/");
}

QVariant getOntologyIcon(Entity *entity)
{
    QString termType = entity->getValueByAttributeName("Ontology Term Type");

    if (termType=="Category")
        return QIcon(":/neuron_annotator/resources/folder.png");

    else if (termType=="Enum")
        return QIcon(":/neuron_annotator/resources/folder_page.png");

    else if (termType=="Interval")
        return QIcon(":/neuron_annotator/resources/page_white_code.png");

    else if (termType=="Tag")
        return QIcon(":/neuron_annotator/resources/page_white.png");

    else if (termType=="Text")
        return QIcon(":/neuron_annotator/resources/page_white_text.png");

    else if (termType=="EnumItem")
        return QIcon(":/neuron_annotator/resources/page.png");

    return QVariant();
}

// This icon implementation mirrors the Console's EntityTreeCellRenderer and OntologyTreeCellRenderer.
QVariant getIcon(Entity *entity)
{
    QString type = *entity->entityType;
    qDebug() << type;

    if (type=="Folder") {
        return QIcon(":/neuron_annotator/resources/folder.png");
    }
    else if (type=="LSM Stack Pair") {
        return QIcon(":/neuron_annotator/resources/folder_image.png");
    }
    else if (type=="Neuron Separator Pipeline Result") {
        return QIcon(":/neuron_annotator/resources/folder_image.png");
    }
    else if (type=="Sample") {
        return QIcon(":/neuron_annotator/resources/beaker.png");
    }
    else if (type=="Tif 2D Image") {
        return QIcon(":/neuron_annotator/resources/image.png");
    }
    else if (type=="Tif 3D Image" || type=="LSM Stack" || type=="Tif 3D Label Mask") {
        return QIcon(":/neuron_annotator/resources/images.png");
    }
    else if (type=="Ontology Element" || type=="Ontology Root") {
        return getOntologyIcon(entity);
    }
    else if (type=="Annotation") {
        return QIcon(":/neuron_annotator/resources/page_white_edit.png");
    }

    return QVariant();
}


