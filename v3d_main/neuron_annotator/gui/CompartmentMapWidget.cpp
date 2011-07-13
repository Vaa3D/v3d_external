#include "CompartmentMapWidget.h"

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
    qDebug()<<"CompartmentMapWidget initializeGL ... ...";

    _isSoftwareGL = false;
    GLeeInit();

    renderer = new Renderer_tex2(this);

    // settings
    renderer->bShowBoundingBox = false; //
    renderer->bShowAxes        = false;

    renderer->tryTexCompress = false; // texture
    renderer->tryTex3D       = false;
    renderer->tryTexNPT      = false;
    renderer->tryTexStream   = true;

    renderer->lineType   = false; // swc

    // prepare
    if (renderer)
    {
        loadAtlas();
        if(renderer->hasError())	POST_CLOSE(this);

        //qDebug()<<"label surf ..."<<((Renderer_tex2 *)renderer)->listLabelSurf.size();
        //qDebug()<<"triangle ..."<<((Renderer_tex2 *)renderer)->list_listTriangle.size();
        //qDebug()<<"glist label ..."<<((Renderer_tex2 *)renderer)->list_glistLabel.size();

        listLabelSurf = ((Renderer_tex2 *)renderer)->getListLabelSurf();

        compartmentList.clear();
        compartmentList<<QString("All On")<<QString("All Off");
        for(int i=0; i<listLabelSurf.size(); i++)
        {
            compartmentList<<listLabelSurf[i].name;
        }
        pCompartmentComboBox->addItems(compartmentList);

        update();
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
    float d = (event->delta())/100;  //
#define MOUSE_ZOOM(dz)    (int(dz*4* MOUSE_SENSITIVE));

    int zoomStep = MOUSE_ZOOM(d);

    setZoom((-zoomStep) + _zoom); // scroll down to zoom in

    event->accept();
}

void CompartmentMapWidget::switchCompartment(int num)
{
    qDebug()<<"CompartmentMapWidget num ... ..."<<num;

    listLabelSurf = ((Renderer_tex2 *)renderer)->getListLabelSurf();

    if(num==0) // all on
    {
        for(int i=0; i<listLabelSurf.size(); i++)
        {
            listLabelSurf[i].on = true;
        }
    }
    else if(num==1) // all off
    {
        for(int i=0; i<listLabelSurf.size(); i++)
        {
            listLabelSurf[i].on = false;
        }
    }
    else
    {
        listLabelSurf[num-2].on = !(listLabelSurf[num-2].on);
    }

    ((Renderer_tex2 *)renderer)->setListLabelSurf(listLabelSurf);

    update();
}

void CompartmentMapWidget::setComboBox(QComboBox *compartmentComboBox)
{
    pCompartmentComboBox = compartmentComboBox;
}
