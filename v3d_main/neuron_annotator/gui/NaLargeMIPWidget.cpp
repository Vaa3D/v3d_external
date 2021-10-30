#include "NaLargeMIPWidget.h"
#include <cassert>
#include <vector>
#include <map>
#include "NeuronQAction.h"
#include "../data_model/MipMergedData.h"


//////////////////////////////
// NaLargeMIPWidget methods //
//////////////////////////////

NaLargeMIPWidget::NaLargeMIPWidget(QWidget * parent)
    : Na2DViewer(parent)
    , highlightedNeuronMaskPixmap(200, 200)
    , highlightedNeuronIndex(-1)
    , mipMergedData(NULL)
    , viewerContextMenu(NULL)
    , neuronContextMenu(NULL)
{
    // Test image
    pixmap = QPixmap(200, 200);
    pixmap.fill(Qt::black);
    highlightedNeuronMaskPixmap.fill(Qt::transparent);
    updateDefaultScale();
    // resetView();

    setCursor(Qt::OpenHandCursor);

    connect(&mouseClickManager, SIGNAL(singleClick(QPoint)),
            this, SLOT(onMouseSingleClick(QPoint)));
    connect(this, SIGNAL(hoverNeuronChanged(int)),
            this, SLOT(onHighlightedNeuronChanged(int)));
    connect(this, SIGNAL(mouseLeftDragEvent(int, int, QPoint)),
            this, SLOT(translateImage(int,int)));

    connect(&cameraModel, SIGNAL(focusChanged(Vector3D)), this, SLOT(update()));
    connect(&cameraModel, SIGNAL(scaleChanged(qreal)), this, SLOT(update()));
    invalidate();
}

NaLargeMIPWidget::~NaLargeMIPWidget()
{
    invalidate();
}

void NaLargeMIPWidget::setMipMergedData(const MipMergedData* mipMergedDataParam)
{
    if (mipMergedData == mipMergedDataParam)
        return; // redundant
    mipMergedData = mipMergedDataParam;
    if (NULL == mipMergedData)
        return;
    connect(mipMergedData, SIGNAL(dataChanged()),
            this, SLOT(initializePixmap()));
    connect(mipMergedData, SIGNAL(invalidated()),
            this, SLOT(invalidate()));
}

void NaLargeMIPWidget::setContextMenus(QMenu* viewerMenu, NeuronContextMenu* neuronMenu)
{
    if (viewerMenu) {
        viewerContextMenu = viewerMenu;
    }
    if (neuronMenu) {
        neuronContextMenu = neuronMenu;
    }
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));
}

/* slot */
void NaLargeMIPWidget::showContextMenu(QPoint point)
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

void NaLargeMIPWidget::updatePixmap()
{
    if (! mipMergedData) {
        invalidate();
        return;
    }
    if (! mipMergedData->representsActualData()) {
        invalidate();
        return;
    }
    {
        MipMergedData::Reader mipReader(*mipMergedData);
        if (! mipReader.hasReadLock()) return;
        // Tanya saw a crash at QPixmap::fromImage(*mipReader.getImage()) 5/25/2012.
        // So I broke this into smaller steps with more bailout opportunities.
        const QImage * img = mipReader.getImage();
        if (img == NULL) return;
        if (img->isNull()) return;
        pixmap = QPixmap::fromImage(*img);
    }
    setRepresentsActualData();
    updateDefaultScale();
}

void NaLargeMIPWidget::initializePixmap()
{
    emit hideProgress();
    // progressBar->hide();
    updatePixmap();
    // resetView();
    update();
    // qDebug() << "Finished MIP data load";
}

// Want to distinguish between double click and single click events
void NaLargeMIPWidget::onMouseSingleClick(QPoint pos)
{
    int neuronMyersIx = neuronAt(pos);
    if (neuronMyersIx >= 0) {
        emit neuronClicked(neuronMyersIx);
        // qDebug() << "clicked Neuron " << neuronAt(pos);
    }
}

void NaLargeMIPWidget::resetView()
{
    cameraModel.setScale(1.0); // fit to window
    qreal focusZ = cameraModel.focus().z();
    Vector3D newFocus(pixmap.size().width()/2.0,
                pixmap.size().height()/2.0,
                focusZ);
    // cerr << newFocus << __LINE__ << __FILE__;
    cameraModel.setFocus(newFocus);
}

void NaLargeMIPWidget::resizeEvent(QResizeEvent * event) {
    updateDefaultScale();
}

