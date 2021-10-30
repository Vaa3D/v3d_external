/*
 * VolumeTreeViewer.cpp
 *
 *  Created on: Nov 15, 2012
 *      Author: Christopher M. Bruns
 */

#include "VolumeTreeViewer.h"
#include "TeapotActor.h"
#include "VolumeSlicesActor.h"
#include <fstream>
#include <iostream>

using namespace std;

VolumeTreeViewer::VolumeTreeViewer(QWidget* parent)
    : QGLWidget(parent)
    , trackball(camera)
    , bIsInitialized(false)
{
    qRegisterMetaType<Vector3D>("Vector3D");
    qRegisterMetaType<Rotation3D>("Rotation3D");

    connect(&camera, SIGNAL(focusChanged(Vector3D)),
            this, SLOT(update()));
    connect(&camera, SIGNAL(rotationChanged(Rotation3D)),
            this, SLOT(update()));
    connect(&camera, SIGNAL(focusDistanceChanged(double)),
            this, SLOT(update()));

    // For testing only
    // opaqueActors.push_back(ActorPtr(new TeapotActor()));
    boost::shared_ptr<VolumeSlicesActor> volumeActor(new VolumeSlicesActor());
    if (! volumeActor->loadVolume("/Users/brunsc/svn/v3d/v3d_main/neuron_annotator/dev/crop2.v3draw"))
    {
        cerr << "Failed to load volume" << endl;
    }
    volumeActor->setVoxelMicrometers(0.100, 0.100, 0.070); // from Jennifer Colonell
    transparentActors.push_back(volumeActor);
}

VolumeTreeViewer::~VolumeTreeViewer()
{
    if (bIsInitialized) {
        makeCurrent();
        // Draw objects
        ActorList::iterator a;
        // Opaque geometry first, to populate depth buffer
        for (a = opaqueActors.begin(); a != opaqueActors.end(); ++a)
            (*a)->destroyGL();
        for (a = transparentActors.begin(); a != transparentActors.end(); ++a)
            (*a)->destroyGL();
        doneCurrent();
    }
}

/* virtual */
void VolumeTreeViewer::initializeGL()
{
    glShadeModel(GL_SMOOTH);
    glEnable(GL_COLOR_MATERIAL);
    float specular[] = {1,1,1,1};
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    float shininess = 50.0;
    glMaterialfv(GL_FRONT, GL_SHININESS, &shininess);
    float position[] = {1,1,1,0};
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    float diffuse[] = {1,1,1,1};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, diffuse);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, position);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    bIsInitialized = true;
}

/* virtual */
void VolumeTreeViewer::mouseDoubleClickEvent(QMouseEvent* event)
{
    trackball.mouseDoubleClickEvent(event);
}

/* virtual */
void VolumeTreeViewer::mouseMoveEvent(QMouseEvent* event)
{
    trackball.mouseMoveEvent(event);
}

/* virtual */
void VolumeTreeViewer::mousePressEvent(QMouseEvent* event)
{
    trackball.mousePressEvent(event);
}

/* virtual */
void VolumeTreeViewer::mouseReleaseEvent(QMouseEvent* event)
{
    trackball.mouseReleaseEvent(event);
}


/* virtual */
void VolumeTreeViewer::paintGL()
{
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);

    // Clear background
    glClearColor(0.1, 0.1, 0.1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up camera
    CameraSetterGL cameraSetter(camera);

    // Draw objects
    ActorList::iterator a;
    // Opaque geometry first, to populate depth buffer
    for (a = opaqueActors.begin(); a != opaqueActors.end(); ++a)
        (*a)->paintGL();
    for (a = transparentActors.begin(); a != transparentActors.end(); ++a)
        (*a)->paintGL();
}

/* virtual */
void VolumeTreeViewer::resizeGL(int width, int height)
{
    camera.setViewportPixels(width, height);
}

void VolumeTreeViewer::wheelEvent(QWheelEvent* event)
{
    trackball.wheelEvent(event);
}


