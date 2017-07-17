//last touch by Hanchuan Peng, 20170615.

#ifndef __V3DR_GL_VR_H__
#define __V3DR_GL_VR_H__

#include "../basic_c_fun/v3d_interface.h"

class My4DImage;
class MainWindow;
//bool doimageVRViewer(int argc, char *argv[]);
bool doimageVRViewer(NeuronTree nt, My4DImage *img4d, MainWindow *pmain);

#endif

