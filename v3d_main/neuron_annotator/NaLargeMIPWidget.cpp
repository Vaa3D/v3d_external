#include "NaLargeMIPWidget.h"
#include <cassert>
#include <vector>
#include <map>

// Screen Y-axis is flipped with respect to data Y-axis in V3D 3D viewer
static const qreal flip_X =  1;
static const qreal flip_Y = -1;
static const qreal flip_Z = -1;

/////////////////////////////
// MipDisplayImage methods //
/////////////////////////////

MipDisplayImage::MipDisplayImage()
{
    connect(&originalData, SIGNAL(processedXColumn(int)),
            this, SLOT(processedXColumnSlot(int)));
}

void MipDisplayImage::processedXColumnSlot(int c) {
    // qDebug() << "foo " << c;
    emit processedXColumn(c);
}

void MipDisplayImage::loadImageData(const My4DImage* img, const My4DImage* maskImg) {
    load4DImage(img, maskImg);
}

void MipDisplayImage::load4DImage(const My4DImage* img, const My4DImage* maskImg)
{
    image = QImage(QSize(img->getXDim(), img->getYDim()), QImage::Format_RGB32);
    originalData.loadMy4DImage(img, maskImg);
    // Default display exactly entire intensity range
    displayMin = originalData.dataMin;
    // displayMin = 0.0; // example ct image has a very large minimum
    displayMax = originalData.dataMax;
    setGamma(1.0);
    updateCorrectedIntensities();
    emit initialImageDataLoaded();
}

void MipDisplayImage::updateCorrectedIntensities()
{
    for (int x = 0; x < originalData.nColumns(); ++x) {
        for (int y = 0; y < originalData.nRows(); ++y) {
            unsigned char red, green, blue;
            if (originalData.nChannels() == 1)
                red = green = blue = getCorrectedIntensity(x, y, 0);
            else {
                red = getCorrectedIntensity(x, y, 0);
                green = getCorrectedIntensity(x, y, 1);
                blue = getCorrectedIntensity(x, y, 2);
            }
            image.setPixel(x, y, qRgb(red, green, blue));
        }
    }
}

void MipDisplayImage::setGamma(float gamma)
{
    assert(gamma > 0.0);
    displayGamma = gamma;
    float previous_i_out = 0.0; // for slope interpolation
    for (int i = 0; i < 256; ++i) { // gamma table entries
        float i_in = i/255.0; // range 0.0-1.0
        float i_out = std::pow(i_in, gamma);
        if (i_out > 1.0) i_out = 1.0;
        if (i_out < 0.0) i_out = 0.0;
        i_out *= 255.0; // scale to 8-bit for RGB use
        gammaTable[i] = i_out;
        if (i > 0) {
            // first derivative of gamma table for interpolation
            // linear interpolation
            dGammaTable[i - 1] = i_out - previous_i_out;
        }
        previous_i_out = i_out;
    }
    dGammaTable[255] = 0.0; // final unused entry for neatness
}

// i_in as original float value
// Output in range 0-255
unsigned char MipDisplayImage::getCorrectedIntensity(float i_in) const
{
    if (i_in <= displayMin) return 0;
    if (i_in >= displayMax) return 255;
    // HDR correction
    float i_out = (i_in - displayMin) / (displayMax - displayMin);
    // gamma correction
    i_out *= 255.0; // scale to use 256 entry gamma lookup table
    int ix = int(i_out);
    int d_ix = i_out - ix; // fractional part for interpolation
    assert(d_ix >= 0.0);
    assert(d_ix <= 1.0);
    i_out = gammaTable[ix] + d_ix * dGammaTable[ix];
    int answer = int(i_out + 0.4999);
    if (answer <= 0) return 0;
    if (answer >= 255) return 255;
    return (unsigned char)answer;
}

unsigned char MipDisplayImage::getCorrectedIntensity(int x, int y, int c) const
{
    return getCorrectedIntensity(originalData[x][y][c]);
}

//////////////////////////////
// NaLargeMIPWidget methods //
//////////////////////////////

NaLargeMIPWidget::NaLargeMIPWidget(QWidget * parent)
    : QWidget(parent), mipImage(NULL), imageUpdateThread(this)
{
    // Test image
    pixmap = QPixmap(200, 200);
    pixmap.fill(Qt::black);
    updateDefaultScale();
    resetView();

    setMouseTracking(true); // respond to mouse hover events
    setCursor(Qt::OpenHandCursor);
    imageUpdateThread.start();

    // Progress bar for when image is being processed
    progressBar = new QProgressBar(this);
    progressBar->setValue(24);
    QGridLayout * gridLayout = new QGridLayout(this);
    gridLayout->addWidget(progressBar, 0, 0, 1, 1);
    progressBar->hide();
    connect(progressBar, SIGNAL(valueChanged(int)),
            this, SLOT(update()));
}

NaLargeMIPWidget::~NaLargeMIPWidget()
{
    if (mipImage) {
        delete mipImage;
        mipImage = NULL;
    }
}

