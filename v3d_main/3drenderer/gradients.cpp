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

#include "gradients.h"

#define __running_road__
//------------ initialize -------------------------------------------------------------------------------
// GradientEditor::setNormalCurve()                       /// input colormap curve
// {
// 	   set_shade_points();
//     updateAlphaStops();
// }
//------------ signal road --------------------------------------------------------------------------------
// ShadeWidget->connect(HoverPoints, SIGNAL(pointsChanged()), ShadeWidget, SIGNAL(colorsChanged()));
// GradientEditor->connect(ShadeWidget, SIGNAL(colorsChanged()), GradientEditor, SLOT(pointsUpdated()));
// GradientEditor::pointsUpdated()
// {
//     updateAlphaStops(); // combined stops from {R G B A} ShadeWidget
//     m_alpha_shade->setGradientStops() { m_alpha_shade->generateShade(); }
//	   emit gradientStopsChanged();                       /// trigger output colormap
// }
// this->connect(GradientEditor, SIGNAL(gradientStopsChanged()), this, SLOT(updateColormap()))
//-------------------------------------------------------------------------------------------------------


#define __ShadeWidget__

ShadeWidget::ShadeWidget(ShadeType type, QWidget *parent)
    : QWidget(parent), m_shade_type(type)
{
	m_size = QSize(150, 50);
	m_alpha_gradient = QLinearGradient(0, 0, 0, 0);

	// Checkers background
    if (m_shade_type == ARGBShade) {
        QPixmap pm(20, 20);
        QPainter pmp(&pm);
        pmp.fillRect(0, 0, 10, 10, Qt::lightGray);
        pmp.fillRect(10, 10, 10, 10, Qt::lightGray);
        pmp.fillRect(0, 10, 10, 10, Qt::darkGray);
        pmp.fillRect(10, 0, 10, 10, Qt::darkGray);
        pmp.end();
        QPalette pal = this->palette();
        pal.setBrush(backgroundRole(), QBrush(pm));
        setPalette(pal);
        setAutoFillBackground(true);

    } else {
        setAttribute(Qt::WA_NoBackground); //filled by gradient brush later

    }

    m_hoverPoints = new HoverPoints(this, HoverPoints::CircleShape);
    m_hoverPoints->setSortType(HoverPoints::XSort);
    m_hoverPoints->setConnectionType(HoverPoints::LineConnection);


// 081220, these points will make many rubbish signal
//    QPolygonF points;
//    points << QPointF(0, sizeHint().height())  << QPointF(sizeHint().width(), 0);
//    m_hoverPoints->setPoints(points);
//    m_hoverPoints->setPointLock(0, HoverPoints::LockToLeft);
//    m_hoverPoints->setPointLock(1, HoverPoints::LockToRight);


    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred); //081215

    //connect(m_hoverPoints, SIGNAL(pointsChanged(const QPolygonF &)), this, SIGNAL(colorsChanged(const QPolygonF &)));
    connect(m_hoverPoints, SIGNAL(pointsChanged(const QPolygonF &)), this, SLOT(changeColors(const QPolygonF &)));
}


QRgb ShadeWidget::colorF(qreal f) // 0<=f<=1
{
	//note: this->width() is same as m_hoverPoints->boundingRect().height()
	return colorX( f * width() );
}

