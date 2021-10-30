/*
 * VolumeTreeViewer.h
 *
 *  Created on: Nov 15, 2012
 *      Author: Christopher M. Bruns
 */

#ifndef VOLUMETREEVIEWER_H_
#define VOLUMETREEVIEWER_H_

// Include GLee before any other OpenGL includes
#include "../../3drenderer/GLee_r.h"
//
#include "Camera3D.h"
#include "TrackballInteractorCamera.h"
#include "../render/ActorGL.h"
#include <QGLWidget>
#include <QMouseEvent>
#include "boost/shared_ptr.hpp"
#include <vector>

class VolumeTreeViewer : public QGLWidget
{
public:
    VolumeTreeViewer(QWidget* parent = NULL);
    virtual ~VolumeTreeViewer();

    virtual void initializeGL();
    virtual void mouseDoubleClickEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void paintGL();
    virtual void resizeGL(int width, int height);
    virtual void wheelEvent(QWheelEvent* event);

protected:
    Camera3D camera;
    TrackballInteractorCamera trackball;

    typedef boost::shared_ptr<ActorGL> ActorPtr;
    typedef std::vector<ActorPtr> ActorList;
    ActorList opaqueActors;
    ActorList transparentActors;
    bool bIsInitialized;
};

#endif /* VOLUMETREEVIEWER_H_ */
