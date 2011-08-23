#ifndef NA_LARGE_MIP_WIDGET_H
#define NA_LARGE_MIP_WIDGET_H

#include <QWidget>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include "Na2DViewer.h"
#include "MouseClickManager.h"


// Large maximum intensity projection viewer for Neuron Annotator
// mode of V3D
class NaLargeMIPWidget : public Na2DViewer
{
    Q_OBJECT

public:
    NaLargeMIPWidget(QWidget* parent);
    virtual ~NaLargeMIPWidget();
    virtual void paintEvent(QPaintEvent *event);
    // Drag with mouse to translate
    virtual void mouseMoveEvent(QMouseEvent * event);
    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent * event);
    virtual void wheelEvent(QWheelEvent * e); // zoom with scroll wheel
    virtual void resizeEvent(QResizeEvent * event);
    int neuronAt(const QPoint& p);
    bool saveImage(QString filename);
    void setMipMergedData(const MipMergedData& mipMergedDataParam)
    {
        mipMergedData = &mipMergedDataParam;
        connect(mipMergedData, SIGNAL(dataChanged()),
                this, SLOT(initializePixmap()));
    }

signals:
    // message intended for main window status area
    void statusMessage(const QString&);
    void volumeDataUpdated(const My4DImage*, const My4DImage*);
    void hoverNeuronChanged(int);
    void neuronClicked(int);
    void neuronDisplayToggled(int neuronIndex, bool checked);

    void setProgress(int val);
    void setProgressMax(int val);
    void showProgress();
    void hideProgress();

public slots:
    virtual void toggleNeuronDisplay(NeuronSelectionModel::NeuronIndex index, bool checked);
    virtual void updateFullVolume() {/* TODO */}

    void initializePixmap(); // when a new image has loaded
    // Want to distinguish between double click and single click events
    void onMouseSingleClick(QPoint pos);

protected slots:
    void onHighlightedNeuronChanged(int neuronIndex);

protected:
    void resetView();
    void paintIntensityNumerals(QPainter& painter);
    void updatePixmap();

    // QImage * highlightedNeuronMaskImage;
    QPixmap highlightedNeuronMaskPixmap;
    int highlightedNeuronIndex;
    const MipMergedData * mipMergedData;

private:
    typedef Na2DViewer super;
};

#endif // NA_LARGE_MIP_WIDGET_H