QRgb ShadeWidget::colorX(qreal x) //also called by GradientEditor::updateAlphaStops()
{
	//TODO: using normalized point position is the best solution

    QRgb argb = 0; //0xffffffff;
    QPolygonF pts = m_hoverPoints->points();
    QRectF bound = m_hoverPoints->boundingRect();  //110721

    //Q_ASSERT(pts.size()>=2);
    for (int i=1; i < pts.size(); ++i)  //if (pts.size()<2) do nothing
    {
    	//110721, clamp x into valid range, fixed bug of fast horizontal resize
        if (x < pts.first().x())  x = pts.first().x();
        if (x > pts.last().x())   x = pts.last().x();

        if (pts.at(i-1).x() <= x && x <= pts.at(i).x()) //found the segment that x belongs to
        {
        	qreal x1 = pts.at(i-1).x();
        	qreal y1 = pts.at(i-1).y();
        	qreal x2 = pts.at(i).x();
        	qreal y2 = pts.at(i).y();
        	if (x2-x1 <= 0)
        		continue;
        	qreal y = (x-x1)/(x2-x1)*(y2-y1) +y1;

            qreal h = bound.height(); //110721, fixed bug of fast vertical resize
					//m_shade_image.height(); //this is wrong
    		y = 1 - qMax(0.0, qMin(h, y))/h;
    		int I = (y*255); //090719

    		if (m_shade_type == RedShade)
    		{
        		argb = qRgba( I,0,0, 255 );
    		}
    		else if (m_shade_type == GreenShade)
    		{
        		argb = qRgba( 0,I,0, 255 );
    		}
    		else if (m_shade_type == BlueShade)
    		{
        		argb = qRgba( 0,0,I, 255 );
    		}
            else //ARGBShade
            {
        		argb = qRgba( 0,0,0, I ); //081215
            }

    		break;//break loop of finding
        }
    }
    return argb;
}


void ShadeWidget::setGradientStops(const QGradientStops &stops)
{
    if (m_shade_type == ARGBShade) {

    	m_alpha_gradient = QLinearGradient(0, 0, width(), 0);
        m_alpha_gradient.setStops(stops);

        m_shade_image = QImage();
        generateShade();
        update();
    }
}

void ShadeWidget::generateShade()
{
    if (! m_shade_image.isNull() && m_shade_image.size()==size()) return;

	if (m_shade_type == ARGBShade)
	{
		m_shade_image = QImage(size(), QImage::Format_ARGB32_Premultiplied);
		m_shade_image.fill(0); //090722 this is needed on Win32 but not on MacX.

		QPainter p(&m_shade_image);

		p.fillRect(rect(), m_alpha_gradient);

	}
	else
	{
		QLinearGradient shade(0, 0, 0, height());

		m_shade_image = QImage(size(), QImage::Format_RGB32);
		QPainter p(&m_shade_image);

		shade.setColorAt(1, Qt::black);      //rect bottom is black
		if (m_shade_type == RedShade)
			shade.setColorAt(0, Qt::red);    //rect top is red
		else if (m_shade_type == GreenShade)
			shade.setColorAt(0, Qt::green);  //rect top is green
		else
			shade.setColorAt(0, Qt::blue);   //rect top is blue

		p.fillRect(rect(), shade);
	}
}


void ShadeWidget::paintEvent(QPaintEvent *)
{
    generateShade();

    QPainter p(this);
    p.drawImage(0, 0, m_shade_image);

    p.setPen(QColor(146, 146, 146));
    p.drawRect(0, 0, width() - 1, height() - 1);
}


///////////////////////////////////////////////////////////////////
#define __GradientEditor__

GradientEditor::GradientEditor(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *vbox = new QVBoxLayout(parent); // CORRECTION: this==>parent, 081214 RZC
    vbox->setSpacing(10);
    vbox->setMargin(5);

    m_red_shade=m_green_shade=m_blue_shade=m_alpha_shade=0;
    m_red_shade = new ShadeWidget(ShadeWidget::RedShade, this);
    m_green_shade = new ShadeWidget(ShadeWidget::GreenShade, this);
    m_blue_shade = new ShadeWidget(ShadeWidget::BlueShade, this);
    m_alpha_shade = new ShadeWidget(ShadeWidget::ARGBShade, this);

    vbox->addWidget(m_red_shade);
    vbox->addWidget(m_green_shade);
    vbox->addWidget(m_blue_shade);
    vbox->addWidget(m_alpha_shade);

    connect(m_red_shade, SIGNAL(colorsChanged(int, const QPolygonF &)), this, SLOT(pointsUpdated(int, const QPolygonF &)));
    connect(m_green_shade, SIGNAL(colorsChanged(int, const QPolygonF &)), this, SLOT(pointsUpdated(int, const QPolygonF &)));
    connect(m_blue_shade, SIGNAL(colorsChanged(int, const QPolygonF &)), this, SLOT(pointsUpdated(int, const QPolygonF &)));
    connect(m_alpha_shade, SIGNAL(colorsChanged(int, const QPolygonF &)), this, SLOT(pointsUpdated(int, const QPolygonF &)));
}


