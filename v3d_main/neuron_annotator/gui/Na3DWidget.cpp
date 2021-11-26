#include "../3drenderer/GLee2glew.h" //2020-2-10 RZC

#include "Na3DWidget.h"
#include "../render/CubeTestActorGL.h"
#include "v3d_core.h"
#include "../3drenderer/renderer_gl2.h"
#include "RendererNeuronAnnotator.h"
#include "../DataFlowModel.h"
#include <iostream>
#include <cmath>
#include <cassert>
#include <stdint.h>
#include "../data_model/VolumeTexture.h"
#include "../data_model/DataColorModel.h"
#include "../utility/FooDebug.h"
#include "../render/CameraTransformGL.h"

using namespace std;
using namespace jfrc;

Na3DWidget::Na3DWidget(QWidget* parent)
        : V3dR_GLWidget(NULL, parent, "Title")
        , incrementalDataColorModel(NULL)
        , bResizeEnabled(true)
        , viewerContextMenu(NULL)
        , neuronContextMenu(NULL)
        , bShowCornerAxes(true)
        , bClickIsWaiting(false)
        , undoStack(NULL)
        , cachedRelativeScale(1.0)
        , stereo3DMode(jfrc::STEREO_OFF)
        , bStereoSwapEyes(false)
        , defaultVolumeTextureId(0)
        , defaultColormapTextureId(0)
        , defaultVisibilityTextureId(0)
        , defaultLabelTextureId(0)
        , bGLIsInitialized(false)
        , cachedDefaultFocusIsDirty(true)
        , bLabelTextureIsDirty(false)
        , bVisibilityTextureIsDirty(false)
        , bColorMapTextureIsDirty(false)
        , bSignalTextureIsDirty(false)
        , globalScreenPosX(0)
        , globalScreenPosY(0)
        , xVoxelSizeInMicrons(0)
        , bPaintScaleBar(true)
{
    if (renderer) {
        delete renderer;
        renderer = NULL;
    }
    _idep = new iDrawExternalParameter();
    qDebug() << "Na3DWidget constructor; _idep = " << _idep;
    _idep->image4d = NULL;
    resetView();
    //setVolCompress(false); // might look nicer?

    setAutoFillBackground(false); // needed for combining 2D and 3D rendering in paintEvent()

    // This method for eliminating tearing artifacts works but is supposedly obsolete;
    // http://stackoverflow.com/questions/5174428/how-to-change-qglformat-for-an-existing-qglwidget-at-runtime
    // valgrind has some complaints about the context
#ifdef ENABLE_STEREO
    QGLFormat * glFormat = new QGLFormat(format());
    glFormat->setDoubleBuffer(true);
    glFormat->setStereo(true);
    setFormat(*glFormat);
#endif

    bHasQuadStereo = true;
    if (! context()->format().stereo())
        bHasQuadStereo = false;

#if defined(USE_Qt5)
    if ( format().swapBehavior() != QSurfaceFormat::SwapBehavior::DoubleBuffer)
#else
    if (! context()->format().doubleBuffer())
#endif
        bHasQuadStereo = false;
    if(bHasQuadStereo) {
        // qDebug() << "Quad buffer stereo 3D is supported";
    }
    else {
        // qDebug() << "Quad buffer stereo 3D is NOT supported";
    }
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

    widgetStopwatch.start();
    invalidate();

    // Insert test actor to develop new ActorGL system:
    // opaqueActors.push_back( ActorPtr(new CubeTestActorGL()) );
}

Na3DWidget::~Na3DWidget()
{
    makeCurrent();
    if (_idep != NULL) {
        _idep->data4dp = NULL; // NA images are managed elsewhere
        delete _idep;
        _idep = NULL;
    }
    if (rotateCursor) delete rotateCursor; rotateCursor = NULL;
    glDeleteTextures(1, &defaultVolumeTextureId);
    glDeleteTextures(1, &defaultColormapTextureId);
    glDeleteTextures(1, &defaultVisibilityTextureId);
    glDeleteTextures(1, &defaultLabelTextureId);
    defaultVolumeTextureId =
            defaultColormapTextureId =
            defaultVisibilityTextureId =
            defaultLabelTextureId = 0;
}

/* slot */
void Na3DWidget::clearImage() {
    if (_idep != NULL)
        _idep->data4dp = NULL;
    RendererNeuronAnnotator * rna = getRendererNa();
    if (rna != NULL)
        rna->clearImage();
}

/* slot */
bool Na3DWidget::loadSignalTexture3D(size_t w, size_t h, size_t d, const uint32_t* texture_data)
{
    // fooDebug() << "Na3DWidget::loadSignalTexture3D()" << w << h << d << __FILE__ << __LINE__;
    if (NULL == texture_data)
        return false;
    QElapsedTimer stopwatch;
    stopwatch.start();

    if (_idep==0) {
        _idep = new iDrawExternalParameter();
        _idep->data4dp = NULL;
    }

    makeCurrent();

    /*
    if (NULL != renderer) {
        delete renderer;
        renderer = NULL;
    }
    */

    RendererNeuronAnnotator* ra = getRendererNa();
    if (NULL == ra)
    {
        choiceRenderer();
        ra = getRendererNa();
        if (NULL == ra)
            return false;
        ra->loadShader();
    }
    // ra->setSingleVolumeDimensions(w, h, d);

    {
        // check for previous errors
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            qDebug() << "OpenGL error" << err << __FILE__ << __LINE__;
            return false;
        }
    }
    // Upload volume image as an OpenGL 3D texture
    // glPushAttrib(GL_TEXTURE_BIT | GL_ENABLE_BIT); // remember previous OpenGL state
    glActiveTextureARB(GL_TEXTURE0_ARB); // multitexturing index, because there are other textures
    if (0 == defaultVolumeTextureId)
        glGenTextures(1, &defaultVolumeTextureId); // allocate a handle for this texture
    assert(0 != defaultVolumeTextureId);
    glEnable(GL_TEXTURE_3D); // we are using a 3D texture
    glBindTexture(GL_TEXTURE_3D, defaultVolumeTextureId); // make this the current texture
    // Black/zero beyond edge of texture
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    // Interpolate between texture values
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    {
        // check for new errors
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            qDebug() << "OpenGL error" << err << __FILE__ << __LINE__;
            // glPopAttrib();
            return false;
        }
    }
    // qDebug() << width << height << depth << (long)texture_data;
    // Load the data onto video card
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexImage3DEXT(GL_TEXTURE_3D,
        0, ///< mipmap level; zero means base level
        GL_RGBA8, ///< texture format, in bytes per pixel
        w,
        h,
        d,
        0, // border
        GL_BGRA, // image format
        GL_UNSIGNED_INT_8_8_8_8_REV, // image type
        (GLvoid*)texture_data);
    // glPopAttrib(); // restore OpenGL state
    {
        // check for new errors
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            qDebug() << "OpenGL error" << err << __FILE__ << __LINE__;
            return false;
        }
    }
    glDisable(GL_TEXTURE_3D); // we were using a 3D texture

    ra->set3dTextureMode(defaultVolumeTextureId);

    ra->setupData(_idep);

    // doneCurrent();

    // qDebug() << "Uploading 3D volume texture took"
    //        << stopwatch.elapsed()
    //        << "milliseconds";
    emit signalTextureLoaded();
    update();

    return true;
}

/* virtual */
void Na3DWidget::preparingRenderer() // renderer->setupData & init, 100719 extracted to a function
{
        // qDebug() << "Na3DWidget::V3dR_GLWidget::preparingRenderer" << __FILE__ << __LINE__;

        if (_isSoftwareGL) setRenderMode_Cs3d(true); //090724 set renderer mode before paint

        //=============================================================================
        {
                if (renderer)
                {
                        renderer->setupData(this->_idep);
                        if (renderer->hasError())	POST_CLOSE(this);
                        renderer->getLimitedDataSize(_data_size); //for update slider size
                }

                if (renderer)
                {
                        renderer->initialize(renderer->class_version()); //090705 RZC
                        if (renderer->hasError())	POST_CLOSE(this);
                }
        }
        //=============================================================================

        // when initialize done, update status of control widgets
        QCoreApplication::sendEvent(this, new QEvent(QEvent::Type(QEvent_InitControlValue)));
        if (_isSoftwareGL)
        {
                emit changeDispType_cs3d(true);  // 081215, set check-box must after changeVolumeCutRange()
        }
        if (supported_TexCompression())
        {
                // qDebug("	GL texture compression supported, enable texture compression function");
                emit changeEnableVolCompress(true);
        }
        if (supported_GLSL())
        {
                // qDebug("	GL shading language supported, enable volume colormap function");
                emit changeEnableVolColormap(true);
        }

        QCoreApplication::postEvent(this, new QEvent(QEvent::Type(QEvent_OpenFiles))); // POST_EVENT(this, QEvent::Type(QEvent_OpenFiles));
        QCoreApplication::postEvent(this, new QEvent(QEvent::Type(QEvent_Ready))); // POST_EVENT(this, QEvent::Type(QEvent_Ready)); //081124
}

