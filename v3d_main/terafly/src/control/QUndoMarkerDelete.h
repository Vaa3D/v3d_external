#ifndef QUNDOMARKERDELETE_H
#define QUNDOMARKERDELETE_H

#include <QUndoCommand>
#include "../control/CPlugin.h"
#include "v3d_interface.h"

class terafly::QUndoMarkerDelete : public QUndoCommand
{
    private:

        tf::CViewer* source;       //where the command has been applied
        LocationSimple marker;              //the marker being deleted
        bool redoFirstTime;                 //to disable redo's first call

    public:

        QUndoMarkerDelete(tf::CViewer* _source, LocationSimple _marker);

        // undo and redo methods
        virtual void undo();
        virtual void redo();
};

#endif // QUNDOMARKERDELETE_H