void GradientEditor::pointsUpdated(int type, const QPolygonF &pts)
{
//	qDebug() << "GradientEditor::pointsUpdated";
//	qDebug() << QString("type[%1]").arg(type) << pts;

	if (pts.size()<2)  return; //081220, Rubbish signal

	QGradientStops stops = updateAlphaStops();

    emit gradientStopsChanged(stops); //trigger external slot to output colormap
}

inline static bool my_x_less_than(const QPointF &p1, const QPointF &p2)
{
    return p1.x() < p2.x();
}


inline static bool my_y_less_than(const QPointF &p1, const QPointF &p2)
{
    return p1.y() < p2.y();
}


QGradientStops GradientEditor::updateAlphaStops()
{
	//qDebug() << "GradientEditor::updtaAlphaStops";

	qreal w = m_alpha_shade->width();

    QPolygonF points;
    if (m_red_shade)    points += m_red_shade->hoverPoints()->points();
    if (m_green_shade)  points += m_green_shade->hoverPoints()->points();
    if (m_blue_shade)   points += m_blue_shade->hoverPoints()->points();
    if (m_alpha_shade)  points += m_alpha_shade->hoverPoints()->points();

    qSort(points.begin(), points.end(), my_x_less_than);

    QGradientStops stops;

    if (points.size()>1) //110721
    	for (int i=0; i<points.size(); i++)
		{
			qreal x = (points.at(i).x());
			if (x<0 || x > w) 	continue;
			//if (i < points.size()-1 && x == points.at(i+1).x()) 	continue;

			QColor color(qRed(  m_red_shade->colorX(x)), // 081215
						 qGreen(m_green_shade->colorX(x)),
						 qBlue( m_blue_shade->colorX(x)),
						 qAlpha(m_alpha_shade->colorX(x))
						 );

			stops << QGradientStop(x/w, color);

			if (x>=w) break; //090719
		}

    m_alpha_shade->setGradientStops(stops);  //generateShade, only for ARGBShade type
    return stops;
}

//////////////////////////////////////////////////////////////////////

static void set_shade_points(const QPolygonF &points, ShadeWidget *shade)
{
    if (!shade || points.size()<=0)
        return;
    shade->hoverPoints()->setPoints(points);
    shade->hoverPoints()->setPointLock(0, HoverPoints::LockToLeft);
    shade->hoverPoints()->setPointLock(points.size()-1, HoverPoints::LockToRight);
    shade->update();
}

void GradientEditor::setGradientStops(bool mask[4], const QGradientStops &stops)
{
    qreal w_red = m_red_shade->width();
    qreal w_green = m_green_shade->width();
    qreal w_blue = m_blue_shade->width();
    qreal w_alpha = m_alpha_shade->width();

    qreal h_red = m_red_shade->height();
    qreal h_green = m_green_shade->height();
    qreal h_blue = m_blue_shade->height();
    qreal h_alpha = m_alpha_shade->height();

    QPolygonF pts_red, pts_green, pts_blue, pts_alpha;

    for (int i=0; i<stops.size(); i++)
    {
        qreal pos = qMax(0.0, qMin(1.0, stops.at(i).first));
        QRgb color = stops.at(i).second.rgba();

        if (mask[0])  pts_red   << QPointF(pos * w_red,   (1 - qRed(color)/255.0)   * h_red);
        if (mask[1])  pts_green << QPointF(pos * w_green, (1 - qGreen(color)/255.0) * h_green);
        if (mask[2])  pts_blue  << QPointF(pos * w_blue,  (1 - qBlue(color)/255.0)  * h_blue);
        if (mask[3])  pts_alpha << QPointF(pos * w_alpha, (1 - qAlpha(color)/255.0) * h_alpha);
    }

    if (mask[0])  set_shade_points(pts_red, m_red_shade);
    if (mask[1])  set_shade_points(pts_green, m_green_shade);
    if (mask[2])  set_shade_points(pts_blue, m_blue_shade);
    if (mask[3])  set_shade_points(pts_alpha, m_alpha_shade);

    //pointsUpdated(); //081215
    updateAlphaStops(); //081220
}