/* slot */
bool Na3DWidget::loadLabelTexture3D(size_t w, size_t h, size_t d, const uint16_t* texture_data)
{
    // qDebug() << "Na3DWidget::loadLabelTexture3D()" << w << h << d;
    if (NULL == texture_data)
        return false;
    QTime stopwatch;
    stopwatch.start();
    makeCurrent();

    // check for previous errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        qDebug() << "OpenGL error" << err << __FILE__ << __LINE__;
        return false;
    }

    // Upload Label image as an OpenGL 3D texture
    // glPushAttrib(GL_TEXTURE_BIT | GL_ENABLE_BIT); // remember previous OpenGL state
    glActiveTextureARB(GL_TEXTURE3_ARB); // multitexturing index, because there are other textures
    if (0 == defaultLabelTextureId)
        glGenTextures(1, &defaultLabelTextureId); // allocate a handle for this texture
    assert(0 != defaultLabelTextureId);
    // label texture is in unit 3
    glEnable(GL_TEXTURE_3D); // we are using a 3D texture
    glBindTexture(GL_TEXTURE_3D, defaultLabelTextureId); // make this the current texture
    // Black/zero beyond edge of texture
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    // GL_NEAREST ensures that we get an actual non-interpolated label value.
    // Interpolation would be crazy wrong.
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    {
        // check for new errors
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            qDebug() << "OpenGL error" << err << __FILE__ << __LINE__;
            // glPopAttrib();
            return false;
        }
    }
    // qDebug() << width << height << depth << (long)texture_data;
    // Load the data onto video card
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexImage3DEXT(GL_TEXTURE_3D,
                 0, // mipmap level
                 GL_INTENSITY16, // texture format
                 w,
                 h,
                 d,
                 0, // border
                 GL_RED, // image format
                 GL_UNSIGNED_SHORT, // image type
        (GLvoid*)texture_data);
    glDisable(GL_TEXTURE_3D);
    glActiveTextureARB(GL_TEXTURE0_ARB);
    // glPopAttrib(); // restore OpenGL state
    {
        // check for new errors
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            qDebug() << "OpenGL error" << err << __FILE__ << __LINE__;
            return false;
        }
    }

    // qDebug() << "Uploading 3D label texture took"
    //         << stopwatch.elapsed()
    //         << "milliseconds";
    emit labelTextureLoaded();
    update();

    return true;
}

/* slot */
bool Na3DWidget::loadVisibilityTexture2D(const uint32_t* texture_data)
{
    // qDebug() << "Na3DWidget::loadVisibilityTexture2D()" << __FILE__ << __LINE__;
    if (NULL == texture_data)
        return false;
    QTime stopwatch;
    stopwatch.start();
    makeCurrent();

    // check for previous errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        qDebug() << "OpenGL error" << err << __FILE__ << __LINE__;
        return false;
    }

    // Upload Visibility image as an OpenGL 2D texture
    // glPushAttrib(GL_TEXTURE_BIT | GL_ENABLE_BIT); // remember previous OpenGL state
    glActiveTextureARB(GL_TEXTURE2_ARB); // multitexturing index, because there are other textures
    if (0 == defaultVisibilityTextureId)
        glGenTextures(1, &defaultVisibilityTextureId); // allocate a handle for this texture
    assert(0 != defaultVisibilityTextureId);
    // Visibility texture is in unit 3
    glEnable(GL_TEXTURE_2D); // we are using a 2D texture
    glBindTexture(GL_TEXTURE_2D, defaultVisibilityTextureId); // make this the current texture
    // Black/zero beyond edge of texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    // GL_NEAREST ensures that we get an actual non-interpolated visibility value.
    // Interpolation would be crazy wrong.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    {
        // check for new errors
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            qDebug() << "OpenGL error" << err << __FILE__ << __LINE__;
            // glPopAttrib();
            return false;
        }
    }
    // qDebug() << width << height << depth << (long)texture_data;
    // Load the data onto video card
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexImage2D(GL_TEXTURE_2D, // target
                 0, // level
                 GL_RGBA, // texture format
                 256, // width
                 256, // height
                 0, // border
                 GL_RGBA, // image format
                 GL_UNSIGNED_BYTE, // image type
                 (GLvoid*)texture_data);
    glDisable(GL_TEXTURE_2D); // we are using a 2D texture
    glActiveTextureARB(GL_TEXTURE0_ARB);
    // glPopAttrib(); // restore OpenGL state
    {
        // check for new errors
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            qDebug() << "OpenGL error" << err << __FILE__ << __LINE__;
            return false;
        }
    }

    // qDebug() << "Uploading 2D visibility texture took"
    //         << stopwatch.elapsed()
    //         << "milliseconds";
    emit visibilityTextureLoaded();
    update();

    return true;
}

/* slot */
void Na3DWidget::initializeDefaultTextures()
{
    // (quickly) Set all textures to non-pathological values, including
    // volume, colormap, neuron visibility, and neuron label

    if (! bGLIsInitialized)
        return;

    makeCurrent();
    clearImage();

    // 3D volume texture in unit 0 set to all black
    {
        defaultVolumeTextureData.assign((size_t)8*8*8, (uint32_t)0);
        loadSignalTexture3D(8,8,8,&defaultVolumeTextureData[0]);
    }

    // 2D colormap texture maps colors to themselves
    {
        defaultColormapTextureData.assign((size_t)4*256, (uint32_t)0);
        for (int c = 0; c < 4; ++c) {
            uint32_t color_mask = 0xff << (8 * c); // 0,1,2 => red,green,blue
            if (3 == c)
                color_mask = 0x00aaaaaa; // gray for channel 4
            for (int i = 0; i < 256; ++i) {
                // 0xAABBGGRR
                uint32_t alpha_mask = i << 24; // 0xAA000000
                defaultColormapTextureData[c*256+i] = alpha_mask & color_mask;
            }
        }
        loadColorMapTexture2D(&defaultColormapTextureData[0]);
    }

    // 2D visibility texture maps everything to red
    {
        defaultVisibilityTextureData.assign((size_t)256*256, (uint32_t)0x000000ff); // red == visible but not selected
        loadVisibilityTexture2D(&defaultVisibilityTextureData[0]);
    }

    // 3D neuron label texture all zero == background
    {
        defaultLabelTextureData.assign((size_t)8*8*8, (uint16_t)0);
        loadLabelTexture3D(8, 8, 8, &defaultLabelTextureData[0]);
    }

    glActiveTextureARB(GL_TEXTURE0_ARB);
}


/* virtual */
RendererNeuronAnnotator* Na3DWidget::getRendererNa()
{
    return dynamic_cast<RendererNeuronAnnotator*>(renderer);
}

/* virtual */
const RendererNeuronAnnotator* Na3DWidget::getRendererNa() const
{
    return dynamic_cast<RendererNeuronAnnotator*>(renderer);
} // const version CMB

/* slot */
void Na3DWidget::updateScreenPosition()  // for stencil based 3D modes
{
    QPoint p = mapToGlobal(QPoint(0, 0));
    bool bChanged = false;
    if (globalScreenPosX != p.x()) {
        globalScreenPosX = p.x();
        bChanged = true;
    }
    if (globalScreenPosY != p.y()) {
        globalScreenPosY = p.y();
        bChanged = true;
    }
    if (bChanged)
        update();
}

/* virtual */
void Na3DWidget::moveEvent(QMoveEvent * event)
{
    updateScreenPosition();
    V3dR_GLWidget::moveEvent(event);
}

