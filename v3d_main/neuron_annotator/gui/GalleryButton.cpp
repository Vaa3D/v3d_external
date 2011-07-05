#include <QtGui>
#include <cmath>
#include "GalleryButton.h"

GalleryButton::GalleryButton(const QImage & image, QString name, int index, QWidget *parent)
        : QWidget(parent)
        , scaledThumbnail(NULL)
        , correctedScaledThumbnail(NULL)
        , bImageUpdating(false)
{
    this->index=index;
    setMouseTracking(true); // respond to mouse hover events
    QSize thumbnailSize(140, 140);
    scaledThumbnail = new QImage(image.scaled(
            thumbnailSize,
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation));
    QVBoxLayout *layout = new QVBoxLayout();
    pushButton = new QPushButton();
    pushButton->setCheckable(true);
    QIcon icon(QPixmap::fromImage(image));
    pushButton->setIcon(icon);
    pushButton->setIconSize(thumbnailSize);
    QLabel* label = new QLabel(name);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(pushButton);
    layout->addWidget(label);
    this->setLayout(layout);
    // setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    connect(pushButton, SIGNAL(clicked(bool)), this, SLOT(buttonPress(bool)));
    // Multithreaded image update must hand off to single GUI thread for final GUI pixmap update.
    connect(this, SIGNAL(thumbnailImageUpdated()),
            this, SLOT(updateThumbnailIcon()));
}

GalleryButton::~GalleryButton() {
    if (scaledThumbnail) {
        // buttonMutex.lock();
        delete scaledThumbnail;
        scaledThumbnail = NULL;
        // buttonMutex.unlock();
    }
    if (correctedScaledThumbnail) {
        // buttonMutex.lock();
        delete correctedScaledThumbnail;
        correctedScaledThumbnail = NULL;
        // buttonMutex.unlock();
    }
}

/* virtual */
void GalleryButton::paintEvent(QPaintEvent *paintEvent)
{
    super::paintEvent(paintEvent);

    /* draw highlight border */
    bool thisIsTheHighlightedFragment = false;
    if (thisIsTheHighlightedFragment) {
        QPainter painter(this);
        QPen pen;
        pen.setColor(Qt::yellow);
        pen.setWidth(4);
        painter.setPen(pen);
        painter.drawRoundedRect(QRect(5, 5, width() - 5, height() - 5), 10, 10);
        painter.end();
    }
}

/* virtual */
void GalleryButton::mouseMoveEvent(QMouseEvent *moveEvent) {
    super::mouseMoveEvent(moveEvent);
    emit fragmentHover(index);
}

void GalleryButton::buttonPress(bool checked) {
    emit declareChange(index, checked);
}

void GalleryButton::setBrightness(const BrightnessCalibrator<int>& calibrator)
{
    if (! scaledThumbnail) return;
    // buttonMutex.lock(); // Avoid possible race condition
    if (bImageUpdating) {
        // buttonMutex.unlock();
        return;
    }
    // possible race condition right here...
    bImageUpdating = true;
    // buttonMutex.unlock(); // The race is over.  We won!

    if (! correctedScaledThumbnail)
        correctedScaledThumbnail = new QImage(*scaledThumbnail);
    for (int x = 0; x < scaledThumbnail->width(); ++x) {
        for (int y = 0; y < scaledThumbnail->height(); ++y) {
            QColor c(scaledThumbnail->pixel(x, y));
            c.setRed(calibrator.getCorrectedByte(c.red()));
            c.setGreen(calibrator.getCorrectedByte(c.green()));
            c.setBlue(calibrator.getCorrectedByte(c.blue()));
            correctedScaledThumbnail->setPixel(x, y, c.rgb());
        }
    }
    emit thumbnailImageUpdated(); // Pixmap operations must be done in GUI thread
    bImageUpdating = false;
}

void GalleryButton::updateThumbnailIcon() {
    if (! correctedScaledThumbnail) return;
    pushButton->setIcon(QIcon(QPixmap::fromImage(*correctedScaledThumbnail)));
    emit widgetChanged(index);
}