void GradientEditor::setNormalCurve(int j, const QPolygonF &curve)
{
    if (j<0 || j>3)  return;

	qreal w_red = m_red_shade->width();
    qreal w_green = m_green_shade->width();
    qreal w_blue = m_blue_shade->width();
    qreal w_alpha = m_alpha_shade->width();

    qreal h_red = m_red_shade->height();
    qreal h_green = m_green_shade->height();
    qreal h_blue = m_blue_shade->height();
    qreal h_alpha = m_alpha_shade->height();

    QPolygonF pts;

    for (int i=0; i<curve.size(); i++)
    {
        qreal x = qMax(0.0, qMin(1.0,  curve.at(i).x()));
        qreal y = qMax(0.0, qMin(1.0,  curve.at(i).y()));

        if (j==0)   pts << QPointF(x * w_red,    (1-y) * h_red);  // flip-y, 081220
        if (j==1)   pts << QPointF(x * w_green,  (1-y) * h_green);
        if (j==2)   pts << QPointF(x * w_blue,   (1-y) * h_blue);
        if (j==3)   pts << QPointF(x * w_alpha,  (1-y) * h_alpha);
    }

    if (j==0)  set_shade_points(pts, m_red_shade);
    if (j==1)  set_shade_points(pts, m_green_shade);
    if (j==2)  set_shade_points(pts, m_blue_shade);
    if (j==3)  set_shade_points(pts, m_alpha_shade);

    //pointsUpdated(); //081215
    updateAlphaStops(); //081220
}

QPolygonF GradientEditor::normalCurve(int j) const
{
	qreal w_red = m_red_shade->width();
    qreal w_green = m_green_shade->width();
    qreal w_blue = m_blue_shade->width();
    qreal w_alpha = m_alpha_shade->width();

    qreal h_red = m_red_shade->height();
    qreal h_green = m_green_shade->height();
    qreal h_blue = m_blue_shade->height();
    qreal h_alpha = m_alpha_shade->height();

    QPolygonF pts;

	if (j==0 && m_red_shade)	pts = m_red_shade->hoverPoints()->points();
	if (j==1 && m_green_shade)  pts = m_green_shade->hoverPoints()->points();
	if (j==2 && m_blue_shade)   pts = m_blue_shade->hoverPoints()->points();
	if (j==3 && m_alpha_shade)  pts = m_alpha_shade->hoverPoints()->points();

	for (int i=0; i<pts.size(); i++) // normalize & flip-y, 081220
	{
        qreal x = pts.at(i).x();
        qreal y = pts.at(i).y();

        if (j==0)   pts[i] = QPointF(x / w_red,    1 - y / h_red);  // flip-y, 081220
        if (j==1)   pts[i] = QPointF(x / w_green,  1 - y / h_green);
        if (j==2)   pts[i] = QPointF(x / w_blue,   1 - y / h_blue);
        if (j==3)   pts[i] = QPointF(x / w_alpha,  1 - y / h_alpha);
	}

    //qDebug() << "normalCurve" << pts;
	return pts;
}


//the follows are not used in v3d
#define __not_used_in_v3d__
//////////////////////////////////////////////////////////////////////
#define __GradientRenderer__

