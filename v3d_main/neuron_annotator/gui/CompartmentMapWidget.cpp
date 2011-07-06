#include "CompartmentMapWidget.h"
#include "v3d_core.h"
#include "../3drenderer/Renderer_gl2.h"
#include <iostream>
#include <cmath>
#include <cassert>

using namespace std;

CompartmentMapWidget::CompartmentMapWidget(QWidget* parent): V3dR_GLWidget(NULL, parent, "Compartment Map")
{}

CompartmentMapWidget::~CompartmentMapWidget()
{}

void CompartmentMapWidget::loadAtlas()
{
    //QString atlasfile(":/atlas/flybraincompartmentmap.v3ds"); // .qrc
    QString flybrainatlas("flybraincompartmentmap.v3ds");

    QString atlasfile;

    if(QFile::exists(QDir::currentPath().append("/").append(flybrainatlas)))
    {
        atlasfile = QDir::currentPath().append("/").append(flybrainatlas);
    }
    else if(QFile::exists(QDir::homePath().append("/").append(flybrainatlas)))
    {
        atlasfile = QDir::homePath().append("/").append(flybrainatlas);
    }
    else if(QFile::exists(QDir::rootPath().append("/").append(flybrainatlas)))
    {
        atlasfile = QDir::rootPath().append("/").append(flybrainatlas);
    }
    else if(QFile::exists(QDir::tempPath().append("/").append(flybrainatlas)))
    {
        atlasfile = QDir::tempPath().append("/").append(flybrainatlas);
    }

    if(QFile::exists(atlasfile))
    {
        ((Renderer_tex2 *)renderer)->loadV3DSFile(atlasfile);
        //((Renderer_tex2 *)renderer)->loadObjectFromFile(atlasfile.toStdString().c_str());

        updateTool();
        //renderer->paint(); //POST_updateGL();
        //update();
    }
}

// display OpenGL graphics
void CompartmentMapWidget::initializeGL()
{
    _isSoftwareGL = false;
    GLeeInit();

    renderer = new Renderer_tex2(this);

    // settings
    renderer->bShowBoundingBox = true;
    renderer->bShowAxes        = true;

    renderer->tryTexCompress = false;
    renderer->tryTex3D       = false;
    renderer->tryTexNPT      = false;
    renderer->tryTexStream   = true;

    renderer->lineType   = true;

    // prepare
    if (renderer)
    {
        loadAtlas();
        if(renderer->hasError())	POST_CLOSE(this);
    }

}
void CompartmentMapWidget::resizeGL(int width, int height){
    V3dR_GLWidget::resizeGL(width, height);
}
void CompartmentMapWidget::paintGL(){
    V3dR_GLWidget::paintGL();
}

// event
void CompartmentMapWidget::focusInEvent(QFocusEvent* e){
    V3dR_GLWidget::focusInEvent(e);
}
void CompartmentMapWidget::focusOutEvent(QFocusEvent* e){
    V3dR_GLWidget::focusOutEvent(e);
}
void CompartmentMapWidget::enterEvent(QEvent *e){
    V3dR_GLWidget::enterEvent(e);
}
void CompartmentMapWidget::leaveEvent(QEvent *e){
    V3dR_GLWidget::leaveEvent(e);
}
void CompartmentMapWidget::mousePressEvent(QMouseEvent *event){
    V3dR_GLWidget::mousePressEvent(event);
}
void CompartmentMapWidget::mouseReleaseEvent(QMouseEvent *event){
    V3dR_GLWidget::mouseReleaseEvent(event);
}
void CompartmentMapWidget::mouseMoveEvent(QMouseEvent *event){
    V3dR_GLWidget::mouseMoveEvent(event);
}
void CompartmentMapWidget::wheelEvent(QWheelEvent *event){
    V3dR_GLWidget::wheelEvent(event);
}
