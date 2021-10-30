#ifndef QGRADIENTBAR_H
#define QGRADIENTBAR_H

#include <QWidget>
#include "../control/CPlugin.h"

class terafly::QGradientBar : public QWidget
{
    Q_OBJECT

    private:

        int _nsteps;
        int _step;
        bool _active;
        std::string _text;
        std::vector<QColor> _colors;        // alternative to gradient: n different colors for n macro-steps

    public:

        QGradientBar(QWidget *parent = 0) : QWidget(parent), _nsteps(-1), _step(0){}
        bool finished(){return _step >= _nsteps;}
        int step(){return _step;}
        void setNSteps(int nsteps){_nsteps = nsteps;}
        void setStep(int step){_step = step;}
        void setText(const std::string & newText){_text = newText;}
        void setMultiColor(std::vector<QColor> colors){_colors = colors;}

    protected:

        //needed to draw the arrow with QPainter
        void paintEvent(QPaintEvent * evt);

    signals:

    public slots:

};

#endif // QGRADIENTBAR_H
