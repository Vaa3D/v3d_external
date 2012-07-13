#include <QDebug>
#include <QString>
#include "CustomClipPlanes.h"

namespace jfrc {


///////////////
// ClipPlane //
///////////////

ClipPlane::ClipPlane() : std::vector<GLdouble>(4, 0.0)
{
    disable(); // initialize to (0,0,0,1), which never clips (x,y,z,1)
}

// set to (0,0,0,1), which never clips (x,y,z,1)
void ClipPlane::disable()
{
    ClipPlane& p = *this;
    p[0] = p[1] = p[2] = 0.0;
    p[3] = 1.0;
}


/////////////////////////
// AddClipPlaneCommand //
/////////////////////////

AddClipPlaneCommand::AddClipPlaneCommand(CustomClipPlanes* planesParam, const ClipPlane& plane, QUndoCommand *parent)
    : QUndoCommand(parent)
    , planes(planesParam)
    , plane(plane)
{
    setText(QObject::tr("Clip Plane %1").arg(planes->getNextClipPlaneIndex() + 1));
}

AddClipPlaneCommand::~AddClipPlaneCommand()
{
}

void AddClipPlaneCommand::undo()
{
    planes->simpleUndoPlane(planeIndex);
}

void AddClipPlaneCommand::redo()
{
    planeIndex = planes->simpleAddPlane(plane);
}


//////////////////////
// CustomClipPlanes //
//////////////////////

CustomClipPlanes::CustomClipPlanes()
    : std::vector<ClipPlane>(12) // maximum of 12 planes
    , nextClipPlaneIndex(0)
    , undoStack(NULL)
{}

void CustomClipPlanes::clearAll()
{
    for (int p = 0; p < size(); ++p)
        (*this)[p].disable();
    nextClipPlaneIndex = 0;
    undoStack->clear();
}

// Add a clip plane
void CustomClipPlanes::addPlane(double x, double y, double z, double w)
{
    if (NULL == undoStack)
        simpleAddPlane(ClipPlane(x, y, z, w));
    else
        undoStack->push(new AddClipPlaneCommand(this, ClipPlane(x, y, z, w)));
}

// Add a clip plane
int CustomClipPlanes::simpleAddPlane(const ClipPlane& rhs)
{
    int result = nextClipPlaneIndex;
    // qDebug() << "adding plane" << result + 1;
    ClipPlane& p = (*this)[nextClipPlaneIndex];
    p = rhs;
    ++nextClipPlaneIndex;
    if (nextClipPlaneIndex >= size())
        nextClipPlaneIndex = 0;
    return result;
}

// Remove the latest added plane
void CustomClipPlanes::simpleUndoPlane(int planeIndex)
{
    // qDebug() << "removing plane" << planeIndex + 1;
    int topPlaneIndex = nextClipPlaneIndex - 1;
    if (topPlaneIndex < 0)
        topPlaneIndex = (int)size() - 1;
    if (planeIndex == topPlaneIndex)
        nextClipPlaneIndex = topPlaneIndex;
    (*this)[planeIndex].disable();
}


} // namespace jfrc
