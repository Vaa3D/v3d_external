/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include "old_arthurstyle.h"
#include "old_arthurwidgets.h"
#include <QLayout>
#include <QPainter>
#include <QPainterPath>
#include <QPixmapCache>
#include <QRadioButton>
#include <QString>
#include <QStyleOption>
#include <QtDebug>
#include <QtGlobal>

QPixmap cached(const QString &img)
{
    if (QPixmap *p = QPixmapCache::find(img))
        return *p;

    QPixmap pm;
    pm = QPixmap::fromImage(QImage(img), Qt::OrderedDither | Qt::OrderedAlphaDither);
    if (pm.isNull())
        return QPixmap();

    QPixmapCache::insert(img, pm);
    return pm;
}


//@ADD: 2020-2-9 RZC
#if defined(USE_Qt5)
    #define  QWindowsStyle  QProxyStyle
#endif
// #ifdef USE_Qt5
// ArthurStyle::ArthurStyle()
//     : QProxyStyle() //by PHC 2020/01/31
// {
// #ifndef USE_Qt5
//     Q_INIT_RESOURCE(shared); //commented 20200201 by PHC as in QT5 it is found yet
// #endif
// }
// #else

ArthurStyle::ArthurStyle()
	: QWindowsStyle() //by PHC 2020/01/31
{
#ifndef USE_Qt5
	Q_INIT_RESOURCE(shared); //commented 20200201 by PHC as in QT5 it is found yet
#endif
}

//#endif


void ArthurStyle::drawHoverRect(QPainter *painter, const QRect &r) const
{
    qreal h = r.height();
    qreal h2 = r.height() / qreal(2);
    QPainterPath path;
    path.addRect(r.x() + h2, r.y() + 0, r.width() - h2 * 2, r.height());
    path.addEllipse(r.x(), r.y(), h, h);
    path.addEllipse(r.x() + r.width() - h, r.y(), h, h);
    path.setFillRule(Qt::WindingFill);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(191, 215, 191));
    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawPath(path);
}


void ArthurStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                                QPainter *painter, const QWidget *widget) const
{

    Q_ASSERT(option);
    switch (element) {
    case PE_FrameFocusRect:
        break;

    case PE_IndicatorRadioButton:
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            bool hover = (button->state & State_Enabled) && (button->state & State_MouseOver);
            painter->save();
            QPixmap radio;
            if (hover)
                drawHoverRect(painter, widget->rect());

            if (button->state & State_Sunken)
                radio = cached(":res/images/radiobutton-on.png");
            else if (button->state & State_On)
                radio = cached(":res/images/radiobutton_on.png");
            else
                radio = cached(":res/images/radiobutton_off.png");
            painter->drawPixmap(button->rect.topLeft(), radio);

            painter->restore();
        }
        break;

    case PE_PanelButtonCommand:
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            bool hover = (button->state & State_Enabled) && (button->state & State_MouseOver);

            painter->save();
            const QPushButton *pushButton = qobject_cast<const QPushButton *>(widget);
            Q_ASSERT(pushButton);
            QWidget *parent = pushButton->parentWidget();
            if (parent && qobject_cast<QGroupBox *>(parent)) {
                QLinearGradient lg(0, 0, 0, parent->height());
                lg.setColorAt(0, QColor(224,224,224));
                lg.setColorAt(1, QColor(255,255,255));
                painter->setPen(Qt::NoPen);
                painter->setBrush(lg);
                painter->setBrushOrigin(-widget->mapToParent(QPoint(0,0)));
                painter->drawRect(button->rect);
                painter->setBrushOrigin(0,0);
            }

            bool down = (button->state & State_Sunken) || (button->state & State_On);

            QPixmap left, right, mid;
            if (down) {
                left = cached(":res/images/button_pressed_cap_left.png");
                right = cached(":res/images/button_pressed_cap_right.png");
                mid = cached(":res/images/button_pressed_stretch.png");
            } else {
                left = cached(":res/images/button_normal_cap_left.png");
                right = cached(":res/images/button_normal_cap_right.png");
                mid = cached(":res/images/button_normal_stretch.png");
            }
            painter->drawPixmap(button->rect.topLeft(), left);
            painter->drawTiledPixmap(QRect(button->rect.x() + left.width(),
                                           button->rect.y(),
                                           button->rect.width() - left.width() - right.width(),
                                           left.height()),
                                     mid);
            painter->drawPixmap(button->rect.x() + button->rect.width() - right.width(),
                                button->rect.y(),
                                right);
            if (hover)
                painter->fillRect(widget->rect().adjusted(3,5,-3,-5), QColor(31,127,31,63));
            painter->restore();
        }
        break;

    case PE_FrameGroupBox:
        if (const QStyleOptionFrameV2 *group
                = qstyleoption_cast<const QStyleOptionFrameV2 *>(option)) {
            const QRect &r = group->rect;

            painter->save();
            int radius = 14;
            int radius2 = radius*2;
            QPainterPath clipPath;
            clipPath.moveTo(radius, 0);
            clipPath.arcTo(r.right() - radius2, 0, radius2, radius2, 90, -90);
            clipPath.arcTo(r.right() - radius2, r.bottom() - radius2, radius2, radius2, 0, -90);
            clipPath.arcTo(r.left(), r.bottom() - radius2, radius2, radius2, 270, -90);
            clipPath.arcTo(r.left(), r.top(), radius2, radius2, 180, -90);
            painter->setClipPath(clipPath);
            QPixmap titleStretch = cached(":res/images/title_stretch.png");
            QPixmap topLeft = cached(":res/images/groupframe_topleft.png");
            QPixmap topRight = cached(":res/images/groupframe_topright.png");
            QPixmap bottomLeft = cached(":res/images/groupframe_bottom_left.png");
            QPixmap bottomRight = cached(":res/images/groupframe_bottom_right.png");
            QPixmap leftStretch = cached(":res/images/groupframe_left_stretch.png");
            QPixmap topStretch = cached(":res/images/groupframe_top_stretch.png");
            QPixmap rightStretch = cached(":res/images/groupframe_right_stretch.png");
            QPixmap bottomStretch = cached(":res/images/groupframe_bottom_stretch.png");
            QLinearGradient lg(0, 0, 0, r.height());
            lg.setColorAt(0, QColor(224,224,224));
            lg.setColorAt(1, QColor(255,255,255));
            painter->setPen(Qt::NoPen);
            painter->setBrush(lg);
            painter->drawRect(r.adjusted(0, titleStretch.height()/2, 0, 0));
            painter->setClipping(false);

            int topFrameOffset = titleStretch.height()/2 - 2;
            painter->drawPixmap(r.topLeft() + QPoint(0, topFrameOffset), topLeft);
            painter->drawPixmap(r.topRight() - QPoint(topRight.width()-1, 0)
                                + QPoint(0, topFrameOffset), topRight);
            painter->drawPixmap(r.bottomLeft() - QPoint(0, bottomLeft.height()-1), bottomLeft);
            painter->drawPixmap(r.bottomRight() - QPoint(bottomRight.width()-1,
                                bottomRight.height()-1), bottomRight);

            QRect left = r;
            left.setY(r.y() + topLeft.height() + topFrameOffset);
            left.setWidth(leftStretch.width());
            left.setHeight(r.height() - topLeft.height() - bottomLeft.height() - topFrameOffset);
            painter->drawTiledPixmap(left, leftStretch);

            QRect top = r;
            top.setX(r.x() + topLeft.width());
            top.setY(r.y() + topFrameOffset);
            top.setWidth(r.width() - topLeft.width() - topRight.width());
            top.setHeight(topLeft.height());
            painter->drawTiledPixmap(top, topStretch);

            QRect right = r;
            right.setX(r.right() - rightStretch.width()+1);
            right.setY(r.y() + topRight.height() + topFrameOffset);
            right.setWidth(rightStretch.width());
            right.setHeight(r.height() - topRight.height()
                            - bottomRight.height() - topFrameOffset);
            painter->drawTiledPixmap(right, rightStretch);

            QRect bottom = r;
            bottom.setX(r.x() + bottomLeft.width());
            bottom.setY(r.bottom() - bottomStretch.height()+1);
            bottom.setWidth(r.width() - bottomLeft.width() - bottomRight.width());
            bottom.setHeight(bottomLeft.height());
            painter->drawTiledPixmap(bottom, bottomStretch);
            painter->restore();
        }
        break;

    default:
#ifdef Q_OS_MAC // MK, Feb, 2020
        QProxyStyle::drawPrimitive(element, option, painter, widget); //by PHC 20200131
#else
		QWindowsStyle::drawPrimitive(element, option, painter, widget);
