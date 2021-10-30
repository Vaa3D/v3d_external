#include "ZoomSpinBox.h"
#include <cmath>
#include <cassert>
#include <QDebug>
#include <limits>

/* explicit */
ZoomSpinBox::ZoomSpinBox(QWidget *parent /* = 0 */ )
    : QSpinBox(parent)
    , validator(1e-9, 1e9, 3, this)
{
    // Integer range is unlimited
    // (but float range is strictly greater than zero)
    setMinimum(-10000);
    setMaximum(10000);
    setZoomValue(1.0);
    setSuffix(" X");
    connect(this, SIGNAL(valueChanged(int)),
            this, SLOT(onValueChanged(int)));
}

void ZoomSpinBox::setZoomValue(qreal doubleValue)
{
    setValue(intValueFromDoubleValue(doubleValue));
}

void ZoomSpinBox::onValueChanged(int intValue) {
    emit(zoomValueChanged(doubleValueFromIntValue(intValue)));
}

void ZoomSpinBox::reset() {
    setZoomValue(1.0);
}

/* virtual */
QString ZoomSpinBox::textFromValue(int intValue) const
{
    return QString("%1").arg(doubleValueFromIntValue(intValue), 2, 'f', 2, '0');
}

/* virtual */
int ZoomSpinBox::valueFromText(const QString & text) const
{
    double doubleValue = text.toDouble();
    return intValueFromDoubleValue(doubleValue);
}

/* virtual */
QValidator::State ZoomSpinBox::validate(QString & text, int & pos) const {
    return validator.validate(text, pos);
}

/* static */
int ZoomSpinBox::intValueFromDoubleValue(qreal doubleValue)
{
    int result = 0;
    if (doubleValue <= 0)
        result = -10000;
    else if (doubleValue == std::numeric_limits<qreal>::infinity())
        result = 10000;
    else
        result = int(floor(log(doubleValue) * 100.0 + 0.4999));
    // assert(doubleValue == doubleValueFromIntValue(result));
    return result;
}

/* static */
qreal ZoomSpinBox::doubleValueFromIntValue(int intValue)
{
    qreal result = 1.0;
    if (intValue < -10000)
        intValue = -10000;
    else if (intValue > 10000)
        intValue = 10000;
    result = exp(intValue / 100.0);
    if (intValue != intValueFromDoubleValue(result)) {
        qDebug() << "Oops: " << intValue << ", " << intValueFromDoubleValue(result) << ", " << result;
    }
    return result;
}



