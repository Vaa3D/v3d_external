//by Hanchuan Peng
//090516
//090604: add bool b_disp_QTDialog

#ifndef __V3D_MESSAGE_H__
#define __V3D_MESSAGE_H__

#include <QString>

void v3d_msg(char *msg, bool b_disp_QTDialog=true);
void v3d_msg(const QString & msg, bool b_disp_QTDialog=true);

#endif


