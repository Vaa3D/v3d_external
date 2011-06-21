#ifndef GALLERYBUTTON_H
#define GALLERYBUTTON_H

#include <QtGui>
#include "BrightnessCalibrator.h"

class GalleryButton : public QWidget
{
    Q_OBJECT
public:
    explicit GalleryButton(const QPixmap & pixmap, QString name, int index, QWidget *parent = 0);
    int getIndex() { return index; }
    QString getName() { return label->text(); }
    bool isChecked() { return pushButton->isChecked(); }
    void setChecked(bool checked) { pushButton->setChecked(checked); }
    ~GalleryButton();

signals:
    void declareChange(int index, bool checked);
    // thumbnailImageUpdated() signal is emitted when the internal
    // "correctedScaledThumbnail" has changed, suggesting that a GUI
    // pixmap update would be a good idea.
    void thumbnailImageUpdated();

public slots:
    void buttonPress(bool checked);
    void setBrightness(const BrightnessCalibrator<int>& calibrator);

protected slots:
    // updateThumbnailIcon() updates the GUI pixmap for this button to reflect the
    // curent state of the internal correctedScaledThumbnail image.  Pixmap updates
    // like this MUST be done in the GUI thread, so multithreading is impossible for
    // this operation.
    void updateThumbnailIcon();

private:
    QPushButton* pushButton;
    QLabel* label;
    int index;
    // scaledThumbnail is a small version of the MIP image; small for efficient interactive updating.
    QImage * scaledThumbnail;
    // correctedScaledThumbnail is a gamma corrected version of scaledThumbnail.
    QImage * correctedScaledThumbnail;
    volatile bool bImageUpdating; // hack for gamma update
    QMutex buttonMutex; // Mutex object to help avoid multithread race conditions.
};

#endif // GALLERYBUTTON_H