#endif
        break;
    }
    return;
}


void ArthurStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                                     QPainter *painter, const QWidget *widget) const
{
    switch (control) {
    case CC_Slider:
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            QRect groove = subControlRect(CC_Slider, option, SC_SliderGroove, widget);
            QRect handle = subControlRect(CC_Slider, option, SC_SliderHandle, widget);

            painter->save();

            bool hover = (slider->state & State_Enabled) && (slider->state & State_MouseOver);
            if (hover) {
                QRect moderated = widget->rect().adjusted(0, 4, 0, -4);
                drawHoverRect(painter, moderated);
            }

            if ((option->subControls & SC_SliderGroove) && groove.isValid()) {
                QPixmap grv = cached(":res/images/slider_bar.png");
                painter->drawPixmap(QRect(groove.x() + 5, groove.y(),
                                          groove.width() - 10, grv.height()),
                                    grv);
            }
            if ((option->subControls & SC_SliderHandle) && handle.isValid()) {
                QPixmap hndl = cached(":res/images/slider_thumb_on.png");
                painter->drawPixmap(handle.topLeft(), hndl);
            }

            painter->restore();
        }
        break;
    case CC_GroupBox:
        if (const QStyleOptionGroupBox *groupBox
                = qstyleoption_cast<const QStyleOptionGroupBox *>(option)) {
            QStyleOptionGroupBox groupBoxCopy(*groupBox);
            groupBoxCopy.subControls &= ~SC_GroupBoxLabel;
#ifdef Q_OS_MAC // MK, Feb, 2020
            QProxyStyle::drawComplexControl(control, &groupBoxCopy, painter, widget); //by PHC 20200131
#else
			QWindowsStyle::drawComplexControl(control, &groupBoxCopy, painter, widget);
#endif

            if (groupBox->subControls & SC_GroupBoxLabel) {
                const QRect &r = groupBox->rect;
                QPixmap titleLeft = cached(":res/images/title_cap_left.png");
                QPixmap titleRight = cached(":res/images/title_cap_right.png");
                QPixmap titleStretch = cached(":res/images/title_stretch.png");
                int txt_width = groupBox->fontMetrics.width(groupBox->text) + 20;
                painter->drawPixmap(r.center().x() - txt_width/2, 0, titleLeft);
                QRect tileRect = subControlRect(control, groupBox, SC_GroupBoxLabel, widget);
                painter->drawTiledPixmap(tileRect, titleStretch);
                painter->drawPixmap(tileRect.x() + tileRect.width(), 0, titleRight);
                int opacity = 31;
                painter->setPen(QColor(0, 0, 0, opacity));
                painter->drawText(tileRect.translated(0, 1),
                                  Qt::AlignVCenter | Qt::AlignHCenter, groupBox->text);
                painter->drawText(tileRect.translated(2, 1),
                                  Qt::AlignVCenter | Qt::AlignHCenter, groupBox->text);
                painter->setPen(QColor(0, 0, 0, opacity * 2));
                painter->drawText(tileRect.translated(1, 1),
                                  Qt::AlignVCenter | Qt::AlignHCenter, groupBox->text);
                painter->setPen(Qt::white);
                painter->drawText(tileRect, Qt::AlignVCenter | Qt::AlignHCenter, groupBox->text);
            }
        }
        break;
    default:
#ifdef Q_OS_MAC // MK, Feb, 2020
        QProxyStyle::drawComplexControl(control, option, painter, widget);
#else
		QWindowsStyle::drawComplexControl(control, option, painter, widget);
#endif
        break;
    }
    return;
}

