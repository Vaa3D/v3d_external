#ifndef QUNDOMARKERCREATE_H
#define QUNDOMARKERCREATE_H

#include <QUndoCommand>
#include "../control/CPlugin.h"
#include "v3d_interface.h"

class terafly::QUndoMarkerCreate : public QUndoCommand
{
    private:

        tf::CViewer* source;       //where the command has been applied
        LocationSimple marker;              //the marker being created
        bool redoFirstTime;                 //to disable redo's first call

    public:

        QUndoMarkerCreate(tf::CViewer* _source, LocationSimple _marker);

        // undo and redo methods
        virtual void undo();
        virtual void redo();
    
};

class terafly::QUndoVaa3DNeuron : public QUndoCommand
{
    private:

        tf::CViewer* source;       //where the command has been applied
        bool redoFirstTime;         //to disable redo's first call

    public:

        QUndoVaa3DNeuron(tf::CViewer* _source);

        // undo and redo methods
        virtual void undo();
        virtual void redo();

};

#endif // QUNDOMARKERCREATE_H