GradientRenderer::GradientRenderer(QWidget *parent)
	:QWidget(parent)//    : ArthurFrame(parent)
{
    m_hoverPoints = new HoverPoints(this, HoverPoints::CircleShape);
    m_hoverPoints->setPointSize(QSize(20, 20));
    m_hoverPoints->setConnectionType(HoverPoints::NoConnection);
    m_hoverPoints->setEditable(false);

    QVector<QPointF> points;
    points << QPointF(100, 100) << QPointF(200, 200);
    m_hoverPoints->setPoints(points);

    m_spread = QGradient::PadSpread;
    m_gradientType = Qt::LinearGradientPattern;
}

void GradientRenderer::setGradientStops(const QGradientStops &stops)
{
    m_stops = stops;
    update();
}


void GradientRenderer::mousePressEvent(QMouseEvent *)
{
//    setDescriptionEnabled(false);
}

void GradientRenderer::paint(QPainter *p)
{
    QPolygonF pts = m_hoverPoints->points();

    QGradient g;

    if (m_gradientType == Qt::LinearGradientPattern) {
        g = QLinearGradient(pts.at(0), pts.at(1));

    } else if (m_gradientType == Qt::RadialGradientPattern) {
        QLineF line(pts.at(0), pts.at(1));
        if (line.length() > 132)
            line.setLength(132);
        g = QRadialGradient(line.p1(), qMin(width(), height()) / 3.0, line.p2());
    } else {
        QLineF l(pts.at(0), pts.at(1));
        qreal angle = l.angle(QLineF(0, 0, 1, 0));
        if (l.dy() > 0)
            angle = 360 - angle;
        g = QConicalGradient(pts.at(0), angle);
    }

    for (int i=0; i<m_stops.size(); ++i)
        g.setColorAt(m_stops.at(i).first, m_stops.at(i).second);

    g.setSpread(m_spread);

    p->setBrush(g);
    p->setPen(Qt::NoPen);

    p->drawRect(rect());

}

///////////////////////////////////////////////////////////////////////
#define __GradientWidget__

GradientWidget::GradientWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Gradients");

    m_renderer = new GradientRenderer(this);

    QGroupBox *mainGroup = new QGroupBox(this);
    mainGroup->setTitle("Gradients");

    QGroupBox *editorGroup = new QGroupBox(mainGroup);
    editorGroup->setAttribute(Qt::WA_ContentsPropagated);
    editorGroup->setTitle("Color Editor");
    m_editor = new GradientEditor(editorGroup);

    QGroupBox *typeGroup = new QGroupBox(mainGroup);
    typeGroup->setAttribute(Qt::WA_ContentsPropagated);
    typeGroup->setTitle("Gradient Type");
    m_linearButton = new QRadioButton("Linear Gradient", typeGroup);
    m_radialButton = new QRadioButton("Radial Gradient", typeGroup);
    m_conicalButton = new QRadioButton("Conical Gradient", typeGroup);

    QGroupBox *spreadGroup = new QGroupBox(mainGroup);
    spreadGroup->setAttribute(Qt::WA_ContentsPropagated);
    spreadGroup->setTitle("Spread Method");
    m_padSpreadButton = new QRadioButton("Pad Spread", spreadGroup);
    m_reflectSpreadButton = new QRadioButton("Reflect Spread", spreadGroup);
    m_repeatSpreadButton = new QRadioButton("Repeat Spread", spreadGroup);

    QGroupBox *defaultsGroup = new QGroupBox(mainGroup);
    defaultsGroup->setAttribute(Qt::WA_ContentsPropagated);
    defaultsGroup->setTitle("Defaults");
    QPushButton *default1Button = new QPushButton("1", defaultsGroup);
    QPushButton *default2Button = new QPushButton("2", defaultsGroup);
    QPushButton *default3Button = new QPushButton("3", defaultsGroup);
    QPushButton *default4Button = new QPushButton("Reset", editorGroup);

    QPushButton *showSourceButton = new QPushButton(mainGroup);
    showSourceButton->setText("Show Source");
