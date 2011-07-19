#ifndef COMPARTMENTMAPWIDGET_H
#define COMPARTMENTMAPWIDGET_H

#include "../../3drenderer/v3dr_glwidget.h"
#include "../../3drenderer/renderer_tex2.h"
#include <cmath>
#include <iostream>
#include <QStringListModel>
#include <QListView>

#if defined (_MSC_VER)
#include "../basic_c_fun/vcdiff.h"
#else
#endif

// Define a class for visualizing compartment map
// interactive select compartment
class CompartmentMapWidget : public V3dR_GLWidget
{
    Q_OBJECT

public:
    CompartmentMapWidget(QWidget* parent);
    virtual ~CompartmentMapWidget();

public:
    void loadAtlas();
    void setComboBox(QComboBox *compartmentComboBox);
    void setCurrentIndex(int row, bool flag);

public slots:
    void switchCompartment(int num);

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
    QStringList compartmentList;

    QComboBox *pCompartmentComboBox;

    QListView* listView;
    QStringListModel model;

//    QList <Triangle*> list_listTriangle;
//    QList <GLuint> list_glistLabel;
//    BoundingBox labelBB;
};

#endif // COMPARTMENTMAPWIDGET_H
