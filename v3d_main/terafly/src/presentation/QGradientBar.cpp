#if defined(USE_Qt5)
#include <QPainter>
#endif

#include "QGradientBar.h"

using namespace terafly;

void QGradientBar::paintEvent(QPaintEvent * evt)
{
    //creatin QPainter object, enabling antialising and setting brush color differently if button is enabled or not
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if(isEnabled() && _nsteps != -1)
    {
        //drawing gradient or solid color
        if(_colors.size())
        {
            painter.setBrush(_colors[std::min(size_t(_step/(float(_nsteps) / _colors.size())), _colors.size()-1)]);
            painter.setPen(QColor(0, 0, 0, 0));
            painter.drawRect(0, 0, width(), height());
        }
        else
        {
            QLinearGradient fade(0, 0, width(), height());
            fade.setColorAt(0.0, QColor(0,      0,      255));
            fade.setColorAt(0.333, QColor(0,      255,    255));
            fade.setColorAt(0.666, QColor(255,    255,    0));
            fade.setColorAt(1.0, QColor(255,    0,      0));
            painter.fillRect(0, 0, width(), height(), fade);
        }

        //drawing white bar
        painter.setBrush(QColor(255, 255, 255));
        painter.setPen(QColor(0, 0, 0, 0));
        painter.drawRect(static_cast<int>(  (_step+1)*width()/float(_nsteps)  +0.5f), 0,  static_cast<int>(  width()-(_step+1)*width()/float(_nsteps)  +0.5f), height());

        //drawing border
        painter.setBrush(QColor(255, 255, 255, 0));
        painter.setPen(QColor(0, 0, 0));
        painter.drawRect(0, 0, width(), height());

        // draw text if any
        if(_text.size())
            painter.drawText(QRectF(0, 2, width(), height()-2), Qt::AlignCenter, _text.c_str());
    }
    else
    {
        painter.setBrush(QColor(0, 0, 0, 0));
        painter.setPen(Qt::gray);
        painter.drawRect(0, 0, width(), height());

        // draw text if any
        if(_text.size())
            painter.drawText(QRectF(0, 2, width(), height()-2), Qt::AlignCenter, _text.c_str());
    }
}