#ifdef QT_OPENGL_SUPPORT
//    QPushButton *enableOpenGLButton = new QPushButton(mainGroup);
//    enableOpenGLButton->setText("Use OpenGL");
//    enableOpenGLButton->setCheckable(true);
//    enableOpenGLButton->setChecked(m_renderer->usesOpenGL());
//    if (!QGLFormat::hasOpenGL())
//        enableOpenGLButton->hide();
#endif
    QPushButton *whatsThisButton = new QPushButton(mainGroup);
    whatsThisButton->setText("What's This?");
    whatsThisButton->setCheckable(true);

    // Layouts
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(m_renderer);
    mainLayout->addWidget(mainGroup);

    mainGroup->setFixedWidth(180);
    QVBoxLayout *mainGroupLayout = new QVBoxLayout(mainGroup);
    mainGroupLayout->addWidget(editorGroup);
    mainGroupLayout->addWidget(typeGroup);
    mainGroupLayout->addWidget(spreadGroup);
    mainGroupLayout->addWidget(defaultsGroup);
    mainGroupLayout->addStretch(1);
    mainGroupLayout->addWidget(showSourceButton);
#ifdef QT_OPENGL_SUPPORT
//    mainGroupLayout->addWidget(enableOpenGLButton);
#endif
    mainGroupLayout->addWidget(whatsThisButton);

    QVBoxLayout *editorGroupLayout = new QVBoxLayout(editorGroup);
    editorGroupLayout->addWidget(m_editor);

    QVBoxLayout *typeGroupLayout = new QVBoxLayout(typeGroup);
    typeGroupLayout->addWidget(m_linearButton);
    typeGroupLayout->addWidget(m_radialButton);
    typeGroupLayout->addWidget(m_conicalButton);

    QVBoxLayout *spreadGroupLayout = new QVBoxLayout(spreadGroup);
    spreadGroupLayout->addWidget(m_padSpreadButton);
    spreadGroupLayout->addWidget(m_repeatSpreadButton);
    spreadGroupLayout->addWidget(m_reflectSpreadButton);

    QHBoxLayout *defaultsGroupLayout = new QHBoxLayout(defaultsGroup);
    defaultsGroupLayout->addWidget(default1Button);
    defaultsGroupLayout->addWidget(default2Button);
    defaultsGroupLayout->addWidget(default3Button);
    editorGroupLayout->addWidget(default4Button);

    connect(m_editor, SIGNAL(gradientStopsChanged(const QGradientStops &)),
            m_renderer, SLOT(setGradientStops(const QGradientStops &)));

    connect(m_linearButton, SIGNAL(clicked()), m_renderer, SLOT(setLinearGradient()));
    connect(m_radialButton, SIGNAL(clicked()), m_renderer, SLOT(setRadialGradient()));
    connect(m_conicalButton, SIGNAL(clicked()), m_renderer, SLOT(setConicalGradient()));

    connect(m_padSpreadButton, SIGNAL(clicked()), m_renderer, SLOT(setPadSpread()));
    connect(m_reflectSpreadButton, SIGNAL(clicked()), m_renderer, SLOT(setReflectSpread()));
    connect(m_repeatSpreadButton, SIGNAL(clicked()), m_renderer, SLOT(setRepeatSpread()));

    connect(default1Button, SIGNAL(clicked()), this, SLOT(setDefault1()));
    connect(default2Button, SIGNAL(clicked()), this, SLOT(setDefault2()));
    connect(default3Button, SIGNAL(clicked()), this, SLOT(setDefault3()));
    connect(default4Button, SIGNAL(clicked()), this, SLOT(setDefault4()));

    connect(showSourceButton, SIGNAL(clicked()), m_renderer, SLOT(showSource()));
