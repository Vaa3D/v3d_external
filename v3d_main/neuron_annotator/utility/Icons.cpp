#include "Icons.h"
#include "../entity_model/Entity.h"

Icons::Icons()
{
}

QVariant Icons::getCachedIcon(const QString & filename)
{
    if (cache.contains(filename)) return cache.value(filename, QVariant());
    QString path = ":/neuron_annotator/resources/";
    path.append(filename);
    QIcon icon(path);
    cache.insert(filename, icon);
    return icon;
}

//---------------------------------------------------------------------------------------
// Static functions
//---------------------------------------------------------------------------------------

Icons &Icons::get()
{
  static Icons obj;
  return obj;
}

QVariant Icons::getIcon(const QString & filename)
{
    return get().getCachedIcon(filename);
}

QVariant Icons::getOntologyIcon(Entity *entity)
{
    QString termType = entity->getValueByAttributeName("Ontology Term Type");

    if (termType=="Category")
        return getIcon("folder.png");

    else if (termType=="Enum")
        return getIcon("folder_page.png");

    else if (termType=="Interval")
        return getIcon("page_white_code.png");

    else if (termType=="Tag")
        return getIcon("page_white.png");

    else if (termType=="Text")
        return getIcon("page_white_text.png");

    else if (termType=="EnumItem")
        return getIcon("page.png");

    else if (termType=="EnumText")
        return getIcon("page_go.png");

    return QVariant();
}

QVariant Icons::getIcon(Entity *entity)
{
    QString type = *entity->entityType;

    if (type=="Folder") {
        return getIcon("folder.png");
    }
    else if (type=="Sample" || type=="Screen Sample") {
        return getIcon("beaker.png");
    }
    else if (type=="Supporting Data") {
        return getIcon("folder_page.png");
    }
    else if (type=="Sample Processing Result") {
        return getIcon("folder_image.png");
    }
    else if (type=="Alignment Result") {
        return getIcon("folder_image.png");
    }
    else if (type=="Neuron Separator Pipeline Result") {
        return getIcon("bricks.png");
    }
    else if (type=="LSM Stack Pair") {
        return getIcon("page_copy.png");
    }
    else if (type=="Tif 2D Image" || type=="Image 2D") {
        return getIcon("image.png");
    }
    else if (type=="Text File") {
        return getIcon("page_white.png");
    }
    else if (type=="Image 3D" || type=="Tif 3D Image" || type=="LSM Stack" || type=="Tif 3D Label Mask" || type=="Stitched V3D Raw Stack" || type=="Aligned Brain Stack") {
        return getIcon("images.png");
    }
    else if (type=="Neuron Fragment"||type=="Curated Neuron") {
        return getIcon("brick.png");
    }
    else if (type=="Neuron Fragment Collection"||type=="Curated Neuron Collection") {
        return getIcon("folder_brick.png");
    }
    else if (type=="Ontology Element" || type=="Ontology Root") {
        return getOntologyIcon(entity);
    }
    else if (type=="Annotation") {
        return getIcon("page_white_edit.png");
    }
    else if (type=="Annotation") {
        return getIcon("fruit_fly_small.png");
    }

    //    return QVariant(); // blank icon
    return getIcon("bullet_error.png");

}