bool NaLargeMIPWidget::loadMy4DImage(const My4DImage* img, const My4DImage* maskImg)
{
    int imageC = img->getCDim();
    // Unsure how to map colors if not 1 or 3 colors
    if ((imageC != 1)  && (imageC != 3)) {
        qDebug() << "Error: number of channels = " << img->getCDim();
        return false;
    }
    // Delegate computation of maximum intensity projection
    // to MipDisplayImage class
    if (mipImage) {
        delete mipImage;
        mipImage = NULL;
    }
    // Put loading of image data into another thread
    qDebug() << "Starting MIP data load";
    mipImage = new MipDisplayImage();
    mipImage->moveToThread(&imageUpdateThread);
    connect(this, SIGNAL(volumeDataUpdated(const My4DImage*, const My4DImage*)),
            mipImage, SLOT(loadImageData(const My4DImage*, const My4DImage*)));
    connect(mipImage, SIGNAL(initialImageDataLoaded()),
            this, SLOT(initializePixmap()));
    progressBar->setMaximum(img->getXDim());
    connect(mipImage, SIGNAL(processedXColumn(int)),
            progressBar, SLOT(setValue(int)));
    progressBar->show();
    emit volumeDataUpdated(img, maskImg); // Start image processing in another thread.
    return true;
}

void NaLargeMIPWidget::initializePixmap()
{
    // Image processing is done, finish up in the GUI thread.
    progressBar->hide();
    pixmap = QPixmap::fromImage(mipImage->image);
    updateDefaultScale();
    resetView();
    update();
    qDebug() << "Finished MIP data load";
}

void NaLargeMIPWidget::updateDefaultScale()
{
    float screenWidth = width();
    float screenHeight = height();
    float objectWidth = pixmap.size().width();
    float objectHeight = pixmap.size().height();

    if (screenWidth < 1) return;
    if (screenHeight < 1) return;
    if (objectWidth < 1) return;
    if (objectHeight < 1) return;
    float scaleX = screenWidth / objectWidth;
    float scaleY = screenHeight / objectHeight;
    // fit whole pixmap in window, with bars if necessary
    defaultScale = scaleX > scaleY ? scaleY : scaleX;
}

void NaLargeMIPWidget::resetView()
{
    cameraModel.setScale(1.0); // fit to window
    qreal focusZ = cameraModel.focus().z();
    cameraModel.setFocus(Vector3D(
            pixmap.size().width()/2.0,
            pixmap.size().height()/2.0,
            focusZ));
}

void NaLargeMIPWidget::resizeEvent(QResizeEvent * event) {
    updateDefaultScale();
}

void NaLargeMIPWidget::paintIntensityNumerals(QPainter& painter)
{
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

    // Iterate over only the image pixels that are visible
    for (int x = int(v_img_upleft.x() - 0.5); x <= int(v_img_downright.x() + 0.5); ++x) {
        // qDebug() << "x = " << x;
        if (x < 0)
            continue;
        if (x >= mipImage->originalData.nColumns())
            continue;
        for (int y = int(v_img_upleft.y() - 0.5); y <= int(v_img_downright.y() + 0.5); ++y) {
            // qDebug() << "y = " << y;
            if (y < 0)
                continue;
            if (y >= mipImage->originalData.nRows())
                continue;
            // Transform image pixel coordinates back to viewport coordinates
            QPointF v = X_view_img * QPointF(x, y);
            // qDebug() << x << ", " << y << "; " << v.x() << ", " << v.y();
            // Print original data intensity, not displayed intensity
            // But choose font color based on displayed intensity.
            unsigned int red = qRed(mipImage->image.pixel(x, y));
            unsigned int green = qGreen(mipImage->image.pixel(x, y));
            unsigned int blue = qBlue(mipImage->image.pixel(x, y));
            // human color perception is important here
            float displayIntensity = 0.30 * red + 0.58 * green + 0.12 * blue;
            if (displayIntensity < 128)
                painter.setPen(Qt::white);
            else
                painter.setPen(Qt::black);

            int nC = mipImage->originalData.nChannels();
            float lineHeight = scale / (nC + 1.0);
            // Write a neat little column of numbers inside each pixel
            for (int c = 0; c < nC; ++c) {
                float val = mipImage->originalData[x][y][c];
                painter.drawText(QRectF(v.x(), v.y() + (c + 0.5) * lineHeight, scale, lineHeight),
                                 Qt::AlignHCenter | Qt::AlignVCenter,
                                 QString("%1").arg(val));
            }
        }
    }
}

