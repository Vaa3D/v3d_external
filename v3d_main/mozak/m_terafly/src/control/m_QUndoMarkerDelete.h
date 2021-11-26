#ifndef QUNDOMARKERDELETE_H
#define QUNDOMARKERDELETE_H

#include "v3d_interface.h"
#include <QUndoCommand>
#include "m_CPlugin.h"

class teramanager::QUndoMarkerDelete : public QUndoCommand
{
    private:

        itm::CViewer* source;       //where the command has been applied
        LocationSimple marker;              //the marker being deleted
        bool redoFirstTime;                 //to disable redo's first call

    public:

        QUndoMarkerDelete(itm::CViewer* _source, LocationSimple _marker);

        // undo and redo methods
        virtual void undo();
        virtual void redo();
};

#endif // QUNDOMARKERDELETE_H
