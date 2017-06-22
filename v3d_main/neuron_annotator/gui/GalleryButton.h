#ifndef GALLERYBUTTON_H
#define GALLERYBUTTON_H

#include "Na3DWidget.h"
#include "NeuronContextMenu.h"

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
  #include <QtWidgets>
#else
  #include <QtGui>
#endif

class GalleryButton : public QWidget
{
    Q_OBJECT

public:
    enum ButtonType {
        NEURON_BUTTON,
        OVERLAY_BUTTON
    };

    typedef NeuronSelectionModel::NeuronIndex NeuronIndex;
    static const int ThumbnailPixelHeight = 140;

    explicit GalleryButton(
            const QImage & image,
            QString name,
            int index,
            ButtonType type,
            QWidget *parent = 0);
    ~GalleryButton();
    int getIndex() { return index; }
    QString getLabelText() {return label->text();}
    QString getName() { return label->text(); }
    bool isChecked() { return pushButton->isChecked(); }
    void setChecked(bool checked) { pushButton->setChecked(checked); }
    virtual void paintEvent(QPaintEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    // virtual void mousePressEvent(QMouseEvent *);
    // void setNa3DWidget(Na3DWidget *inputNa3DWidget);
    void setContextMenu(NeuronContextMenu* menu);
    void setNeuronSelectionModel(const NeuronSelectionModel&);

signals:
    void fragmentVisibilityChanged(int index, bool visible);
    void fragmentSelectionChanged(int index, bool selected); // not used
    void fragmentHighlightChanged(NeuronIndex fragmentIndex, bool highlighted); // not used

public slots:
    /// Returns true if value changed
    bool setFragmentVisibility(bool checked);
    bool setFragmentSelection(bool selected);
    bool setFragmentHighlighted(bool highlighted);
    bool updateVisibility();
    // updateThumbnailIcon() updates the GUI pixmap for this button to reflect the
    // curent state of the internal correctedScaledThumbnail image.  Pixmap updates
    // like this MUST be done in the GUI thread, so multithreading is impossible for
    // this operation.
    void setThumbnailIcon(const QImage& scaledImage);
    void showContextMenu(QPoint point);
    void setLabelText(const QString& text) {label->setText(text);}

private:
    typedef QWidget super;

    QPushButton* pushButton;
    QLabel* label;
    NeuronIndex index;
    NeuronContextMenu* neuronContextMenu;
    bool bIsVisible;
    bool bIsSelected;
    bool bIsHighlighted;
    const NeuronSelectionModel* neuronSelectionModel;
    ButtonType buttonType;
};

#endif // GALLERYBUTTON_H
