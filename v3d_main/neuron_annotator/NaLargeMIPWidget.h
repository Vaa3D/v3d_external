#ifndef NA_LARGE_MIP_WIDGET_H
#define NA_LARGE_MIP_WIDGET_H

#include "MipData.h"
#include <QWidget>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include "NaViewer.h"
#include "MouseClickManager.h"
#include "BrightnessCalibrator.h"

// MipDisplayImage is a derived class of QImage, intended
// to encapsulate gamma/HDR correctability.
class MipDisplayImage : public QObject
{
    Q_OBJECT

public:
    explicit MipDisplayImage();
    virtual ~MipDisplayImage();
    void setGamma(float gamma);

    MipData originalData;

    // contain rather than inherit...
    QImage image;
    typedef std::vector<QImage*> neuronHighlightImages_t;
    neuronHighlightImages_t neuronHighlightImages;

signals:
    void initialImageDataLoaded(); // primary MIP image ready for display
    void maskDataLoaded(); // ready for neuron mask toggling
    void processedXColumn(int);

public slots:
    void loadImageData(const My4DImage* img, const My4DImage* maskImg);
    void processedXColumnSlot(int);
    void onDataIntensitiesUpdated();
    // when a neuron has been toggled on or off
    void toggleNeuronDisplay(int neuronIndex, bool checked);

protected:
    unsigned char getCorrectedIntensity(float i_in) const;
    unsigned char getCorrectedIntensity(int x, int y, int c) const;
    void updateCorrectedIntensities();
    void load4DImage(const My4DImage* img, const My4DImage* maskImg = NULL);
    BrightnessCalibrator<float> brightnessCalibrator;
};

// Large maximum intensity projection viewer for Neuron Annotator
// mode of V3D
class NaLargeMIPWidget : public QWidget, public NaViewer
{
    Q_OBJECT

public:
    NaLargeMIPWidget(QWidget* parent);
    virtual ~NaLargeMIPWidget();
    // virtual bool loadMy4DImage(const My4DImage* my4DImage);
    virtual bool loadMy4DImage(const My4DImage* my4DImage, const My4DImage* maskImg = NULL);
    virtual void paintEvent(QPaintEvent *event);
    // Drag with mouse to translate
    virtual void mouseMoveEvent(QMouseEvent * event);
    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent * event);
    // double click to center
    virtual void mouseDoubleClickEvent(QMouseEvent * event);
    virtual void wheelEvent(QWheelEvent * e); // zoom with scroll wheel
    virtual void resizeEvent(QResizeEvent * event);
    void translateImage(int dx, int dy);
    int neuronAt(const QPoint& p);

signals:
    // message intended for main window status area
    void statusMessage(const QString&);
    void volumeDataUpdated(const My4DImage*, const My4DImage*);
    void hoverNeuronChanged(int);
    void neuronClicked(int);
    void neuronDisplayToggled(int neuronIndex, bool checked);

public slots:
    void annotationModelUpdate(QString updateType);
    void showCrosshair(bool b) {NaViewer::showCrosshair(b); update();}
    void initializePixmap(); // when a new image has loaded
    // Want to distinguish between double click and single click events
    void onMouseSingleClick(QPoint pos);
    void setGammaBrightness(double gamma);

protected slots:
    void onHighlightedNeuronChanged(int neuronIndex);

protected:
    void updateDefaultScale();
    void resetView();
    void paintIntensityNumerals(QPainter& painter);
    void updatePixmap();

    MipDisplayImage * mipImage;
    QPixmap pixmap;

    // QImage * highlightedNeuronMaskImage;
    QPixmap highlightedNeuronMaskPixmap;
    int highlightedNeuronIndex;

    QPainter painter;
    QTransform X_img_view;
    QTransform X_view_img;
    QThread imageUpdateThread;
    QProgressBar * progressBar;
    // Help distinguish between single clicks and double clicks
    MouseClickManager mouseClickManager;
};

#endif // NA_LARGE_MIP_WIDGET_H