void NaLargeMIPWidget::paintEvent(QPaintEvent *event)
{
    // qDebug() << "paint MIP " << width() << ", " << height();
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // first fill background with black
    // painter.fillRect(0, 0, width(), height(), Qt::black);

    // adjust painter coordinate system to place image correctly
    float scale = defaultScale * cameraModel.scale();
    qreal tx = pixmap.width()/2.0 + flip_X * (cameraModel.focus().x() - pixmap.width()/2.0);
    qreal ty = pixmap.height()/2.0 + flip_Y * (cameraModel.focus().y() - pixmap.height()/2.0);
    painter.translate(width()/2.0 - tx * scale, height()/2.0 - ty * scale);
    painter.scale(scale, scale);

    // I want to convert screen coordinates to image coordinates;
    // The QPainter object knows this transformation.
    // This nomenclature for the transforms, e.g. X_view_img , comes from the
    // advanced dynamics community at Stanford, specifically the disciples of Thomas Kane.
    X_view_img = painter.transform();
    X_img_view = painter.transform().inverted();

    painter.drawPixmap(0, 0, pixmap); // magic!

    if (bPaintCrosshair) {
        // qDebug() << "paint crosshair";
        // Q: Why all this complicated math instead of just [width()/2, height()/2]?
        // A: This helps debug/document placement of image focus
        qreal cx = pixmap.width()/2.0 + flip_X * (cameraModel.focus().x() - pixmap.width()/2.0);
        qreal cy = pixmap.height()/2.0 + flip_Y * (cameraModel.focus().y() - pixmap.height()/2.0);
        QPointF f(cx, cy);
        QPointF dx(10.0 / scale, 0); // crosshair size is ten pixels
        QPointF dy(0, 10.0 / scale);
        painter.setPen(Qt::cyan);
        painter.drawLine(f - dx, f + dx);
        painter.drawLine(f - dy, f + dy);
    }

    // At large zoom levels, write the intensity values at each pixel
    if (scale > 40) { // 40 display pixels per image pixel
        paintIntensityNumerals(painter);
    }
    painter.end();
}

void NaLargeMIPWidget::translateImage(int dx, int dy)
{
    if (!dx && !dy) return;
    float scale = defaultScale * cameraModel.scale();
    cameraModel.setFocus(cameraModel.focus() - Vector3D(flip_X * dx/scale, flip_Y * dy/scale, 0));
    update();
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
        cameraModel.setFocus(cameraModel.focus() + Vector3D(flip_X * dx2, flip_Y * dy2, 0));
    }
    update();
}

void NaLargeMIPWidget::mousePressEvent(QMouseEvent * event)
{
    // Consider starting a translation drag operation
    if (event->buttons() & Qt::LeftButton) {
        bMouseIsDragging = true;
        oldDragX = event->pos().x();
        oldDragY = event->pos().y();
        setCursor(Qt::ClosedHandCursor);
    }
    else {
        bMouseIsDragging = false;
    }
}

void NaLargeMIPWidget::mouseReleaseEvent(QMouseEvent * event)
{
    // End any drag event
    bMouseIsDragging = false;
    setCursor(Qt::OpenHandCursor);
}

// Drag in widget to translate the MIP image in x,y
void NaLargeMIPWidget::mouseMoveEvent(QMouseEvent * event)
{
    // Hover action: status message
    // Write status message when hovering with mouse.
    // Notice statement "setMouseTracking(true)" in constructor.
    if (Qt::NoButton == event->buttons())
    {
        // Hover to show (x, y, value) in status bar
        if (!mipImage) return;
        QPointF v_img = X_img_view * QPointF(event->pos());
        int x = v_img.x();
        int y = v_img.y();
        int z = 0;
        QString value("<None>"); // default value
        if ( (x >= 0) && (x < mipImage->originalData.nColumns())
            && (y >= 0) && (y < mipImage->originalData.nRows()) )
        {
            z = mipImage->originalData[x][y].z;
            value = "";
            int nC = mipImage->originalData.nChannels();
            if (nC > 1) value += "[";
            for (int c = 0; c < nC; ++c) {
                if (c > 0) value += ", ";
                float val = mipImage->originalData[x][y][c];
                value += QString("%1").arg(val);
            }
            if (nC > 1) value += "]";
        }

        emit statusMessage(QString("x = %1, y = %2, z = %3, value = %4")
                           .arg(x)
                           .arg(y)
                           .arg(z)
                           .arg(value)
                           );
        bMouseIsDragging = false;
        return;
    }

    if (! (event->buttons() & Qt::LeftButton) ) {
        // qDebug() << "Not left button...";
        bMouseIsDragging = false;
        return;
    }

    int dx = event->pos().x() - oldDragX;
    int dy = event->pos().y() - oldDragY;
    oldDragX = event->pos().x();
    oldDragY = event->pos().y();

    // Do nothing until the second drag point is reached
    if (!bMouseIsDragging) {
        bMouseIsDragging = true;
        return;
    }

    // Left drag action: translate
    // Mouse drag to translate image
    translateImage(dx, dy);
}

// Move focus on double click
void NaLargeMIPWidget::mouseDoubleClickEvent(QMouseEvent * event)
{
    if (event->button() != Qt::LeftButton)
        return;
    double dx = event->pos().x() - width()/2.0;
    double dy = event->pos().y() - height()/2.0;
    translateImage(-dx, -dy);
}

void NaLargeMIPWidget::annotationModelUpdate(QString updateType) {
    // Stub
}
