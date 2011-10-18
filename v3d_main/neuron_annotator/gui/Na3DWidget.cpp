#include "Na3DWidget.h"
#include "v3d_core.h"
#include "../3drenderer/renderer_gl2.h"
#include "RendererNeuronAnnotator.h"
#include <iostream>
#include <cmath>
#include <cassert>

using namespace std;


Na3DWidget::Na3DWidget(QWidget* parent)
        : V3dR_GLWidget(NULL, parent, "Title")
        , incrementalDataColorModel(NULL)
        , bResizeEnabled(true)
        , viewerContextMenu(NULL)
        , neuronContextMenu(NULL)
        , bShowCornerAxes(true)
{
    if (renderer) {
        delete renderer;
        renderer = NULL;
    }
    _idep = new iDrawExternalParameter();
    _idep->image4d = NULL;
    resetView();
    // setVolCompress(false); // might look nicer?

    // This method for eliminating tearing artifacts works but is supposedly obsolete;
    // http://stackoverflow.com/questions/5174428/how-to-change-qglformat-for-an-existing-qglwidget-at-runtime
    // valgrind has some complaints about the context
    QGLFormat glFormat(context()->format());
    glFormat.setDoubleBuffer(true); // attempt to reduce tearing on Mac
    glFormat.setStereo(true);
    setFormat(glFormat);
    bHasQuadStereo = true;
    if (! context()->format().stereo())
        bHasQuadStereo = false;
    if (! context()->format().doubleBuffer())
        bHasQuadStereo = false;
    emit quadStereoSupported(bHasQuadStereo);

    rotateCursor = new QCursor(QPixmap(":/pic/rotate_icon.png"), 5, 5);
    // setMouseTracking(true); // for hover action in mouseMoveEvent()
    updateCursor();
    repaint();

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

void Na3DWidget::setStereoOff(bool b)
{
    if (b)
        setStereoMode(RendererNeuronAnnotator::STEREO_OFF);
}

void Na3DWidget::setStereoLeftEye(bool b)
{
    if (b)
        setStereoMode(RendererNeuronAnnotator::STEREO_LEFT_EYE);
}

void Na3DWidget::setStereoRightEye(bool b)
{
    if (b)
        setStereoMode(RendererNeuronAnnotator::STEREO_RIGHT_EYE);
}

void Na3DWidget::setStereoQuadBuffered(bool b)
{
    if (!b) return;
    if (!bHasQuadStereo) {
        emit quadStereoSupported(bHasQuadStereo);
        qDebug() << "Error: Quad buffered stereo is not supported on this computer.";
        return;
    }
    setStereoMode(RendererNeuronAnnotator::STEREO_QUAD_BUFFERED);
}

void Na3DWidget::setStereoAnaglyphRedCyan(bool b)
{
    if (b)
        setStereoMode(RendererNeuronAnnotator::STEREO_ANAGLYPH_RED_CYAN);
}

void Na3DWidget::setStereoAnaglyphGreenMagenta(bool b)
{
    if (b)
        setStereoMode(RendererNeuronAnnotator::STEREO_ANAGLYPH_GREEN_MAGENTA);
}

void Na3DWidget::setStereoRowInterleaved(bool b)
{
    if (b)
        setStereoMode(RendererNeuronAnnotator::STEREO_ROW_INTERLEAVED);
}


void Na3DWidget::setStereoMode(int m)
{
    if (! getRendererNa()) return;
    getRendererNa()->setStereoMode(m);
    update();
}

/* slot */
void Na3DWidget::clearLandmarks()
{
    if (! getRendererNa()) return;
    getRendererNa()->clearLandmarks();
    setShowMarkers(0);
    // updateHighlightNeurons();
}

/* slot */
void Na3DWidget::setLandmarks(const QList<ImageMarker> landmarks)
{
    qDebug() << "Na3DWidget::setLandmarks" << landmarks.size();
    if (! getRendererNa()) return;
    getRendererNa()->setLandmarks(landmarks);
    updateHighlightNeurons();
}

void Na3DWidget::annotationModelUpdate(QString updateType)
{
    QList<QString> list=updateType.split(QRegExp("\\s+"));

    if (updateType.startsWith("NEURONMASK_UPDATE")) {
        QString indexString=list.at(1);
        QString checkedString=list.at(2);
        int index=indexString.toInt();
        bool checked=(checkedString.toInt()==1);
        toggleNeuronDisplay(index, checked);
    }
    else if (updateType.startsWith("FULL_UPDATE")) {
        updateFullVolume();
    }
}

// Override updateImageData() to avoid that modal progress dialog
/* virtual */ /* public slot */
void Na3DWidget::updateImageData()
{
    if (!renderer) return;
    // TODO - push progress signals into renderer, where it might be possible to make them finer
    emit progressMessageChanged(QString("Updating 3D viewer data"));
    emit progressValueChanged(30);
    QCoreApplication::processEvents();
    makeCurrent();
    renderer->setupData(this->_idep);
    if (renderer->hasError()) {
        emit progressComplete(); // TODO - not strong enough
        return;
    }
    renderer->getLimitedDataSize(_data_size); //for update slider size
    emit progressValueChanged(70);
    QCoreApplication::processEvents();
    makeCurrent();
    renderer->reinitializeVol(renderer->class_version()); //100720
    if (renderer->hasError()) {
        emit progressComplete(); // TODO - not strong enough
        return;
    }
    emit progressValueChanged(100);
    emit progressComplete();
    // when initialize done, update status of control widgets
    //SEND_EVENT(this, QEvent::Type(QEvent_InitControlValue)); // use event instead of signal
    emit signalVolumeCutRange(); //100809

    update();
}

void Na3DWidget::resetView()
{
    // qDebug() << "reset";
    cameraModel.setScale(1.0); // fit to window
    Vector3D newFocus = getDefaultFocus();
    // cerr << newFocus << __LINE__ << __FILE__ << endl;
    cameraModel.setFocus(newFocus); // center view
    cameraModel.setRotation(Rotation3D()); // identity rotation
}

Vector3D Na3DWidget::getDefaultFocus() const
{
    Vector3D result(0, 0, 0);
    if (! dataFlowModel) return result;
    NaVolumeData::Reader volumeReader(dataFlowModel->getVolumeData());
    if (! volumeReader.hasReadLock()) return result;
    const Image4DProxy<My4DImage>& volumeProxy = volumeReader.getOriginalImageProxy();
    result = Vector3D(  volumeProxy.sx / 2.0
                      , volumeProxy.sy / 2.0
                      , volumeProxy.sz / 2.0);
    return result;
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
    if ( (QApplication::keyboardModifiers() & Qt::ShiftModifier) )
    { // shift drag to translate
        if (QApplication::mouseButtons() & Qt::LeftButton)
            setCursor(Qt::ClosedHandCursor); // dragging
        else
            setCursor(Qt::OpenHandCursor); // hovering
    }
    else
    { // regular-drag to rotate
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

    // I'm not sure what to do if user is dragging with non-left mouse button,
    // so revert to default V3D behavior.
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

    // shift-drag to translate
    if (  (event->modifiers() & (Qt::ShiftModifier)))
    {
        // qDebug() << "translate";
        translateImage(dx, dy);
        update();
        return;
    }
    // regular-drag to rotate
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
}

void Na3DWidget::mouseReleaseEvent(QMouseEvent * event)
{
    mouseClickManager.mouseReleaseEvent(event);
    updateCursor();
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

void Na3DWidget::setContextMenus(QMenu* viewerMenuParam, NeuronContextMenu* neuronMenuParam)
{
    if (viewerMenuParam) {
        viewerContextMenu = viewerMenuParam;
    }
    if (neuronMenuParam) {
        neuronContextMenu = neuronMenuParam;
    }
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));
}

/* slot */
void Na3DWidget::showContextMenu(QPoint point)
{
    // Myers index (GUI index) is one less than the volume label field index
    int neuronMyersIx = neuronAt(point);
    // qDebug() << "context menu for neuron" << neuronIx;
    // -1 means click outside of volume
    // 0 means background
    // >=1 means neuron fragment with  index neuronIx-1
    if (neuronMyersIx >= 0) { // neuron clicked
        neuronContextMenu->exec(mapToGlobal(point), neuronMyersIx);
    }
    else {
        // non neuron case
        viewerContextMenu->exec(mapToGlobal(point));
    }
}

// neuronAt() returns the index of a neuron shown at screen position pos.
// It is cobbled from methods:
//      Na3DWidget::highlightNeuronAtPosition(QPoint pos)
//      NeuronSelector::updateSelectedPosition()
//      NeuronSelector::getIndexSelectedNeuron()
int Na3DWidget::neuronAt(QPoint pos)
{
    int neuronIx = -1;
    if (!renderer) return neuronIx;
    XYZ loc = getRendererNa()->screenPositionToVolumePosition(pos);

    qreal xlc = loc.x + 0.5;
    qreal ylc = loc.y + 0.5;
    qreal zlc = loc.z + 0.5;

    // getIndexSelectedNeuron();
    int numNeuron = 0;
    // sum of pixels of each neuron mask in the cube
    std::vector<int> sum;
    {
        NeuronSelectionModel::Reader selectionReader(
                dataFlowModel->getNeuronSelectionModel());
        if (! selectionReader.hasReadLock()) return -1;

        NaVolumeData::Reader volumeReader(dataFlowModel->getVolumeData());
        if (! volumeReader.hasReadLock()) return -1;
        const Image4DProxy<My4DImage>& neuronProxy = volumeReader.getNeuronMaskProxy();

        const int sx = neuronProxy.sx;
        const int sy = neuronProxy.sy;
        const int sz = neuronProxy.sz;
        const int NB = 3;

        numNeuron = selectionReader.getMaskStatusList().size();

        sum.assign(numNeuron, 0);

        //
        V3DLONG xb = xlc-NB; if(xb<0) xb = 0;
        V3DLONG xe = xlc+NB; if(xe>sx) xe = sx-1;
        V3DLONG yb = ylc-NB; if(yb<0) yb = 0;
        V3DLONG ye = ylc+NB; if(ye>sy) ye = sy-1;
        V3DLONG zb = zlc-NB; if(zb<0) zb = 0;
        V3DLONG ze = zlc+NB; if(ze>sz) ze = sz-1;

        // const unsigned char *neuronMask = neuronProxy.img0->getRawData();
        const QList<bool>& maskStatusList=selectionReader.getMaskStatusList();

        for(V3DLONG k=zb; k<=ze; k++)
        {
                V3DLONG offset_k = k*sx*sy;
                for(V3DLONG j=yb; j<=ye; j++)
                {
                        V3DLONG offset_j = offset_k + j*sx;
                        for(V3DLONG i=xb; i<=xe; i++)
                        {
                                V3DLONG idx = offset_j + i;

                                // int cur_idx = neuronMask[idx] - 1; // value of mask stack - convert to 0...n-1 neuron index
                                int cur_idx = neuronProxy.value_at(i, j, k, 0) - 1;

                                if(cur_idx>=0 && maskStatusList.at(cur_idx)) // active masks
                                {
                                        sum[cur_idx]++;
                                }
                        }
                }
        }
    } // release locks

    int neuronSum=0;
    for(V3DLONG i=0; i<numNeuron; i++)
    {

        if(sum[i]>0 && sum[i]>neuronSum) {
            neuronSum=sum[i];
            neuronIx = i;
        }
    }

    if (neuronIx < 0)
        neuronIx = -1; // Index zero is a real fragment

    return neuronIx;
}

void Na3DWidget::highlightNeuronAtPosition(QPoint pos)
{
    qDebug() << "Na3DWidget::highlightNeuronAtPosition" << __FILE__ << __LINE__;
    if (!getRendererNa()) return;
    // avoid crash w/ NaN markerViewMatrix
    if (getRendererNa()->hasBadMarkerViewMatrix()) {
        return;
    }
    // qDebug()<<"left click ... ...";
    XYZ loc = getRendererNa()->screenPositionToVolumePosition(pos);
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
    if (bResizeEnabled)
    {
        updateDefaultScale();
        resizeGL(width(), height());
    }
}

/* virtual */
void Na3DWidget::resizeGL(int w, int h)
{
    if (bResizeEnabled)
        V3dR_GLWidget::resizeGL(w, h);
}

void Na3DWidget::updateDefaultScale()
{
    float screenWidth = width();
    float screenHeight = height();

    if (! dataFlowModel) return;
    NaVolumeData::Reader volumeReader(dataFlowModel->getVolumeData());
    if (! volumeReader.hasReadLock()) return;
    const Image4DProxy<My4DImage>& volumeProxy = volumeReader.getOriginalImageProxy();
    float objectWidth = volumeProxy.sx;
    float objectHeight = volumeProxy.sy;

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
    if (! getRendererNa()) return;
    // qDebug() << "update zoom";

    float desiredPixelsPerImageVoxel = defaultScale * relativeScale;
    float desiredVerticalImageVoxelsDisplayed = height() / desiredPixelsPerImageVoxel;
    float desiredVerticalGlUnitsDisplayed = desiredVerticalImageVoxelsDisplayed * glUnitsPerImageVoxel();
    float desiredVerticalApertureInRadians = 2.0 * atan2(desiredVerticalGlUnitsDisplayed/2.0f, (float)getRendererNa()->getViewDistance());
    float desiredVerticalApertureInDegrees = desiredVerticalApertureInRadians * 180.0 / 3.14159;
    if (desiredVerticalApertureInDegrees > 180.0)
        desiredVerticalApertureInDegrees = 180.0; // gl limit
    float desiredZoomRatio = desiredVerticalApertureInDegrees / getRendererNa()->getViewAngle();
    getRendererNa()->setInternalZoomRatio(desiredZoomRatio);

    bool bAutoClipNearFar = true; // TODO - expose this variable
    // set near and far clip planes
    if (bAutoClipNearFar)
        getRendererNa()->setDepthClip(2.0 * desiredVerticalGlUnitsDisplayed);

    getRendererNa()->setupView(width(), height());
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

void Na3DWidget::updateHighlightNeurons()
{
    setShowMarkers(2); // show markers
    enableMarkerLabel(false); // but don't show labels
}

void Na3DWidget::updateIncrementalColors()
{
    // QTime stopwatch;
    // stopwatch.start();
    // qDebug() << "Na3DWidget::updateIncrementalColors()" << __FILE__ << __LINE__;
    if (! incrementalDataColorModel) return;
    if (! renderer) return;
    {
        DataColorModel::Reader colorReader(*incrementalDataColorModel);
        if (incrementalDataColorModel->readerIsStale(colorReader)) return;

        const size_t numChannels = colorReader.getNumberOfDataChannels();
        if (numChannels > 4) {
            qDebug() << "Coloring more than 4 channels is not implemented.  Sorry.";
            // TODO
            return;
        }

        // Populate clever opengl color map texture
        for (int rgb = 0; rgb < 4; ++rgb) // loop red, then green, then blue
        {
            QRgb channelColor = colorReader.getChannelColor(rgb);
            Renderer_gl2* renderer = (Renderer_gl2*)getRenderer();
            for (int i_in = 0; i_in < 256; ++i_in)
            {
                // R/G/B color channel value is sum of data channel values
                qreal i_out_f = colorReader.getChannelScaledIntensity(rgb, i_in / 255.0) * 255.0;
                // qDebug() << rgb << i_in << i_out_f << colorReader.getChannelScaledIntensity(rgb, i_in / 255.0)
                //     << colorReader.getChannelVisibility(rgb);
                if (i_out_f < 0.0f) i_out_f = 0.0f;
                if (i_out_f > 255.0) i_out_f = 255.0;
                unsigned char i_out = (unsigned char) (i_out_f + 0.49999);
                // Intensities are set in the alpha channel only
                // (i.e. not R, G, or B)
                renderer->colormap[rgb][i_in].r = qRed(channelColor);
                renderer->colormap[rgb][i_in].g = qGreen(channelColor);
                renderer->colormap[rgb][i_in].b = qBlue(channelColor);
                renderer->colormap[rgb][i_in].a = i_out;
            }
        }
    } // release read lock
    // qDebug() << "Na3DWidget::updateIncrementalColors()" << __FILE__ << __LINE__;
    update();
    // repaint();
    // qDebug() << "3D viewer color update took" << stopwatch.elapsed() << "milliseconds"; // takes zero milliseconds
}

float Na3DWidget::glUnitsPerImageVoxel() const
{
    return getRendererNa()->glUnitsPerImageVoxel();
}

float Na3DWidget::getZoomScale() const
{ // in (vertical) screen pixels per image voxel at focus point
    // theta is half the true vertical view aperture in radians.
    if (! getRendererNa()) return 1.0;
    float theta = (getRendererNa()->getZoomedPerspectiveViewAngle() / 2.0) * (3.14159 / 180.0);
    float screen_height_gl = 2.0 * getRendererNa()->getViewDistance() * std::tan(theta);
    float screen_pixels_per_gl_unit = height() / screen_height_gl;
    float answer = screen_pixels_per_gl_unit * glUnitsPerImageVoxel();
    // qDebug() << "screen pixels per image pixel = " << answer;
    // return answer * 6.28; // TODO - lose the mystery 6 tc image
    // return answer * 3.5; // TODO - fudge factor is ~3.5 for E1.tif
    return answer;
}

void Na3DWidget::choiceRenderer()
{
    if (!getRendererNa()) {
        if (renderer) { // Renderer but not RendererNeuronAnnotator
            delete renderer;
            renderer = NULL;
        }
        // qDebug("Na3DWidget::choiceRenderer");
        _isSoftwareGL = false;
        makeCurrent();
        GLeeInit();
        renderer = new RendererNeuronAnnotator(this);
        connect(getRendererNa(), SIGNAL(progressValueChanged(int)),
                this, SIGNAL(progressValueChanged(int)));
        connect(getRendererNa(), SIGNAL(progressComplete()),
                this, SIGNAL(progressComplete()));
        connect(getRendererNa(), SIGNAL(progressMessageChanged(QString)),
                this, SIGNAL(progressMessageChanged(QString)));
        connect(getRendererNa(), SIGNAL(progressAborted(QString)),
                this, SIGNAL(progressAborted(QString)));
        connect(getRendererNa(), SIGNAL(alphaBlendingChanged(bool)),
                this, SLOT(setAlphaBlending(bool)));
        connect(getRendererNa(), SIGNAL(alphaBlendingChanged(bool)),
                this, SLOT(update()));
        connect(this, SIGNAL(alphaBlendingChanged(bool)),
                getRendererNa(), SLOT(setAlphaBlending(bool)));
        connect(this, SIGNAL(showCornerAxesChanged(bool)),
                getRendererNa(), SLOT(setShowCornerAxes(bool)));
        connect(getRendererNa(), SIGNAL(showCornerAxesChanged()),
                this, SLOT(setShowCornerAxes(bool)));
        getRendererNa()->setShowCornerAxes(bShowCornerAxes);
    }
}

/* slot */
void Na3DWidget::setAlphaBlending(bool b)
{
    if (! getRendererNa()) return;
    getRendererNa()->setAlphaBlending(b);
}

/* slot */
void Na3DWidget::setShowCornerAxes(bool b)
{
    if (b == bShowCornerAxes) return;
    bShowCornerAxes = b;
    emit showCornerAxesChanged(bShowCornerAxes);
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

/* virtual */
void Na3DWidget::setDataFlowModel(const DataFlowModel& dataFlowModelParam)
{
    NaViewer::setDataFlowModel(dataFlowModelParam);
    connect(&dataFlowModelParam.getNeuronSelectionModel(), SIGNAL(initialized()),
          this, SLOT(onVolumeDataChanged()));

    connect(this, SIGNAL(neuronClearAll()), &dataFlowModelParam.getNeuronSelectionModel(), SLOT(clearAllNeurons()));
    connect(this, SIGNAL(neuronClearAllSelections()), &dataFlowModelParam.getNeuronSelectionModel(), SLOT(clearSelection()));
    connect(this, SIGNAL(neuronIndexChanged(int)), &dataFlowModelParam.getNeuronSelectionModel(), SLOT(selectExactlyOneNeuron(int)));

    connect(&dataFlowModel->getNeuronSelectionModel(), SIGNAL(selectionChanged()),
            this, SLOT(onNeuronSelectionChanged()));

    // Fast-but-approximate color update
    incrementalDataColorModel = &dataFlowModelParam.getFast3DColorModel();
    connect(incrementalDataColorModel, SIGNAL(dataChanged()),
            this, SLOT(updateIncrementalColors()));
}

void Na3DWidget::toggleNeuronDisplay(NeuronSelectionModel::NeuronIndex index, bool checked)
{
    // Don't update single neuron if full update is underway
    if (updateFullVolumeStatus.running()) return;

    bool needFullUpdate = false;
    {
        // Coalesce multiple toggle events
        SlotMerger toggleNeuronMerger(toggleNeuronDisplayStatus);
        if (! toggleNeuronMerger.shouldRun())
            return; // first call is already running
        if (toggleNeuronMerger.skippedCallCount() > 0) {
            needFullUpdate = true; // some calls were discarded, so do full update
        }
        else { // just one call, perform clever single neuron update
            emit progressMessageChanged(QString("Updating textures"));
            QCoreApplication::processEvents();
            getRendererNa()->updateCurrentTextureMask(index, (checked ? 1 : 0));
            // More toggles might have happened during the update
            if (toggleNeuronMerger.skippedCallCount() > 0) {
                needFullUpdate = true;
            }
            else {
                getRendererNa()->paint();
                update();
            }
        }
    } // pop SlotMerger off stack before full update;
    if (needFullUpdate) {
        updateFullVolume();
    }
}

/* slot */
void Na3DWidget::onNeuronSelectionChanged() { // highlight selected neurons
    // TODO - for now we are still highlighting just one neuron (at most)
    if (!dataFlowModel) return;
    int neuronIx = -1;
    {
        NeuronSelectionModel::Reader selectionReader(dataFlowModel->getNeuronSelectionModel());
        if (! selectionReader.hasReadLock()) return;
        const QList<bool>& selectList = selectionReader.getNeuronSelectList();
        for (int i = 0; i < selectList.size(); ++i) {
            if (selectList[i]) {
                neuronIx = i;
                break; // just the first neuron for now
            }
        }
    }
    if (neuronIx < 0) {
        qDebug() << "clearing landmarks" << __FILE__ << __LINE__;
        clearLandmarks(); // should remove selection
    }
    else {

    }
}

void Na3DWidget::onVolumeDataChanged()
{
    init_members();
    if (_idep==0) {
        _idep = new iDrawExternalParameter();
    }

    RendererNeuronAnnotator* rend = NULL;
    bool bSucceeded = true;
    {
        NaVolumeData::Reader volumeReader(dataFlowModel->getVolumeData());
        if (! volumeReader.hasReadLock()) return;
        const Image4DProxy<My4DImage>& imgProxy = volumeReader.getOriginalImageProxy();

        // TODO - get some const correctness in here...
        // TODO - wean from _idep->image4d
        _idep->image4d = imgProxy.img0;
        if (renderer) {
            delete renderer;
            renderer = NULL;
        }
        choiceRenderer();
        settingRenderer();
        updateImageData();
        updateDefaultScale();
        resetView();
        updateCursor();
        rend = getRendererNa();
        if (! getRendererNa()->populateNeuronMaskAndReference(volumeReader)) {
            qDebug() << "RendererNeuronAnnotator::populateNeuronMaskAndReference() failed";
            bSucceeded = false;
        }
    } // release locks before emit
    if (! bSucceeded)
    {
        emit progressAborted("");
        return;
    }
    makeCurrent(); // Make sure subsequent OpenGL calls go here. (might make no difference here)
    if (rend != getRendererNa()) {
        emit progressAborted("");
        return; // stale
    }
    emit progressComplete();
    if (! getRendererNa()->initializeTextureMasks()) {
        qDebug() << "RendererNeuronAnnotator::initializeTextureMasks() failed";
        emit progressAborted("");
        return;
    }
    resetVolumeBoundary();
    setThickness(dataFlowModel->getZRatio());
    update();
}

void Na3DWidget::resetVolumeBoundary()
{
    NaVolumeData::Reader volumeReader(dataFlowModel->getVolumeData());
    if (! volumeReader.hasReadLock()) return;
    const Image4DProxy<My4DImage>& imgProxy = volumeReader.getOriginalImageProxy();
    setXCut0(0); setXCut1(imgProxy.sx - 1);
    setYCut0(0); setYCut1(imgProxy.sy - 1);
    setZCut0(0); setZCut1(imgProxy.sz - 1);
    // Sometimes renderer is not in sync with 3DWidget; then above calls might short circuit as "no-change"
    if (renderer) {
        renderer->setXCut0(0); renderer->setXCut1(imgProxy.sx - 1);
        renderer->setYCut0(0); renderer->setYCut1(imgProxy.sy - 1);
        renderer->setZCut0(0); renderer->setZCut1(imgProxy.sz - 1);
    }
}

void Na3DWidget::updateFullVolume()
{
    // Coalesce queued calls to updateFullVolume
    SlotMerger updateFullMerger(updateFullVolumeStatus);
    if (! updateFullMerger.shouldRun())
        return; // first call is already running
    emit progressMessageChanged(QString("Updating all textures"));
    if (tryUpdateFullVolume())
        emit progressComplete();
    else {
        emit progressAborted("");
        return;
    }
    getRendererNa()->paint();
    update();
}

bool Na3DWidget::tryUpdateFullVolume()
{
    RendererNeuronAnnotator* ra = getRendererNa(); // Prepare to abort if renderer pointer gets stale later
    if (! getRendererNa()) return false;
    if (! dataFlowModel) return false;
    {
        // TODO - refresh these read locks frequently!
        NaVolumeData::Reader volumeReader(dataFlowModel->getVolumeData());
        if (! volumeReader.hasReadLock()) return false;
        NeuronSelectionModel::Reader selectionReader(dataFlowModel->getNeuronSelectionModel());
        if (! selectionReader.hasReadLock()) return false;

        // Change requiring full reload of texture image stacks
        QList<int> tempList;
        for (int i=0;i<selectionReader.getMaskStatusList().size();i++) {
            if (selectionReader.neuronMaskIsChecked(i)) {
                tempList.append(i);
            }
        }
        QList<RGBA8*> overlayList;
        const QList<bool> overlayStatusList=selectionReader.getOverlayStatusList();
        for (int i=0;i<overlayStatusList.size();i++) {
            if (overlayStatusList.at(i)) {
                // renderer can get out of sync...
                RGBA8* texture = getRendererNa()->getOverlayTextureByAnnotationIndex(i);
                if (! texture) return false; // something not initialized
                overlayList.append(texture);
            }
        }
        QCoreApplication::processEvents(); // let gui catch up
        if (ra != getRendererNa()) return false; // renderer changed during processEvents
        // make sure readers are OK
        if (! volumeReader.refreshLock()) return false;
        if (! selectionReader.refreshLock()) return false;
        makeCurrent();
        getRendererNa()->rebuildFromBaseTextures(tempList, overlayList);
    } // release locks
    if (ra != getRendererNa()) return false;
    return true;
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

void Na3DWidget::setXCutLock(int b)
{
    if (b)	dxCut = _xCut1-_xCut0;
    else    dxCut = 0;
    lockX = b? 1:0;
}
void Na3DWidget::setYCutLock(int b)
{
    if (b)	dyCut = _yCut1-_yCut0;
    else    dyCut = 0;
    lockY = b? 1:0;
}
void Na3DWidget::setZCutLock(int b)
{
    if (b)	dzCut = _zCut1-_zCut0;
    else    dzCut = 0;
    lockZ = b? 1:0;
}
