#ifndef COMPARTMENTMAPWIDGET_H
#define COMPARTMENTMAPWIDGET_H

#include "../../3drenderer/v3dr_glwidget.h"
#include "../../v3d/xformwidget.h"
#include "../../3drenderer/renderer_tex2.h"
#include <cmath>

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
    void init();
    
public:
    //Renderer_tex2* renderer;
};

#endif // COMPARTMENTMAPWIDGET_H
