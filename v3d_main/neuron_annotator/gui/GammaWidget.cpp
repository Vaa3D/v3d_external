#include "GammaWidget.h"
#include <cmath>
#include <cassert>

GammaWidget::GammaWidget(QWidget * parent)
    : QWidget(parent)
{
    // TODO - avoid Qt warning message about multiple layouts
    setupUi(this);
    connect(this, SIGNAL(gammaBrightnessChanged(qreal)),
            this, SLOT(updateGammaLineEdit(qreal)));
}

void GammaWidget::updateGammaLineEdit(qreal gamma)
{
    double d = gamma_lineEdit->text().toDouble();
    if (d == gamma)
        return;
    // complex logic to leave "non-canonical but equivalent" strings unchanged
    if (d > 0.0) { // string parsed OK
        if ( gamma_slider->value() == gammaIntFromDouble(d) ) // value is essentially unchanged
            return; // Leave string alone; value is good and user might be editing it.
        if ( gammaDoubleFromInt(gamma_slider->value()) == d)
            return; // A slightly different way for the number to be OK
    }
    gamma_lineEdit->setText(QString::number(gamma, 'f', 2));
}

void GammaWidget::on_gamma_lineEdit_textChanged(const QString & value)
{
    double gamma = value.toDouble();
    if (gamma > 0.0) // parse OK
        setGammaBrightness(gamma);
}

void GammaWidget::on_gamma_slider_valueChanged(int value)
{
    double gamma = gammaDoubleFromInt(value);
    if (gamma > 0.0)
        emit gammaBrightnessChanged(gamma);
}

void GammaWidget::setGammaBrightness(double gamma)
{
    if (! (gamma > 0.0))
        return; // nonsense
    if (gamma == gammaDoubleFromInt(gamma_slider->value()))
        return; // no change
    int gammaInt = gammaIntFromDouble(gamma);
    if (gammaInt == gamma_slider->value())
        return; // no important change
    gamma_slider->setValue(gammaInt);
}

void GammaWidget::reset() {
    setGammaBrightness(1.0);
}

// Final arbiter of whether gamma value has changed is integer
// -100 * log2(gamma)
int GammaWidget::gammaIntFromDouble(double gamma)
{
    double log2Gamma = std::log(gamma)/std::log(2.0);
    int result = int(floor(log2Gamma * 100.0 + 0.5));
    return result;
}

double GammaWidget::gammaDoubleFromInt(int value)
{
    double log2Gamma = value / 100.0;
    double gamma = std::pow(2.0, log2Gamma);
    assert(gammaIntFromDouble(gamma) == value);
    return gamma;
}



