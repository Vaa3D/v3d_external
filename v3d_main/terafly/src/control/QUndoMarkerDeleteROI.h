#ifndef QUNDOMARKERDELETEROI_H
#define QUNDOMARKERDELETEROI_H

#include <QUndoCommand>
#include "../control/CPlugin.h"
#include "v3d_interface.h"

class terafly::QUndoMarkerDeleteROI : public QUndoCommand
{
    private:

        tf::CViewer* source;       //where the command has been applied
        QList<LocationSimple> markers;      //the markers being deleted
        bool redoFirstTime;                 //to disable redo's first call

    public:

        QUndoMarkerDeleteROI(tf::CViewer* _source, QList<LocationSimple> _markers);

        // undo and redo methods
        virtual void undo();
        virtual void redo();
};

#endif // QUNDOMARKERDELETEROI_H
