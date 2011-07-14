#include "Na3DWidget.h"
#include "v3d_core.h"
#include "../3drenderer/Renderer_gl2.h"
#include "RendererNeuronAnnotator.h"
#include <iostream>
#include <cmath>
#include <cassert>

using namespace std;

Na3DWidget::Na3DWidget(QWidget* parent)
        : V3dR_GLWidget(NULL, parent, "Title")
{
    _idep = new iDrawExternalParameter();
    _idep->image4d = NULL;
    _volCompress = false;
    resetView();
    // setVolCompress(false); // might look nicer?

    // This method for eliminating tearing artifacts works but is supposedly obsolete;
    // http://stackoverflow.com/questions/5174428/how-to-change-qglformat-for-an-existing-qglwidget-at-runtime
    QGLFormat glFormat(context()->format());
    glFormat.setDoubleBuffer(true); // attempt to reduce tearing on Mac
    setFormat(glFormat);

    rotateCursor = new QCursor(QPixmap(":/pic/rotate_icon.png"), 5, 5);
    // setMouseTracking(true); // for hover action in mouseMoveEvent()
    updateCursor();

    connect(&cameraModel, SIGNAL(focusChanged(const Vector3D&)),
             this, SLOT(updateFocus(const Vector3D&)));
    connect(&cameraModel, SIGNAL(scaleChanged(qreal)),
            this, SLOT(updateRendererZoomRatio(qreal)));
    connect(&cameraModel, SIGNAL(rotationChanged(const Rotation3D&)),
            this, SLOT(updateRotation(const Rotation3D&)));

    connect(&mouseClickManager, SIGNAL(singleClick(QPoint)),
            this, SLOT(onMouseSingleClick(QPoint)));
    connect(&mouseClickManager, SIGNAL(possibleSingleClickAlert()),
            this, SLOT(onPossibleSingleClickAlert()));
    connect(&mouseClickManager, SIGNAL(notSingleClick()),
            this, SLOT(onNotSingleClick()));
}

Na3DWidget::~Na3DWidget()
{
    delete _idep; _idep = NULL;
    if (rotateCursor) delete rotateCursor; rotateCursor = NULL;
}

void Na3DWidget::resetView()
{
    // qDebug() << "reset";
    cameraModel.setScale(1.0); // fit to window
    Vector3D newFocus = getDefaultFocus();
    // cerr << newFocus << __LINE__ << __FILE__ << endl;
    cameraModel.setFocus(newFocus); // center view
    cameraModel.setRotation(Rotation3D()); // identity rotation
    // Avoid crash before data are initialized
    if (!_idep) return;
    if (!_idep->image4d) return;
    update();
}

Vector3D Na3DWidget::getDefaultFocus() const {
    if (_idep && _idep->image4d)
        return Vector3D(_idep->image4d->getXDim()/2.0,
                             _idep->image4d->getYDim()/2.0,
                             _idep->image4d->getZDim()/2.0);
    else return Vector3D(0, 0, 0);
}

// Translate view by dx,dy screen pixels
void Na3DWidget::translateImage(int dx, int dy)
{
    if (!dx && !dy) return; // no motion
    // too big is probably an error
    if (std::abs(dx) > 2000) return;
    if (std::abs(dy) > 2000) return;
    Vector3D dFocus_eye(-dx  * flip_X,
                        -dy  * flip_Y,
                        -0.0 * flip_Z);
    Vector3D dFocus_obj = ~Rotation3D(mRot) * dFocus_eye;
    dFocus_obj /= getZoomScale();
    Vector3D newFocus = focus() + dFocus_obj;
    // cerr << newFocus << __LINE__ << __FILE__;
    cameraModel.setFocus(newFocus);
    update();
}

void Na3DWidget::updateCursor()
{
    // qDebug() << "updateCursor()";
    if (QApplication::keyboardModifiers() & Qt::ShiftModifier)
    { // shift-drag to translate
        if (QApplication::mouseButtons() & Qt::LeftButton)
            setCursor(Qt::ClosedHandCursor); // dragging
        else
            setCursor(Qt::OpenHandCursor); // hovering
    }
    else
    { // drag to rotate
        if (rotateCursor)
            setCursor(*rotateCursor);
        else
            setCursor(Qt::ArrowCursor); // whatever...
    }
}

/* virtual */
void Na3DWidget::keyPressEvent(QKeyEvent *e)
{
    updateCursor();
    V3dR_GLWidget::keyPressEvent(e);
}

/* virtual */
void Na3DWidget::keyReleaseEvent(QKeyEvent *e)
{
    updateCursor();
    V3dR_GLWidget::keyReleaseEvent(e);
}

