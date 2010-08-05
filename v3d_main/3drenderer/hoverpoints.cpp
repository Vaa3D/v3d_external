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

#undef QT_OPENGL_SUPPORT
#ifdef QT_OPENGL_SUPPORT
#include <QGLWidget>
#endif

//#include "arthurwidgets.h"
#include "hoverpoints.h"

//#define printf

HoverPoints::HoverPoints(QWidget *widget, PointShape shape)
    : QObject(widget)
{
    m_widget = widget;
    m_shape = shape;

    m_connectionType = CurveConnection;
    m_sortType = NoSort;
    m_pointPen = QPen(QColor(255, 255, 255, 191), 1);
    m_connectionPen = QPen(QColor(255, 255, 255, 127), 2);
    m_pointBrush = QBrush(QColor(191, 191, 191, 127));
    m_pointSize = QSize(11, 11);
    m_currentIndex = -1;
    m_editable = true;
    m_enabled = true;

    widget->installEventFilter(this); // 081220 move after m_currentIndex=-1
    connect(this, SIGNAL(pointsChanged(const QPolygonF &)),  m_widget, SLOT(update()));
}


void HoverPoints::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;
        m_widget->update();
    }
}


bool HoverPoints::eventFilter(QObject *object, QEvent *event)
{
    if (object == m_widget && m_enabled) {
        switch (event->type()) {

        case QEvent::MouseButtonPress:
        {
            QMouseEvent *me = (QMouseEvent *) event;

            QPointF clickPos = me->pos();
            int index = -1;
            for (int i=0; i<m_points.size(); ++i) {
                QPainterPath path;
                if (m_shape == CircleShape)
                    path.addEllipse(pointBoundingRect(i));
                else
                    path.addRect(pointBoundingRect(i));

                if (path.contains(clickPos)) {
                    index = i;
                    break;
                }
            }

            if (me->button() == Qt::LeftButton) {
                if (index == -1) {
                    if (!m_editable)
                        return false;
                    int pos = 0;
                    // Insert sort for x or y
                    if (m_sortType == XSort) {
                        for (int i=0; i<m_points.size(); ++i)
                            if (m_points.at(i).x() > clickPos.x()) {
                                pos = i;
                                break;
                            }
                    } else if (m_sortType == YSort) {
                        for (int i=0; i<m_points.size(); ++i)
                            if (m_points.at(i).y() > clickPos.y()) {
                                pos = i;
                                break;
                            }
                    }

                    m_points.insert(pos, clickPos);
                    m_locks.insert(pos, 0);
                    m_currentIndex = pos;
                    firePointChange();
                } else {
                    m_currentIndex = index;
                }
                return true;

            } else if (me->button() == Qt::RightButton) {
                if (index >= 0 && m_editable) {
                    if (m_locks[index] == 0) {
                        m_locks.remove(index);
                        m_points.remove(index);
                    }
                    firePointChange();
                    return true;
                }
            }

        }
        break;

        case QEvent::MouseButtonRelease:
            m_currentIndex = -1;
            break;

        case QEvent::MouseMove:
            if (m_currentIndex >= 0)
            {
                movePoint(m_currentIndex, ((QMouseEvent *)event)->pos());
                firePointChange(); //090719
            }
            break;

        case QEvent::Resize:
        {
            QResizeEvent *e = (QResizeEvent *) event;
            if (e->oldSize().width() == 0 || e->oldSize().height() == 0)
                break;
            qreal stretch_x = e->size().width() / qreal(e->oldSize().width());
            qreal stretch_y = e->size().height() / qreal(e->oldSize().height());

            for (int i=0; i<m_points.size(); ++i) {
                QPointF p = m_points[i];
                movePoint(i, QPointF(p.x() * stretch_x, p.y() * stretch_y)); //090719, false);
            }
            if(m_points.size())  firePointChange(); //081220, add if(m_points.size())
            break;
        }

        case QEvent::Paint:
        {
            QWidget *that_widget = m_widget;
            m_widget = 0;
            QApplication::sendEvent(object, event);
            m_widget = that_widget;
            paintPoints();
#ifdef QT_OPENGL_SUPPORT
            ArthurFrame *af = qobject_cast<ArthurFrame *>(that_widget);
            if (af && af->usesOpenGL())
                af->glWidget()->swapBuffers();
#endif
            return true;
        }
        default:
            break;
        }
    }

    return false;
}


