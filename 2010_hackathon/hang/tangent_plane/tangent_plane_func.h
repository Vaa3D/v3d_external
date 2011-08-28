/* tangent_plane_func.h
 * This is a tangent plane plugin
 * 2011-08-26 : by Hang Xiao
 */
 
#ifndef __TANGENT_PLANE_FUNC_H__
#define __TANGENT_PLANE_FUNC_H__

#include <v3d_interface.h>

int get_tangent_plane(V3DPluginCallback2 &callback, QWidget *parent);
int tracking_without_branch(V3DPluginCallback2 &callback, QWidget *parent);

#endif

