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

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




/****************************************************************************
**
** Copyright (C) 2005-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file.  Alternatively you may (at
** your option) use any later version of the GNU General Public
** License if such license has been publicly approved by Trolltech ASA
** (or its successors, if any) and the KDE Free Qt Foundation. In
** addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.2, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/. If
** you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech, as the sole
** copyright holder for Qt Designer, grants users of the Qt/Eclipse
** Integration plug-in the right for the Qt/Eclipse Integration to
** link to functionality provided by Qt Designer and its related
** libraries.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not expressly
** granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef GRADIENTS_H
#define GRADIENTS_H
#include <QWidget>
#include <version_control.h>
#if defined(USE_Qt5)
#include <QtWidgets>
#else
#include <QtGui>
#endif
//#include <hoverpoints.h> //by PHC 2020/1/31
#include "v3d_hoverpoints.h"
#include <QRadioButton>
class ShadeWidget : public QWidget
{
    Q_OBJECT;

public:
    enum ShadeType {
        RedShade   = 0,
        GreenShade = 1,
        BlueShade  = 2,
        ARGBShade  = 3,
    };

    ShadeWidget(ShadeType type, QWidget *parent);

    void setGradientStops(const QGradientStops &stops);
    void setSize(const QSize s) { m_size = s; }

    QSize sizeHint() const { return m_size; }
    HoverPoints* hoverPoints() const { return m_hoverPoints; }

    QRgb colorF(qreal f); // 0<=f<=1
    QRgb colorX(qreal x); //to interpolate color from stops of curve

public slots:
    void changeColors(const QPolygonF &pts) { emit colorsChanged(m_shade_type, pts); } //m_hoverPoints --> this
signals:
    void colorsChanged(int type, const QPolygonF &); // this --> parent(GradientEditor)

protected:
    virtual void paintEvent(QPaintEvent *e);

private:
    void generateShade();

    ShadeType m_shade_type;
    QSize m_size;
    QImage m_shade_image;             // shade buffer
    HoverPoints *m_hoverPoints;
    QLinearGradient m_alpha_gradient; // a brush
};


class GradientEditor : public QWidget
{
    Q_OBJECT;

public:
    GradientEditor(QWidget *parent);

    void setGradientStops(bool mask[4], const QGradientStops &stops);
    void setGradientStops(const QGradientStops &stops) { bool mask[4] = {1,1,1,1};	setGradientStops(mask, stops); }

    void setNormalCurve(int j, const QPolygonF &curve);
    QPolygonF normalCurve(int j) const;  // 0.0<=(x,y)<=1.0, y from bottom to top

    QRgb colorF(qreal f) //0<=f<=1
    {
        int r,g,b,a;    	r=g=b=a=0;
        if (m_red_shade)	r = qRed(m_red_shade->colorF(f));
        if (m_green_shade)	g = qGreen(m_green_shade->colorF(f));
        if (m_blue_shade)	b = qBlue(m_blue_shade->colorF(f));
        if (m_alpha_shade)	a = qAlpha(m_alpha_shade->colorF(f));
        return qRgba(r, g, b, a);
    }

public slots:
    void pointsUpdated(int type, const QPolygonF &);
    QGradientStops updateAlphaStops(); //081220, Separated from pointsUpdated

signals:
    void gradientStopsChanged(const QGradientStops&); //trigger external slot to output colormap

protected:
    ShadeWidget *m_red_shade;
    ShadeWidget *m_green_shade;
    ShadeWidget *m_blue_shade;
    ShadeWidget *m_alpha_shade;

};




class GradientRenderer : public QWidget //ArthurFrame
{
    Q_OBJECT;

public:
    GradientRenderer(QWidget *parent);
    void paint(QPainter *p);

    QSize sizeHint() const { return QSize(400, 400); }

    HoverPoints *hoverPoints() const { return m_hoverPoints; }
    void mousePressEvent(QMouseEvent *e);

public slots:
    void setGradientStops(const QGradientStops &stops);

    void setPadSpread() { m_spread = QGradient::PadSpread; update(); }
    void setRepeatSpread() { m_spread = QGradient::RepeatSpread; update(); }
    void setReflectSpread() { m_spread = QGradient::ReflectSpread; update(); }

    void setLinearGradient() { m_gradientType = Qt::LinearGradientPattern; update(); }
    void setRadialGradient() { m_gradientType = Qt::RadialGradientPattern; update(); }
    void setConicalGradient() { m_gradientType = Qt::ConicalGradientPattern; update(); }


private:
    QGradientStops m_stops;
    HoverPoints *m_hoverPoints;

    QGradient::Spread m_spread;
    Qt::BrushStyle m_gradientType;
};


class GradientWidget : public QWidget
{
    Q_OBJECT;

public:
    GradientWidget(QWidget *parent=0);

public slots:
    void setDefault1() { setDefault(1); }
    void setDefault2() { setDefault(2); }
    void setDefault3() { setDefault(3); }
    void setDefault4() { setDefault(4); }

private:
    void setDefault(int i);

    GradientRenderer *m_renderer;
    GradientEditor *m_editor;

    QRadioButton *m_linearButton;
    QRadioButton *m_radialButton;
    QRadioButton *m_conicalButton;
    QRadioButton *m_padSpreadButton;
    QRadioButton *m_reflectSpreadButton;
    QRadioButton *m_repeatSpreadButton;

};

#endif // GRADIENTS_H
