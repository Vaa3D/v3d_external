/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it.

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) 鈥淰3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,鈥� Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) 鈥淎utomatic reconstruction of 3D neuron structures using a graph-augmented deformable model,鈥� Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




/*
 * renderer_gl2.h
 *
 *  Created on: Nov 20, 2008
 *      Author: ruanzongcai
 */

#ifndef RENDERER_GL2_H_
#define RENDERER_GL2_H_

#include "GLee2glew.h" ////2020-2-10

#include "glsl_r.h"
#include "renderer_gl1.h"


#define N_CHANNEL 3
#define FILL_CHANNEL 4  // power_of_two

/////////////////////////////////////////////////////////////////////////


class Renderer_gl2 : public Renderer_gl1
{
    friend class V3dr_colormapDialog; // for access colormap, colormap_curve

public:
    Renderer_gl2(void* widget);
    virtual ~Renderer_gl2();
    virtual const int class_version() {return 2;}

public:
// link to OpenGL window
    virtual void initialize(int version=2);

// link to View control
    virtual void toggleShader();
    virtual void toggleObjShader();
    virtual void applyColormapToImage();
    virtual RGB8 lookupColormap(RGB8 inC, int op);
    virtual void toggleTexStream();

    virtual void togglePolygonMode() {polygonMode = (polygonMode +1) %5;} // FILL,LINE,POINT, transparent,outline

// link to Rendering function
protected:
    virtual void loadShader();  // called by initialize()  	// makeCurrent
    virtual void cleanShader(); // called by ~Renderer_gl2 	// makeCurrent

    virtual void drawObj();  // called by paint()
    //virtual void drawVol();  // called by paint() //use default.

    virtual void equAlphaBlendingProjection();
    virtual void equMaxIntensityProjection();
    virtual void equMinIntensityProjection();
    virtual void equCrossSection();
    virtual void shaderTexBegin(bool stream); //090722 because different shader for XYZ/F-slice
    virtual void shaderTexEnd();
    virtual void initColormap();
    virtual bool supported_TexStream(); // called by loadVol()
    virtual void setupTexStreamBuffer(); // called by subloadTex()
    virtual void cleanTexStreamBuffer(); // called by ~Renderer_gl2 	// makeCurrent
    virtual void setupStackTexture(bool bfirst);
    virtual void _streamTex(int stack_i, int slice_i, int step, int slice0, int slice1);
    virtual void _streamTex_end();
    virtual bool _streamTex_ready();

public:
    RGBA8 colormap[FILL_CHANNEL][256];      // [n-channel][256-intensity]

////////////////////////////////////


    cwc::glShaderManager SMgr;
    cwc::glShader *shader, *shaderTex2D, *shaderTex3D, *shaderObj;

    GLuint texColormap; // nearest filter, [x-coord for intensity][y-coord for channel]
    // RGBA8 colormap[FILL_CHANNEL][256];      // [n-channel][256-intensity]
    QPolygonF colormap_curve[N_CHANNEL][4]; // [n-channel][RGBA]

    GLuint pboZ, pboY, pboX;
    GLenum pbo_texture_format, pbo_image_format, pbo_image_type;

private:
    void init_members()
    {
        shader = shaderTex2D=shaderTex3D = shaderObj = 0;
        texColormap = 0;
        pboZ = pboY = pboX = 0;
        pbo_texture_format = pbo_image_format = pbo_image_type = -1;
    }
};

// helping function

inline void set_colormap_curve(QPolygonF &curve, qreal x, int iy) // 0.0<=(x)<=1.0, 0<=(iy)<=255
{
    x = qMax(0.0, qMin(1.0,  x));
    qreal y = qMax(0.0, qMin(1.0,  iy/255.0));
    curve << QPointF(x, y);
}
inline void set_colormap_curve(QPolygonF &curve, qreal x, qreal y) // 0.0<=(x, y)<=1.0
{
    x = qMax(0.0, qMin(1.0,  x));
    y = qMax(0.0, qMin(1.0,  y));
    curve << QPointF(x, y);
}


#endif /* RENDERER_GL2_H_ */
