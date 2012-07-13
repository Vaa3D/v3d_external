#ifndef CUSTOMCLIPPLANES_H
#define CUSTOMCLIPPLANES_H

#include "../../3drenderer/GLee_r.h"
#include <QUndoStack>
#include <QUndoCommand>
#include <vector>

namespace jfrc {

class ClipPlane : public std::vector<GLdouble> {
public:
    ClipPlane();
    ClipPlane(double x, double y, double z, double w)
        : std::vector<GLdouble>(4, 0.0)
    {
        ClipPlane& lhs = *this;
        lhs[0] = x; lhs[1] = y; lhs[2] = z; lhs[3] = w;
    }
    ClipPlane& operator=(const ClipPlane& rhs) {
        ClipPlane& lhs = *this;
        for (int i = 0; i < 4; ++i)
            lhs[i] = rhs[i];
        return *this;
    }
    void disable(); // set to (0,0,0,1), which never clips (x,y,z,1)
};

// Ring data structure of custom clip planes.
// After ten planes are defined, new planes will overwrite the oldest remaining planes.
class CustomClipPlanes : public std::vector<ClipPlane>
{
public:
    CustomClipPlanes();
    void setUndoStack(QUndoStack& undoStackParam) {undoStack = &undoStackParam;}
    void addPlane(double x, double y, double z, double w); // Add a clip plane
    int getNextClipPlaneIndex() const {return nextClipPlaneIndex;}

    // "simple" method versions do not invoke undo/redo infrastructure; for use by undo/redo infrastructure
    int simpleAddPlane(const ClipPlane& rhs); // returns index of added plane
    void simpleUndoPlane(int planeIndex); // Remove the latest added plane
    void clearAll();

protected:
    int nextClipPlaneIndex;
    QUndoStack * undoStack;
};


class AddClipPlaneCommand : public QUndoCommand
{
public:
    AddClipPlaneCommand(CustomClipPlanes* planesParam, const ClipPlane& plane, QUndoCommand *parent = NULL);
    ~AddClipPlaneCommand();
    void undo();
    void redo();

private:
    ClipPlane plane;
    CustomClipPlanes* planes;
    int planeIndex;
};


} // namespace jfrc

#endif // CUSTOMCLIPPLANES_H