void Na3DWidget::setUndoStack(QUndoStack& undoStackParam) // for undo/redo custom clip planes
{
    if (undoStack != &undoStackParam) {
        undoStack = &undoStackParam;
        // Ensure 3d viewer updates when user does "undo" on clip plane.
        connect(undoStack, SIGNAL(indexChanged(int)),
                this, SLOT(update()));
    }
    RendererNeuronAnnotator * ra = getRendererNa();
    if (NULL != ra)
        ra->setUndoStack(undoStackParam);
}

/* virtual */
void Na3DWidget::initializeGL()
{
    // Is the world ready for sRGB?  It's the right thing to do...
    // glEnable(GL_FRAMEBUFFER_SRGB_EXT);

    makeCurrent(); // probably unnecessary
    int gm3ts;
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &gm3ts);
    // qDebug() << "GL_MAX_3D_TEXTURE_SIZE" << gm3ts;
    GLboolean hasStereo;
    glGetBooleanv(GL_STEREO, &hasStereo);
    if(hasStereo) {
        // qDebug() << "OpenGL context supports stereo 3D";
        glDrawBuffer(GL_BACK);
    }
    else
        // qDebug() << "OpenGL context does not support stereo 3D";
    //V3dR_GLWidget::initializeGL();

    init_members();

    bGLIsInitialized = true; // before initializeDefaultTextures()!

    // TODO - will I ever find a use for initializeDefaultTextures?
    initializeDefaultTextures();
}

/* slot */
void Na3DWidget::setCustomCutMode()
{
    getRendererNa()->setShowClipGuide(true);
    update();
}

/* slot */
void Na3DWidget::cancelCustomCutMode()
{
    getRendererNa()->setShowClipGuide(false);
    update();
}

/* slot */
void Na3DWidget::applyCustomCut()
{
    if (renderer)
        getRendererNa()->applyCustomCut(cameraModel);
    cancelCustomCutMode();
}

/* slot */
void Na3DWidget::applyCustomKeepPlane()
{
    // qDebug() << "Na3DWidget::applyCustomKeepPlane()" << __FILE__ << __LINE__;
    if (renderer)
        getRendererNa()->applyKeepCut(cameraModel);
    cancelCustomCutMode();
}

// VolumeTexture methods that must be run in the main/OpenGL thread are implemented in
// Na3DViewer slots

/* slot */
bool Na3DWidget::loadSignalTexture()
{
    // qDebug() << "Na3DWidget::loadSignalTexture()" << __FILE__ << __LINE__;
    // emit benchmarkTimerPrintRequested("Starting to upload signal texture to video card");
    if (NULL == dataFlowModel) {
        bSignalTextureIsDirty = false;
        invalidate();
        return false;
    }
    const jfrc::VolumeTexture& volumeTexture = dataFlowModel->getVolumeTexture();
    if (! volumeTexture.representsActualData()) {
        bSignalTextureIsDirty = false;
        invalidate();
        return false;
    }
    bool bSucceeded = true;
    bool bFullSizeChanged = false;
    bool bTextureSizeChanged = false;
    bSignalTextureIsDirty = true;
    jfrc::Dimension fullSize, textureSize;
    {
        jfrc::VolumeTexture::Reader textureReader(volumeTexture);
        if (volumeTexture.readerIsStale(textureReader))
            return false;
        fullSize = textureReader.originalImageSize();
        bFullSizeChanged = (fullSize != getRendererNa()->getOriginalVolumeDimensions());
        textureSize = textureReader.paddedTextureSize();
        // fooDebug() << textureSize.x() << __FILE__ << __LINE__;
        bTextureSizeChanged = (textureSize != getRendererNa()->getPaddedTextureDimensions());
        const uint32_t* data = textureReader.signalData3D();
        clearImage(); // will repopulate image4d farther down.
        if (! loadSignalTexture3D(textureSize.x(), textureSize.y(), textureSize.z(), data))
            return false;
        getRendererNa()->updateSettingsFromVolumeTexture(textureReader);
    } // Release locks
    renderer->getLimitedDataSize(_data_size); //for update slider size
    {
        // Populate renderer::data4dp for use by picking routine
        const NaVolumeData::Reader volumeReader(dataFlowModel->getVolumeData());
        if (volumeReader.hasReadLock())
        {
            const Image4DProxy<My4DImage>& imgProxy = volumeReader.getOriginalImageProxy();
            _idep->image4d = imgProxy.img0;
            getRendererNa()->setupData(_idep);
        }
    }
    if (bSucceeded) {
        if (bFullSizeChanged)
        {
            cachedDefaultFocusIsDirty = true;
            double zThickness = dataFlowModel->getZRatio();
            // qDebug() << "Maybe setting z thickness to" << dataFlowModel->getZRatio();
            if ((zThickness > 1e-6) && (zThickness < 1e6)) {
                // qDebug() << "Setting z thickness to" << dataFlowModel->getZRatio();
                setThickness(dataFlowModel->getZRatio());
            }
            updateDefaultScale();
            resetView();
            resetVolumeCutRange();
            resetSlabThickness();
            getRendererNa()->clearClipPlanes();
            cameraModel.setFocus(Vector3D(fullSize.x()/2.0,
                                          fullSize.y()/2.0,
                                          fullSize.z()/2.0));
        }
        if (bTextureSizeChanged)
        {
            resetVolumeCutRange();
        }
        setRepresentsActualData();
        bSignalTextureIsDirty = false;
        update();
        // emit benchmarkTimerPrintRequested("Finished uploading signal texture to video card");
    }
    return bSucceeded;
}

/* slot */
bool Na3DWidget::loadLabelTexture()
{
    // qDebug() << "Na3DWidget::loadLabelTexture" << __FILE__ << __LINE__;
    if (NULL == dataFlowModel) {
        bLabelTextureIsDirty = false;
        return false;
    }
    bool bSucceeded = true;
    bLabelTextureIsDirty = true;
    {
        const jfrc::VolumeTexture& volumeTexture = dataFlowModel->getVolumeTexture();
        jfrc::VolumeTexture::Reader textureReader(volumeTexture);
        if (volumeTexture.readerIsStale(textureReader))
            return false;

        jfrc::Dimension size = textureReader.paddedTextureSize();
        const uint16_t* data = textureReader.labelData3D();
        if (! loadLabelTexture3D(size.x(), size.y(), size.z(), data))
            return false;
        // qDebug() << "Na3DWidget::loadLabelTexture" << __FILE__ << __LINE__;
    } // Release locks
    if (bSucceeded) {
        bLabelTextureIsDirty = false;
        update();
    }
    return bSucceeded;
}

/* slot */
bool Na3DWidget::loadVisibilityTexture()
{
    // qDebug() << "Na3DWidget::loadVisibilityTexture()" << __FILE__ << __LINE__;
    if (NULL == dataFlowModel) {
        bVisibilityTextureIsDirty = false;
        return false;
    }
    bool bSucceeded = true;
    bVisibilityTextureIsDirty = true;
    {
        const jfrc::VolumeTexture& volumeTexture = dataFlowModel->getVolumeTexture();
        jfrc::VolumeTexture::Reader textureReader(volumeTexture);
        if (volumeTexture.readerIsStale(textureReader))
            return false;
        const uint32_t* data = textureReader.visibilityData2D();
        if (! loadVisibilityTexture2D(data))
            return false;
    } // Release locks
    if (bSucceeded) {
        bVisibilityTextureIsDirty = false;
        update();
    }
    return bSucceeded;
}

/* slot */
// TODO - refactor colormap response into this method
bool Na3DWidget::loadColorMapTexture()
{
    // qDebug() << "Na3DWidget::loadColorMapTexture()" << __FILE__ << __LINE__;
    if (NULL == dataFlowModel) {
        bColorMapTextureIsDirty = false;
        return false;
    }
    bool bSucceeded = true;
    bColorMapTextureIsDirty = true;
    {
        const jfrc::VolumeTexture& volumeTexture = dataFlowModel->getVolumeTexture();
        jfrc::VolumeTexture::Reader textureReader(volumeTexture);
        if (volumeTexture.readerIsStale(textureReader))
            return false;
        const uint32_t* data = textureReader.colorMapData2D();
        if (! loadColorMapTexture2D(data))
            return false;
    } // Release locks
    if (bSucceeded) {
        bColorMapTextureIsDirty = false;
        update();
    }
    return bSucceeded;
}

