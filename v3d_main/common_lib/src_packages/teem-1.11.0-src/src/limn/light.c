/*
  Teem: Tools to process and visualize scientific data and images             .
  Copyright (C) 2012, 2011, 2010, 2009  University of Chicago
  Copyright (C) 2008, 2007, 2006, 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  (LGPL) as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  The terms of redistributing and/or modifying this software also
  include exceptions to the LGPL that facilitate static linking.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "limn.h"


/*
******** limnLightSet()
**
** turns on a light
**
*/
void
limnLightSet(limnLight *lit, int which, int vsp,
             float r, float g, float b,
             float x, float y, float z) {

  if (lit && AIR_IN_CL(0, which, LIMN_LIGHT_NUM-1)) {
    lit->on[which] = 1;
    lit->vsp[which] = vsp;
    ELL_4V_SET(lit->col[which], r, g, b, 1.0);
    ELL_4V_SET(lit->_dir[which], x, y, z, 0.0);
  }
}

/*
******** limnLightAmbientSet()
**
** sets the ambient light color
*/
void
limnLightAmbientSet(limnLight *lit, float r, float g, float b) {

  if (lit) {
    ELL_4V_SET(lit->amb, r, g, b, 1.0);
  }
}

/*
******** limnLightUpdate()
**
** copies information from the _dir vectors to the dir vectors. This
** needs to be called even if there are no viewspace lights, so that
** the dir vectors are set and normalized.  If there are no viewspace
** lights, "cam" can actually be passed as NULL, but don't get carried
** away...
**
** returns 1 if there was a problem in the camera, otherwise 0.
*/
int
limnLightUpdate(limnLight *lit, limnCamera *cam) {
  static const char me[]="limnLightUpdate";
  double dir[3], _dir[3], uvn[9]={0,0,0,0,0,0,0,0,0}, norm;
  int i;

  if (cam) {
    if (limnCameraUpdate(cam)) {
      biffAddf(LIMN, "%s: trouble in camera", me);
      return 1;
    }
    ELL_34M_EXTRACT(uvn, cam->V2W);
  }
  for (i=0; i<LIMN_LIGHT_NUM; i++) {
    ELL_3V_COPY(_dir, lit->_dir[i]);
    if (cam && lit->vsp[i]) {
      ELL_3MV_MUL(dir, uvn, _dir);
    } else {
      ELL_3V_COPY(dir, _dir);
    }
    ELL_3V_NORM(dir, dir, norm);
    ELL_4V_SET_TT(lit->dir[i], float, dir[0], dir[1], dir[2], 0.0);
  }
  return 0;
}

/*
******** limnLightSwitch
**
** can toggle a light on/off
**
** returns 1 on error, 0 if okay
*/
void
limnLightSwitch(limnLight *lit, int which, int on) {

  if (lit && AIR_IN_CL(0, which, LIMN_LIGHT_NUM-1)) {
    lit->on[which] = on;
  }
}

void
limnLightReset(limnLight *lit) {
  int i;

  if (lit) {
    ELL_4V_SET(lit->amb, 0, 0, 0, 1);
    for (i=0; i<LIMN_LIGHT_NUM; i++) {
      ELL_4V_SET(lit->_dir[i], 0, 0, 0, 0);
      ELL_4V_SET(lit->dir[i], 0, 0, 0, 0);
      ELL_4V_SET(lit->col[i], 0, 0, 0, 1);
      lit->on[i] = AIR_FALSE;
      lit->vsp[i] = AIR_FALSE;
    }
  }
}
