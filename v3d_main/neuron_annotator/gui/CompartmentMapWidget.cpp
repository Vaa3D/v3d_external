#include "CompartmentMapWidget.h"
#include "v3d_core.h"
#include "../3drenderer/Renderer_gl2.h"
#include <iostream>
#include <cmath>
#include <cassert>

using namespace std;

CompartmentMapWidget::CompartmentMapWidget(QWidget* parent): V3dR_GLWidget(NULL, parent, "Title")
{
    _idep = new iDrawExternalParameter();
    _idep->image4d = NULL;
    _volCompress = false;

    // This method for eliminating tearing artifacts works but is supposedly obsolete;
    // http://stackoverflow.com/questions/5174428/how-to-change-qglformat-for-an-existing-qglwidget-at-runtime
    QGLFormat glFormat(context()->format());
    glFormat.setDoubleBuffer(true); // attempt to reduce tearing on Mac
    setFormat(glFormat);

    //
    renderer = new Renderer_tex2(this);
}

CompartmentMapWidget::~CompartmentMapWidget()
{
    delete _idep; _idep = NULL;
}

void CompartmentMapWidget::init()
{
    //
    //QString atlasfile(":/atlas/flybraincompartmentmap.v3ds");
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
        //renderer->loadV3DSFile(atlasfile);
        renderer->loadObjectFromFile(atlasfile.toStdString().c_str());
        updateTool();
        //POST_updateGL();
        update();
    }

}