#ifdef QT_OPENGL_SUPPORT
//    connect(enableOpenGLButton, SIGNAL(clicked(bool)), m_renderer, SLOT(enableOpenGL(bool)));
#endif
    connect(whatsThisButton, SIGNAL(clicked(bool)), m_renderer, SLOT(setDescriptionEnabled(bool)));
    connect(whatsThisButton, SIGNAL(clicked(bool)),
            m_renderer->hoverPoints(), SLOT(setDisabled(bool)));
    connect(m_renderer, SIGNAL(descriptionEnabledChanged(bool)),
            whatsThisButton, SLOT(setChecked(bool)));
    connect(m_renderer, SIGNAL(descriptionEnabledChanged(bool)),
            m_renderer->hoverPoints(), SLOT(setDisabled(bool)));

//    m_renderer->loadSourceFile(":res/gradients.cpp");
//    m_renderer->loadDescription(":res/gradients.html");

    QTimer::singleShot(50, this, SLOT(setDefault1()));
}

void GradientWidget::setDefault(int config)
{
    QGradientStops stops;
    QPolygonF points;
    switch (config) {
    case 1:
        stops << QGradientStop(0.00, QColor::fromRgba(0));
        stops << QGradientStop(0.04, QColor::fromRgba(0xff131360));
        stops << QGradientStop(0.08, QColor::fromRgba(0xff202ccc));
        stops << QGradientStop(0.42, QColor::fromRgba(0xff93d3f9));
        stops << QGradientStop(0.51, QColor::fromRgba(0xffb3e6ff));
        stops << QGradientStop(0.73, QColor::fromRgba(0xffffffec));
        stops << QGradientStop(0.92, QColor::fromRgba(0xff5353d9));
        stops << QGradientStop(0.96, QColor::fromRgba(0xff262666));
        stops << QGradientStop(1.00, QColor::fromRgba(0));
        m_linearButton->animateClick();
        m_repeatSpreadButton->animateClick();
        break;

    case 2:
        stops << QGradientStop(0.00, QColor::fromRgba(0xffffffff));
        stops << QGradientStop(0.11, QColor::fromRgba(0xfff9ffa0));
        stops << QGradientStop(0.13, QColor::fromRgba(0xfff9ff99));
        stops << QGradientStop(0.14, QColor::fromRgba(0xfff3ff86));
        stops << QGradientStop(0.49, QColor::fromRgba(0xff93b353));
        stops << QGradientStop(0.87, QColor::fromRgba(0xff264619));
        stops << QGradientStop(0.96, QColor::fromRgba(0xff0c1306));
        stops << QGradientStop(1.00, QColor::fromRgba(0));
        m_radialButton->animateClick();
        m_padSpreadButton->animateClick();
        break;

    case 3:
        stops << QGradientStop(0.00, QColor::fromRgba(0));
        stops << QGradientStop(0.10, QColor::fromRgba(0xffe0cc73));
        stops << QGradientStop(0.17, QColor::fromRgba(0xffc6a006));
        stops << QGradientStop(0.46, QColor::fromRgba(0xff600659));
        stops << QGradientStop(0.72, QColor::fromRgba(0xff0680ac));
        stops << QGradientStop(0.92, QColor::fromRgba(0xffb9d9e6));
        stops << QGradientStop(1.00, QColor::fromRgba(0));
        m_conicalButton->animateClick();
        m_padSpreadButton->animateClick();
        break;

    case 4:
        stops << QGradientStop(0.00, QColor::fromRgba(0xff000000));
        stops << QGradientStop(1.00, QColor::fromRgba(0xffffffff));
        break;

    default:
        qWarning("bad default: %d\n", config);
        break;
    }

    QPolygonF pts;
    int h_off = m_renderer->width() / 10;
    int v_off = m_renderer->height() / 8;
    pts << QPointF(m_renderer->width() / 2, m_renderer->height() / 2)
        << QPointF(m_renderer->width() / 2 - h_off, m_renderer->height() / 2 - v_off);

    m_editor->setGradientStops(stops);
    m_renderer->hoverPoints()->setPoints(pts);
    m_renderer->setGradientStops(stops);
}
