#ifndef GALLERYBUTTON_H
#define GALLERYBUTTON_H

#include <QtGui>
#include "BrightnessCalibrator.h"
#include "../FragmentSelectionModel.h"
#include "Na3DWidget.h"

class GalleryButton : public QWidget
{
    Q_OBJECT

public:
    typedef FragmentSelectionModel::FragmentIndex FragmentIndex;
    static const int ThumbnailPixelHeight = 140;

    explicit GalleryButton(const QImage & image, QString name, int index, QWidget *parent = 0);
    int getIndex() { return index; }
    QString getName() { return label->text(); }
    bool isChecked() { return pushButton->isChecked(); }
    void setChecked(bool checked) { pushButton->setChecked(checked); }
    ~GalleryButton();
    virtual void paintEvent(QPaintEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    void setNa3DWidget(Na3DWidget *inputNa3DWidget);

signals:
    void declareChange(int index, bool checked);
    // thumbnailImageUpdated() signal is emitted when the internal
    // "correctedScaledThumbnail" has changed, suggesting that a GUI
    // pixmap update would be a good idea.
    void thumbnailImageUpdated();
    void fragmentHover(FragmentIndex fragmentIndex);
    // void widgetChanged(FragmentIndex);

public slots:
    void buttonPress(bool checked);
    void setBrightness(const BrightnessCalibrator<int>& calibrator);

protected slots:
    // updateThumbnailIcon() updates the GUI pixmap for this button to reflect the
    // curent state of the internal correctedScaledThumbnail image.  Pixmap updates
    // like this MUST be done in the GUI thread, so multithreading is impossible for
    // this operation.
    void updateThumbnailIcon();
    void setThumbnailIcon(const QImage& scaledImage);

private:
    typedef QWidget super;

    QPushButton* pushButton;
    QLabel* label;
    FragmentIndex index;
    // scaledThumbnail is a small version of the MIP image; small for efficient interactive updating.
    QImage * scaledThumbnail;
    // correctedScaledThumbnail is a gamma corrected version of scaledThumbnail.
    QImage * correctedScaledThumbnail;
    volatile bool bImageUpdating; // hack for gamma update

    Na3DWidget *p3DWidget;
};

#endif // GALLERYBUTTON_H