QRect ArthurStyle::subControlRect(ComplexControl control, const QStyleOptionComplex *option,
                                  SubControl subControl, const QWidget *widget) const
{
    QRect rect;

    switch (control) {
    default:
#ifdef Q_OS_MAC // MK, Feb, 2020
        rect = QProxyStyle::subControlRect(control, option, subControl, widget);
#else
		rect = QWindowsStyle::subControlRect(control, option, subControl, widget);
#endif
        break;
    case CC_GroupBox:
        if (const QStyleOptionGroupBox *group
                = qstyleoption_cast<const QStyleOptionGroupBox *>(option)) {
            switch (subControl) {
            default:
#ifdef Q_OS_MAC // MK, Feb, 2020
				rect = QProxyStyle::subControlRect(control, option, subControl, widget);
#else
				rect = QWindowsStyle::subControlRect(control, option, subControl, widget);
#endif
                break;
            case SC_GroupBoxContents:
#ifdef Q_OS_MAC // MK, Feb, 2020
				rect = QProxyStyle::subControlRect(control, option, subControl, widget);
#else
				rect = QWindowsStyle::subControlRect(control, option, subControl, widget);
#endif
                rect.adjust(0, -8, 0, 0);
                break;
            case SC_GroupBoxFrame:
                rect = group->rect;
                break;
            case SC_GroupBoxLabel:
                QPixmap titleLeft = cached(":res/images/title_cap_left.png");
                QPixmap titleRight = cached(":res/images/title_cap_right.png");
                QPixmap titleStretch = cached(":res/images/title_stretch.png");
                int txt_width = group->fontMetrics.width(group->text) + 20;
                rect = QRect(group->rect.center().x() - txt_width/2 + titleLeft.width(), 0,
                             txt_width - titleLeft.width() - titleRight.width(),
                             titleStretch.height());
                break;
            }
        }
        break;
    }

    if (control == CC_Slider && subControl == SC_SliderHandle) {
        rect.setWidth(13);
        rect.setHeight(27);
    } else if (control == CC_Slider && subControl == SC_SliderGroove) {
        rect.setHeight(9);
        rect.moveTop(27/2 - 9/2);
    }
    return rect;
}

QSize ArthurStyle::sizeFromContents(ContentsType type, const QStyleOption *option,
                                    const QSize &size, const QWidget *widget) const
{
#ifdef Q_OS_MAC // MK, Feb, 2020
    QSize newSize = QProxyStyle::sizeFromContents(type, option, size, widget);
#else
	QSize newSize = QWindowsStyle::sizeFromContents(type, option, size, widget);
#endif


    switch (type) {
    case CT_RadioButton:
        newSize += QSize(20, 0);
        break;

    case CT_PushButton:
        newSize.setHeight(26);
        break;

    case CT_Slider:
        newSize.setHeight(27);
        break;

    default:
        break;
    }

    return newSize;
}

int ArthurStyle::pixelMetric(PixelMetric pm, const QStyleOption *opt, const QWidget *widget) const
{
    if (pm == PM_SliderLength)
        return 13;
#ifdef Q_OS_MAC // MK, Feb, 2020
    return QProxyStyle::pixelMetric(pm, opt, widget);
#else
	return QWindowsStyle::pixelMetric(pm, opt, widget);
#endif
}

void ArthurStyle::polish(QWidget *widget)
{
    if (widget->layout() && qobject_cast<QGroupBox *>(widget)) {
        if (widget->findChildren<QGroupBox *>().size() == 0) {  //by PHC 20200131
            widget->layout()->setSpacing(0);
            widget->layout()->setMargin(12);
        } else {
            widget->layout()->setMargin(13);
        }
    }

    if (qobject_cast<QPushButton *>(widget)
        || qobject_cast<QRadioButton *>(widget)
        || qobject_cast<QSlider *>(widget)) {
        widget->setAttribute(Qt::WA_Hover);
    }

    QPalette pal = widget->palette();
    if (widget->isWindow()) {
        pal.setColor(QPalette::Background, QColor(241, 241, 241));
        widget->setPalette(pal);
    }

}

void ArthurStyle::unpolish(QWidget *widget)
{
    if (qobject_cast<QPushButton *>(widget)
        || qobject_cast<QRadioButton *>(widget)
        || qobject_cast<QSlider *>(widget)) {
        widget->setAttribute(Qt::WA_Hover, false);
    }
}

void ArthurStyle::polish(QPalette &palette)
{
    palette.setColor(QPalette::Background, QColor(241, 241, 241));
}

QRect ArthurStyle::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    QRect r;
    switch(element) {
    case SE_RadioButtonClickRect:
        r = widget->rect();
        break;
    case SE_RadioButtonContents:
        r = widget->rect().adjusted(20, 0, 0, 0);
        break;
    default:
#ifdef Q_OS_MAC // MK, Feb, 2020  
        r = QProxyStyle::subElementRect(element, option, widget);
#else
		r = QWindowsStyle::subElementRect(element, option, widget);
#endif
        break;
    }

    if (qobject_cast<const QRadioButton*>(widget))
        r = r.adjusted(5, 0, -5, 0);

    return r;
}
