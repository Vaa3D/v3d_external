
#include "renderer_gl1.h"

#include "QUndoMarkerCreate.h"
#include "../control/CViewer.h"
#include "v3dr_glwidget.h"
#include "../presentation/PAnoToolBar.h"
#include "PMain.h"

tf::QUndoMarkerCreate::QUndoMarkerCreate(tf::CViewer* _source, LocationSimple _marker) : QUndoCommand()
{
    source = _source;
    marker = _marker;
    redoFirstTime = true;

    // tell main GUI that annotations have been changed
    PMain::getInstance()->annotationsChanged();
}

// undo and redo methods
void tf::QUndoMarkerCreate::undo()
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    // get markers from Vaa3D
    QList<LocationSimple> vaa3dMarkers = source->V3D_env->getLandmark(source->window);

    // remove the marker just created
    for(int i=0; i<vaa3dMarkers.size(); i++)
        if(vaa3dMarkers[i].x == marker.x && vaa3dMarkers[i].y == marker.y && vaa3dMarkers[i].z == marker.z)
            vaa3dMarkers.removeAt(i);

    // set new markers
    source->V3D_env->setLandmark(source->window, vaa3dMarkers);
    source->V3D_env->pushObjectIn3DWindow(source->window);

    //update visible markers
    PAnoToolBar::instance()->buttonMarkerRoiViewChecked(PAnoToolBar::instance()->buttonMarkerRoiView->isChecked());

    // end select mode
    //source->view3DWidget->getRenderer()->endSelectMode();

    // tell main GUI that annotations have been changed
    PMain::getInstance()->annotationsChanged();
}

void tf::QUndoMarkerCreate::redo()
{
    /**/tf::debug(tf::LEV1, tf::strprintf("redoFirstTime = %s", redoFirstTime ? "true" : "false").c_str(), __itm__current__function__);

    // first time redo's call is aborted: we don't want it to be called once the command is pushed into the QUndoStack
    if(!redoFirstTime)
    {
        // get markers from Vaa3D
        QList<LocationSimple> vaa3dMarkers = source->V3D_env->getLandmark(source->window);

        // add previously deleted marker
        vaa3dMarkers.push_back(marker);

        // set new markers
        source->V3D_env->setLandmark(source->window, vaa3dMarkers);
        source->V3D_env->pushObjectIn3DWindow(source->window);

        //update visible markers
        PAnoToolBar::instance()->buttonMarkerRoiViewChecked(PAnoToolBar::instance()->buttonMarkerRoiView->isChecked());

        // end select mode
        //source->view3DWidget->getRenderer()->endSelectMode();

        // tell main GUI that annotations have been changed
        PMain::getInstance()->annotationsChanged();
    }
    else
        redoFirstTime = false;
}



tf::QUndoVaa3DNeuron::QUndoVaa3DNeuron(tf::CViewer* _source) : QUndoCommand()
{
    source = _source;
    redoFirstTime = true;

    // tell main GUI that annotations have been changed
    PMain::getInstance()->annotationsChanged();
}

// undo and redo methods
void tf::QUndoVaa3DNeuron::undo()
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    if (v3dr_getImage4d(source->view3DWidget->_idep) && source->view3DWidget->renderer)
    {
        v3dr_getImage4d(source->view3DWidget->_idep)->proj_trace_history_undo();
        v3dr_getImage4d(source->view3DWidget->_idep)->update_3drenderer_neuron_view(source->view3DWidget, (Renderer_gl1*)source->view3DWidget->renderer);//090924

        source->view3DWidget->update();

        // tell main GUI that annotations have been changed
        PMain::getInstance()->annotationsChanged();
    }
}

void tf::QUndoVaa3DNeuron::redo()
{
    /**/tf::debug(tf::LEV1, tf::strprintf("redoFirstTime = %s", redoFirstTime ? "true" : "false").c_str(), __itm__current__function__);

    // first time redo's call is aborted: we don't want it to be called once the command is pushed into the QUndoStack
    if(!redoFirstTime)
    {
        if (v3dr_getImage4d(source->view3DWidget->_idep) && source->view3DWidget->renderer)
        {
            v3dr_getImage4d(source->view3DWidget->_idep)->proj_trace_history_redo();
            v3dr_getImage4d(source->view3DWidget->_idep)->update_3drenderer_neuron_view(source->view3DWidget, (Renderer_gl1*)source->view3DWidget->renderer);//090924

            source->view3DWidget->update();

            // tell main GUI that annotations have been changed
            PMain::getInstance()->annotationsChanged();
        }
    }
    else
        redoFirstTime = false;
}