// Drag mouse to rotate; shift-drag to translate.
void Na3DWidget::mouseMoveEvent(QMouseEvent * event)
{
    // updateCursor();

    // Maybe write status message when hovering with mouse.
    // (Before doing this, we would need to enable buttonless mouseMoveEvent propagation.)
    // Notice statement "setMouseTracking(true)" in constructor.
    if (Qt::NoButton == event->buttons())
    {
        // TODO hover
        bMouseIsDragging = false;
        return;
    }

    // I'm not sure what to do if user is dragging with non-left mouse button
    if (! (event->buttons() & Qt::LeftButton) )
    {
        bMouseIsDragging = false;
        V3dR_GLWidget::mouseMoveEvent(event); // use classic V3D behavior
        return;
    }

    // Now we know we're dragging
    int dx = event->pos().x() - oldDragX;
    int dy = event->pos().y() - oldDragY;
    oldDragX = event->pos().x();
    oldDragY = event->pos().y();
    // Don't do anything if this is the very first position of the drag
    if (!bMouseIsDragging) { // no effect until the second drag position
        bMouseIsDragging = true;
        return;
    }

    bMouseIsDragging = true;
    // Should not happen
    if (! (dx || dy)) // no motion?!?!
        return;

    // Shift-drag to translate
    if (event->modifiers() & Qt::ShiftModifier)
    {
        // qDebug() << "translate";
        translateImage(dx, dy);
        update();
        return;
    }
    // Regular drag to rotate
    else
    {
        // qDebug() << "rotate";
        bool bUseClassicV3dRotation = false;
        if (bUseClassicV3dRotation)
            V3dR_GLWidget::mouseMoveEvent(event); // regular V3D rotate behavior
        else {
            Rotation3D oldRotation(mRot);
            // std::cout << "old rotation = " << oldRotation << std::endl;
            // dragging across the entire viewport should be roughly 360 degrees rotation
            qreal rotAnglePerPixel = 2.0 * 3.14159 / ((width() + height()) / 2);
            qreal dragDistance = std::sqrt((double)(dx*dx + dy*dy));
            qreal rotAngle = dragDistance * rotAnglePerPixel;
            // std::cout << "Angle = " << rotAngle << std::endl;
            // rotation axis is perpendicular to the direction of the drag
            // I think we have to use "dx" instead of "-dx" to compensate for a y-flip deep in V3D
            UnitVector3D rotAxis(dy, dx, 0);
            // std::cout << "rotation axis = " << rotAxis << std::endl;
            Rotation3D dragRotation;
            dragRotation.setRotationFromAngleAboutUnitVector(rotAngle, rotAxis);
            // std::cout << "drag rotation = " << dragRotation << std::endl;
            Rotation3D newRotation = dragRotation * oldRotation;  // TODO check order, inversion
            // std::cout << "new rotation = " << newRotation << std::endl;
            cameraModel.setRotation(newRotation);

            update(); // since this mouseMoveEvent() method is interactive.
            return;
        }
    }
}

void Na3DWidget::mousePressEvent(QMouseEvent * event)
{
    mouseClickManager.mousePressEvent(event);
    updateCursor();
    // Remember this mouse position so we can keep track of the move vector
    oldDragX = event->pos().x();
    oldDragY = event->pos().y();
    bMouseIsDragging = true;

	// V3dR_GLWidget::mousePressEvent(event);
	if (event->button()==Qt::LeftButton)
	{
		lastPos = event->pos();
		t_mouseclick_left = clock();
	}

	if (event->button()==Qt::RightButton && renderer)
	{
                //if (renderer->hitPoint(event->x(), event->y()))  //pop-up menu or marker definition

            if(renderer->hitMenu(event->x(), event->y()))
            {
                updateTool();
            }

            V3dR_GLWidget::update();
	}
}

void Na3DWidget::mouseReleaseEvent(QMouseEvent * event)
{
    mouseClickManager.mouseReleaseEvent(event);
    // updateCursor();
    bMouseIsDragging = false;

	// V3dR_GLWidget::mouseReleaseEvent(event);
	int clicktime = fabs(clock() - t_mouseclick_left);
        bool left_quickclick = false;

	if(clicktime<1000)
                left_quickclick = true;

	//qDebug()<<"release ..."<<left_quickclick<<"time elapse ..."<<clicktime<<t_mouseclick;

	if (event->button()==Qt::RightButton && renderer) //right-drag
    {
                (renderer->movePen(event->x(), event->y(), false));
		updateTool();
                    V3dR_GLWidget::update();
    }

        if (event->button()==Qt::LeftButton && renderer && left_quickclick) // left click
    {
            // Moved single click response to MouseClickManager
            // highlightNeuronAtPosition(event->pos());
            // return;
    }

}

