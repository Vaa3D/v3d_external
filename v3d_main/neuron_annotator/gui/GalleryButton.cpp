
#include <cmath>
#include "GalleryButton.h"

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
  #include <QtWidgets>
#else
  #include <QtGui>
#endif

GalleryButton::GalleryButton(
        const QImage & image,
        QString name,
        int indexParam,
        ButtonType type,
        QWidget *parent)
    : QWidget(parent)
    , index(indexParam)
    , buttonType(type)
    , neuronContextMenu(NULL)
    , bIsVisible(true)
    , bIsSelected(false)
    , bIsHighlighted(false)
    , neuronSelectionModel(NULL)
{
    setMouseTracking(true); // respond to mouse hover events
    QVBoxLayout *layout = new QVBoxLayout();
    pushButton = new QPushButton();
    pushButton->setCheckable(true);
    QIcon icon(QPixmap::fromImage(image));
    pushButton->setIcon(icon);
    // QSize thumbnailSize(GalleryButton::ThumbnailPixelHeight, GalleryButton::ThumbnailPixelHeight);
    pushButton->setIconSize(image.size());
    label = new QLabel(name);
    label->setAlignment(Qt::AlignLeft);
    layout->addWidget(pushButton);
    layout->addWidget(label);
    this->setLayout(layout);
    // setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    connect(pushButton, SIGNAL(toggled(bool)), this, SLOT(setFragmentVisibility(bool)));
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
    setFragmentHighlighted(true);
}

void GalleryButton::setThumbnailIcon(const QImage& scaledImage)
{
    pushButton->setIcon(QIcon(QPixmap::fromImage(scaledImage)));
    pushButton->setIconSize(scaledImage.size());
}

void GalleryButton::setContextMenu(NeuronContextMenu* menu)
{
    if (!menu) return;
    neuronContextMenu = menu;
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));
}

void GalleryButton::setNeuronSelectionModel(const NeuronSelectionModel& n)
{
    // qDebug() << "GalleryButton::setNeuronSelectionModel" << index;
    neuronSelectionModel = &n;
    switch(buttonType) {
    case NEURON_BUTTON:
        connect(this, SIGNAL(fragmentVisibilityChanged(int,bool)),
                neuronSelectionModel, SLOT(updateNeuronMask(int,bool)));
        connect(neuronSelectionModel, SIGNAL(visibilityChanged()),
                this, SLOT(updateVisibility()));
        break;
    case OVERLAY_BUTTON:
        // Reference channel is initially OFF.  Ensure it is set that way
        {
            NeuronSelectionModel::Reader reader(n);
            if (reader.hasReadLock()) {
                if (reader.getOverlayStatusList().size() > index)
                    setFragmentVisibility(reader.getOverlayStatusList()[index]);
            }
        }
        connect(this, SIGNAL(fragmentVisibilityChanged(int,bool)),
                neuronSelectionModel, SLOT(updateOverlay(int,bool)));
        connect(neuronSelectionModel, SIGNAL(visibilityChanged()),
                this, SLOT(updateVisibility()));
        break;
    }
}

/* slot */
bool GalleryButton::setFragmentVisibility(bool visible)
{
    // qDebug() << "GalleryButton::setFragmentVisibility" << visible << bIsVisible << index;
    if (visible == bIsVisible) return false; // no change
    bIsVisible = visible;
    setChecked(visible);
    emit fragmentVisibilityChanged(index, bIsVisible);
    return true;
}

/* slot */
bool GalleryButton::setFragmentSelection(bool selected)
{
    if (selected == bIsSelected) return false; // no change
    bIsSelected = selected;
    emit fragmentSelectionChanged(index, bIsSelected);
    return true;
}

/* slot */
bool GalleryButton::setFragmentHighlighted(bool highlighted)
{
    if (highlighted == bIsHighlighted) return false; // no change
    bIsHighlighted = highlighted;
    emit fragmentHighlightChanged(index, bIsHighlighted);
    return true;
}

/* slot */
bool GalleryButton::updateVisibility()
{
    // qDebug() << "GalleryButton::updateVisibility()" << index;
    if (NULL == neuronSelectionModel) return false;
    if (index < 0) return false;
    bool newVisibility = true;
    {
        if (! neuronSelectionModel->representsActualData())
            return false;
        NeuronSelectionModel::Reader selectionReader(*neuronSelectionModel);
        if (! selectionReader.hasReadLock())
            return false;;
        switch(buttonType) {
            case NEURON_BUTTON:
                if (selectionReader.getMaskStatusList().size() <= index) return false;
                newVisibility = selectionReader.getMaskStatusList()[index];
                break;
            case OVERLAY_BUTTON:
                if (selectionReader.getOverlayStatusList().size() <= index) return false;
                newVisibility = selectionReader.getOverlayStatusList()[index];
                break;
        }
    } // release lock
    setFragmentVisibility(newVisibility);
    return true;
}

/* slot */
void GalleryButton::showContextMenu(QPoint point)
{
    if (! neuronContextMenu) return;
    bool neuronIsVisible = true;
    {
        NeuronSelectionModel::Reader selectionReader(*neuronSelectionModel);
        if (selectionReader.hasReadLock())
            neuronIsVisible = selectionReader.getMaskStatusList()[index];
    }
    neuronContextMenu->exec(mapToGlobal(point), index, neuronIsVisible);
}
