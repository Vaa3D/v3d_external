/* recenterimageplugin.h
 * 2009-08-14: created by Yang Yu
 * 2010-05-20, by PHC for compatability to VC compiler
 */


#ifndef __RECENTERIMAGEPLUGIN_H__
#define __RECENTERIMAGEPLUGIN_H__

//  recenter subject image into the center of target image
//
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

#if defined (_MSC_VER)  //2010-05-20, by PHC for compatability to VC compiler
#include "../../../v3d_main/basic_c_fun/vcdiff.h"
#else
#endif

class ReCenterImagePlugin : public QObject, public V3DSingleImageInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DSingleImageInterface)

public:
    QStringList menulist() const;
    void processImage(const QString &arg, Image4DSimple *p4DImage, QWidget *parent);
};

#endif