void HoverPoints::paintPoints()
{
    QPainter p;
#ifdef QT_OPENGL_SUPPORT
    ArthurFrame *af = qobject_cast<ArthurFrame *>(m_widget);
    if (af && af->usesOpenGL())
        p.begin(af->glWidget());
    else
        p.begin(m_widget);
#else
    p.begin(m_widget);
#endif

    p.setRenderHint(QPainter::Antialiasing);

    if (m_connectionPen.style() != Qt::NoPen && m_connectionType != NoConnection) {
        p.setPen(m_connectionPen);

        if (m_connectionType == CurveConnection) {
            QPainterPath path;
            path.moveTo(m_points.at(0));
            for (int i=1; i<m_points.size(); ++i) {
                QPointF p1 = m_points.at(i-1);
                QPointF p2 = m_points.at(i);
                qreal distance = p2.x() - p1.x();

                path.cubicTo(p1.x() + distance / 2, p1.y(),
                             p1.x() + distance / 2, p2.y(),
                             p2.x(), p2.y());
            }
            p.drawPath(path);
        } else {
            p.drawPolyline(m_points);
        }
    }

    p.setPen(m_pointPen);
    p.setBrush(m_pointBrush);

    for (int i=0; i<m_points.size(); ++i) {
        QRectF bounds = pointBoundingRect(i);
        if (m_shape == CircleShape)
        {
			if (m_locks.at(i)) p.drawRect(bounds);  else
				p.drawEllipse(bounds);
        }
        else //RectangleShape
        {
        	if (m_locks.at(i)) p.drawEllipse(bounds);  else
        		p.drawRect(bounds);
        }
    }
}

static QPointF bound_point(const QPointF &point, const QRectF &bounds, int lock)
{
    QPointF p = point;

    qreal left = bounds.left();
    qreal right = bounds.right();
    qreal top = bounds.top();
    qreal bottom = bounds.bottom();

    if (p.x() < left || (lock & HoverPoints::LockToLeft)) p.setX(left);
    else if (p.x() > right || (lock & HoverPoints::LockToRight)) p.setX(right);

    if (p.y() < top || (lock & HoverPoints::LockToTop)) p.setY(top);
    else if (p.y() > bottom || (lock & HoverPoints::LockToBottom)) p.setY(bottom);

    return p;
}

void HoverPoints::setPoints(const QPolygonF &points)
{
    m_points.clear();
    for (int i=0; i<points.size(); ++i)
        m_points << bound_point(points.at(i), boundingRect(), 0);

    m_locks.clear();
    if (m_points.size() > 0) {
        m_locks.resize(m_points.size());

        m_locks.fill(0); //no lock
    }
}


void HoverPoints::movePoint(int index, const QPointF &point)
{
    m_points[index] = bound_point(point, boundingRect(), m_locks.at(index));
}


void HoverPoints::firePointChange()
{
//    printf("HoverPoints::firePointChange(), current=%d\n", m_currentIndex);

    if (m_sortType != NoSort) {

        QPointF oldCurrent;
        if (m_currentIndex != -1) {
            oldCurrent = m_points[m_currentIndex];
        }

        if (m_sortType == XSort)
            qSort(m_points.begin(), m_points.end(), x_less_than);
        else if (m_sortType == YSort)
            qSort(m_points.begin(), m_points.end(), y_less_than);

        // Compensate for changed order...
        if (m_currentIndex != -1) {
            for (int i=0; i<m_points.size(); ++i) {
                if (m_points.at(i) == oldCurrent) {
                    m_currentIndex = i;
                    break;
                }
            }
        }

//        //090719
//        if (m_sortType == XSort)
//        {
//            qreal right = m_points.end()->x();
//            qreal right_y = -1;
//            int n_right = 0;
//            for (int i=0; i<m_points.size(); ++i) {
//                if (m_points.at(i).x() == right) {
//                	if (n_right==0) right_y = m_points.at(i).y();
//                	else m_points[i].ry() = right_y;
//                	n_right++;
//                    break;
//                }
//            }
//           // printf(" - firePointChange(), n_right=%d\n", n_right);
//        }
//        else if (m_sortType == YSort)
//        {
//			qreal bottom = m_points.end()->y();
//            qreal bottom_x = -1;
//            int n_bottom = 0;
//            for (int i=0; i<m_points.size(); ++i) {
//                if (m_points.at(i).y() == bottom) {
//                	if (n_bottom==0) bottom_x = m_points.at(i).x();
//                	else m_points[i].rx() = bottom_x;
//                	n_bottom++;
//                    break;
//                }
//            }
//        }
    }

//     for (int i=0; i<m_points.size(); ++i) {
//         printf(" - point(%2d)=[%.2f, %.2f], lock=%d\n",
//                i, m_points.at(i).x(), m_points.at(i).y(), m_locks.at(i));
//     }

    if (m_points.size())
    	emit pointsChanged(m_points);
}
