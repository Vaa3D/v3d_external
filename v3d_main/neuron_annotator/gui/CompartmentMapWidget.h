#ifndef COMPARTMENTMAPWIDGET_H
#define COMPARTMENTMAPWIDGET_H

#include <iostream>
#include "../../3drenderer/v3dr_glwidget.h"
#include "../../3drenderer/renderer_gl1.h"
#include "../geometry/Rotation3D.h"
#include "CompartmentMapComboBox.h"

// Define a class for visualizing compartment map
// interactive select compartment
class CompartmentMapWidget : public V3dR_GLWidget
{
    //Q_OBJECT

public:
    CompartmentMapWidget(QWidget* parent);
    virtual ~CompartmentMapWidget();

public:
    void loadAtlas();
    void setComboBox(CompartmentMapComboBox *compartmentComboBox);
    void setCurrentIndex(int row, bool flag);

//signals:
    void viscomp3dview(const QList <LabelSurf>);

//public slots:
    void switchCompartment(int num);
    void setRotation(const Rotation3D&);
    void setFocus(const Vector3D& f);

protected:
    virtual void initializeGL();
    virtual void resizeGL(int width, int height);
    virtual void paintGL();

    virtual void focusInEvent(QFocusEvent* e);
    virtual void focusOutEvent(QFocusEvent* e);
    virtual void enterEvent(QEvent *e);
    virtual void leaveEvent(QEvent *e);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);
    
public:
    QList <LabelSurf> listLabelSurf; // labelfield surf
    CompartmentMapComboBox *pCompartmentComboBox;

//    QList <Triangle*> list_listTriangle;
//    QList <GLuint> list_glistLabel;
//    BoundingBox labelBB;
};

#endif // COMPARTMENTMAPWIDGET_H
