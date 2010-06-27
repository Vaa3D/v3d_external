
//by Hanchuan Peng
//2009-06-26

#ifndef __example_reset_xyz_resolution_PLUGIN_H__
#define __example_reset_xyz_resolution_PLUGIN_H__


#include <v3d_interface.h>

class example_reset_xyz_resolutionPlugin : public QObject, public V3DSingleImageInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DSingleImageInterface)

public:
    QStringList menulist() const;
    void processImage(const QString &arg, Image4DSimple *image, QWidget *parent);
};

#endif