void Na3DWidget::onMouseSingleClick(QPoint pos)
{
    // qDebug() << "single left click!";
    highlightNeuronAtPosition(pos);
    updateCursor();
}

void Na3DWidget::onPossibleSingleClickAlert()
{
    // Immediate visual feedback that a click has been initiated
    // qDebug() << "possible single click";
    setCursor(Qt::BusyCursor);
}

void Na3DWidget::onNotSingleClick()
{
    updateCursor();
}

void Na3DWidget::highlightNeuronAtPosition(QPoint pos)
{
    // qDebug()<<"left click ... ...";
    XYZ loc = ((Renderer_tex2*)getRenderer())->selectPosition( pos.x(),  pos.y() );
    // select neuron: set x, y, z and emit signal
    // qDebug()<<"emit a signal ...";
    emit neuronSelected(loc.x, loc.y, loc.z);
    update(); // TODO - this update() should be postponed until the response to whatever happens after neuronSelected(...) completes.
}

// Zoom using mouse wheel
void Na3DWidget::wheelEvent(QWheelEvent * e)
{
    double oldZoom = cameraModel.scale(); // used for smart zooming
    // Update renderer camera aperture angle
    wheelZoom(e->delta());

    // Zoom, to be like in Google Earth, depends on cursor position
    // But (maybe) only shift focus when zooming IN.
    bool doSmartZoom = false; // perhaps smart zooming is just annoying
    if (doSmartZoom)
    {
        double factor = cameraModel.scale()/oldZoom;
        if (factor == 1.0) return;
        double scale = getZoomScale();
        double dx = e->pos().x() - width()/2.0;
        double dy = e->pos().y() - height()/2.0;
        Vector3D dFocus(dx * flip_X,
                        dy * flip_Y,
                        0  * flip_Z);
        dFocus *= (factor - 1.0) / scale;
        dFocus = ~(cameraModel.rotation()) * dFocus;
        Vector3D newFocus = cameraModel.focus() + dFocus;
        // cerr << newFocus << __LINE__ << __FILE__;
        cameraModel.setFocus(newFocus);
    }
    update();
}

// Move focus on double click
void Na3DWidget::mouseDoubleClickEvent(QMouseEvent * event)
{
    mouseClickManager.mouseDoubleClickEvent(event);
    updateCursor();
    if (event->button() != Qt::LeftButton) {
        V3dR_GLWidget::mouseDoubleClickEvent(event);
        return;
    }
    double dx = event->pos().x() - width()/2.0;
    double dy = event->pos().y() - height()/2.0;
    translateImage(-dx, -dy);
}

void Na3DWidget::resizeEvent(QResizeEvent * event)
{
    updateDefaultScale();
    resizeGL(width(), height());
}

void Na3DWidget::updateDefaultScale()
{
    float screenWidth = width();
    float screenHeight = height();
    if (!_idep) return;
    if (!_idep->image4d) return;
    float objectWidth = _idep->image4d->getXDim();
    float objectHeight = _idep->image4d->getYDim();

    if (screenWidth < 1) return;
    if (screenHeight < 1) return;
    if (objectWidth < 1) return;
    if (objectHeight < 1) return;
    float scaleX = screenWidth / objectWidth;
    float scaleY = screenHeight / objectHeight;
    defaultScale = scaleX > scaleY ? scaleY : scaleX;  // fit whole pixmap in window, with bars if necessary
    updateRendererZoomRatio(cameraModel.scale());
}

void Na3DWidget::updateRendererZoomRatio(qreal relativeScale)
{
    // qDebug() << "update zoom";
    RendererNeuronAnnotator* renderer = (RendererNeuronAnnotator*)getRenderer();

    float desiredPixelsPerImageVoxel = defaultScale * relativeScale;
    float desiredVerticalImageVoxelsDisplayed = height() / desiredPixelsPerImageVoxel;
    float desiredVerticalGlUnitsDisplayed = desiredVerticalImageVoxelsDisplayed * glUnitsPerImageVoxel();
    float desiredVerticalApertureInRadians = 2.0 * atan2(desiredVerticalGlUnitsDisplayed/2.0f, (float)renderer->getViewDistance());
    float desiredVerticalApertureInDegrees = desiredVerticalApertureInRadians * 180.0 / 3.14159;
    if (desiredVerticalApertureInDegrees > 180.0)
        desiredVerticalApertureInDegrees = 180.0; // gl limit
    float desiredZoomRatio = desiredVerticalApertureInDegrees / renderer->getViewAngle();
    renderer->setInternalZoomRatio(desiredZoomRatio);
    renderer->setupView(width(), height());
}

