#include "CompartmentMapWidget.h"

CompartmentMapWidget::CompartmentMapWidget(QWidget* parent): V3dR_GLWidget(NULL, parent, "Compartment Map")
{
    //m_lw = new QListWidget(parent);
}

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
    else if(QFile::exists(qApp->applicationDirPath().append("/").append(flybrainatlas)))
    {
        atlasfile = qApp->applicationDirPath().append("/").append(flybrainatlas);
    }
    else if(QFile::exists(qApp->applicationDirPath().append("/../../../").append(flybrainatlas)))
    {
        atlasfile = qApp->applicationDirPath().append("/../../../").append(flybrainatlas);
    }

    if(QFile::exists(atlasfile))
    {
        ((Renderer_gl1 *)renderer)->loadV3DSFile(atlasfile);
        //((Renderer_gl1 *)renderer)->loadObjectFromFile(atlasfile.toStdString().c_str());

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

    renderer = new Renderer_gl1(this);

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

        //qDebug()<<"label surf ..."<<((Renderer_gl1 *)renderer)->listLabelSurf.size();
        //qDebug()<<"triangle ..."<<((Renderer_gl1 *)renderer)->list_listTriangle.size();
        //qDebug()<<"glist label ..."<<((Renderer_gl1 *)renderer)->list_glistLabel.size();

        listLabelSurf = ((Renderer_gl1 *)renderer)->getListLabelSurf();

        pCompartmentComboBox->addItem("All On", true);
        pCompartmentComboBox->addItem("All Off", false);
        for(int i=0; i<listLabelSurf.size(); i++)
        {
            pCompartmentComboBox->addItem(listLabelSurf[i].name, true);
        }

        update();
    }

}
void CompartmentMapWidget::resizeGL(int width, int height){
    V3dR_GLWidget::resizeGL(width, height);
}
void CompartmentMapWidget::paintGL(){
    V3dR_GLWidget::paintGL();
}

void CompartmentMapWidget::setRotation(const Rotation3D& newRotation)
{
    // Update mRot cached opengl matrix
    newRotation.setGLMatrix(mRot);
    // Update _xRot, _yRot, _zRot Euler angles
    Vector3D eulerAngles = newRotation.convertBodyFixedXYZRotationToThreeAngles();
    eulerAngles *= 180.0 / 3.14159; // convert radians to degrees
    _xRot = eulerAngles[0];
    _yRot = eulerAngles[1];
    _zRot = eulerAngles[2];
    while(_xRot < 0.0) _xRot += 360.0;
    while(_yRot < 0.0) _yRot += 360.0;
    while(_zRot < 0.0) _zRot += 360.0;
    // Yes, this is an absolute orientation.  I don't even want to think about
    // whatever that non-absolute case entails.
    _absRot = true;
    dxRot = dyRot = dzRot = 0;
    update();
}

void CompartmentMapWidget::setFocus(const Vector3D& f)
{
    Rotation3D R_eye_obj(mRot);
    // _[xyz]Shift variables are relative to the center of the volume
    Vector3D defaultFocus(0, 0, 0);
    Vector3D shift_eye = R_eye_obj * (f - defaultFocus);
    // _xShift is in gl coordinates scaled by 100/1.4
    // see V3dR_GLWidget::paintGL() method in v3dr_glwidget.cpp
    double glUnitsPerImageVoxel = 0.01; // TODO - set this correctly
    shift_eye *= -glUnitsPerImageVoxel * 100.0f/1.4f;
    _xShift = shift_eye.x();
    _yShift = shift_eye.y();
    _zShift = shift_eye.z();
    dxShift=dyShift=dzShift=0;
    update();
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
    qDebug()<<"switch status of compartment ... #"<<num;

    if(num<0 || num>listLabelSurf.size()+1) return;

    listLabelSurf = ((Renderer_gl1 *)renderer)->getListLabelSurf();

    if(num==0) // all on
    {
        //gui
        pCompartmentComboBox->setItemData(0, true);
        pCompartmentComboBox->setItemData(1, false);
        for(int i=0; i<listLabelSurf.size(); i++)
        {
            pCompartmentComboBox->setItemData(i+2, true);
        }

        //widget
        for(int i=0; i<listLabelSurf.size(); i++)
        {
            listLabelSurf[i].on = true;
        }
    }
    else if(num==1) // all off
    {
        //gui
        pCompartmentComboBox->setItemData(0, false);
        pCompartmentComboBox->setItemData(1, true);
        for(int i=0; i<listLabelSurf.size(); i++)
        {
            pCompartmentComboBox->setItemData(i+2, false);
        }

        //widget
        for(int i=0; i<listLabelSurf.size(); i++)
        {
            listLabelSurf[i].on = false;
        }
    }
    else
    {
        //gui
        pCompartmentComboBox->setItemData(0, false);
        pCompartmentComboBox->setItemData(1, false);

        if(pCompartmentComboBox->itemData(num).toBool())
            pCompartmentComboBox->setItemData(num, false);
        else
            pCompartmentComboBox->setItemData(num, true);

        //widget
        listLabelSurf[num-2].on = !(listLabelSurf[num-2].on);
    }

    ((Renderer_gl1 *)renderer)->setListLabelSurf(listLabelSurf);

    update();

    // test codes
    emit viscomp3dview(listLabelSurf);
}

void CompartmentMapWidget::setComboBox(CompartmentMapComboBox *compartmentComboBox)
{
    pCompartmentComboBox = compartmentComboBox;

    QScrollBar *comboScrollBar = new QScrollBar(this);
    pCompartmentComboBox->view()->setVerticalScrollBar(comboScrollBar);

    //QString styleSheet = "QComboBox QListView{color:black; background-color:white; selection-color:yellow; selection-background-color:blue;}";
    //pCompartmentComboBox->setStyleSheet(styleSheet);

}