bool Na3DWidget::loadColorMapTexture2D(const uint32_t* data)
{
    // qDebug() << "Na3DWidget::loadColorMapTexture2D" << __FILE__ << __LINE__;
    if (NULL == data)
        return false;
    makeCurrent();

    // check for previous errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        qDebug() << "OpenGL error" << err << __FILE__ << __LINE__;
        return false;
    }

    // Upload ColorMap image as an OpenGL 3D texture
    glActiveTextureARB(GL_TEXTURE1_ARB);
    if (0 == defaultColormapTextureId)
        glGenTextures(1, &defaultColormapTextureId); // allocate a handle for this texture
    assert(0 != defaultColormapTextureId);
    // colormap texture is in unit 1
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, defaultColormapTextureId);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // MUST use nearest filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // MUST use nearest filter

    glTexImage2D(GL_TEXTURE_2D, // target
                 0, // level
                 GL_RGBA,
                 256, // width
                 4,   // height
                 0, // border
                 GL_RGBA, // image format
                 GL_UNSIGNED_BYTE, // image type
                 data);
    glDisable(GL_TEXTURE_2D);
    glActiveTextureARB(GL_TEXTURE0_ARB);
    {
        // check for new errors
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            qDebug() << "OpenGL error" << err << __FILE__ << __LINE__;
            return false;
        }
    }
    if (NULL != getRendererNa())
        getRendererNa()->setColorMapTextureId(defaultColormapTextureId);
    emit colorMapTextureLoaded();
    update();

    return true;
}

/* virtual */
void Na3DWidget::settingRenderer() // before renderer->setupData & init
{
    // qDebug() << "Na3DWidget::settingRenderer()" << __FILE__ << __LINE__;
    RendererNeuronAnnotator* rend = getRendererNa();
    {
        rend->loadShader(); // Actually use those fancy textures
    }
}

void Na3DWidget::setStereoOff(bool b)
{
    if (b)
        setStereoMode(jfrc::STEREO_OFF);
}

void Na3DWidget::setStereoLeftEye(bool b)
{
    if (b)
        setStereoMode(jfrc::STEREO_LEFT_EYE);
}

void Na3DWidget::setStereoRightEye(bool b)
{
    if (b)
        setStereoMode(jfrc::STEREO_RIGHT_EYE);
}

void Na3DWidget::setStereoQuadBuffered(bool b)
{
    if (!b) return;
    if (!bHasQuadStereo) {
        emit quadStereoSupported(bHasQuadStereo);
        qDebug() << "Error: Quad buffered stereo is not supported on this computer.";
        return;
    }
    setStereoMode(jfrc::STEREO_QUAD_BUFFERED);
}

void Na3DWidget::setStereoAnaglyphRedCyan(bool b)
{
    if (b)
        setStereoMode(jfrc::STEREO_ANAGLYPH_RED_CYAN);
}

void Na3DWidget::setStereoAnaglyphGreenMagenta(bool b)
{
    if (b)
        setStereoMode(jfrc::STEREO_ANAGLYPH_GREEN_MAGENTA);
}

void Na3DWidget::setStereoRowInterleaved(bool b)
{
    // qDebug() << "Na3DWidget::setStereoRowInterleaved(" << b << __FILE__ << __LINE__;
    if (b)
        setStereoMode(jfrc::STEREO_ROW_INTERLEAVED);
}

void Na3DWidget::setStereoColumnInterleaved(bool b)
{
    // qDebug() << "Na3DWidget::setStereoColumnInterleaved(" << b << __FILE__ << __LINE__;
    if (b)
        setStereoMode(jfrc::STEREO_COLUMN_INTERLEAVED);
}

void Na3DWidget::setStereoCheckerInterleaved(bool b)
{
    // qDebug() << "Na3DWidget::setStereoCheckerInterleaved(" << b << __FILE__ << __LINE__;
    if (b)
        setStereoMode(jfrc::STEREO_CHECKER_INTERLEAVED);
}

void Na3DWidget::setStereoMode(int m)
{
    // qDebug() << "Na3DWidget::setStereoMode()" << m << __FILE__ << __LINE__;
    if (m != stereo3DMode) {
        stereo3DMode = (jfrc::Stereo3DMode)m;
        update();
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
        emit progressAborted("Renderer error");
        return;
    }
    emit progressValueChanged(70);
    QCoreApplication::processEvents();
    makeCurrent();
    renderer->reinitializeVol(renderer->class_version()); //100720
    if (renderer->hasError()) {
        emit progressAborted("Renderer error");
        return;
    }
    RendererNeuronAnnotator* ra = getRendererNa();
    if (!ra) {
        emit progressAborted("No RendererNeuronAnnotator object");
        return;
    }
    bool bSuccess = true; // convoluted logic to avoid emitting before lock is release
    if (NULL == dataFlowModel)
        bSuccess = false;
    else
    {
        const jfrc::VolumeTexture& volumeTexture = dataFlowModel->getVolumeTexture();
        jfrc::VolumeTexture::Reader textureReader(volumeTexture); // acquire lock
        if (volumeTexture.readerIsStale(textureReader))
            bSuccess = false;
        else {
            if (textureReader.use3DSignalTexture())
                ra->set3dTextureMode(defaultVolumeTextureId);
            ra->updateSettingsFromVolumeTexture(textureReader);
            renderer->getLimitedDataSize(_data_size); //for update slider size
        }
    } // release lock
    if (! bSuccess) {
        emit progressAborted("Reading texture sizes failed");
    }
    else {
        emit progressValueChanged(100);
        emit progressComplete();
    }
    // when initialize done, update status of control widgets
    //SEND_EVENT(this, QEvent::Type(QEvent_InitControlValue)); // use event instead of signal
    emit signalVolumeCutRange(); //100809

    update();
}

void Na3DWidget::resetView()
{
    // qDebug() << "reset";
    updateDefaultScale();
    cameraModel.setScale(1.0); // fit to window
    Vector3D newFocus = getDefaultFocus();
    // cerr << newFocus << __LINE__ << __FILE__ << endl;
    cameraModel.setFocus(newFocus); // center view
    cameraModel.setRotation(Rotation3D()); // identity rotation
    resetVolumeCutRange();
    resetSlabThickness();
    if (NULL != getRendererNa())
        getRendererNa()->clearClipPlanes();
}

void Na3DWidget::resetRotation() {
    cameraModel.setRotation(Rotation3D());
    update();
}

Vector3D Na3DWidget::getDefaultFocus() const
{
    // TODO - reading from the VolumeTexture is slow and brittle
    // so cache the default focus for frequent ordinary use
    if (! cachedDefaultFocusIsDirty)
        return cachedDefaultFocus;

    Vector3D result(0, 0, 0);
    if (! dataFlowModel) return result;

    const jfrc::VolumeTexture& volumeTexture = dataFlowModel->getVolumeTexture();
    jfrc::VolumeTexture::Reader textureReader(volumeTexture);
    if (volumeTexture.readerIsStale(textureReader))
        return result;
    jfrc::Dimension size = textureReader.originalImageSize();

    result = Vector3D(  size.x() / 2.0 - 0.5
                      , size.y() / 2.0 - 0.5
                      , size.z() / 2.0 - 0.5);

    cachedDefaultFocus = result;
    cachedDefaultFocusIsDirty = false;

    return result;
}

// Version with x/y/z ratio proportional to Euclidean space,
// instead of to volume voxel extents
Vector3D Na3DWidget::getDefaultFocusInMicrometers() const
{
    Vector3D f = getDefaultFocus();
    f.z() *= _thickness;
    return f;
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
    Vector3D dFocus_obj = ~cameraModel.rotation() * dFocus_eye;
    dFocus_obj /= getZoomScale();
    dFocus_obj.z() /= _thickness; // Euclidean scale to image scale
    Vector3D newFocus = focus() + dFocus_obj;
    // cerr << newFocus << __LINE__ << __FILE__;
    cameraModel.setFocus(newFocus);
    update();
}

void Na3DWidget::updateCursor()
{
    // qDebug() << "updateCursor()";
    if(bClickIsWaiting) {
        setCursor(Qt::BusyCursor);
        return;
    }
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

    // Only forward certain keys to Vaa3D classic effect
    switch(e->key()) {
    // case Qt::Key_Minus: // zoom out
    // case Qt::Key_Equal: // zoom in
    case Qt::Key_F: // pixel interpolation
        //V3dR_GLWidget::keyPressEvent(e);
        break;
    default:
        QOpenGLWidget_proxy::keyPressEvent(e);
    }
}

