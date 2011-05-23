#ifndef NA3DWIDGET_H
#define NA3DWIDGET_H

#include "../3drenderer/v3dr_glwidget.h"
#include "../v3d/xformwidget.h"
#include "NaViewer.h"
#include "Rotation3D.h"

#if defined (_MSC_VER)
#include "../basic_c_fun/vcdiff.h"
#else
#endif

// Derived class of V3dR_GLWidget so we can use a simpler constructor
// for the convenience of Qt Designer,
// and to customize features for NeuronAnnotator.
class Na3DWidget : public V3dR_GLWidget, public NaViewer
{
    Q_OBJECT

public:
    Na3DWidget(QWidget* parent);
    virtual ~Na3DWidget();
    bool loadMy4DImage(const My4DImage* my4DImage, const My4DImage* neuronMaskImage = NULL);
    bool populateNeuronMask(const My4DImage* neuronMaskImage);

    const Vector3D& focus() const {return cameraModel.focus();}
    float getZoomScale() const; // in viewport pixels per image voxel at focus

    virtual void mouseMoveEvent(QMouseEvent * event);
    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent * event);
    virtual void wheelEvent(QWheelEvent * e); // zoom with scroll wheel
    virtual void mouseDoubleClickEvent(QMouseEvent * event); // double click to center

    virtual void resizeEvent(QResizeEvent * event);

public slots:
    void setGammaBrightness(double gamma);
    virtual void annotationModelUpdate(QString updateType);
    void resetView();
    void translateImage(int dx, int dy);
    void showCrosshair(bool b) {NaViewer::showCrosshair(b); update();}
	void updateHighlightNeurons(bool b);

protected slots:
    // focus setting should be done via cameraModel, not with these methods.
    void updateRendererZoomRatio(qreal relativeScale);
    void updateRotation(const Rotation3D&);
    void updateFocus(const Vector3D& f);
	
signals:
	void neuronSelected(double x, double y, double z);

protected:
    virtual void paintGL();
    void paintFiducial(const Vector3D& v);
    Vector3D getDefaultFocus() const;
    void prepareImageData();
    virtual void choiceRenderer();
    float glUnitsPerImageVoxel() const;
    void updateDefaultScale();
};

#endif // NA3DWIDGET_H