void Na3DWidget::updateFocus(const Vector3D& f)
{
    Rotation3D R_eye_obj(mRot);
    // _[xyz]Shift variables are relative to the center of the volume
    Vector3D shift_eye = R_eye_obj * (f - getDefaultFocus());
    // _xShift is in gl coordinates scaled by 100/1.4
    // see V3dR_GLWidget::paintGL() method in v3dr_glwidget.cpp
    shift_eye *= -glUnitsPerImageVoxel() * 100.0f/1.4f;
    _xShift = shift_eye.x();
    _yShift = shift_eye.y();
    _zShift = shift_eye.z();
    dxShift=dyShift=dzShift=0;
}

void Na3DWidget::updateRotation(const Rotation3D & newRotation)
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
    // Restore the focus point.  So the user does not get lost!
    Vector3D f = cameraModel.focus();
    // qDebug() << "focus = " << f.x() << ", " << f.y() << ", " << f.z();
    updateFocus(f);
}

void Na3DWidget::updateHighlightNeurons(bool b)
{
	enableMarkerLabel(b); // show markers' label

	updateWithTriView(); // update list markers and 3d viewer
}

void Na3DWidget::setGammaBrightness(qreal gamma)
{
    // I sort of hope this will address everyone's manual brightness needs.
    Renderer_gl2* renderer = (Renderer_gl2*)getRenderer();
    if (! renderer) return;
    brightnessCalibrator.setGamma(gamma);
    // cout << "gamma = " << gamma << endl;
    // Map input intensities to output intensities
    // using the renderer's "colormap" texture.
    for (int i_in = 0; i_in < 256; ++i_in)
    {
        unsigned char i_out = brightnessCalibrator.getCorrectedByte(i_in);
        for (int channel = 0; channel < FILL_CHANNEL; ++channel)
        {
            // Intensities are set in the alpha channel only
            // (i.e. not R, G, or B)
            renderer->colormap[channel][i_in].a = i_out;
        }
    }
    update();
}

bool Na3DWidget::loadMy4DImage(const My4DImage* my4DImage, const My4DImage* neuronMaskImage) {
    if (_idep==0) {
        _idep = new iDrawExternalParameter();
    } else {
        if (_idep->image4d!=0) {
            // delete _idep->image4d;
        }
    }
    // TODO - get some const correctness in here...
    _idep->image4d = const_cast<My4DImage*>(my4DImage);
    prepareImageData();
    updateDefaultScale();

    // if (neuronMaskImage) {
    //     if (!populateNeuronMask(neuronMaskImage)) return false;
    // }

    resetView();
    updateCursor();
    update();
    return true;
}

float Na3DWidget::glUnitsPerImageVoxel() const
{
    const RendererNeuronAnnotator* renderer = (const RendererNeuronAnnotator*)getRenderer();
    return renderer->glUnitsPerImageVoxel();
}

float Na3DWidget::getZoomScale() const
{ // in (vertical) screen pixels per image voxel at focus point
    // theta is half the true vertical view aperture in radians.
    const RendererNeuronAnnotator* renderer = (const RendererNeuronAnnotator*)getRenderer();
    float theta = (renderer->getZoomedPerspectiveViewAngle() / 2.0) * (3.14159 / 180.0);
    float screen_height_gl = 2.0 * renderer->getViewDistance() * std::tan(theta);
    float screen_pixels_per_gl_unit = height() / screen_height_gl;
    float answer = screen_pixels_per_gl_unit * glUnitsPerImageVoxel();
    // qDebug() << "screen pixels per image pixel = " << answer;
    // return answer * 6.28; // TODO - lose the mystery 6 tc image
    // return answer * 3.5; // TODO - fudge factor is ~3.5 for E1.tif
    return answer;
}

bool Na3DWidget::populateNeuronMaskAndReference(const My4DImage* neuronMaskImage, const My4DImage* referenceImage) {
    RendererNeuronAnnotator* rend = (RendererNeuronAnnotator*)getRenderer();
    if (!rend->populateNeuronMaskAndReference(neuronMaskImage, referenceImage)) {
        return false;
    }
    if (!rend->initializeTextureMasks()) {
        return false;
    }
    return true;
}

void Na3DWidget::prepareImageData() {
    // V3D crashes unless updateminmaxvalues is called
    _idep->image4d->updateminmaxvalues();
    if (!renderer) choiceRenderer();
    updateImageData();
}