/* virtual */
void Na3DWidget::keyReleaseEvent(QKeyEvent *e)
{
    updateCursor();
    //V3dR_GLWidget::keyReleaseEvent(e);
}

// Drag mouse to rotate; shift-drag to translate.
void Na3DWidget::mouseMoveEvent(QMouseEvent * event)
{
    // qDebug() << "Na3DWidget::mouseMoveEvent()";

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
    /*
    if (! (event->buttons() & Qt::LeftButton) )
    {
        bMouseIsDragging = false;
        V3dR_GLWidget::mouseMoveEvent(event); // use classic V3D behavior
        return;
    }
     */

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

    // qDebug() << "dx, dy = " << dx << dy;

    // shift-drag or middle drag to translate
    if (   (event->modifiers() & Qt::ShiftModifier)
        || (event->buttons() & Qt::MidButton) )
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
            //V3dR_GLWidget::mouseMoveEvent(event); // regular V3D rotate behavior
            return;
        else {
            Rotation3D oldRotation = cameraModel.rotation();
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
                    //V3dR_GLWidget::update();
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
    // qDebug() << "Na3DWidget::onMouseSingleClick" << pos << __FILE__ << __LINE__;
    highlightNeuronAtPosition(pos);
    bClickIsWaiting = false; // turn off busy cursor
    updateCursor();
}

void Na3DWidget::onPossibleSingleClickAlert()
{
    // Immediate visual feedback that a click has been initiated
    // qDebug() << "possible single click";
    bClickIsWaiting = true; // busy cursor
    updateCursor();
}

void Na3DWidget::onNotSingleClick()
{
    bClickIsWaiting = false; // turn off busy cursor
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

// Don't update if the current rotation is within 0.5 of the specified integer angle
void Na3DWidget::setXYZBodyRotationInt(int rotX, int rotY, int rotZ)
{
    Vector3D rotXYZInDegrees = cameraModel.rotation().convertBodyFixedXYZRotationToThreeAngles() * 180.0 / 3.14159;
    int oldRotX = round(rotXYZInDegrees.x());
    int oldRotY = round(rotXYZInDegrees.y());
    int oldRotZ = round(rotXYZInDegrees.z());
    if (eulerAnglesAreEquivalent(rotX, rotY, rotZ, oldRotX, oldRotY, oldRotZ))
        return; // no significant change
    Vector3D newRot = Vector3D(rotX, rotY, rotZ) * 3.14159 / 180.0;
    cameraModel.setRotation(Rotation3D().setRotationFromBodyFixedXYZAngles(newRot.x(), newRot.y(), newRot.z()));
    update();
}

/* static */
int Na3DWidget::radToDeg(double angleInRadians) {
    return round(angleInRadians * 180.0 / 3.14159);
}

/* static */
bool Na3DWidget::eulerAnglesAreEquivalent(int x1, int y1, int z1, int x2, int y2, int z2) // in degrees
{
    if (   anglesAreEqual(x1, x2)
        && anglesAreEqual(y1, y2)
        && anglesAreEqual(z1, z2) )
    {
        return true;
    }
    // Euler angles are equivalent if y' = -y + 180, x' = x + 180, z' = z + 180
    int x3 = x2 + 180;
    // int x3 = x2;
    int y3 = -y2 + 180;
    int z3 = z2 + 180;
    if (   anglesAreEqual(x1, x3)
        && anglesAreEqual(y1, y3)
        && anglesAreEqual(z1, z3) )
    {
        return true;
    }
    // qDebug() << x1 << ", " << y1 << ", " << z1 << ", " << x2 << ", " << y2 << ", " << z2;
    return false;
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

    // getIndexSelectedNeuron();
    int numNeuron = 0;
    // sum of pixels of each neuron mask in the cube
    if (NULL == dataFlowModel) return neuronIx;
    std::vector<int> sum;
    {
        NeuronSelectionModel::Reader selectionReader(
                dataFlowModel->getNeuronSelectionModel());
        if (! selectionReader.hasReadLock()) return -1;

        NaVolumeData::Reader volumeReader(dataFlowModel->getVolumeData());
        if (! volumeReader.hasReadLock()) return -1;
        const Image4DProxy<My4DImage>& neuronProxy = volumeReader.getNeuronMaskProxy();

        XYZ loc = getRendererNa()->screenPositionToVolumePosition(pos, volumeReader);

        qreal xlc = loc.x + 0.5;
        qreal ylc = loc.y + 0.5;
        qreal zlc = loc.z + 0.5;

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
    // qDebug() << "Na3DWidget::highlightNeuronAtPosition" << pos << __FILE__ << __LINE__;
    if (!getRendererNa()) return;
    // avoid crash w/ NaN markerViewMatrix
    if (getRendererNa()->hasBadMarkerViewMatrix()) {
        return;
    }
    // qDebug()<<"left click ... ...";
    if (! dataFlowModel)
        return;

    XYZ loc;
    {
        NaVolumeData::Reader volumeReader(dataFlowModel->getVolumeData());
        if (! volumeReader.hasReadLock()) return;
        // Single channel single volumes used to crash at this point,
        // because RendererNeuronAnnotator assumes it has 4 channels.
        // This is the easiest place I could find to stanch the flow.
        if (! volumeReader.hasNeuronMask()) return;
        if (volumeReader.getNumberOfNeurons() < 1) return;
        loc = getRendererNa()->screenPositionToVolumePosition(pos, volumeReader);
    }

    // select neuron: set x, y, z and emit signal
    // qDebug()<<"emit a signal ...";
    // qDebug() << "emitting neuronSelected()" << loc.x << loc.y << loc.z << __FILE__ << __LINE__;
    emit neuronSelected(loc.x, loc.y, loc.z);
    // update(); // TODO - this update() should be postponed until the response to whatever happens after neuronSelected(...) completes.
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
       // V3dR_GLWidget::mouseDoubleClickEvent(event);
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
//    if (bResizeEnabled)
//        V3dR_GLWidget::resizeGL(w, h);
}

void Na3DWidget::updateDefaultScale()
{
    float screenWidth = width();
    float screenHeight = height();
    if (screenWidth < 1) return;
    if (screenHeight < 1) return;

    if (! dataFlowModel) return;

    const jfrc::VolumeTexture& volumeTexture = dataFlowModel->getVolumeTexture();
    jfrc::VolumeTexture::Reader textureReader(volumeTexture);
    if (volumeTexture.readerIsStale(textureReader))
        return;
    Dimension size = textureReader.originalImageSize();
    float objectWidth = size.x();
    float objectHeight = size.y();

    if (objectWidth < 1) return;
    if (objectHeight < 1) return;
    float scaleX = screenWidth / objectWidth;
    float scaleY = screenHeight / objectHeight;
    defaultScale = scaleX > scaleY ? scaleY : scaleX;  // fit whole pixmap in window, with bars if necessary
    updateRendererZoomRatio(cameraModel.scale());
}

void Na3DWidget::updateRendererZoomRatio(qreal relativeScale)
{
    cachedRelativeScale = relativeScale;
    if (! getRendererNa()) return;
    // qDebug() << "update zoom";

    float desiredPixelsPerImageVoxel = defaultScale * relativeScale;
    float desiredVerticalImageVoxelsDisplayed = height() / desiredPixelsPerImageVoxel;
    float desiredVerticalGlUnitsDisplayed = desiredVerticalImageVoxelsDisplayed * glUnitsPerImageVoxel();
    // Correct for downsampled OpenGL texture?
    // desiredVerticalGlUnitsDisplayed *= getRendererNa()->sampleScaleX;
    float desiredVerticalApertureInRadians = 2.0 * atan2(desiredVerticalGlUnitsDisplayed/2.0f, (float)getRendererNa()->getViewDistance());
    float desiredVerticalApertureInDegrees = desiredVerticalApertureInRadians * 180.0 / 3.14159;
    if (desiredVerticalApertureInDegrees > 180.0)
        desiredVerticalApertureInDegrees = 180.0; // gl limit
    float desiredZoomRatio = desiredVerticalApertureInDegrees / getRendererNa()->getViewAngle();
    getRendererNa()->setInternalZoomRatio(desiredZoomRatio);

    bool bAutoClipNearFar = false; // TODO - expose this variable
    // set near and far clip planes
    if (bAutoClipNearFar) {
        getRendererNa()->setSlabThickness(3.0 * desiredVerticalImageVoxelsDisplayed);
        // getRendererNa()->setDepthClip(3.0 * desiredVerticalGlUnitsDisplayed);
    }

    getRendererNa()->setupView(width(), height());
}

void Na3DWidget::updateFocus(const Vector3D& f)
{
    Rotation3D R_eye_obj = cameraModel.rotation();
    // _[xyz]Shift variables are relative to the center of the volume
    Vector3D shift_img = f - getDefaultFocus();
    shift_img.z() *= _thickness;
    Vector3D shift_eye = R_eye_obj * shift_img;
    // _xShift is in gl coordinates scaled by 100/1.4
    // see V3dR_GLWidget::paintGL() method in v3dr_glwidget.cpp
    shift_eye *= -glUnitsPerImageVoxel() * 100.0f/1.4f;
    _xShift = shift_eye.x();
    _yShift = shift_eye.y();
    _zShift = shift_eye.z();
    dxShift=dyShift=dzShift=0;
    emit slabPositionChanged(getSlabPosition());
}

Vector3D Na3DWidget::getCameraFocusInMicrometers() const
{
    Vector3D f = cameraModel.focus();
    f.z() *= _thickness;
    return f;
}

void Na3DWidget::setCameraFocusInMicrometers(const Vector3D& f)
{
    Vector3D focusInGround = f;
    focusInGround.z() /= _thickness; // convert from micrometers to volume voxel frame
    cameraModel.setFocus(focusInGround);
}

Rotation3D Na3DWidget::getCameraRotationInGround() const
{
    return cameraModel.rotation();
}

void Na3DWidget::setCameraRotationInGround(const Rotation3D& rotation)
{
    cameraModel.setRotation(rotation);
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
        if (numChannels < 1)
            return;

        // Populate clever opengl color map texture
        for (int rgb = 0; rgb < 4; ++rgb) // loop red, then green, then blue, then reference
        {
            QRgb channelColor = 0;
            bool haveData = false;
            if (rgb < numChannels) {
                haveData = true;
                // qDebug() << "color" << rgb;
                channelColor = colorReader.getChannelColor(rgb);
            }
            Renderer_gl2* renderer = (Renderer_gl2*)getRenderer();
            for (int i_in = 0; i_in < 256; ++i_in)
            {
                // R/G/B color channel value is sum of data channel values
                qreal i_out_f = 0.0;
                if (haveData)
                    i_out_f = colorReader.getChannelScaledIntensity(rgb, i_in / 255.0) * 255.0;
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
                // qDebug() << "  i_in:" << i_in << "; i_out:" << i_out;
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
        //renderer->setThickness(_thickness);

        getRendererNa()->setColorMapTextureId(defaultColormapTextureId);
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
        connect(getRendererNa(), SIGNAL(showCornerAxesChanged(bool)),
                this, SLOT(setShowCornerAxes(bool)));
        getRendererNa()->setShowCornerAxes(bShowCornerAxes);

        connect(getRendererNa(), SIGNAL(slabThicknessChanged(int)),
                this, SIGNAL(slabThicknessChanged(int)));

        // Retain current setting for alpha blending
        // qDebug() << "alpha blending = " << (_renderMode == Renderer::rmAlphaBlendingProjection) << __FILE__ << __LINE__;
        setAlphaBlending(_renderMode == Renderer::rmAlphaBlendingProjection);
        if (undoStack)
            getRendererNa()->setUndoStack(*undoStack);
        updateScreenPosition();

        // Is this too early to upload default textures?
        // more like too LATE.  Initialize should occur before new
        // data flow model load.
        // getRendererNa()->initializeDefaultTextures();
    }
}

/* slot */
void Na3DWidget::setSlabThickness(int val) // range 0-1000; 0 means maximally close
{
    // qDebug() << "Na3DWidget::setSlabThickness" << val;
    RendererNeuronAnnotator* ra = getRendererNa();
    if (!ra) return;
    if (! ra->setSlabThickness(val))
        return;
    ra->setupView(width(), height());
    update();
}

int Na3DWidget::getSlabPosition() const
{
    Vector3D viewDirection = ~cameraModel.rotation() * Vector3D(0, 0, 1);
    // cout << "view direction = " << viewDirection << endl;
    Vector3D df = cameraModel.focus() - getDefaultFocus();
    int slabPosition = int(df.dot(viewDirection) + 0.5);
    // cout << "slab position = " << slabPosition << endl;
    return slabPosition;
}

/* slot */
void Na3DWidget::setSlabPosition(int val) // range 0-1000, 1000 means maximally far
{
    // qDebug() << "Na3DWidget::setSlabPosition" << val;
    int oldSlabPosition = getSlabPosition();
    if (val == oldSlabPosition) return; // no change
    float dSlabPos = val - oldSlabPosition;
    Vector3D viewDirection = ~cameraModel.rotation() * Vector3D(0, 0, 1);
    Vector3D dFocus = viewDirection * dSlabPos;
    dFocus.z() /= _thickness; // Euclidean scale to image scale
    Vector3D newFocus = dFocus + cameraModel.focus();
    // qDebug() << newFocus.x() << newFocus.y() << newFocus.z();
    cameraModel.setFocus(newFocus);
    emit slabPositionChanged(val);
    update();
}

/* slot */
void Na3DWidget::clipSlab()
{
    RendererNeuronAnnotator* ra = getRendererNa();
    if (!ra) return;
    ra->clipSlab(cameraModel);
    // Reset slab now that clip plane recapitulates cut
    ra->resetSlabThickness();
}

/* slot */
bool Na3DWidget::resetSlabThickness()
{
    RendererNeuronAnnotator* ra = getRendererNa();
    if (!ra) return false;
    return ra->resetSlabThickness();
}

/* slot */
void Na3DWidget::setAlphaBlending(bool b)
{
    if (b)
        _renderMode = int(Renderer::rmAlphaBlendingProjection);
    else
        _renderMode = int(Renderer::rmMaxIntensityProjection);

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
    makeCurrent();
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

/* virtual */
void Na3DWidget::paintEvent(QPaintEvent *event)
{
    // 3D rendering
#if defined(USE_Qt5)
    update();
#else
    updateGL();
#endif

    // 2D rendering overlay
    QPainter painter(this);
    if (bPaintScaleBar)
        scaleBar.paint(xVoxelSizeInMicrons, getZoomScale(), width(), height(), painter);
    painter.end();
}

void Na3DWidget::paintGL()
{
    makeCurrent();
    if (! representsActualData()) {
        glClearColor(0.63, 0.63, 0.64, 1.0); // try to match Qt::gray
        glClear(GL_COLOR_BUFFER_BIT);
        return;
    }

    // TODO - move textures to Volume/RendererNeuronAnnotator
    if (bSignalTextureIsDirty) loadSignalTexture();
    if (bLabelTextureIsDirty) loadLabelTexture();
    if (bColorMapTextureIsDirty) loadColorMapTexture();
    if (bVisibilityTextureIsDirty) loadVisibilityTexture();

    CameraTransformGL cameraTransformGL(*this); // magic stack-scoped effect
    paint_stereo();

    // emit benchmarkTimerPrintRequested("Finished painting 3D widget");
    // qDebug() << "Frame render took" << timer.elapsed() << "milliseconds";
    emit scenePainted();
}

void Na3DWidget::paint_mono(bool clearColorFirst)
{
    makeCurrent();
    // Reset background color and depth
    if (clearColorFirst) {
        glClearColor(0, 0, 0, 0); // always black for now TODO
        glClear(GL_COLOR_BUFFER_BIT);
    }
    glClearDepth(1);
    glDepthRange(0, 1);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Step 1: paint opaque geometry (such as meshes), which affects depth buffer
    ActorList::iterator ai;
    {
        ActorList& al = opaqueActors;
        for (ai = al.begin(); ai != al.end(); ++ai)
            (*ai)->paintGL();
    }

    // Step 2: paint transparent geometry, such as volume rendering
    {
        ActorList& al = transparentActors;
        for (ai = al.begin(); ai != al.end(); ++ai)
            (*ai)->paintGL();
    }
    // TODO - wrap RendererNeuronAnnotator in LegacyRendererActorGL
    RendererNeuronAnnotator* ra = getRendererNa();
    double s0 = glUnitsPerImageVoxel();
    double s1 = 1.0 / s0;
    if (NULL != ra)
    {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glScaled(s1, s1, s1);
        ra->paint();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }

    // Step 3: paint overlay geometry, such as axes, crosshair, scale bar etc.
    {
        ActorList& al = hudActors;
        for (ai = al.begin(); ai != al.end(); ++ai)
            (*ai)->paintGL();
    }
    // TODO - wrap crosshair in CrosshairActorGL
    // Draw focus position to ensure it remains in center of screen,
    // for debugging
    if (bPaintCrosshair)
    {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glScaled(s1, s1, s1);
        Vector3D focus0 = cameraModel.focus();
        // Convert from object coordinates to gl coordinates
        focus0 -= getDefaultFocus(); // reverse glTranslate(.5,.5,.5)
        focus0 *= glUnitsPerImageVoxel(); // scale to [-1,1]^3
        // Flip axes corresponding to v3dr_glwidget flip_X, flip_Y, flip_Z
        focus0.x() *= flip_X;
        focus0.y() *= flip_Y;
        focus0.z() *= flip_Z;
        focus0.z() *= _thickness;
        // Don't allow other geometry to obscure the marker
        // glClear(GL_DEPTH_BUFFER_BIT); // Destroys depth buffer; probably too harsh
        glPushAttrib(GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT); // save color and depth test
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_LINE_SMOOTH);
        glColor3f(0,0,0); // black marker color
        glLineWidth(3.0);
        paintFiducial(focus0);
        glColor3f(1.0f,1.0f,0.7f); // pale yellow marker color
        glLineWidth(1.5);
        paintFiducial(focus0);
        glMatrixMode(GL_MODELVIEW);
        glPopAttrib();
        glPopMatrix();
    }

    /*
    CubeTestActorGL cubeTest;
    cubeTest.paintGL();
    */
}

void Na3DWidget::paint_stereo()
{
    makeCurrent();
    // absolute screen coordinates for stencilling
    int stencilLeft = globalScreenPosX;
    int stencilTop = globalScreenPosY;

    makeCurrent();
    // cerr << "stereo3DMode = " << stereo3DMode << endl;
    switch(stereo3DMode)
    {
    case jfrc::STEREO_OFF:
        glDrawBuffer(GL_BACK); // Avoid flicker on non-Quadro Mac
        paint_mono();
        break;
    case jfrc::STEREO_LEFT_EYE:
        {
            jfrc::StereoEyeView v(jfrc::StereoEyeView::LEFT, bStereoSwapEyes? jfrc::StereoEyeView::RIGHT : jfrc::StereoEyeView::LEFT);
            paint_mono();
        }
        break;
    case jfrc::STEREO_RIGHT_EYE:
        {
            jfrc::StereoEyeView v(jfrc::StereoEyeView::RIGHT, bStereoSwapEyes? jfrc::StereoEyeView::LEFT : jfrc::StereoEyeView::RIGHT);
            paint_mono();
        }
        break;
    case jfrc::STEREO_ANAGLYPH_RED_CYAN:
        {
            jfrc::AnaglyphRedCyanEyeView v(jfrc::StereoEyeView::LEFT, bStereoSwapEyes? jfrc::StereoEyeView::RIGHT : jfrc::StereoEyeView::LEFT);
            paint_mono();
        }
        {
            jfrc::AnaglyphRedCyanEyeView v(jfrc::StereoEyeView::RIGHT, bStereoSwapEyes? jfrc::StereoEyeView::LEFT : jfrc::StereoEyeView::RIGHT);
            paint_mono();
        }
        break;
    case jfrc::STEREO_ANAGLYPH_GREEN_MAGENTA:
        {
            jfrc::AnaglyphGreenMagentaEyeView v(jfrc::StereoEyeView::LEFT, bStereoSwapEyes? jfrc::StereoEyeView::RIGHT : jfrc::StereoEyeView::LEFT);
            paint_mono();
        }
        {
            jfrc::AnaglyphGreenMagentaEyeView v(jfrc::StereoEyeView::RIGHT, bStereoSwapEyes? jfrc::StereoEyeView::LEFT : jfrc::StereoEyeView::RIGHT);
            paint_mono();
        }
        break;
    case jfrc::STEREO_QUAD_BUFFERED:
        {
            jfrc::QuadBufferView v(jfrc::StereoEyeView::LEFT, bStereoSwapEyes? jfrc::StereoEyeView::RIGHT : jfrc::StereoEyeView::LEFT);
            paint_mono();
        }
        {
            jfrc::QuadBufferView v(jfrc::StereoEyeView::RIGHT, bStereoSwapEyes? jfrc::StereoEyeView::LEFT : jfrc::StereoEyeView::RIGHT);
            paint_mono();
        }
        break;
    case jfrc::STEREO_ROW_INTERLEAVED:
        {
            {
                jfrc::RowInterleavedStereoView v(jfrc::StereoEyeView::LEFT, bStereoSwapEyes? jfrc::StereoEyeView::RIGHT : jfrc::StereoEyeView::LEFT);
                v.fillStencil(stencilLeft, stencilTop, width(), height());
                paint_mono();
            }
            {
                jfrc::RowInterleavedStereoView v(jfrc::StereoEyeView::RIGHT, bStereoSwapEyes? jfrc::StereoEyeView::LEFT : jfrc::StereoEyeView::RIGHT);
                // DO NOT CLEAR
                paint_mono(false);
            }
            break;
        }
    case jfrc::STEREO_COLUMN_INTERLEAVED:
        {
            {
                jfrc::ColumnInterleavedStereoView v(jfrc::StereoEyeView::LEFT, bStereoSwapEyes? jfrc::StereoEyeView::RIGHT : jfrc::StereoEyeView::LEFT);
                v.fillStencil(stencilLeft, stencilTop, width(), height());
                paint_mono();
            }
            {
                jfrc::ColumnInterleavedStereoView v(jfrc::StereoEyeView::RIGHT, bStereoSwapEyes? jfrc::StereoEyeView::LEFT : jfrc::StereoEyeView::RIGHT);
                // DO NOT CLEAR
                paint_mono(false);
            }
            break;
        }
    case jfrc::STEREO_CHECKER_INTERLEAVED:
        {
            const GLubyte* stipple = jfrc::RowInterleavedStereoView::checkStipple0;
            // qDebug() << screenRowParity << screenColumnParity;
            if ( ((globalScreenPosX + globalScreenPosY) % 2) == 1 )
                stipple = jfrc::RowInterleavedStereoView::checkStipple1;
            {
                jfrc::CheckerInterleavedStereoView v(jfrc::StereoEyeView::LEFT, bStereoSwapEyes? jfrc::StereoEyeView::RIGHT : jfrc::StereoEyeView::LEFT, stipple);
                v.fillStencil(stencilLeft, stencilTop, width(), height());
                paint_mono();
            }
            {
                jfrc::CheckerInterleavedStereoView v(jfrc::StereoEyeView::RIGHT, bStereoSwapEyes? jfrc::StereoEyeView::LEFT : jfrc::StereoEyeView::RIGHT, stipple);
                // DO NOT CLEAR
                paint_mono(false);
            }
            break;
        }


    default:
        qDebug() << "Error: Unsupported Stereo mode" << stereo3DMode;
        paint_mono();
        break;
    }
}


/* virtual */
void Na3DWidget::setDataFlowModel(const DataFlowModel* dataFlowModelParam)
{
    NaViewer::setDataFlowModel(dataFlowModelParam);

    // volumeTexture.setDataFlowModel(dataFlowModelParam);
    bVisibilityTextureIsDirty =
            bLabelTextureIsDirty =
            bColorMapTextureIsDirty =
            bSignalTextureIsDirty = false;

    // No connecting if it's NULL
    if (dataFlowModel == NULL) {
        incrementalDataColorModel = NULL;
        return;
    }

    // if (bGLIsInitialized)
    //     dataFlowModel->getVolumeTexture().initializeGL();

    incrementalDataColorModel = &dataFlowModel->getFast3DColorModel();

    const jfrc::VolumeTexture& volumeTexture = dataFlowModel->getVolumeTexture();

    // OpenGL calls must be made here, in Na3DWidget, so listen for
    // signals that textures are ready for upload.
    connect(&volumeTexture, SIGNAL(signalTextureChanged()),
        this, SLOT(loadSignalTexture()));
    connect(&volumeTexture, SIGNAL(labelTextureChanged()),
            this, SLOT(loadLabelTexture()));
    connect(&volumeTexture, SIGNAL(visibilityTextureChanged()),
            this, SLOT(loadVisibilityTexture()));
    connect(&volumeTexture, SIGNAL(colorMapTextureChanged()),
            this, SLOT(loadColorMapTexture()));
    // connect(&volumeTexture, SIGNAL(signalTextureChanged()),
    //       this, SLOT(DEPRECATEDonVolumeTextureDataChanged()));

    // Promote progress from VolumeTexture to 3D viewer
    connect(&volumeTexture, SIGNAL(progressValueChanged(int)),
            this, SIGNAL(progressValueChanged(int)));
    connect(&volumeTexture, SIGNAL(progressAborted(QString)),
            this, SIGNAL(progressAborted(QString)));
    connect(&volumeTexture, SIGNAL(progressMessageChanged(QString)),
            this, SIGNAL(progressMessageChanged(QString)));
    connect(&volumeTexture, SIGNAL(progressComplete()),
            this, SIGNAL(progressComplete()));
    connect(&volumeTexture, SIGNAL(invalidated()),
            this, SLOT(invalidate()));
    // Reset clip planes when a new data set is loaded
    connect(&volumeTexture, SIGNAL(actualDataRepresented()),
            this, SLOT(resetView()));

    connect(this, SIGNAL(neuronClearAll()), &dataFlowModel->getNeuronSelectionModel(), SLOT(clearAllNeurons()));
    connect(this, SIGNAL(neuronClearAllSelections()), &dataFlowModel->getNeuronSelectionModel(), SLOT(clearSelection()));
    connect(this, SIGNAL(neuronIndexChanged(int)), &dataFlowModel->getNeuronSelectionModel(), SLOT(selectExactlyOneNeuron(int)));

    // Fast-but-approximate color update
    connect(incrementalDataColorModel, SIGNAL(dataChanged()),
            this, SLOT(updateIncrementalColors()));

    // show selected neuron
    connect(this, SIGNAL(neuronShown(const QList<int>)),
            &dataFlowModel->getNeuronSelectionModel(), SLOT(showFirstSelectedNeuron()));
    connect(this, SIGNAL(neuronShown(const QList<int>)),
            &dataFlowModel->getNeuronSelectionModel(), SLOT(showOverlays(const QList<int>)));
    connect(this, SIGNAL(neuronShown(const QList<int>)),
            &dataFlowModel->getNeuronSelectionModel(), SLOT(clearSelection()));

    connect(this, SIGNAL(neuronShownAll(const QList<int>)),
            &dataFlowModel->getNeuronSelectionModel(), SLOT(showAllNeurons()));
    connect(this, SIGNAL(neuronShownAll(const QList<int>)),
            &dataFlowModel->getNeuronSelectionModel(), SLOT(showOverlays(const QList<int>)));
    connect(this, SIGNAL(neuronShownAll(const QList<int>)),
            &dataFlowModel->getNeuronSelectionModel(), SLOT(clearSelection()));
}

// Refactor to respond to changes in VolumeTexture, not to NaVolumeData
void Na3DWidget::DEPRECATEDonVolumeTextureDataChanged()
{
    qDebug() << "Na3DWidget::onDEPRECATEDVolumeDataChanged()" << __FILE__ << __LINE__;
    // Remember whether alpha blending was on before calling init_members();
    bool alphaBlendingMode = (_renderMode == Renderer::rmAlphaBlendingProjection);
    init_members();
    if (alphaBlendingMode)
        _renderMode = Renderer::rmAlphaBlendingProjection;
    if (_idep==0) {
        _idep = new iDrawExternalParameter();
    }

    RendererNeuronAnnotator* rend = NULL;
    bool bSucceeded = true;
    emit progressMessageChanged("Copying textures to video card");
    emit progressValueChanged(15);
    {
        NaVolumeData::Reader volumeReader(dataFlowModel->getVolumeData());
        if (! volumeReader.hasReadLock()) return;
        const Image4DProxy<My4DImage>& imgProxy = volumeReader.getOriginalImageProxy();

        // TODO - get some const correctness in here...
        // TODO - wean from _idep->image4d
        // _idep->image4d = imgProxy.img0;
        _idep->image4d = NULL; // seems OK!, but it's not - picker uses image4d
        if (renderer) {
            delete renderer;
            renderer = NULL;
        }
        choiceRenderer();

        // was in settingRenderer() July 2012
        // This is the moment when textures should be uploaded
        if (NULL != dataFlowModel) {
            if (volumeReader.doUpdateSignalTexture())
                loadSignalTexture();
            loadLabelTexture();
            // uploadNeuronVisibilityTextureGL();
            loadColorMapTexture();
        }

        settingRenderer();

        rend = getRendererNa();
        const jfrc::VolumeTexture& volumeTexture = dataFlowModel->getVolumeTexture();
        jfrc::VolumeTexture::Reader textureReader(volumeTexture);
        if (volumeTexture.readerIsStale(textureReader))
        {
            qDebug() << "Error: volumeTexture.populateVolume() failed" << __FILE__ << __LINE__;
            bSucceeded = false;
        }
        else
        {
            // succeeded
            makeCurrent();
            rend->setupData(_idep);
            if (volumeReader.doUpdateSignalTexture()) {
                if (textureReader.use3DSignalTexture())
                    rend->set3dTextureMode(defaultVolumeTextureId);
                rend->updateSettingsFromVolumeTexture(textureReader);
            }
        }
        updateImageData();

        updateDefaultScale();
        resetView();
        updateCursor();

    } // release locks before emit
    if (! bSucceeded)
    {
        emit progressAborted("");
        return;
    }
    if (rend != getRendererNa()) {
        emit progressAborted("");
        return; // stale
    }
    emit progressComplete();

    resetVolumeCutRange();
    setThickness(dataFlowModel->getZRatio());
    updateIncrementalColors(); // Otherwise reference channel might be garbled

    update();
}

void Na3DWidget::resetVolumeCutRange()
{
    if (NULL == dataFlowModel)
        return;
    VolumeTexture::Reader textureReader(dataFlowModel->getVolumeTexture());
    if (dataFlowModel->getVolumeTexture().readerIsStale(textureReader))
        return;
    jfrc::Dimension size = textureReader.originalImageSize();
    int mx = size.x() - 1;
    int my = size.y() - 1;
    int mz = size.z() - 1;
    setXCut0(0); setXCut1(mx);
    setYCut0(0); setYCut1(my);
    setZCut0(0); setZCut1(mz);
    // Sometimes renderer is not in sync with 3DWidget; then above calls might short circuit as "no-change"
    if (renderer) {
        renderer->setXCut0(0); renderer->setXCut1(mx);
        renderer->setYCut0(0); renderer->setYCut1(my);
        renderer->setZCut0(0); renderer->setZCut1(mz);
    }
}

bool Na3DWidget::screenShot(QString filename)
{
#if defined(USE_Qt5)
    QImage image = this->grabFramebuffer();
#else
    QImage image = this->grabFrameBuffer();
#endif

    if (image.save(filename, 0, 100)) //uncompressed
    {
        printf("Successful to save screen-shot: [%s]\n",  filename.toUtf8().data());
        return true;
    }
    else
    {
        printf("Failed to save screen-shot: [%s]\n",  filename.toUtf8().data());
        return false;
    }
}

void Na3DWidget::setXCutLock(bool b)
{
    if (b)	dxCut = _xCut1-_xCut0;
    else    dxCut = 0;
    lockX = b? 1:0;
}
void Na3DWidget::setYCutLock(bool b)
{
    if (b)	dyCut = _yCut1-_yCut0;
    else    dyCut = 0;
    lockY = b? 1:0;
}
void Na3DWidget::setZCutLock(bool b)
{
    if (b)	dzCut = _zCut1-_zCut0;
    else    dzCut = 0;
    lockZ = b? 1:0;
}

/* static */
int Na3DWidget::round(double d)
{
    return floor(d + 0.5);
}

/* static */
bool Na3DWidget::anglesAreEqual(int a1, int a2) // in degrees
{
    if (a1 == a2)
        return true; // trivially equal
    else if (((a1 - a2) % 360) == 0)
        return true;
    else
        return false;
}


