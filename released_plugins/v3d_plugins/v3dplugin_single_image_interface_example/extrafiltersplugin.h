
#ifndef EXTRAFILTERSPLUGIN_H
#define EXTRAFILTERSPLUGIN_H


#include <v3d_interface.h>

class ExtraFilters : public QObject, public V3DSingleImageInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DSingleImageInterface);

public:
    QStringList menulist() const;
    void processImage(const QString &arg, Image4DSimple *image, QWidget *parent);
};

#endif
