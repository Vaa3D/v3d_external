#include <QtGui>
#include <cmath>
#include "GalleryButton.h"

GalleryButton::GalleryButton(const QPixmap & pixmap, QString name, int index, QWidget *parent)
        : QWidget(parent)
        , scaledThumbnail(NULL)
        , correctedScaledThumbnail(NULL)
        , bImageUpdating(false)
{
    this->index=index;
    QSize thumbnailSize(140, 140);
    scaledThumbnail = new QImage(pixmap.scaled(
            thumbnailSize,
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation).toImage());
    QVBoxLayout *layout = new QVBoxLayout();
    pushButton = new QPushButton();
    pushButton->setCheckable(true);
    QIcon icon(pixmap);
    pushButton->setIcon(icon);
    pushButton->setIconSize(thumbnailSize);
    QLabel* label = new QLabel(name);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(pushButton);
    layout->addWidget(label);
    this->setLayout(layout);
    connect(pushButton, SIGNAL(clicked(bool)), this, SLOT(buttonPress(bool)));
    // Multithreaded image update must hand off to single GUI thread for final GUI pixmap update.
    connect(this, SIGNAL(thumbnailImageUpdated()),
            this, SLOT(updateThumbnailIcon()));
}

GalleryButton::~GalleryButton() {
    if (scaledThumbnail) {
        buttonMutex.lock();
        delete scaledThumbnail;
        scaledThumbnail = NULL;
        buttonMutex.unlock();
    }
    if (correctedScaledThumbnail) {
        buttonMutex.lock();
        delete correctedScaledThumbnail;
        correctedScaledThumbnail = NULL;
        buttonMutex.unlock();
    }
}

void GalleryButton::buttonPress(bool checked) {
    emit declareChange(index, checked);
}

void GalleryButton::setBrightness(const BrightnessCalibrator<int>& calibrator)
{
    if (! scaledThumbnail) return;
    buttonMutex.lock(); // Avoid possible race condition
    if (bImageUpdating) {
        buttonMutex.unlock();
        return;
    }
    // possible race condition right here...
    bImageUpdating = true;
    buttonMutex.unlock(); // The race is over.  We won!

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
}