void NaLargeMIPWidget::paintIntensityNumerals(QPainter& painter)
{
    if (!mipMergedData) return;
    MipMergedData::Reader mipReader(*mipMergedData);
    if (!mipReader.hasReadLock()) return;

    // qDebug() << "numerals";
    QPointF v_img_upleft = X_img_view * painter.viewport().topLeft();
    QPointF v_img_downright = X_img_view * painter.viewport().bottomRight();
    // qDebug() << v_img_upleft;
    // qDebug() << v_img_downright;

    // clear transform for text rendering, otherwise font size is harder to manage
    painter.resetTransform();

    QFont font = painter.font();
    float scale = defaultScale * cameraModel.scale();
    font.setPixelSize(scale/4.0);
    font.setStyleStrategy(QFont::NoAntialias); // text rendering can be slow
    painter.setFont(font);

    // qDebug() << "nColumns = " << mipImage->originalData.nColumns();
    // qDebug() << "nRows = " << mipImage->originalData.nRows();

    const Image4DProxy<My4DImage>& dataProxy = mipReader.getLayerDataProxy();
    const int imageZ = mipReader.getMergedImageLayerIndex();
    const QImage* image = mipReader.getImage();

    // Iterate over only the image pixels that are visible
    for (int x = int(v_img_upleft.x() - 0.5); x <= int(v_img_downright.x() + 0.5); ++x) {
        // qDebug() << "x = " << x;
        if (x < 0)
            continue;
        if (x >= dataProxy.sx)
            continue;
        for (int y = int(v_img_upleft.y() - 0.5); y <= int(v_img_downright.y() + 0.5); ++y) {
            // qDebug() << "y = " << y;
            if (y < 0)
                continue;
            if (y >= dataProxy.sy)
                continue;
            // Transform image pixel coordinates back to viewport coordinates
            QPointF v = X_view_img * QPointF(x, y);
            // qDebug() << x << ", " << y << "; " << v.x() << ", " << v.y();
            // Print original data intensity, not displayed intensity
            // But choose font color based on displayed intensity.
            unsigned int red = qRed(image->pixel(x, y));
            unsigned int green = qGreen(image->pixel(x, y));
            unsigned int blue = qBlue(image->pixel(x, y));
            // human color perception is important here
            float displayIntensity = 0.30 * red + 0.58 * green + 0.12 * blue;
            if (displayIntensity < 128)
                painter.setPen(Qt::white);
            else
                painter.setPen(Qt::black);

            int nC = dataProxy.sc;
            float lineHeight = scale / (nC + 1.0);
            // Write a neat little column of numbers inside each pixel
            for (int c = 0; c < nC; ++c) {
                float val = dataProxy.value_at(x, y, imageZ, c);
                painter.drawText(QRectF(v.x(), v.y() + (c + 0.5) * lineHeight, scale, lineHeight),
                                 Qt::AlignHCenter | Qt::AlignVCenter,
                                 QString("%1").arg(val));
            }
        }
    }
}

void NaLargeMIPWidget::paintEvent(QPaintEvent *event)
{
    if (! representsActualData()) {
        painter.begin(this);
        painter.fillRect(0, 0, width(), height(), Qt::gray);
        painter.end();
        return;
    }

    // qDebug() << "paint MIP " << width() << ", " << height();
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // use zoom scale to determine when to show numbers/unsmoothed pixels
    float scale = defaultScale * cameraModel.scale();
    bool showNumbers = (scale > 40); // 40 display pixels per image pixel
    if (! showNumbers)
        // smoothing is nicer on the eyes at low zoom levels, but confusing when numbers  are shown.
        painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // first fill background with black
    // painter.fillRect(0, 0, width(), height(), Qt::black);

    // adjust painter coordinate system to place image correctly
    transformPainterToCurrentCamera(painter);

    painter.drawPixmap(0, 0, pixmap); // magic!
    if (highlightedNeuronIndex > 0) { // zero means background
        if (highlightedNeuronMaskPixmap.size() != pixmap.size()) {
            // qDebug() << "pixmap size = " << pixmap.size();
            // qDebug() << "but highlight size = " << highlightedNeuronMaskPixmap.size();
        }
        else {
            // qDebug() << "Painting highlight for neuron " << highlightedNeuronIndex;
            // painter.drawPixmap(0, 0, highlightedNeuronMaskPixmap); // disable for now...
        }
    }

    if (bPaintCrosshair) paintCrosshair(painter);

    // At large zoom levels, write the intensity values at each pixel
    if (showNumbers) {
        paintIntensityNumerals(painter);
    }
    painter.end();
}

