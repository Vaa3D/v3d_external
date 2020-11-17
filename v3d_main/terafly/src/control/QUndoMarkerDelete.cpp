
#include "renderer_gl1.h"

#include "QUndoMarkerDelete.h"
#include "../control/CViewer.h"
#include "v3dr_glwidget.h"
#include "../presentation/PAnoToolBar.h"
#include "PMain.h"

tf::QUndoMarkerDelete::QUndoMarkerDelete(tf::CViewer* _source, LocationSimple _marker) :  QUndoCommand()
{
    source = _source;
    marker = _marker;
    redoFirstTime = true;

    // tell main GUI that annotations have been changed
    PMain::getInstance()->annotationsChanged();
}

// undo and redo methods
void tf::QUndoMarkerDelete::undo()
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

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

void tf::QUndoMarkerDelete::redo()
{
    /**/tf::debug(tf::LEV1, tf::strprintf("redoFirstTime = %s", redoFirstTime ? "true" : "false").c_str(), __itm__current__function__);

    // first time redo's call is aborted: we don't want it to be called once the command is pushed into the QUndoStack
    if(!redoFirstTime)
    {
        // get markers from Vaa3D
        QList<LocationSimple> vaa3dMarkers = source->V3D_env->getLandmark(source->window);

        // remove again the marker previosly deleted
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
    else
        redoFirstTime = false;
}
