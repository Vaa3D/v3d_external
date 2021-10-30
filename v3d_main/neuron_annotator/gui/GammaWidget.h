#ifndef GAMMA_WIDGET_H_
#define GAMMA_WIDGET_H_

#include <QWidget>
#include "ui_GammaWidget.h"

// GUI widget with slider and text entry for updating gamma value
class GammaWidget : public QWidget, public Ui::GammaWidget
{
    Q_OBJECT

public:
    GammaWidget(QWidget * parent = NULL);

signals:
    void gammaBrightnessChanged(qreal gamma);

public slots:
    void setGammaBrightness(qreal gamma);
    void on_gamma_lineEdit_textChanged(const QString & value);
    void on_gamma_slider_valueChanged(int gammaInt);
    void reset();

protected slots:
    void updateGammaLineEdit(qreal gamma);

protected:
    static int gammaIntFromDouble(double gamma);
    static double gammaDoubleFromInt(int value);
};

#endif /* GAMMA_WIDGET_H_ */

