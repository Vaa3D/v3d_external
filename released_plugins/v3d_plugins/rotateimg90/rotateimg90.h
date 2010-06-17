
#ifndef ROTATEIMG90PLUGIN_H
#define ROTATEIMG90PLUGIN_H


#include <v3d_interface.h>

class RotateImg90Plugin : public QObject, public V3DSingleImageInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DSingleImageInterface)

public:
    QStringList menulist() const;
    void processImage(const QString &arg, Image4DSimple *image, QWidget *parent);
};

#endif