void Na3DWidget::choiceRenderer() {
        // qDebug("Na3DWidget::choiceRenderer");
        _isSoftwareGL = false;
        GLeeInit();
        renderer = new RendererNeuronAnnotator(this);
}

// Draw a little 3D cross for testing
// In GL coordinates, where volume is contained within [-1,1]^3
void Na3DWidget::paintFiducial(const Vector3D& v) {
    qreal dd1 = 4.0 * glUnitsPerImageVoxel() / getZoomScale();
    qreal dd2 = 10.0 * glUnitsPerImageVoxel() / getZoomScale(); // 10 pixel crosshair
    qreal x0 = v.x();
    qreal y0 = v.y();
    qreal z0 = v.z();
    glBegin(GL_LINES);
      glVertex3f(x0-dd1,y0,z0);
      glVertex3f(x0-dd2,y0,z0);
      glVertex3f(x0+dd1,y0,z0);
      glVertex3f(x0+dd2,y0,z0);
      glVertex3f(x0,y0-dd1,z0);
      glVertex3f(x0,y0-dd2,z0);
      glVertex3f(x0,y0+dd1,z0);
      glVertex3f(x0,y0+dd2,z0);
      glVertex3f(x0,y0,z0-dd1);
      glVertex3f(x0,y0,z0-dd2);
      glVertex3f(x0,y0,z0+dd1);
      glVertex3f(x0,y0,z0+dd2);
    glEnd();
}

void Na3DWidget::paintGL()
{
    V3dR_GLWidget::paintGL();

    // Draw focus position to ensure it remains in center of screen,
    // for debugging
    if (bPaintCrosshair)
    {
        Vector3D focus0 = cameraModel.focus();
        // Convert from object coordinates to gl coordinates
        focus0 -= getDefaultFocus(); // reverse glTranslate(.5,.5,.5)
        focus0 *= glUnitsPerImageVoxel(); // scale to [-1,1]^3
        // Flip axes corresponding to v3dr_glwidget flip_X, flip_Y, flip_Z
        focus0.x() *= flip_X;
        focus0.y() *= flip_Y;
        focus0.z() *= flip_Z;
        // Don't allow other geometry to obscure the marker
        // glClear(GL_DEPTH_BUFFER_BIT); // Destroys depth buffer; probably too harsh
        glPushAttrib(GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT); // save color and depth test
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_LINE_SMOOTH);
        glColor3f(0,0,0); // black marker color
        glLineWidth(3.0);
        paintFiducial(focus0);
        glColor3f(1.0,1.0,0.7); // pale yellow marker color
        glLineWidth(1.5);
        paintFiducial(focus0);
        glPopAttrib();
    }
}

void Na3DWidget::annotationModelUpdate(QString updateType) {

    RendererNeuronAnnotator* ra = (RendererNeuronAnnotator*)renderer;
    QProgressDialog progressDialog( QString("Updating textures"), 0, 0, 100, this, Qt::Tool | Qt::WindowStaysOnTopHint);
    progressDialog.setAutoClose(true);
    QList<QString> list=updateType.split(QRegExp("\\s+"));

    if (updateType.startsWith("NEURONMASK_UPDATE")) {
        QString indexString=list.at(1);
        QString checkedString=list.at(2);
        int index=indexString.toInt();
        bool checked=(checkedString.toInt()==1);
        ra->updateCurrentTextureMask(index, (checked ? 1 : 0), progressDialog);

    } else if (updateType.startsWith("FULL_UPDATE")) {
        // Change requiring full reload of texture image stacks
        QList<int> tempList;
        for (int i=0;i<annotationSession->getMaskStatusList().size();i++) {
            if (annotationSession->neuronMaskIsChecked(i)) {
                tempList.append(i);
            }
        }
        QList<RGBA8*> overlayList;
        QList<bool> overlayStatusList=annotationSession->getOverlayStatusList();
        for (int i=0;i<overlayStatusList.size();i++) {
            if (overlayStatusList.at(i)) {
                overlayList.append(ra->getOverlayTextureByAnnotationIndex(i));
            }
        }
        ra->rebuildFromBaseTextures(tempList, overlayList, progressDialog);
    }
    ra->paint();
    update();
}

bool Na3DWidget::screenShot(QString filename)
{
    QImage image = this->grabFrameBuffer();

    if (image.save(filename, QFileInfo(filename).suffix().toStdString().c_str(), 100)) //uncompressed
    {
        printf("Successful to save screen-shot: [%s]\n",  filename.toAscii().data());
        return true;
    }
    else
    {
        printf("Failed to save screen-shot: [%s]\n",  filename.toAscii().data());
        return false;
    }
}