// Zoom using mouse wheel
void NaLargeMIPWidget::wheelEvent(QWheelEvent * e) // mouse wheel
{
    double oldZoom = cameraModel.scale();
    wheelZoom(e->delta());
    double factor = cameraModel.scale()/oldZoom;

    // Zoom like in Google earth depends on cursor position
    bool doSmartZoom = false;
    // Smart zoom only when zooming *in*, not when zooming *out*,
    // to minimize loss of painstakingly adjusted foucs.
    // if (e->delta() < 0) doSmartZoom = true; // adjust focus when zooming in
    if (doSmartZoom) {
        double scale = cameraModel.scale() * defaultScale;
        double dx = e->pos().x() - width()/2.0;
        double dy = e->pos().y() - height()/2.0;
        double dx2 = ((factor - 1.0) * dx) / scale;
        double dy2 = ((factor - 1.0) * dy) / scale;
        Vector3D newFocus = cameraModel.focus() + Vector3D(flip_X * dx2, flip_Y * dy2, 0);
        // cerr << newFocus << __LINE__ << __FILE__;
        cameraModel.setFocus(newFocus);
    }
    update();
}

/* virtual */
void NaLargeMIPWidget::mousePressEvent(QMouseEvent * event)
{
    super::mousePressEvent(event);
    // Feedback for starting a translation drag operation
    if (event->buttons() & Qt::LeftButton)
        setCursor(Qt::ClosedHandCursor);
}

/* virtual */
void NaLargeMIPWidget::mouseReleaseEvent(QMouseEvent * event)
{
    super::mouseReleaseEvent(event);
    setCursor(Qt::OpenHandCursor);
}

int NaLargeMIPWidget::neuronAt(const QPoint& p) const
{
    int answer = -1;
    if (! mipMergedData) return answer;
    MipMergedData::Reader mipReader(*mipMergedData);
    if (! mipReader.hasReadLock()) return answer;

    const Image4DProxy<My4DImage>& neuronProxy = mipReader.getLayerNeuronProxy();
    int z = mipReader.getMergedImageLayerIndex();
    if (z < 0) return answer;
    if (z >= neuronProxy.sz) return answer;

    QPointF v_img = X_img_view * QPointF(p);
    int x = v_img.x();
    int y = v_img.y();
    if (x < 0) return answer;
    if (y < 0) return answer;
    if (x >= neuronProxy.sx) return answer;
    if (y >= neuronProxy.sy) return answer;
    answer = neuronProxy.value_at(x, y, z, 0) - 1; // -1 : volume labelfield index to Myers index
    return answer;
}

void NaLargeMIPWidget::onHighlightedNeuronChanged(int neuronIx)
{
    // TODO
}

// Drag in widget to translate the MIP image in x,y
void NaLargeMIPWidget::mouseMoveEvent(QMouseEvent * event)
{
    if (! representsActualData())
        return;

    super::mouseMoveEvent(event);

    // Hover action: status message
    // Write status message when hovering with mouse.
    // Notice statement "setMouseTracking(true)" in constructor.
    if (Qt::NoButton == event->buttons())
    {
        // Hover to show (x, y, value) in status bar
        if (! mipMergedData) return;
        MipMergedData::Reader mipReader(*mipMergedData);
        if (! mipReader.hasReadLock()) return;

        const Image4DProxy<My4DImage>& neuronProxy = mipReader.getLayerNeuronProxy();
        const Image4DProxy<My4DImage>& zProxy = mipReader.getLayerZProxy();
        const Image4DProxy<My4DImage>& dataProxy = mipReader.getLayerDataProxy();
        const int mergeIndex = mipReader.getMergedImageLayerIndex();

        QPointF v_img = X_img_view * QPointF(event->pos());
        int x = int( floor(v_img.x()) );
        int y = int( floor(v_img.y()) );
        int z = 0;
        int neuronIx = -1;
        QString value("<None>"); // default value
        if ( (x >= 0) && (x < neuronProxy.sx)
            && (y >= 0) && (y < neuronProxy.sy) )
        {
            z = zProxy.value_at(x, y, mergeIndex, 0);
            value = "";
            int nC = dataProxy.sc;
            if (nC > 1) value += "[";
            for (int c = 0; c < nC; ++c) {
                if (c > 0) value += ", ";
                float val = dataProxy.value_at(x, y, mergeIndex, c);
                value += QString("%1").arg(val, 4);
            }
            if (nC > 1) value += "]";
            neuronIx = neuronProxy.value_at(x, y, mergeIndex, 0) - 1;
        }

        QString msg;
        msg = QString("x =%1, y =%2, z =%3, value =%4")
                      .arg(x, 3)
                      .arg(y, 3)
                      .arg(z, 3)
                      .arg(value);


        if (neuronIx >= 0) { // Zero means background in label field, so -1 in Myers index
            msg = QString("Neuron fragment %1; ").arg(neuronIx, 2) + msg;
            if (neuronIx != highlightedNeuronIndex) {
                highlightedNeuronIndex = neuronIx;
                emit(hoverNeuronChanged(neuronIx));
            }
        }

        emit statusMessage(msg);
        return;
    }
}

bool NaLargeMIPWidget::saveImage(QString filename){
    if (pixmap.save(filename, QFileInfo(filename).suffix().toStdString().c_str(), 100)) //uncompressed
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

