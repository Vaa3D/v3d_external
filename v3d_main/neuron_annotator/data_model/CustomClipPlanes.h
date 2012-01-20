#ifndef CUSTOMCLIPPLANES_H
#define CUSTOMCLIPPLANES_H

#include <vector>
#include "../../3drenderer/GLee_r.h"

namespace jfrc {

class ClipPlane : public std::vector<GLdouble> {
public:
    ClipPlane() : std::vector<GLdouble>(4, 0.0)
    {
        (*this)[3] = 1.0; // initialize to (0,0,0,1), which never clips (x,y,z,1)
    }
};

class CustomClipPlanes : public std::vector<ClipPlane>
{
public:
    CustomClipPlanes(int numPlanes) : std::vector<ClipPlane>(numPlanes)
    {}
};

} // namespace jfrc

#endif // CUSTOMCLIPPLANES_H
