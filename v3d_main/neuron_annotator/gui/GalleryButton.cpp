#include <QtGui>
#include <cmath>
#include "GalleryButton.h"

GalleryButton::GalleryButton(const QImage & image, QString name, int index, QWidget *parent)
        : QWidget(parent)
{
    this->index=index;
    setMouseTracking(true); // respond to mouse hover events
    QVBoxLayout *layout = new QVBoxLayout();
    pushButton = new QPushButton();
    pushButton->setCheckable(true);
    QIcon icon(QPixmap::fromImage(image));
    pushButton->setIcon(icon);
    // QSize thumbnailSize(GalleryButton::ThumbnailPixelHeight, GalleryButton::ThumbnailPixelHeight);
    pushButton->setIconSize(image.size());
    label = new QLabel(name);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(pushButton);
    layout->addWidget(label);
    this->setLayout(layout);
    // setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    connect(pushButton, SIGNAL(clicked(bool)), this, SLOT(buttonPress(bool)));
}

GalleryButton::~GalleryButton()
{}

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

void GalleryButton::mousePressEvent(QMouseEvent * event)
{
    if (event->button()==Qt::RightButton)
    {
        if(getName().isEmpty())
        {
            return;
        }
        else if(getName().contains("Neuron"))
        {
            p3DWidget->setNeuronIndex(index);
        }
        else
        {
            p3DWidget->setNeuronIndex(-index);
        }

        p3DWidget->onMouseRightClickMenu(event, false);
    }
}

void GalleryButton::buttonPress(bool checked) {
    emit declareChange(index, checked);
}

void GalleryButton::setThumbnailIcon(const QImage& scaledImage)
{
    pushButton->setIcon(QIcon(QPixmap::fromImage(scaledImage)));
    pushButton->setIconSize(scaledImage.size());
}

void GalleryButton::setNa3DWidget(Na3DWidget *inputNa3DWidget){
    p3DWidget = inputNa3DWidget;
}
