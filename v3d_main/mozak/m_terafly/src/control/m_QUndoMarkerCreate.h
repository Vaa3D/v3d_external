#ifndef QUNDOMARKERCREATE_H
#define QUNDOMARKERCREATE_H

#include "v3d_interface.h"
#include <QUndoCommand>
#include "m_CPlugin.h"

class teramanager::QUndoMarkerCreate : public QUndoCommand
{
    private:

        itm::CViewer* source;       //where the command has been applied
        LocationSimple marker;              //the marker being created
        bool redoFirstTime;                 //to disable redo's first call

    public:

        QUndoMarkerCreate(itm::CViewer* _source, LocationSimple _marker);

        // undo and redo methods
        virtual void undo();
        virtual void redo();
    
};

class teramanager::QUndoVaa3DNeuron : public QUndoCommand
{
    private:

        itm::CViewer* source;       //where the command has been applied
        bool redoFirstTime;         //to disable redo's first call

    public:

        QUndoVaa3DNeuron(itm::CViewer* _source);

        // undo and redo methods
        virtual void undo();
        virtual void redo();

};

#endif // QUNDOMARKERCREATE_H
