#ifndef GALLERYBUTTON_H
#define GALLERYBUTTON_H

#include <QtGui>
#include "Na3DWidget.h"

class GalleryButton : public QWidget
{
    Q_OBJECT

public:
    typedef NeuronSelectionModel::NeuronIndex NeuronIndex;
    static const int ThumbnailPixelHeight = 140;

    explicit GalleryButton(const QImage & image, QString name, int index, QWidget *parent = 0);
    ~GalleryButton();
    int getIndex() { return index; }
    QString getName() { return label->text(); }
    bool isChecked() { return pushButton->isChecked(); }
    void setChecked(bool checked) { pushButton->setChecked(checked); }
    virtual void paintEvent(QPaintEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    void setNa3DWidget(Na3DWidget *inputNa3DWidget);

signals:
    void declareChange(int index, bool checked);
    void fragmentHover(NeuronIndex fragmentIndex);

public slots:
    void buttonPress(bool checked);
    // updateThumbnailIcon() updates the GUI pixmap for this button to reflect the
    // curent state of the internal correctedScaledThumbnail image.  Pixmap updates
    // like this MUST be done in the GUI thread, so multithreading is impossible for
    // this operation.
    void setThumbnailIcon(const QImage& scaledImage);

private:
    typedef QWidget super;

    QPushButton* pushButton;
    QLabel* label;
    NeuronIndex index;

    // TODO - Storing a pointer to 3dwidget is poor encapsulation.  Whatever this does should proceed through signals.
    Na3DWidget *p3DWidget;
};

#endif // GALLERYBUTTON_H

